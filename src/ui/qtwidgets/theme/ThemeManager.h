// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <QApplication>
#include <QPalette>

namespace pkt::ui::qtwidgets {

/**
 * @brief Manages application themes and visual styling
 * 
 * Provides centralized theme management for consistent visual appearance
 * across the application, independent of system theme settings.
 */
class ThemeManager {
public:
    /**
     * @brief Apply light theme optimized for poker gameplay
     * @param app The QApplication instance to theme
     */
    static void applyLightTheme(QApplication& app);
    
    /**
     * @brief Apply dark theme (future enhancement)
     * @param app The QApplication instance to theme
     */
    static void applyDarkTheme(QApplication& app);
    
    /**
     * @brief Use system default theme
     * @param app The QApplication instance to theme
     */
    static void applySystemTheme(QApplication& app);
    
private:
    static QPalette createLightPalette();
    static QPalette createDarkPalette();
};

} // namespace pkt::ui::qtwidgets