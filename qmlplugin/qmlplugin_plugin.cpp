/*
 * Copyright (C) 2020 ~ 2021 deepin Technology Co., Ltd.
 *
 * Author:     JiDe Zhang <zhangjide@deepin.org>
 *
 * Maintainer: JiDe Zhang <zhangjide@deepin.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qmlplugin_plugin.h"
#include "dquickwindow.h"
#include "dqmlglobalobject.h"
#include "dquickitemviewport.h"
#include "dquickiconprovider.h"
#include "dhandlecontextmenuwindow.h"
#include "dquickblitframebuffer.h"
#include "dquickglow.h"
#include "dquickinwindowblendblur.h"

#include "private/dconfigwrapper_p.h"
#include "private/dquickiconimage_p.h"
#include "private/dquickdciiconimage_p.h"
#include "private/dquickiconlabel_p.h"
#include "private/dquickbusyindicator_p.h"
#include "private/dquickcontrolpalette_p.h"
#include "private/dsettingscontainer_p.h"

#include <DFontManager>

#include <QQmlEngine>
#include <qqml.h>

DGUI_USE_NAMESPACE

DQUICK_BEGIN_NAMESPACE

template<typename T>
inline void dtkRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    qmlRegisterType<T>(uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType<T>(uri2, versionMajor, versionMinor, qmlName);
}
template<typename T>
inline void dtkRegisterAnonymousType(const char *uri1, const char *uri2, int versionMajor) {
#if (QT_VERSION > QT_VERSION_CHECK(5, 14, 0))
    qmlRegisterAnonymousType<T>(uri1, versionMajor);
    if (uri2)
        qmlRegisterAnonymousType<T>(uri2, versionMajor);
#else
    Q_UNUSED(uri1)
    Q_UNUSED(uri2)
    Q_UNUSED(versionMajor)
    qmlRegisterType<T>();
#endif
}
template<typename T>
inline void dtkRegisterUncreatableType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName, const QString& reason) {
    qmlRegisterUncreatableType<T>(uri1, versionMajor, versionMinor, qmlName, reason);
    if (uri2)
        qmlRegisterUncreatableType<T>(uri2, versionMajor, versionMinor, qmlName, reason);
}
template <typename T>
inline void dtkRegisterSingletonType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName,
                                     QObject *(*callback)(QQmlEngine *, QJSEngine *)) {
    qmlRegisterSingletonType<T>(uri1, versionMajor, versionMinor, qmlName, callback);
    if (uri2)
        qmlRegisterSingletonType<T>(uri2, versionMajor, versionMinor, qmlName, callback);
}
inline void dtkRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/%1.qml");
    const QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType(url, uri2, versionMajor, versionMinor, qmlName);
}
inline void dtkStyleRegisterSingletonType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/style/%1.qml");
    QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterSingletonType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterSingletonType(url, uri2, versionMajor, versionMinor, qmlName);
}
inline void dtkSettingsRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/settings/%1.qml");
    const QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType(url, uri2, versionMajor, versionMinor, qmlName);
}

void QmlpluginPlugin::registerTypes(const char *uri)
{
    // @uri org.deepin.dtk
    qmlRegisterModule(uri, 1, 0);
    // @uri org.deepin.dtk.impl
    const QByteArray implUri = QByteArray(uri).append(".impl");
    qmlRegisterModule(implUri.constData(), 1, 0);
    // @uri org.deepin.dtk.private
    const QByteArray privateUri = QByteArray(uri).append(".private");
    qmlRegisterModule(privateUri.constData(), 1, 0);
    // @uri org.deepin.dtk.controls
    const QByteArray controlsUri = QByteArray(uri).append(".controls");
    qmlRegisterModule(controlsUri.constData(), 1, 0);
    // @uri org.deepin.dtk.style
    const QByteArray styleUri = QByteArray(uri).append(".style");
    qmlRegisterModule(styleUri.constData(), 1, 0);
    // @uri org.deepin.dtk.settings
    const QByteArray settingsUri = QByteArray(uri) + ".settings";
    qmlRegisterModule(settingsUri, 1, 0);

    // for org.deepin.dtk and org.deepin.dtk.impl
    dtkRegisterType<DQuickIconImage>(uri, implUri, 1, 0, "QtIcon");
    dtkRegisterType<DQuickIconLabel>(uri, implUri, 1, 0, "IconLabel");
    dtkRegisterType<DQuickBusyIndicator>(uri, implUri, 1, 0, "BusyIndicator");
    dtkRegisterType<DQuickGlow>(uri, implUri, 1, 0, "GlowEffect");
    dtkRegisterType<DQUICK_NAMESPACE::DQuickDciIconImage>(uri, implUri, 1, 0, "DciIcon");
    dtkRegisterType<DQuickItemViewport>(uri, implUri, 1, 0, "ItemViewport");
    dtkRegisterType<DFontManager>(uri, implUri, 1, 0, "FontManager");
    dtkRegisterType<DHandleContextMenuWindow>(uri, implUri, 1, 0, "ContextMenuWindow");
    dtkRegisterType<DQuickBlitFramebuffer>(uri, implUri, 1, 0, "BlitFramebuffer");
    dtkRegisterType<DConfigWrapper>(uri, implUri, 1, 0, "Config");
    dtkRegisterType<DQuickInWindowBlendBlur>(uri, implUri, 1, 0, "InWindowBlendBlur");
    dtkRegisterType<DQuickControlPalette>(uri, implUri, 1, 0, "Palette");
    dtkRegisterType<DQuickControlColorSelector>(uri, implUri, 1, 0, "ColorSelector");

    dtkRegisterAnonymousType<DQUICK_NAMESPACE::DQuickDciIcon>(uri, implUri, 1);
    dtkRegisterAnonymousType<DQuickControlColor>(uri, implUri, 1);

    dtkRegisterUncreatableType<DQuickWindow>(uri, implUri, 1, 0, "Window", "DQuickWindow Attached");

    qRegisterMetaType<DQUICK_NAMESPACE::DQuickDciIcon>();
    qRegisterMetaType<DQuickControlColor>("ControlColor");

    //DQMLGlobalObject 依赖 DWindowManagerHelper中枚举的定义，所以需要先注册
    dtkRegisterSingletonType<DWindowManagerHelper>(uri, implUri, 1, 0, "WindowManagerHelper",
                                                   [](QQmlEngine *, QJSEngine *) -> QObject * {
        auto helper = DWindowManagerHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    });
    dtkRegisterSingletonType<DGuiApplicationHelper>(uri, implUri, 1, 0, "ApplicationHelper",
                                                    [](QQmlEngine *, QJSEngine *) -> QObject * {
        auto helper = DGuiApplicationHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    });
    qRegisterMetaType<DGuiApplicationHelper::ColorType>("Dtk::Gui::DGuiApplicationHelper::ColorType");
    dtkRegisterSingletonType<DQMLGlobalObject>(uri, implUri, 1, 0, "DTK",
                                               [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new DQMLGlobalObject;
    });

    // 自定义的 QML 控件可以通过把 QML 文件注册到环境中的方式来实现
    // for org.deepin.dtk
    dtkRegisterType(uri, nullptr, 1, 0, "RectangularShadow");
    dtkRegisterType(uri, nullptr, 1, 0, "RectangleBorder");
    dtkRegisterType(uri, nullptr, 1, 0, "CicleSpreadAnimation");

    // for org.deepin.dtk.controls
    // QtQuick Controls
    dtkRegisterType(uri, controlsUri, 1, 0, "Button");
    // DTK Controls
    dtkRegisterType(uri, controlsUri, 1, 0, "LineEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "SearchEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "PasswordEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "WarningButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "SuggestButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ToolButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "IconButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "FloatingButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ButtonBoxButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ButtonBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "TitleBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "DialogTitleBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowMinButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowMaxButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowRestoreButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowCloseButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowQuitFullButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowOptionButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowButtonGroup");
    dtkRegisterType(uri, controlsUri, 1, 0, "DialogWindow");
    dtkRegisterType(uri, controlsUri, 1, 0, "AboutDialog");
    dtkRegisterType(uri, controlsUri, 1, 0, "HelpAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "AboutAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "QuitAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "ThemeMenu");
    dtkRegisterType(uri, controlsUri, 1, 0, "AlertToolTip");
    dtkRegisterType(uri, controlsUri, 1, 0, "FlowStyle");

    // for org.deepin.dtk.style(allowed to override)
    dtkStyleRegisterSingletonType(uri, styleUri, 1, 0, "Style");

    // for org.deepin.dtk.settings
    dtkRegisterType<SettingsOption>(settingsUri, implUri, 1, 0, "SettingsOption");
    dtkRegisterType<SettingsGroup>(settingsUri, implUri, 1, 0, "SettingsGroup");
    dtkRegisterType<SettingsContainer>(settingsUri, implUri, 1, 0, "SettingsContainer");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "SettingsDialog");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "OptionDelegate");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "CheckBox");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "LineEdit");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ComboBox");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "NavigationTitle");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ContentTitle");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ContentBackground");
}

void QmlpluginPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    engine->addImageProvider("dtk.icon", new DQuickIconProvider);
    engine->addImageProvider("dtk.dci.icon", new DQuickDciIconProvider);
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}

DQUICK_END_NAMESPACE
