#pragma once

#include "core/MetricTypes.hpp"

#include <QAbstractListModel>
#include <QHash>
#include <QSet>
#include <QString>
#include <QVariantMap>
#include <vector>

namespace monesys {

class ProcessModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QVariantMap selectedProcess READ selectedProcess NOTIFY selectionChanged)

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
    Q_INVOKABLE void toggleSelected(int visibleRow);
    Q_INVOKABLE void clearSelection();

signals:
    void filterChanged();
    void selectionChanged();

private:
    void rebuildVisible();
    int depthFor(std::int64_t pid, const QHash<qint64, qint64>& parents) const;
    std::vector<ProcessInfo> all_;
    std::vector<int> visible_;
    QSet<qint64> selectedPids_;
    QString filter_;
};

} // namespace monesys
