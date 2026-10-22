#include "appthemepalettes.h"

AppThemePalettes::AppThemePalettes(QObject *parent)
    : QObject{parent}
{}

QPalette AppThemePalettes::tropicalParadise()
{
    QPalette palette;

    QColor lightMint("#E7F7E8");
    QColor softGreen("#B5E6BD");
    QColor mintGreen("#76D5A2");
    QColor tealAccent("#36AFA0");
    QColor deepTeal("#176F72");

    palette.setColor(QPalette::Window, deepTeal);
    palette.setColor(QPalette::Base, QColor("#104E50"));          // Darkened teal background for input fields
    palette.setColor(QPalette::AlternateBase, QColor("#1A5B5E"));

    palette.setColor(QPalette::WindowText, lightMint);
    palette.setColor(QPalette::Text, lightMint);
    palette.setColor(QPalette::PlaceholderText, softGreen);

    palette.setColor(QPalette::Button, tealAccent);
    palette.setColor(QPalette::ButtonText, lightMint);

    palette.setColor(QPalette::Highlight, mintGreen);
    palette.setColor(QPalette::HighlightedText, deepTeal);
    palette.setColor(QPalette::Accent, tealAccent);

    // Muted text colors for disabled fields
    QColor disabledMuted("#448A8C");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledMuted);

    return palette;
}

QPalette AppThemePalettes::spicedPumpkin()
{
    QPalette palette;

    QColor cream("#FFF1E2");
    QColor softApricot("#FBD2A5");
    QColor warmOrange("#F2A15E");
    QColor darkPumpkin("#D97532");
    QColor deepBrown("#91451F");

    palette.setColor(QPalette::Window, deepBrown);
    palette.setColor(QPalette::Base, QColor("#662F13"));         // Darker brown background for entry text fields
    palette.setColor(QPalette::AlternateBase, QColor("#7A3918"));

    palette.setColor(QPalette::WindowText, cream);
    palette.setColor(QPalette::Text, cream);
    palette.setColor(QPalette::PlaceholderText, softApricot);

    palette.setColor(QPalette::Button, darkPumpkin);
    palette.setColor(QPalette::ButtonText, cream);

    palette.setColor(QPalette::Highlight, warmOrange);
    palette.setColor(QPalette::HighlightedText, deepBrown);
    palette.setColor(QPalette::Accent, darkPumpkin);

    // Muted colors for disabled fields
    QColor disabledMuted("#AA6B4E");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledMuted);

    return palette;
}

QPalette AppThemePalettes::royalSapphire()
{
    QPalette palette;

    QColor iceWhite("#E9F0FF");
    QColor lightBlue("#B8CBFA");
    QColor mediumBlue("#7899ED");
    QColor sapphire("#4168C9");
    QColor royalNavy("#233A7A");

    palette.setColor(QPalette::Window, royalNavy);
    palette.setColor(QPalette::Base, QColor("#192956"));
    palette.setColor(QPalette::AlternateBase, QColor("#20336B"));

    palette.setColor(QPalette::WindowText, iceWhite);
    palette.setColor(QPalette::Text, iceWhite);
    palette.setColor(QPalette::PlaceholderText, lightBlue);

    palette.setColor(QPalette::Button, sapphire);
    palette.setColor(QPalette::ButtonText, iceWhite);

    palette.setColor(QPalette::Highlight, mediumBlue);
    palette.setColor(QPalette::HighlightedText, royalNavy);
    palette.setColor(QPalette::Accent, sapphire);

    QColor disabledMuted("#566A9E");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledMuted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledMuted);

    return palette;
}