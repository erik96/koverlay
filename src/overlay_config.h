#pragma once
#include <QObject>
#include <QString>

class OverlayConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QString textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(double panelOpacity READ panelOpacity WRITE setPanelOpacity NOTIFY panelOpacityChanged)
public:
    explicit OverlayConfig(QObject *parent=nullptr) : QObject(parent) {}

    const QString& text() const { return text_; }
    const QString& fontFamily() const { return fontFamily_; }
    int fontSize() const { return fontSize_; }

    const QString& textColor() const { return textColor_; }
    bool bold() const { return bold_; }
    double panelOpacity() const { return panelOpacity_; }

public slots:
    void setText(QString v)       { if (v == text_) return; text_ = std::move(v); emit textChanged(); }
    void setFontFamily(QString v) { if (v == fontFamily_) return; fontFamily_ = std::move(v); emit fontFamilyChanged(); }
    void setFontSize(int v)       { if (v == fontSize_) return; fontSize_ = v; emit fontSizeChanged(); }

    void setTextColor(QString v)  { if (v == textColor_) return; textColor_ = std::move(v); emit textColorChanged(); }
    void setBold(bool v)          { if (v == bold_) return; bold_ = v; emit boldChanged(); }
    void setPanelOpacity(double v){ v = std::clamp(v, 0.0, 1.0); if (v == panelOpacity_) return; panelOpacity_ = v; emit panelOpacityChanged(); }

signals:
    void textChanged();
    void fontFamilyChanged();
    void fontSizeChanged();
    void textColorChanged();
    void boldChanged();
    void panelOpacityChanged();

private:
    QString text_;
    QString fontFamily_;
    int     fontSize_    = 28;

    QString textColor_   = QStringLiteral("#FFFFFF");
    bool    bold_        = true;
    double  panelOpacity_ = 0.35; // matches #59 alpha (~89/255)
};
