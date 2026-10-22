#include "mainwindow.h"

#include <QSettings>
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

#include "appthemepalettes.h"

int main(int argc, char *argv[])
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("fusion"));
    
    switch (settings.value("theme", "tropicalParadise").toString()) {
    case "tropicalParadise":
        a.setPalette(AppThemePalettes::tropicalParadise());
        break;
    case "spicedPumpkin":
        a.setPalette(AppThemePalettes::spicedPumpkin());
        break;
    case "royalSapphire":
        a.setPalette(AppThemePalettes::royalSapphire());
        break;
    default:
        a.setPalette(AppThemePalettes::tropicalParadise());
        break;
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
