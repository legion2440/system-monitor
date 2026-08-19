#pragma once

#include "core/MetricTypes.hpp"
#include <QAbstractListModel>
#include <vector>

namespace monesys {
class NetworkModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role { NameRole=Qt::UserRole+1, Ipv4Role, RxBytesRole, RxPacketsRole, RxErrorsRole, RxDroppedRole, RxFifoRole,
                RxFrameRole, RxCompressedRole, RxMulticastRole, TxBytesRole, TxPacketsRole, TxErrorsRole, TxDroppedRole,
                TxFifoRole, TxCollisionsRole, TxCarrierRole, TxCompressedRole, RxRateRole, TxRateRole,
                RxSessionRole, TxSessionRole };
    explicit NetworkModel(QObject* parent=nullptr);
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index,int role) const override;
    QHash<int,QByteArray> roleNames() const override;
    void updateInterfaces(std::vector<NetworkInterfaceStats> interfaces);
private:
    std::vector<NetworkInterfaceStats> interfaces_;
};
}
