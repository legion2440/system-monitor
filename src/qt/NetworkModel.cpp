#include "qt/NetworkModel.hpp"

namespace monesys {
NetworkModel::NetworkModel(QObject* parent):QAbstractListModel(parent){}
int NetworkModel::rowCount(const QModelIndex& parent) const { return parent.isValid()?0:static_cast<int>(interfaces_.size()); }
QVariant NetworkModel::data(const QModelIndex& index,int role) const {
    if(!index.isValid()||index.row()<0||index.row()>=static_cast<int>(interfaces_.size())) return {};
    const auto& i=interfaces_[static_cast<std::size_t>(index.row())];
    switch(role){
    case NameRole:return QString::fromStdString(i.name); case Ipv4Role:return QString::fromStdString(i.ipv4);
    case RxBytesRole:return QVariant::fromValue<qulonglong>(i.rx.bytes); case RxPacketsRole:return QVariant::fromValue<qulonglong>(i.rx.packets);
    case RxErrorsRole:return QVariant::fromValue<qulonglong>(i.rx.errors); case RxDroppedRole:return QVariant::fromValue<qulonglong>(i.rx.dropped);
    case RxFifoRole:return QVariant::fromValue<qulonglong>(i.rx.fifo); case RxFrameRole:return QVariant::fromValue<qulonglong>(i.rx.frameOrCollisions);
    case RxCompressedRole:return QVariant::fromValue<qulonglong>(i.rx.compressed); case RxMulticastRole:return QVariant::fromValue<qulonglong>(i.rx.multicastOrCarrier);
    case TxBytesRole:return QVariant::fromValue<qulonglong>(i.tx.bytes); case TxPacketsRole:return QVariant::fromValue<qulonglong>(i.tx.packets);
    case TxErrorsRole:return QVariant::fromValue<qulonglong>(i.tx.errors); case TxDroppedRole:return QVariant::fromValue<qulonglong>(i.tx.dropped);
    case TxFifoRole:return QVariant::fromValue<qulonglong>(i.tx.fifo); case TxCollisionsRole:return QVariant::fromValue<qulonglong>(i.tx.frameOrCollisions);
    case TxCarrierRole:return QVariant::fromValue<qulonglong>(i.tx.multicastOrCarrier); case TxCompressedRole:return QVariant::fromValue<qulonglong>(i.tx.compressed);
    case RxRateRole:return i.rxBytesPerSecond; case TxRateRole:return i.txBytesPerSecond; default:return {};}
}
QHash<int,QByteArray> NetworkModel::roleNames() const { return {{NameRole,"name"},{Ipv4Role,"ipv4"},{RxBytesRole,"rxBytes"},{RxPacketsRole,"rxPackets"},{RxErrorsRole,"rxErrors"},{RxDroppedRole,"rxDropped"},{RxFifoRole,"rxFifo"},{RxFrameRole,"rxFrame"},{RxCompressedRole,"rxCompressed"},{RxMulticastRole,"rxMulticast"},{TxBytesRole,"txBytes"},{TxPacketsRole,"txPackets"},{TxErrorsRole,"txErrors"},{TxDroppedRole,"txDropped"},{TxFifoRole,"txFifo"},{TxCollisionsRole,"txCollisions"},{TxCarrierRole,"txCarrier"},{TxCompressedRole,"txCompressed"},{RxRateRole,"rxRate"},{TxRateRole,"txRate"}}; }
void NetworkModel::updateInterfaces(std::vector<NetworkInterfaceStats> interfaces){ beginResetModel(); interfaces_=std::move(interfaces); endResetModel(); }
}
