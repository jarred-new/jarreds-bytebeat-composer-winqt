#include "mainwindow.h"

#include <QSettings>
#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QPalette>

#include "appthemepalettes.h"

int main(int argc, char *argv[])
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/JarredsBeatComposer.ico"));
    a.setStyle(QStyleFactory::create("fusion"));
    
    const QString theme = settings.value("theme", "tropicalParadise").toString();
    if (theme == "tropicalParadise") {
        a.setPalette(AppThemePalettes::tropicalParadise());
    } else if (theme == "spicedPumpkin") {
        a.setPalette(AppThemePalettes::spicedPumpkin());
    } else if (theme == "royalSapphire") {
        a.setPalette(AppThemePalettes::royalSapphire());
    } else {
        a.setPalette(AppThemePalettes::tropicalParadise());
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
