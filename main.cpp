#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QTranslator>

#include <SharedCppLib2/logt.hpp>
#include <SharedCppLib2/platform.hpp>

namespace {

// Qt's menu bar comes out taller than the Windows one; trim it back.
class CompactMenuBarStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;

    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                    const QWidget* widget = nullptr) const override {
        switch (metric) {
        case PM_MenuBarItemSpacing:
        case PM_MenuBarHMargin:
        case PM_MenuBarVMargin:
            return 0;
        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
        }
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == CT_MenuBarItem && widget) {
            s.setHeight(widget->fontMetrics().height() + 6);
        }
        return s;
    }
};

} // namespace


int main(int argc, char *argv[])
{
    logt::claim("main");
    logt::addfile(platform::executable_dir()/"autosudo_gui.log");

    QApplication a(argc, argv);

    // The default style is the Vista era one; this one follows the current Windows look,
    // with the menu bar trimmed to a height that matches the title bar.
    if (QStyle* modern = QStyleFactory::create("windows11")) {
        QApplication::setStyle(new CompactMenuBarStyle(modern));
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "AutoSudoGUI_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

    logt::exit(a.exec());
}
