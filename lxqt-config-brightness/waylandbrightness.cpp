#include "waylandbrightness.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QProcess>
#include <algorithm>

WaylandBrightness::WaylandBrightness() {}

WaylandBrightness::~WaylandBrightness() {}

bool WaylandBrightness::isAvailable() const
{
    return true;
}

bool WaylandBrightness::checkWlGammaCtlAvailable() const
{
    return !QStandardPaths::findExecutable(QStringLiteral("wl-gammactl-rust")).isEmpty();
}

QList<MonitorInfo> WaylandBrightness::getMonitorsInfo()
{
    QList<MonitorInfo> monitors;
    QProcess process;
    process.start(QStringLiteral("wlr-randr"), QStringList());
    if (!process.waitForFinished(3000) || process.exitCode() != 0) {
        qWarning() << "wlr-randr failed, using fallback default monitor";
        MonitorInfo defaultMonitor(-1, QStringLiteral("Default"), -1);
        defaultMonitor.setBrightness(1.0f);
        m_currentBrightness[defaultMonitor.name()] = 1.0f;
        monitors.append(defaultMonitor);
        return monitors;
    }

    QList<MonitorInfo> namedMonitors = parseWlrRandrOutput(process.readAllStandardOutput());
    if (namedMonitors.isEmpty()) {
        qWarning() << "No monitors parsed from wlr-randr, using fallback default monitor";
        MonitorInfo defaultMonitor(-1, QStringLiteral("Default"), -1);
        defaultMonitor.setBrightness(1.0f);
        m_currentBrightness[defaultMonitor.name()] = 1.0f;
        monitors.append(defaultMonitor);
        return monitors;
    }

    for (auto &info : namedMonitors) {
        info.setBrightness(1.0f);
        m_currentBrightness[info.name()] = 1.0f;
        monitors.append(info);
    }
    return monitors;
}

QList<MonitorInfo> WaylandBrightness::parseWlrRandrOutput(const QByteArray &output)
{
    QList<MonitorInfo> monitors;
    QString text = QString::fromUtf8(output);
    QStringList lines = text.split(u'\n', Qt::SkipEmptyParts);
    QString currentOutput;

    QRegularExpression outputRe(QStringLiteral("^(\\S+)"));

    for (const QString &line : lines) {
        QRegularExpressionMatch match = outputRe.match(line);
        if (match.hasMatch()) {
            if (!currentOutput.isEmpty()) {
                MonitorInfo info(-1, currentOutput, -1);
                info.setBrightness(1.0f);
                monitors.append(info);
            }
            currentOutput = match.captured(1);
        }
    }
    if (!currentOutput.isEmpty()) {
        MonitorInfo info(-1, currentOutput, -1);
        info.setBrightness(1.0f);
        monitors.append(info);
    }
    return monitors;
}

void WaylandBrightness::setGammaWithWlGammaCtl(float brightness)
{
    if (!checkWlGammaCtlAvailable()) {
        qWarning() << "wl-gammactl-rust not found in PATH";
        return;
    }

    // 清理所有残留的 wl-gammactl-rust 进程
    QProcess::execute(QStringLiteral("pkill"), QStringList() << QStringLiteral("-f") << QStringLiteral("wl-gammactl-rust"));

    float clamped = std::clamp(brightness, 0.0f, 1.0f);
    QStringList args;
    args << QStringLiteral("-c") << QString::number(clamped)
         << QStringLiteral("-b") << QStringLiteral("1.0")
         << QStringLiteral("-g") << QStringLiteral("1.0")
         << QStringLiteral("-s") << QStringLiteral("1.0");

    qDebug() << "Executing (detached): wl-gammactl-rust" << args.join(QLatin1Char(' '));

    if (!QProcess::startDetached(QStringLiteral("wl-gammactl-rust"), args)) {
        qWarning() << "Failed to start detached wl-gammactl-rust";
    } else {
        qDebug() << "wl-gammactl-rust started detached successfully";
        for (auto &key : m_currentBrightness.keys()) {
            m_currentBrightness[key] = clamped;
        }
    }
}

void WaylandBrightness::setMonitorsSettings(const QList<MonitorInfo> &monitors)
{
    if (monitors.isEmpty()) {
        qWarning() << "No monitors provided to set brightness";
        return;
    }

    float brightness = monitors.first().brightness();
    qDebug() << "Setting global brightness to" << brightness;
    setGammaWithWlGammaCtl(brightness);
}