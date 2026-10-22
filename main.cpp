#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

#include "appthemepalettes.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("fusion"));
    a.setPalette(AppThemePalettes::tropicalParadise());

    MainWindow w;
    w.show();
    return QApplication::exec();
}
