/*
    Copyright (C) 2016  P.L. Lucas <selairi@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "xrandrbrightness.h"
#include "waylandbrightness.h"
#include "displaybrightnessbackend.h"
#include "brightnesswatcher.h"

#include <QDebug>
#include <QTimer>
#include <LXQt/SingleApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include "brightnesssettings.h"

#include <iostream>
#include <cmath>
#include <algorithm>

enum CommandLineParseResult {
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

enum UiMode {
    GUI,
    TUI
};

struct BrightnessConfigData {
    BrightnessConfigData();

    UiMode mode;
    bool increaseBrightness;
    bool decreaseBrightness;
    bool setBrightness;
    float brightnessValue;
    bool resetGamma;
};

BrightnessConfigData::BrightnessConfigData()
    : mode(UiMode::GUI),
      increaseBrightness(false),
      decreaseBrightness(false),
      setBrightness(false),
      brightnessValue(0.0f),
      resetGamma(false)
{
}

static CommandLineParseResult parseCommandLine(QCommandLineParser *parser, BrightnessConfigData *config, QString *errorMessage)
{
    parser->setApplicationDescription(QStringLiteral("LXQt Config Brightness"));
    QCommandLineOption increaseOption(QStringList() << QStringLiteral("i") << QStringLiteral("increase"),
            QObject::tr("Increase brightness."));
    QCommandLineOption decreaseOption(QStringList() << QStringLiteral("d") << QStringLiteral("decrease"),
            QObject::tr("Decrease brightness."));
    QCommandLineOption setOption(QStringList() << QStringLiteral("s") << QStringLiteral("set"),
            QObject::tr("Set brightness from 1 to 100."), QStringLiteral("brightness"));
    QCommandLineOption resetGammaOption(QStringList() << QStringLiteral("r") << QStringLiteral("reset"),
            QObject::tr("Reset gamma to default value."));
    QCommandLineOption helpOption = parser->addHelpOption();
    QCommandLineOption versionOption = parser->addVersionOption();
    parser->addOption(increaseOption);
    parser->addOption(decreaseOption);
    parser->addOption(setOption);
    parser->addOption(resetGammaOption);

    const QStringList args = QCoreApplication::arguments();
    if (args.size() <= 1) { // no arguments given. GUI mode
        config->mode = UiMode::GUI;
        return CommandLineOk;
    } else {
        config->mode = UiMode::TUI;
    }

    if (!parser->parse(QCoreApplication::arguments())) {
        *errorMessage = parser->errorText();
        return CommandLineError;
    }

    if (parser->isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser->isSet(helpOption))
        return CommandLineHelpRequested;

    const bool isIncreaseSet = parser->isSet(increaseOption);
    const bool isDecreaseSet = parser->isSet(decreaseOption);

    const bool isSetBrightnessSet = parser->isSet(setOption);
    float brightnessValue = 0.0f;
    if (isSetBrightnessSet)
        brightnessValue = parser->value(setOption).toFloat();

    const bool isResetGammaSet = parser->isSet(resetGammaOption);

    if ((isIncreaseSet || isDecreaseSet) && isSetBrightnessSet) {
        *errorMessage = QObject::tr("%1: Can't use increase/decrease and set in conjunction").arg(QCoreApplication::applicationName());
        return CommandLineError;
    }

    if (isIncreaseSet && isDecreaseSet) {
        *errorMessage = QObject::tr("%1: Can't use increase and decrease options in conjunction").arg(QCoreApplication::applicationName());
        return CommandLineError;
    }

    config->increaseBrightness = isIncreaseSet;
    config->decreaseBrightness = isDecreaseSet;
    config->setBrightness = isSetBrightnessSet;
    config->brightnessValue = brightnessValue;
    config->resetGamma = isResetGammaSet;

    return CommandLineOk;
}

int main(int argn, char* argv[])
{
    LXQt::SingleApplication app(argn, argv);

    const QString VERINFO = QStringLiteral(LXQT_CONFIG_VERSION
                                           "\nliblxqt   " LXQT_VERSION
                                           "\nQt        " QT_VERSION_STR);
    app.setApplicationVersion(VERINFO);

    // Command line options
    QCommandLineParser parser;

    BrightnessConfigData config;
    QString errorMessage;

    switch(parseCommandLine(&parser, &config, &errorMessage)) {
    case CommandLineOk:
        break;
    case CommandLineError:
        std::cerr << qPrintable(errorMessage);
        std::cerr << "\n\n";
        std::cerr << qPrintable(parser.helpText());
        return EXIT_FAILURE;
    case CommandLineVersionRequested:
        parser.showVersion();
        Q_UNREACHABLE();
    case CommandLineHelpRequested:
        parser.showHelp();
        Q_UNREACHABLE();
    }

    if (config.mode == UiMode::GUI) {
        BrightnessSettings brightnessSettings;
        brightnessSettings.setWindowIcon(QIcon(QLatin1String(ICON_DIR) + QStringLiteral("/brightnesssettings.svg")));
        brightnessSettings.show();
        return app.exec();
    }

    // ---- TUI 模式：强制使用显示器后端（Gamma 亮度），忽略硬件背光 ----
    float sign = (config.decreaseBrightness) ? -1.0f : 1.0f;
    float brightnessValue = std::clamp(config.brightnessValue, 0.0f, 100.0f) / 100.0f; // 0.0~1.0

    // 不再检查背光，直接使用显示器后端（XRandr 或 Wayland）
    DisplayBrightnessBackend *brightness = nullptr;
    if (QGuiApplication::platformName() == QStringLiteral("wayland")) {
        brightness = new WaylandBrightness();
    } else {
        brightness = new XRandrBrightness();
    }

    const QList<MonitorInfo> monitors = brightness->getMonitorsInfo();
    QList<MonitorInfo> monitorsChanged;

    if (config.resetGamma) {
        for (const MonitorInfo &monitor : monitors) {
            MonitorInfo m = monitor;
            m.setBrightness(1.0f);
            monitorsChanged.append(m);
        }
    } else {
        for (const MonitorInfo &monitor : monitors) {
            MonitorInfo m = monitor;
            // 调整计算：使结果在 0.0 ~ 1.0 范围内
            float current = m.brightness(); // 当前亮度，范围 0.0~1.0
            // 根据 increase/decrease 或 set 调整
            float newBrightness;
            if (config.setBrightness) {
                // 直接使用 brightnessValue（0~1）
                newBrightness = brightnessValue;
            } else {
                // increase/decrease: 每次调整 0.05（5%）
                float step = 0.05f * sign;
                newBrightness = current + step;
            }
            // 限制范围
            newBrightness = std::clamp(newBrightness, 0.0f, 1.0f);
            m.setBrightness(newBrightness);
            monitorsChanged.append(m);
        }
    }

    brightness->setMonitorsSettings(monitorsChanged);
    delete brightness;

    return 0;
}