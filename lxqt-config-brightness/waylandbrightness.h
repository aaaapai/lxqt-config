#ifndef WAYLANDBRIGHTNESS_H
#define WAYLANDBRIGHTNESS_H

#include "displaybrightnessbackend.h"
#include <QProcess>
#include <QMap>

class WaylandBrightness : public DisplayBrightnessBackend
{
public:
    WaylandBrightness();
    ~WaylandBrightness() override;

    QList<MonitorInfo> getMonitorsInfo() override;
    void setMonitorsSettings(const QList<MonitorInfo> &monitors) override;
    bool isAvailable() const override;

private:
    bool checkWlrRandrAvailable() const;
    QList<MonitorInfo> parseWlrRandrOutput(const QByteArray &output);
    void runWlrRandr(const QStringList &args);
    QMap<QString, float> m_currentBrightness;
};

#endif