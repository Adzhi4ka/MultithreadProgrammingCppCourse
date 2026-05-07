#include "app-style.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QIODevice>
#include <QPalette>
#include <QString>
#include <QStyleFactory>

namespace client::presentation::styles {

namespace {

void applyDarkPalette(QApplication& app) {
    QPalette palette;

    palette.setColor(QPalette::Window, QColor{"#232629"});
    palette.setColor(QPalette::WindowText, QColor{"#eff0f1"});
    palette.setColor(QPalette::Base, QColor{"#1b1e20"});
    palette.setColor(QPalette::AlternateBase, QColor{"#2a2e32"});
    palette.setColor(QPalette::ToolTipBase, QColor{"#31363b"});
    palette.setColor(QPalette::ToolTipText, QColor{"#eff0f1"});
    palette.setColor(QPalette::Text, QColor{"#eff0f1"});
    palette.setColor(QPalette::Button, QColor{"#31363b"});
    palette.setColor(QPalette::ButtonText, QColor{"#eff0f1"});
    palette.setColor(QPalette::BrightText, QColor{"#f67400"});
    palette.setColor(QPalette::Highlight, QColor{"#3daee9"});
    palette.setColor(QPalette::HighlightedText, QColor{"#ffffff"});
    palette.setColor(QPalette::PlaceholderText, QColor{"#9aa0a6"});
    palette.setColor(QPalette::Link, QColor{"#3daee9"});

    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor{"#767d84"});
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor{"#767d84"});
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor{"#767d84"});
    palette.setColor(QPalette::Disabled, QPalette::Base, QColor{"#202326"});
    palette.setColor(QPalette::Disabled, QPalette::Button, QColor{"#2a2e32"});

    app.setPalette(palette);
}

void applyStyleSheet(QApplication& app) {
    QFile styleSheetFile{":/client/styles/dark.qss"};
    if (!styleSheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    app.setStyleSheet(QString::fromUtf8(styleSheetFile.readAll()));
}

}  // namespace

void applyDarkTheme(QApplication& app) {
    app.setStyle(QStyleFactory::create("Fusion"));
    applyDarkPalette(app);
    applyStyleSheet(app);
}

}  // namespace client::presentation::styles
