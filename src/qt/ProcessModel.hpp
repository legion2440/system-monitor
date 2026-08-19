#pragma once

#include "core/MetricTypes.hpp"

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <QVariantMap>
#include <vector>

namespace monesys {

class ProcessModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QVariantMap selectedProcess READ selectedProcess NOTIFY selectionChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
    Q_PROPERTY(bool orderFrozen READ orderFrozen WRITE setOrderFrozen NOTIFY orderFrozenChanged)
    Q_PROPERTY(bool orderEffectivelyFrozen READ orderEffectivelyFrozen NOTIFY orderFrozenChanged)

public:
    enum Role {
        PidRole = Qt::UserRole + 1, ParentPidRole, NameRole, CommandLineRole, StateCodeRole, StateRole, UserRole, CgroupRole,
        CpuRole, MemoryRole, RssRole, VirtualRole, ReadRole, WriteRole, ThreadsRole, SelectedRole, DepthRole
    };

    explicit ProcessModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void updateProcesses(std::vector<ProcessInfo> processes);

    [[nodiscard]] QString filter() const { return filter_; }
    void setFilter(const QString& filter);
    [[nodiscard]] QVariantMap selectedProcess() const;
    [[nodiscard]] int selectedCount() const { return static_cast<int>(selectionOrder_.size()); }
    [[nodiscard]] bool orderFrozen() const { return orderFrozen_; }
    void setOrderFrozen(bool frozen);
    [[nodiscard]] bool orderEffectivelyFrozen() const { return orderFrozen_ || !selectionOrder_.isEmpty(); }
    Q_INVOKABLE void toggleSelected(int visibleRow);
    Q_INVOKABLE void clearSelection();

signals:
    void filterChanged();
    void selectionChanged();
    void orderFrozenChanged();

private:
    [[nodiscard]] std::vector<int> buildVisible(const std::vector<ProcessInfo>& processes) const;
    [[nodiscard]] std::vector<qint64> visiblePids(const std::vector<ProcessInfo>& processes, const std::vector<int>& visible) const;
    [[nodiscard]] QHash<qint64, int> buildDepthCache(const std::vector<ProcessInfo>& processes) const;
    void applyFrozenOrder(std::vector<ProcessInfo>& processes) const;
    void syncFreezeState();

    std::vector<ProcessInfo> all_;
    std::vector<int> visible_;
    QHash<qint64, int> depthCache_;
    QSet<qint64> selectedPids_;
    QList<qint64> selectionOrder_;
    std::vector<qint64> frozenOrder_;
    QString filter_;
    bool orderFrozen_{false};
};

} // namespace monesys
