#include "qt/ProcessModel.hpp"

#include <algorithm>

namespace monesys {

ProcessModel::ProcessModel(QObject* parent) : QAbstractListModel(parent) {}

int ProcessModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(visible_.size());
}

QVariant ProcessModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(visible_.size())) return {};
    const auto& process = all_[static_cast<std::size_t>(visible_[static_cast<std::size_t>(index.row())])];
    switch (role) {
    case PidRole: return process.pid;
    case ParentPidRole: return process.parentPid;
    case NameRole: return QString::fromStdString(process.name);
    case CommandLineRole: return QString::fromStdString(process.commandLine);
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
    case DepthRole: {
        QHash<qint64, qint64> parents;
        for (const auto& row : all_) parents.insert(row.pid, row.parentPid);
        return depthFor(process.pid, parents);
    }
    default: return {};
    }
}

QHash<int, QByteArray> ProcessModel::roleNames() const {
    return {{PidRole,"pid"},{ParentPidRole,"parentPid"},{NameRole,"name"},{CommandLineRole,"commandLine"},
            {StateRole,"state"},{UserRole,"user"},{CgroupRole,"cgroup"},{CpuRole,"cpu"},{MemoryRole,"memory"},
            {RssRole,"rss"},{VirtualRole,"virtualMemory"},{ReadRole,"readBytes"},{WriteRole,"writeBytes"},
            {ThreadsRole,"threads"},{SelectedRole,"selected"},{DepthRole,"depth"}};
}

void ProcessModel::updateProcesses(std::vector<ProcessInfo> processes) {
    QSet<qint64> livePids;
    for (const auto& process : processes) livePids.insert(process.pid);
    selectedPids_.intersect(livePids);
    beginResetModel();
    all_ = std::move(processes);
    rebuildVisible();
    endResetModel();
    emit selectionChanged();
}

void ProcessModel::setFilter(const QString& filter) {
    if (filter_ == filter) return;
    filter_ = filter;
    beginResetModel();
    rebuildVisible();
    endResetModel();
    emit filterChanged();
}

QVariantMap ProcessModel::selectedProcess() const {
    if (selectedPids_.isEmpty()) return {};
    const auto pid = *selectedPids_.constBegin();
    const auto it = std::find_if(all_.begin(), all_.end(), [pid](const ProcessInfo& process) { return process.pid == pid; });
    if (it == all_.end()) return {};
    return {{"pid",it->pid},{"parentPid",it->parentPid},{"name",QString::fromStdString(it->name)},
            {"commandLine",QString::fromStdString(it->commandLine)},{"state",QString::fromStdString(it->state)},
            {"user",QString::fromStdString(it->user)},{"cgroup",QString::fromStdString(it->cgroup)},
            {"cpu",it->cpuPercent},{"memory",it->memoryPercent},{"rss",QVariant::fromValue<qulonglong>(it->rssBytes)},
            {"virtualMemory",QVariant::fromValue<qulonglong>(it->virtualBytes)},
            {"readBytes",QVariant::fromValue<qulonglong>(it->readBytes)},
            {"writeBytes",QVariant::fromValue<qulonglong>(it->writeBytes)},{"threads",it->threads}};
}

void ProcessModel::toggleSelected(int visibleRow) {
    if (visibleRow < 0 || visibleRow >= static_cast<int>(visible_.size())) return;
    const auto& process = all_[static_cast<std::size_t>(visible_[static_cast<std::size_t>(visibleRow)])];
    if (selectedPids_.contains(process.pid)) selectedPids_.remove(process.pid); else selectedPids_.insert(process.pid);
    const auto modelIndex = index(visibleRow, 0);
    emit dataChanged(modelIndex, modelIndex, {SelectedRole});
    emit selectionChanged();
}

void ProcessModel::clearSelection() {
    if (selectedPids_.isEmpty()) return;
    selectedPids_.clear();
    if (!visible_.empty()) emit dataChanged(index(0,0), index(static_cast<int>(visible_.size()) - 1,0), {SelectedRole});
    emit selectionChanged();
}

void ProcessModel::rebuildVisible() {
    visible_.clear();
    visible_.reserve(all_.size());
    const auto needle = filter_.trimmed().toLower();
    for (std::size_t i = 0; i < all_.size(); ++i) {
        const auto& process = all_[i];
        if (needle.isEmpty() || QString::fromStdString(process.name).toLower().contains(needle) ||
            QString::number(process.pid).contains(needle) || QString::fromStdString(process.user).toLower().contains(needle) ||
            QString::fromStdString(process.commandLine).toLower().contains(needle)) visible_.push_back(static_cast<int>(i));
    }
}

int ProcessModel::depthFor(std::int64_t pid, const QHash<qint64, qint64>& parents) const {
    int depth = 0;
    QSet<qint64> seen;
    auto current = pid;
    while (depth < 12 && parents.contains(current)) {
        if (seen.contains(current)) break;
        seen.insert(current);
        const auto parent = parents.value(current);
        if (parent <= 1 || parent == current || !parents.contains(parent)) break;
        current = parent;
        ++depth;
    }
    return depth;
}

} // namespace monesys
