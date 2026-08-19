#include "qt/SensorModel.hpp"
namespace monesys {
SensorModel::SensorModel(QObject* parent):QAbstractListModel(parent){}
int SensorModel::rowCount(const QModelIndex& parent) const{return parent.isValid()?0:static_cast<int>(sensors_.size());}
QVariant SensorModel::data(const QModelIndex& index,int role) const{
    if(!index.isValid()||index.row()<0||index.row()>=static_cast<int>(sensors_.size()))return{};
    const auto& s=sensors_[static_cast<std::size_t>(index.row())];
    switch(role){case NameRole:return QString::fromStdString(s.name);case ChipRole:return QString::fromStdString(s.chip);case SourceRole:return QString::fromStdString(s.source);case UnitRole:return QString::fromStdString(s.unit);case ValueRole:return s.value;default:return{};}
}
QHash<int,QByteArray> SensorModel::roleNames() const{return{{NameRole,"name"},{ChipRole,"chip"},{SourceRole,"source"},{UnitRole,"unit"},{ValueRole,"value"}};}
void SensorModel::updateSensors(std::vector<SensorInfo> sensors){beginResetModel();sensors_=std::move(sensors);endResetModel();}
}
