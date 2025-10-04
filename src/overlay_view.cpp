#include "overlay_view.h"
#include "overlay_config.h"

#include <QQmlContext>
#include <QScreen>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QDebug>

#include <wayland-client.h>
#include <LayerShellQt/Window>

OverlayView::OverlayView(OverlayConfig *cfg, QWindow *parent)
: QQuickView(parent), cfg_(cfg)
{
    setColor(Qt::transparent);
    setFlags(Qt::FramelessWindowHint);
    setFlag(Qt::WindowTransparentForInput, true);
    setResizeMode(QQuickView::SizeRootObjectToView);

    // expose config to QML
    rootContext()->setContextProperty(QStringLiteral("cfg"), cfg_);

    // layer-shell setup
    auto *ls = LayerShellQt::Window::get(this);
    ls->setLayer(LayerShellQt::Window::LayerOverlay); // or LayerTop if needed
    ls->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    ls->setExclusiveZone(0);
    ls->setAnchors(LayerShellQt::Window::Anchors(
        LayerShellQt::Window::AnchorTop
        | LayerShellQt::Window::AnchorBottom
        | LayerShellQt::Window::AnchorLeft
        | LayerShellQt::Window::AnchorRight));

    setSource(QUrl(QStringLiteral("qrc:/koverlay/Overlay.qml")));
    if (status() == QQuickView::Error) {
        for (const auto &e : errors()) qWarning() << e.toString();
    }

    connect(this, &QWindow::visibleChanged, this, [this](bool v){
        if (!v) return;
        if (auto *ls = LayerShellQt::Window::get(this))
            ls->setLayer(LayerShellQt::Window::LayerOverlay);
        applyEmptyInputRegion();
    });
}

void OverlayView::selectScreenByIndex(int idx) {
    const auto screens = QGuiApplication::screens();
    if (idx >= 0 && idx < screens.size()) setScreen(screens[idx]);
    else if (auto *prim = QGuiApplication::primaryScreen()) setScreen(prim);
}

void OverlayView::toggle() { setVisible(!isVisible()); }

void OverlayView::showOverlay() {
    setGeometry(screen()->geometry());
    showFullScreen();
    raise();
    requestActivate();
    applyEmptyInputRegion();
}

void OverlayView::hideOverlay() { hide(); }

void OverlayView::applyEmptyInputRegion() {
    auto *pni = QGuiApplication::platformNativeInterface();
    auto *wlSurf = static_cast<wl_surface*>(pni->nativeResourceForWindow("surface", this));
    auto *wlComp = static_cast<wl_compositor*>(pni->nativeResourceForIntegration("compositor"));
    if (!wlSurf || !wlComp) return;
    wl_region *empty = wl_compositor_create_region(wlComp);
    wl_surface_set_input_region(wlSurf, empty);
    wl_region_destroy(empty);
}
