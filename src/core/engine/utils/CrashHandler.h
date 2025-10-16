// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <csignal>
#include <cstdlib>
#include <functional>
#include <string>

namespace pkt::core::utils
{

/**
 * @brief Crash handler for capturing and logging segmentation faults and other critical signals.
 *
 * This utility sets up signal handlers for SIGSEGV, SIGABRT, SIGILL, and SIGFPE to capture
 * crash information including stack traces (on supported platforms). When a crash occurs,
 * it logs the crash details and optionally calls a custom handler before terminating.
 *
 * Usage:
 *   CrashHandler::install(logger);
 *   // Or with custom handler:
 *   CrashHandler::install(logger, []() { std::cerr << "Custom cleanup\n"; });
 *
 * Platform support:
 * - Windows (MSVC): Uses CaptureStackBackTrace
 * - Linux/Mac (GCC/Clang): Uses backtrace() and backtrace_symbols()
 */
class CrashHandler
{
  public:
    using CrashCallback = std::function<void(int signal, const std::string& signalName)>;

    /**
     * @brief Install crash handlers for common fatal signals.
     *
     * Sets up signal handlers for:
     * - SIGSEGV (segmentation fault)
     * - SIGABRT (abort signal)
     * - SIGILL (illegal instruction)
     * - SIGFPE (floating point exception)
     *
     * @param callback Optional callback to be invoked before terminating
     */
    static void install(CrashCallback callback = nullptr);

    /**
     * @brief Remove previously installed crash handlers.
     *
     * Restores default signal handlers.
     */
    static void uninstall();

    /**
     * @brief Get a string representation of the current stack trace.
     *
     * Captures the current call stack and formats it as a multi-line string.
     * Maximum depth is platform-dependent (typically 64-128 frames).
     *
     * @param skip Number of top frames to skip (default 0)
     * @return Stack trace as formatted string
     */
    static std::string getStackTrace(int skip = 0);

  private:
    static void signalHandler(int signal);
    static const char* getSignalName(int signal);

    static CrashCallback s_callback;
};

} // namespace pkt::core::utils
