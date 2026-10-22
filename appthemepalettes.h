#ifndef APPTHEMEPALETTES_H
#define APPTHEMEPALETTES_H

#include <QObject>

#include <QPalette>
#include <QColor>

class AppThemePalettes : public QObject
{
    Q_OBJECT
public:
    explicit AppThemePalettes(QObject *parent = nullptr);
    static QPalette tropicalParadise(); // default
    static QPalette spicedPumpkin();
    static QPalette royalSapphire();

};

#endif // APPTHEMEPALETTES_H
