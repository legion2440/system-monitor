#include "qt/ProcessModel.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>

namespace monesys {

ProcessModel::ProcessModel(QObject* parent) : QAbstractListModel(parent) {}

int ProcessModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(visible_.size());
}

QVariant ProcessModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(visible_.size())) return {};
    const auto& process = all_[static_cast<std::size_t>(visible_[static_cast<std::size_t>(index.row())])];
    switch (role) {
    case PidRole: return QVariant::fromValue<qlonglong>(static_cast<qlonglong>(process.pid));
    case ParentPidRole: return QVariant::fromValue<qlonglong>(static_cast<qlonglong>(process.parentPid));
    case NameRole: return QString::fromStdString(process.name);
    case CommandLineRole: return QString::fromStdString(process.commandLine);
    case StateCodeRole: return QString::fromStdString(process.stateCode);
    case StateRole: return QString::fromStdString(process.state);
    case UserRole: return QString::fromStdString(process.user);
    case CgroupRole: return QString::fromStdString(process.cgroup);
    case CpuRole: return process.cpuPercent;
    case MemoryRole: return process.memoryPercent;
    case RssRole: return QVariant::fromValue<qulonglong>(process.rssBytes);
    case VirtualRole: return QVariant::fromValue<qulonglong>(process.virtualBytes);
    case ReadRole: return QVariant::fromValue<qulonglong>(process.readBytes);
    case WriteRole: return QVariant::fromValue<qulonglong>(process.writeBytes);
    case ThreadsRole: return process.threads;
    case SelectedRole: return selectedPids_.contains(process.pid);
    case DepthRole: return depthCache_.value(process.pid, 0);
    default: return {};
    }
}

QHash<int, QByteArray> ProcessModel::roleNames() const {
    return {{PidRole,"pid"},{ParentPidRole,"parentPid"},{NameRole,"name"},{CommandLineRole,"commandLine"},
            {StateCodeRole,"stateCode"},{StateRole,"state"},{UserRole,"user"},{CgroupRole,"cgroup"},{CpuRole,"cpu"},{MemoryRole,"memory"},
            {RssRole,"rss"},{VirtualRole,"virtualMemory"},{ReadRole,"readBytes"},{WriteRole,"writeBytes"},
            {ThreadsRole,"threads"},{SelectedRole,"selected"},{DepthRole,"depth"}};
}

std::vector<int> ProcessModel::buildVisible(const std::vector<ProcessInfo>& processes) const {
    std::vector<int> visible;
    visible.reserve(processes.size());
    const auto needle = filter_.trimmed().toLower();
    for (std::size_t i = 0; i < processes.size(); ++i) {
        const auto& process = processes[i];
        if (needle.isEmpty() || QString::fromStdString(process.name).toLower().contains(needle) ||
            QString::number(process.pid).contains(needle) || QString::fromStdString(process.user).toLower().contains(needle) ||
            QString::fromStdString(process.commandLine).toLower().contains(needle)) {
            visible.push_back(static_cast<int>(i));
        }
    }
    return visible;
}

std::vector<qint64> ProcessModel::visiblePids(const std::vector<ProcessInfo>& processes, const std::vector<int>& visible) const {
    std::vector<qint64> pids;
    pids.reserve(visible.size());
    for (const auto row : visible) pids.push_back(processes[static_cast<std::size_t>(row)].pid);
    return pids;
}

QHash<qint64, int> ProcessModel::buildDepthCache(const std::vector<ProcessInfo>& processes) const {
    QHash<qint64, qint64> parents;
    parents.reserve(static_cast<qsizetype>(processes.size()));
    for (const auto& process : processes) parents.insert(process.pid, process.parentPid);

    QHash<qint64, int> depths;
    depths.reserve(static_cast<qsizetype>(processes.size()));
    for (const auto& process : processes) {
        int depth = 0;
        QSet<qint64> seen;
        auto current = process.pid;
        while (depth < 12 && parents.contains(current)) {
            if (seen.contains(current)) break;
            seen.insert(current);
            const auto parent = parents.value(current);
            if (parent <= 1 || parent == current || !parents.contains(parent)) break;
            current = parent;
            ++depth;
        }
        depths.insert(process.pid, depth);
    }
    return depths;
}

void ProcessModel::syncFreezeState() {
    if (orderEffectivelyFrozen()) {
        if (frozenOrder_.empty()) {
            frozenOrder_.reserve(all_.size());
            for (const auto& process : all_) frozenOrder_.push_back(process.pid);
        }
    } else {
        frozenOrder_.clear();
    }
}

void ProcessModel::applyFrozenOrder(std::vector<ProcessInfo>& processes) const {
    if (frozenOrder_.empty()) return;
    std::unordered_map<qint64, std::size_t> rank;
    rank.reserve(frozenOrder_.size());
    for (std::size_t i = 0; i < frozenOrder_.size(); ++i) rank.emplace(frozenOrder_[i], i);
    const auto unknown = frozenOrder_.size();
    std::stable_sort(processes.begin(), processes.end(), [&rank, unknown](const ProcessInfo& left, const ProcessInfo& right) {
        const auto leftIt = rank.find(left.pid);
        const auto rightIt = rank.find(right.pid);
        const auto leftRank = leftIt == rank.end() ? unknown : leftIt->second;
        const auto rightRank = rightIt == rank.end() ? unknown : rightIt->second;
        return leftRank < rightRank;
    });
}

