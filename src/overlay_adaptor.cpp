#include "overlay_adaptor.h"
#include "overlay_view.h"

OverlayAdaptor::OverlayAdaptor(OverlayView *v)
: QDBusAbstractAdaptor(v), v_(v) {}

void OverlayAdaptor::Toggle() { v_->toggle(); }
void OverlayAdaptor::Show()   { v_->showOverlay(); }
void OverlayAdaptor::Hide()   { v_->hideOverlay(); }
