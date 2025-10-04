#pragma once
#include <QDBusAbstractAdaptor>

class OverlayView;

class OverlayAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.erx.KOverlay")
public:
    explicit OverlayAdaptor(OverlayView *v);
public slots:
    Q_NOREPLY void Toggle();
    Q_NOREPLY void Show();
    Q_NOREPLY void Hide();
private:
    OverlayView *v_;
};
