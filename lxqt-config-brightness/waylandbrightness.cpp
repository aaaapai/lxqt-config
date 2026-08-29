#include "waylandbrightness.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>
#include <algorithm>

WaylandBrightness::WaylandBrightness()
{
}

WaylandBrightness::~WaylandBrightness()
{
}

bool WaylandBrightness::isAvailable() const
{
    return checkWlrRandrAvailable();
}

bool WaylandBrightness::checkWlrRandrAvailable() const
{
    return !QStandardPaths::findExecutable("wlr-randr").isEmpty();
}

QList<MonitorInfo> WaylandBrightness::getMonitorsInfo()
{
    QList<MonitorInfo> monitors;
    if (!isAvailable())
        return monitors;

    QProcess process;
    process.start("wlr-randr", QStringList() << "--verbose");
    if (!process.waitForFinished(3000)) {
        qWarning() << "wlr-randr timed out";
        return monitors;
    }
    if (process.exitCode() != 0) {
        qWarning() << "wlr-randr exited with code" << process.exitCode();
        return monitors;
    }
    return parseWlrRandrOutput(process.readAllStandardOutput());
}

QList<MonitorInfo> WaylandBrightness::parseWlrRandrOutput(const QByteArray &output)
{
    QList<MonitorInfo> monitors;
    QString text = QString::fromUtf8(output);
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    QString currentOutput;
    float brightness = 1.0f;
    bool hasBrightness = false;

    QRegularExpression outputRe("^output\\s+(\\S+)");
    QRegularExpression brightnessRe("^\\s+brightness\\s+([\\d.]+)");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = outputRe.match(line);
        if (match.hasMatch()) {
            if (!currentOutput.isEmpty() && hasBrightness) {
                MonitorInfo info(-1, currentOutput, -1); // id = -1, no backlight support
                info.setBrightness(brightness);
                monitors.append(info);
            }
            currentOutput = match.captured(1);
            hasBrightness = false;
            brightness = 1.0f;
        } else {
            QRegularExpressionMatch bmatch = brightnessRe.match(line);
            if (bmatch.hasMatch()) {
                brightness = bmatch.captured(1).toFloat();
                hasBrightness = true;
            }
        }
    }
    if (!currentOutput.isEmpty() && hasBrightness) {
        MonitorInfo info(-1, currentOutput, -1);
        info.setBrightness(brightness);
        monitors.append(info);
    }
    return monitors;
}

void WaylandBrightness::setMonitorsSettings(const QList<MonitorInfo> &monitors)
{
    if (!isAvailable())
        return;

    for (const MonitorInfo &info : monitors) {
        float brightness = std::clamp(info.brightness(), 0.0f, 2.0f);
        QStringList args;
        args << "--output" << info.name() << "--brightness" << QString::number(brightness);
        QProcess process;
        process.start("wlr-randr", args);
        if (!process.waitForFinished(3000)) {
            qWarning() << "Failed to set brightness for" << info.name();
        } else if (process.exitCode() != 0) {
            qWarning() << "wlr-randr failed for" << info.name() << ":" << process.readAllStandardError();
        }
    }
}