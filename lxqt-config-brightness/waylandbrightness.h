#ifndef WAYLANDBRIGHTNESS_H
#define WAYLANDBRIGHTNESS_H

#include "displaybrightnessbackend.h"
#include <QMap>
#include <QString>

class WaylandBrightness : public DisplayBrightnessBackend
{
public:
    WaylandBrightness();
    ~WaylandBrightness() override;

    QList<MonitorInfo> getMonitorsInfo() override;
    void setMonitorsSettings(const QList<MonitorInfo> &monitors) override;
    bool isAvailable() const override;

private:
    bool checkWlGammaCtlAvailable() const;
    QList<MonitorInfo> parseWlrRandrOutput(const QByteArray &output);
    void setGammaWithWlGammaCtl(float brightness);
    QMap<QString, float> m_currentBrightness;
    qint64 m_currentPid = 0;  // 当前运行的 wl-gammactl-rust 进程 PID
};

#endif