// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "ThemeManager.h"
#include <QColor>
#include <QStyle>

namespace pkt::ui::qtwidgets {

void ThemeManager::applyLightTheme(QApplication& app) {
    // Use Fusion style for consistent cross-platform appearance
    app.setStyle("Fusion");
    app.setPalette(createLightPalette());
}

void ThemeManager::applyDarkTheme(QApplication& app) {
    app.setStyle("Fusion");
    app.setPalette(createDarkPalette());
}

void ThemeManager::applySystemTheme(QApplication& app) {
    // Let Qt use system default palette
    if (app.style()) {
        app.setPalette(app.style()->standardPalette());
    }
}

QPalette ThemeManager::createLightPalette() {
    QPalette palette;
    
    // Light gray background optimized for poker card visibility
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, Qt::black);
    
    // White input fields and text areas
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor(233, 233, 233));
    
    // Tooltip styling
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    
    // Text and button colors
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::BrightText, Qt::red);
    
    // Selection and link colors
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    
    return palette;
}

QPalette ThemeManager::createDarkPalette() {
    QPalette palette;
    
    // Dark theme for future use
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    
    return palette;
}

} // namespace pkt::ui::qtwidgets