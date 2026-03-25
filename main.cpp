#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <SharedCppLib2/logt.hpp>
#include <SharedCppLib2/platform.hpp>

int main(int argc, char *argv[])
{
    logt::claim("main");
    logt::addfile(platform::executable_dir()/"autosudo_gui.log");

    QApplication a(argc, argv);

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
