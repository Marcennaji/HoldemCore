// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "CrashHandler.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>

// Platform-specific includes for stack trace
#ifdef _WIN32
// IMPORTANT: windows.h MUST be included before dbghelp.h
// to ensure proper type definitions (PSTR, HANDLE, PUCHAR, etc.)
#include <windows.h>

#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")
#else

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

#endif

namespace pkt::core::utils
{

CrashHandler::CrashCallback CrashHandler::s_callback = nullptr;

void CrashHandler::install(CrashCallback callback)
{
    s_callback = callback;

    // Install signal handlers for common crash signals
    std::signal(SIGSEGV, signalHandler); // Segmentation fault
    std::signal(SIGABRT, signalHandler); // Abort signal
    std::signal(SIGILL, signalHandler);  // Illegal instruction
    std::signal(SIGFPE, signalHandler);  // Floating point exception

#ifdef _WIN32
    // On Windows, also handle invalid parameter
    std::signal(SIGTERM, signalHandler);
#endif
}

void CrashHandler::uninstall()
{
    std::signal(SIGSEGV, SIG_DFL);
    std::signal(SIGABRT, SIG_DFL);
    std::signal(SIGILL, SIG_DFL);
    std::signal(SIGFPE, SIG_DFL);
#ifdef _WIN32
    std::signal(SIGTERM, SIG_DFL);
#endif

    s_callback = nullptr;
}

void CrashHandler::signalHandler(int signal)
{
    const char* signalName = getSignalName(signal);

    // Write crash info to stderr (async-signal-safe)
    const char* header = "\n========================================\n";
    const char* crashMsg = "CRASH DETECTED: ";
    const char* footer = "========================================\n";
    const char* stackMsg = "\nStack trace:\n";

    // Use write() instead of iostream for signal safety
    std::cerr << header << crashMsg << signalName << " (signal " << signal << ")\n";

    // Get and print stack trace
    std::string stackTrace = getStackTrace(2); // Skip signal handler frames
    std::cerr << stackMsg << stackTrace << footer << std::endl;

    // Call custom callback if provided
    if (s_callback)
    {
        try
        {
            s_callback(signal, signalName);
        }
        catch (...)
        {
            // Ignore exceptions in callback during crash
        }
    }

    // Restore default handler and re-raise to get proper core dump
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

const char* CrashHandler::getSignalName(int signal)
{
    switch (signal)
    {
    case SIGSEGV:
        return "SIGSEGV (Segmentation fault)";
    case SIGABRT:
        return "SIGABRT (Abort)";
    case SIGILL:
        return "SIGILL (Illegal instruction)";
    case SIGFPE:
        return "SIGFPE (Floating point exception)";
#ifdef _WIN32
    case SIGTERM:
        return "SIGTERM (Termination request)";
#endif
    default:
        return "Unknown signal";
    }
}

std::string CrashHandler::getStackTrace(int skip)
{
    std::ostringstream oss;

#ifdef _WIN32
    // Windows implementation using CaptureStackBackTrace
    const int maxFrames = 128;
    void* stack[maxFrames];

    WORD frames = CaptureStackBackTrace(skip, maxFrames, stack, nullptr);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (WORD i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64) (stack[i]);

        oss << "  [" << i << "] ";

        if (SymFromAddr(process, address, nullptr, symbol))
        {
            oss << symbol->Name;
        }
        else
        {
            oss << "0x" << std::hex << address << std::dec;
        }

        // Always include the raw address in hex so tests and consumers
        // have a reliable frame indicator (addresses are platform-agnostic).
        oss << " (0x" << std::hex << address << std::dec << ")";

        // Try to get file and line info
        DWORD displacement;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (SymGetLineFromAddr64(process, address, &displacement, &line))
        {
            oss << " at " << line.FileName << ":" << line.LineNumber;
        }

        oss << "\n";
    }

    free(symbol);
    SymCleanup(process);

#else
    // Unix/Linux implementation using backtrace()
    const int maxFrames = 128;
    void* buffer[maxFrames];

    int frames = backtrace(buffer, maxFrames);
    char** symbols = backtrace_symbols(buffer, frames);

    if (symbols)
    {
        for (int i = skip; i < frames; i++)
        {
            oss << "  [" << (i - skip) << "] ";

            // Try to demangle C++ names
            std::string mangledName = symbols[i];
            size_t begin = mangledName.find('(');
            size_t end = mangledName.find('+', begin);

            if (begin != std::string::npos && end != std::string::npos && begin < end)
            {
                std::string functionName = mangledName.substr(begin + 1, end - begin - 1);

                int status;
                char* demangled = abi::__cxa_demangle(functionName.c_str(), nullptr, nullptr, &status);

                if (status == 0 && demangled)
                {
                    oss << mangledName.substr(0, begin + 1) << demangled << mangledName.substr(end);
                    free(demangled);
                }
                else
                {
                    oss << mangledName;
                }
            }
            else
            {
                oss << mangledName;
            }

            // Also append the raw pointer address to aid detection and tests
            oss << " (0x" << std::hex << reinterpret_cast<uintptr_t>(buffer[i]) << std::dec << ")";
            oss << "\n";
        }

        free(symbols);
    }
    else
    {
        oss << "  (Stack trace unavailable)\n";
    }
#endif

    return oss.str();
}

} // namespace pkt::core::utils
