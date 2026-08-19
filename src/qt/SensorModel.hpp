#pragma once
#include "core/MetricTypes.hpp"
#include <QAbstractListModel>
#include <vector>
namespace monesys {
class SensorModel final:public QAbstractListModel{
    Q_OBJECT
public:
    enum Role{NameRole=Qt::UserRole+1,ChipRole,SourceRole,UnitRole,ValueRole};
    explicit SensorModel(QObject* parent=nullptr);
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index,int role) const override;
    QHash<int,QByteArray> roleNames() const override;
    void updateSensors(std::vector<SensorInfo> sensors);
private: std::vector<SensorInfo> sensors_;
};
}
