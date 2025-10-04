#include <QApplication>
#include <QDBusConnection>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QFileSystemWatcher>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

#include "overlay_config.h"
#include "overlay_view.h"
#include "overlay_adaptor.h"

// ---------- Defaults ----------
static QString defaultOverlayText() {
    return QStringLiteral("⌨ Keybindings:\n• Super+Enter — Terminal\n• Ctrl+Alt+H — Toggle Overlay");
}

static QString resolveConfigPath() {
    const QByteArray env = qgetenv("KOVERLAY_CONFIG");
    if (!env.isEmpty()) {
        QString p = QString::fromLocal8Bit(env);
        QFileInfo fi(p);
        if (fi.isDir()) p = fi.filePath() + "/config.ini";
        return p;
    }
    QString cfgRoot = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (cfgRoot.isEmpty()) return {};
    QDir().mkpath(cfgRoot + "/koverlay");
    return cfgRoot + "/koverlay/config.ini";
}

static QString expandUserPath(QString p) {
    if (p.startsWith("~/")) return QDir::homePath() + p.mid(1);
    return p;
}
static QString readWholeFileUtf8(const QString &path) {
    QFile f(expandUserPath(path));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    return QString::fromUtf8(f.readAll());
}

// Triple-quoted multiline: text=""" ... """
static QString parseOverlayMultilineText(const QString &iniPath) {
    QFile f(iniPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    QTextStream in(&f);
    bool inOverlay = false;
    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('[') && t.endsWith(']')) {
            const QString sec = t.mid(1, t.size()-2).trimmed();
            inOverlay = (sec.compare("overlay", Qt::CaseInsensitive) == 0);
            continue;
        }
        if (!inOverlay) continue;

        static const QRegularExpression rx(R"(^\s*text\s*=\s*(.*)\s*$)");
        const auto m = rx.match(line);
        if (!m.hasMatch()) continue;

        QString rhs = m.captured(1);

        if (rhs.startsWith(R"(""")") || rhs.startsWith("'''")) {
            const QString delim = rhs.left(3);
            QStringList out;
            QString first = rhs.mid(3);
            if (!first.isEmpty()) out << first;
            while (!in.atEnd()) {
                QString l = in.readLine();
                int idx = l.indexOf(delim);
                if (idx >= 0) { out << l.left(idx); break; }
                out << l;
            }
            return out.join('\n');
        }
        return rhs; // single-line
    }
    return {};
}

static void loadSettingsInto(const QString &path, OverlayConfig *oc) {
    QString text      = defaultOverlayText();
    QString family;
    int     size      = 28;
    QString textColor = QStringLiteral("#FFFFFF"); // default white
    bool    bold      = true;
    double  panelOpacity = 0.35;

    if (!path.isEmpty() && QFile::exists(path)) {
        QSettings s(path, QSettings::IniFormat);

        QString grp = "overlay";
        const auto groups = s.childGroups();
        for (const auto &g : groups)
            if (g.compare("overlay", Qt::CaseInsensitive) == 0) { grp = g; break; }

            s.beginGroup(grp);
        const QString textFile = s.value("textFile").toString();
        if (!textFile.isEmpty()) {
            const QString fileText = readWholeFileUtf8(textFile);
            if (!fileText.isEmpty()) text = fileText;
        } else {
            QString multi = parseOverlayMultilineText(path);
            if (!multi.isEmpty()) {
                text = multi;
            } else {
                QString t1 = s.value("text").toString();
                if (!t1.isEmpty()) { t1.replace("\\n", "\n"); text = t1; }
            }
        }
        family    = s.value("fontFamily", family).toString();
        size      = s.value("fontSize", size).toInt();
        textColor = s.value("textColor", textColor).toString(); // e.g. "#ffcc00" or "orange"
        bold      = s.value("bold", bold).toBool();             // true/false or 1/0
        panelOpacity = s.value("panelOpacity", panelOpacity).toDouble();
        s.endGroup();
    }

    oc->setText(text);
    oc->setFontFamily(family);
    oc->setFontSize(size);
    oc->setTextColor(textColor);
    oc->setBold(bold);
    oc->setPanelOpacity(panelOpacity);

    qInfo() << "koverlay: applied config — len(text):" << text.size()
    << ", fontFamily:" << (family.isEmpty() ? "<default>" : family)
    << ", fontSize:" << size
    << ", textColor:" << textColor
    << ", bold:" << bold
    << ", panelOpacity:" << panelOpacity;
}


int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("wayland"));
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("KOverlay — Wayland overlay panel");
    parser.addHelpOption();
    QCommandLineOption showOpt(QStringList{"s","show"}, "Show overlay on start.");
    QCommandLineOption screenIdxOpt(QStringList{"S","screen-index"}, "Screen index (0..N-1)", "index", "0");
    parser.addOption(showOpt);
    parser.addOption(screenIdxOpt);
    parser.process(app);

    auto *cfg = new OverlayConfig(&app);
    const QString cfgPath = resolveConfigPath();
    loadSettingsInto(cfgPath, cfg);

    QFileSystemWatcher watcher;
    if (!cfgPath.isEmpty()) {
        if (!QFile::exists(cfgPath)) {
            QDir().mkpath(QFileInfo(cfgPath).dir().absolutePath());
            QFile f(cfgPath); f.open(QIODevice::WriteOnly); f.close();
        }
        watcher.addPath(cfgPath);
        QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, &app, [cfgPath, cfg, &watcher](){
            if (!watcher.files().contains(cfgPath) && QFile::exists(cfgPath)) watcher.addPath(cfgPath);
            loadSettingsInto(cfgPath, cfg);
            qInfo() << "koverlay: reloaded config from" << cfgPath;
        });
    }

    OverlayView view(cfg);
    view.selectScreenByIndex(parser.value(screenIdxOpt).toInt());

    QDBusConnection session = QDBusConnection::sessionBus();
    session.registerService("org.erx.KOverlay");
    session.registerObject("/Overlay", &view);
    new OverlayAdaptor(&view);

    if (parser.isSet(showOpt)) view.showOverlay(); else view.hide();
    return app.exec();
}
