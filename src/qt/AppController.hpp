#pragma once

#include "core/MetricTypes.hpp"
#include "core/SamplingController.hpp"
#include "core/TimedHistory.hpp"
#include "qt/NetworkModel.hpp"
#include "qt/ProcessModel.hpp"
#include "qt/SensorModel.hpp"

#include <QObject>
#include <QVariantList>
#include <memory>

namespace monesys {

class AppController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString osName READ osName NOTIFY snapshotChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY snapshotChanged)
    Q_PROPERTY(QString hostname READ hostname NOTIFY snapshotChanged)
    Q_PROPERTY(QString cpuModel READ cpuModel NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskTotal READ taskTotal NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskRunning READ taskRunning NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskSleeping READ taskSleeping NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskBlocked READ taskBlocked NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskZombie READ taskZombie NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong taskStopped READ taskStopped NOTIFY snapshotChanged)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY snapshotChanged)
    Q_PROPERTY(double cpuFrequencyMHz READ cpuFrequencyMHz NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList coreUsage READ coreUsage NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong ramTotal READ ramTotal NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong ramUsed READ ramUsed NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong swapTotal READ swapTotal NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong swapUsed READ swapUsed NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong diskTotal READ diskTotal NOTIFY snapshotChanged)
    Q_PROPERTY(qulonglong diskUsed READ diskUsed NOTIFY snapshotChanged)
    Q_PROPERTY(double aggregateRxRate READ aggregateRxRate NOTIFY snapshotChanged)
    Q_PROPERTY(double aggregateTxRate READ aggregateTxRate NOTIFY snapshotChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY snapshotChanged)
    Q_PROPERTY(QString temperatureSource READ temperatureSource NOTIFY snapshotChanged)
    Q_PROPERTY(bool thermalAvailable READ thermalAvailable NOTIFY snapshotChanged)
    Q_PROPERTY(bool fanAvailable READ fanAvailable NOTIFY snapshotChanged)
    Q_PROPERTY(bool fanActive READ fanActive NOTIFY snapshotChanged)
    Q_PROPERTY(double fanRpm READ fanRpm NOTIFY snapshotChanged)
    Q_PROPERTY(double fanLevel READ fanLevel NOTIFY snapshotChanged)
    Q_PROPERTY(QString fanName READ fanName NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList ramHistory READ ramHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList rxHistory READ rxHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList txHistory READ txHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList temperatureHistory READ temperatureHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList fanHistory READ fanHistory NOTIFY historyChanged)
    Q_PROPERTY(ProcessModel* processes READ processes CONSTANT)
    Q_PROPERTY(NetworkModel* network READ network CONSTANT)
    Q_PROPERTY(SensorModel* sensors READ sensors CONSTANT)
    Q_PROPERTY(int pollIntervalMs READ pollIntervalMs WRITE setPollIntervalMs NOTIFY pollIntervalChanged)
    Q_PROPERTY(int graphFps READ graphFps WRITE setGraphFps NOTIFY graphFpsChanged)
    Q_PROPERTY(QString yScaleMode READ yScaleMode WRITE setYScaleMode NOTIFY yScaleModeChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY snapshotChanged)

public:
    explicit AppController(QObject* parent=nullptr);
    ~AppController() override;
    void start();
    [[nodiscard]] QString osName() const; [[nodiscard]] QString userName() const; [[nodiscard]] QString hostname() const; [[nodiscard]] QString cpuModel() const;
    [[nodiscard]] qulonglong taskTotal() const; [[nodiscard]] qulonglong taskRunning() const; [[nodiscard]] qulonglong taskSleeping() const; [[nodiscard]] qulonglong taskBlocked() const; [[nodiscard]] qulonglong taskZombie() const; [[nodiscard]] qulonglong taskStopped() const;
    [[nodiscard]] double cpuUsage() const; [[nodiscard]] double cpuFrequencyMHz() const; [[nodiscard]] QVariantList coreUsage() const;
    [[nodiscard]] qulonglong ramTotal() const; [[nodiscard]] qulonglong ramUsed() const; [[nodiscard]] qulonglong swapTotal() const; [[nodiscard]] qulonglong swapUsed() const; [[nodiscard]] qulonglong diskTotal() const; [[nodiscard]] qulonglong diskUsed() const;
    [[nodiscard]] double aggregateRxRate() const{return aggregateRxRate_;} [[nodiscard]] double aggregateTxRate() const{return aggregateTxRate_;}
    [[nodiscard]] double temperature() const{return temperature_;} [[nodiscard]] QString temperatureSource() const{return temperatureSource_;}
    [[nodiscard]] bool thermalAvailable() const{return snapshot_.capabilities.thermal&&temperature_>-200.0;}
    [[nodiscard]] bool fanAvailable() const{return snapshot_.fan.available;} [[nodiscard]] bool fanActive() const{return snapshot_.fan.active;}
    [[nodiscard]] double fanRpm() const{return snapshot_.fan.rpm;} [[nodiscard]] double fanLevel() const{return snapshot_.fan.levelPercent;} [[nodiscard]] QString fanName() const;
    [[nodiscard]] QVariantList cpuHistory() const; [[nodiscard]] QVariantList ramHistory() const; [[nodiscard]] QVariantList rxHistory() const; [[nodiscard]] QVariantList txHistory() const; [[nodiscard]] QVariantList temperatureHistory() const; [[nodiscard]] QVariantList fanHistory() const;
    [[nodiscard]] ProcessModel* processes(){return &processModel_;} [[nodiscard]] NetworkModel* network(){return &networkModel_;} [[nodiscard]] SensorModel* sensors(){return &sensorModel_;}
    [[nodiscard]] int pollIntervalMs() const{return pollIntervalMs_;} void setPollIntervalMs(int milliseconds);
    [[nodiscard]] int graphFps() const{return graphFps_;} void setGraphFps(int fps);
    [[nodiscard]] QString yScaleMode() const{return yScaleMode_;} void setYScaleMode(const QString& mode);
    [[nodiscard]] bool paused() const{return paused_;} void setPaused(bool paused);
    [[nodiscard]] QString lastError() const{return QString::fromStdString(snapshot_.error);}

signals:
    void snapshotChanged(); void historyChanged(); void pollIntervalChanged(); void graphFpsChanged(); void yScaleModeChanged(); void pausedChanged();

private:
    void applySnapshot(Snapshot snapshot);
    static QVariantList toVariantList(const std::vector<double>& values);
    Snapshot snapshot_;
    ProcessModel processModel_; NetworkModel networkModel_; SensorModel sensorModel_;
    std::unique_ptr<SamplingController> sampler_;
    TimedHistory<double> cpuHistory_,ramHistory_,rxHistory_,txHistory_,temperatureHistory_,fanHistory_;
    double aggregateRxRate_{}; double aggregateTxRate_{}; double temperature_{-273.15}; QString temperatureSource_;
    int pollIntervalMs_{1000}; int graphFps_{30}; QString yScaleMode_{"Auto"}; bool paused_{false};
};

} // namespace monesys
