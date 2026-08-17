#include "qt/AppController.hpp"
#include "platform/ProviderFactory.hpp"

#include <QMetaObject>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace monesys {

AppController::AppController(QObject* parent)
    : QObject(parent), processModel_(this), networkModel_(this), sensorModel_(this),
      cpuHistory_(std::chrono::seconds(60)), ramHistory_(std::chrono::seconds(60)),
      rxHistory_(std::chrono::seconds(60)), txHistory_(std::chrono::seconds(60)),
      temperatureHistory_(std::chrono::seconds(60)), fanHistory_(std::chrono::seconds(60)) {
    sampler_ = std::make_unique<SamplingController>(createPlatformMetricProvider(), [this](Snapshot snapshot) {
        QMetaObject::invokeMethod(this, [this, snapshot=std::move(snapshot)]() mutable { applySnapshot(std::move(snapshot)); }, Qt::QueuedConnection);
    });
    sampler_->setInterval(std::chrono::milliseconds(pollIntervalMs_));
}

AppController::~AppController(){ if(sampler_) sampler_->stop(); }
void AppController::start(){ if(sampler_) sampler_->start(); }
QString AppController::osName() const{return QString::fromStdString(snapshot_.system.osName);} QString AppController::userName() const{return QString::fromStdString(snapshot_.system.userName);} QString AppController::hostname() const{return QString::fromStdString(snapshot_.system.hostname);} QString AppController::cpuModel() const{return QString::fromStdString(snapshot_.system.cpuModel);}
qulonglong AppController::taskTotal() const{return snapshot_.system.tasks.total;} qulonglong AppController::taskRunning() const{return snapshot_.system.tasks.running;} qulonglong AppController::taskSleeping() const{return snapshot_.system.tasks.sleeping;} qulonglong AppController::taskBlocked() const{return snapshot_.system.tasks.uninterruptible;} qulonglong AppController::taskZombie() const{return snapshot_.system.tasks.zombie;} qulonglong AppController::taskStopped() const{return snapshot_.system.tasks.stopped+snapshot_.system.tasks.traced;}
double AppController::cpuUsage() const{return snapshot_.cpu.usagePercent;} double AppController::cpuFrequencyMHz() const{return snapshot_.cpu.frequencyMHz;} QVariantList AppController::coreUsage() const{return toVariantList(snapshot_.cpu.coreUsagePercent);}
qulonglong AppController::ramTotal() const{return snapshot_.memory.ramTotalBytes;} qulonglong AppController::ramUsed() const{return snapshot_.memory.ramUsedBytes;} qulonglong AppController::swapTotal() const{return snapshot_.memory.swapTotalBytes;} qulonglong AppController::swapUsed() const{return snapshot_.memory.swapUsedBytes;} qulonglong AppController::diskTotal() const{return snapshot_.memory.diskTotalBytes;} qulonglong AppController::diskUsed() const{return snapshot_.memory.diskUsedBytes;}
QString AppController::fanName() const{return QString::fromStdString(snapshot_.fan.name);} QVariantList AppController::cpuHistory() const{return toVariantList(cpuHistory_.values());} QVariantList AppController::ramHistory() const{return toVariantList(ramHistory_.values());} QVariantList AppController::rxHistory() const{return toVariantList(rxHistory_.values());} QVariantList AppController::txHistory() const{return toVariantList(txHistory_.values());} QVariantList AppController::temperatureHistory() const{return toVariantList(temperatureHistory_.values());} QVariantList AppController::fanHistory() const{return toVariantList(fanHistory_.values());}

void AppController::setPollIntervalMs(int milliseconds){ static constexpr int allowed[]={250,500,1000,2000,5000}; const auto closest=*std::min_element(std::begin(allowed),std::end(allowed),[milliseconds](int l,int r){return std::abs(l-milliseconds)<std::abs(r-milliseconds);}); if(pollIntervalMs_==closest)return; pollIntervalMs_=closest; sampler_->setInterval(std::chrono::milliseconds(pollIntervalMs_)); emit pollIntervalChanged(); }
void AppController::setGraphFps(int fps){ const int normalized=fps<=10?10:(fps<=30?30:60); if(graphFps_==normalized)return; graphFps_=normalized; emit graphFpsChanged(); }
void AppController::setYScaleMode(const QString& mode){ const auto normalized=(mode=="Fixed"||mode=="Peak")?mode:QStringLiteral("Auto"); if(yScaleMode_==normalized)return; yScaleMode_=normalized; emit yScaleModeChanged(); }
void AppController::setPaused(bool paused){ if(paused_==paused)return; paused_=paused; sampler_->setPaused(paused_); emit pausedChanged(); }

void AppController::applySnapshot(Snapshot snapshot){
    snapshot_=std::move(snapshot); aggregateRxRate_=0.0; aggregateTxRate_=0.0;
    for(const auto& iface:snapshot_.network){ if(iface.name=="lo")continue; aggregateRxRate_+=iface.rxBytesPerSecond; aggregateTxRate_+=iface.txBytesPerSecond; }
    temperature_=-273.15; temperatureSource_.clear();
    if(!snapshot_.sensors.empty()){
        const auto preferred=std::find_if(snapshot_.sensors.begin(),snapshot_.sensors.end(),[](const SensorInfo& sensor){ const auto lower=QString::fromStdString(sensor.name).toLower(); return lower.contains("cpu")||lower.contains("package")||lower.contains("tctl")||lower.contains("coretemp"); });
        const auto& sensor=preferred==snapshot_.sensors.end()?snapshot_.sensors.front():*preferred; temperature_=sensor.value; temperatureSource_=QString::fromStdString(sensor.source);
    }
    const auto now=TimedHistory<double>::Clock::now(); cpuHistory_.push(snapshot_.cpu.usagePercent,now);
    const auto ramPercent=snapshot_.memory.ramTotalBytes==0?0.0:100.0*static_cast<double>(snapshot_.memory.ramUsedBytes)/static_cast<double>(snapshot_.memory.ramTotalBytes);
    ramHistory_.push(ramPercent,now); rxHistory_.push(aggregateRxRate_,now); txHistory_.push(aggregateTxRate_,now);
    if(thermalAvailable())temperatureHistory_.push(temperature_,now); if(snapshot_.fan.available)fanHistory_.push(snapshot_.fan.rpm,now);
    processModel_.updateProcesses(snapshot_.processes); networkModel_.updateInterfaces(snapshot_.network); sensorModel_.updateSensors(snapshot_.sensors);
    emit snapshotChanged(); emit historyChanged();
}

QVariantList AppController::toVariantList(const std::vector<double>& values){ QVariantList list; list.reserve(static_cast<qsizetype>(values.size())); for(const auto value:values)list.push_back(value); return list; }

} // namespace monesys