void ProcessModel::updateProcesses(std::vector<ProcessInfo> processes) {
    QSet<qint64> livePids;
    livePids.reserve(static_cast<qsizetype>(processes.size()));
    for (const auto& process : processes) livePids.insert(process.pid);

    const auto selectedBefore = selectionOrder_.size();
    const auto effectiveFrozenBefore = orderEffectivelyFrozen();
    selectedPids_.intersect(livePids);
    selectionOrder_.removeIf([this](qint64 pid) { return !selectedPids_.contains(pid); });
    syncFreezeState();

    applyFrozenOrder(processes);
    auto newVisible = buildVisible(processes);
    auto newDepthCache = buildDepthCache(processes);
    const auto oldPids = visiblePids(all_, visible_);
    const auto newPids = visiblePids(processes, newVisible);

    if (oldPids == newPids) {
        all_ = std::move(processes);
        visible_ = std::move(newVisible);
        depthCache_ = std::move(newDepthCache);
        if (!visible_.empty()) emit dataChanged(index(0, 0), index(static_cast<int>(visible_.size()) - 1, 0));
    } else {
        beginResetModel();
        all_ = std::move(processes);
        visible_ = std::move(newVisible);
        depthCache_ = std::move(newDepthCache);
        endResetModel();
    }

    if (selectedBefore != selectionOrder_.size()) emit selectionChanged();
    if (effectiveFrozenBefore != orderEffectivelyFrozen()) emit orderFrozenChanged();
}

void ProcessModel::setFilter(const QString& filter) {
    if (filter_ == filter) return;
    beginResetModel();
    filter_ = filter;
    visible_ = buildVisible(all_);
    endResetModel();
    emit filterChanged();
}

void ProcessModel::setOrderFrozen(bool frozen) {
    if (orderFrozen_ == frozen) return;
    orderFrozen_ = frozen;
    syncFreezeState();
    emit orderFrozenChanged();
}

QVariantMap ProcessModel::selectedProcess() const {
    if (selectionOrder_.isEmpty()) return {};
    const auto pid = selectionOrder_.constLast();
    const auto it = std::find_if(all_.begin(), all_.end(), [pid](const ProcessInfo& process) { return process.pid == pid; });
    if (it == all_.end()) return {};
    return {{"pid",QVariant::fromValue<qlonglong>(static_cast<qlonglong>(it->pid))},
            {"parentPid",QVariant::fromValue<qlonglong>(static_cast<qlonglong>(it->parentPid))},
            {"name",QString::fromStdString(it->name)},
            {"commandLine",QString::fromStdString(it->commandLine)},
            {"stateCode",QString::fromStdString(it->stateCode)},
            {"state",QString::fromStdString(it->state)},
            {"user",QString::fromStdString(it->user)},
            {"cgroup",QString::fromStdString(it->cgroup)},
            {"cpu",it->cpuPercent},{"memory",it->memoryPercent},{"rss",QVariant::fromValue<qulonglong>(it->rssBytes)},
            {"virtualMemory",QVariant::fromValue<qulonglong>(it->virtualBytes)},
            {"readBytes",QVariant::fromValue<qulonglong>(it->readBytes)},
            {"writeBytes",QVariant::fromValue<qulonglong>(it->writeBytes)},{"threads",it->threads}};
}

void ProcessModel::toggleSelected(int visibleRow) {
    if (visibleRow < 0 || visibleRow >= static_cast<int>(visible_.size())) return;
    const auto& process = all_[static_cast<std::size_t>(visible_[static_cast<std::size_t>(visibleRow)])];
    const auto effectiveFrozenBefore = orderEffectivelyFrozen();

    if (selectedPids_.contains(process.pid)) {
        selectedPids_.remove(process.pid);
        selectionOrder_.removeAll(process.pid);
    } else {
        selectedPids_.insert(process.pid);
        selectionOrder_.append(process.pid);
    }
    syncFreezeState();

    const auto modelIndex = index(visibleRow, 0);
    emit dataChanged(modelIndex, modelIndex, {SelectedRole});
    emit selectionChanged();
    if (effectiveFrozenBefore != orderEffectivelyFrozen()) emit orderFrozenChanged();
}

void ProcessModel::clearSelection() {
    if (selectionOrder_.isEmpty()) return;
    const auto effectiveFrozenBefore = orderEffectivelyFrozen();
    selectedPids_.clear();
    selectionOrder_.clear();
    syncFreezeState();
    if (!visible_.empty()) emit dataChanged(index(0,0), index(static_cast<int>(visible_.size()) - 1,0), {SelectedRole});
    emit selectionChanged();
    if (effectiveFrozenBefore != orderEffectivelyFrozen()) emit orderFrozenChanged();
}

} // namespace monesys
