#ifndef DISPLAYBRIGHTNESSBACKEND_H
#define DISPLAYBRIGHTNESSBACKEND_H

#include <QList>
#include "monitorinfo.h"

class DisplayBrightnessBackend
{
public:
    virtual ~DisplayBrightnessBackend() = default;
    virtual QList<MonitorInfo> getMonitorsInfo() = 0;
    virtual void setMonitorsSettings(const QList<MonitorInfo> &monitors) = 0;
    virtual bool isAvailable() const = 0;
};

#endif