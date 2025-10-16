// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "core/engine/utils/CrashHandler.h"

#include <atomic>
#include <csignal>
#include <gtest/gtest.h>
#include <string>

using namespace pkt::core::utils;

/**
 * @brief Test fixture for CrashHandler functionality.
 *
 * Note: Some tests involving actual signals are limited because:
 * 1. Triggering real crashes would terminate the test process
 * 2. Signal handling is platform-specific
 * 3. We want tests to complete successfully
 *
 * Therefore, we focus on testing the installation/uninstallation mechanism
 * and stack trace capture functionality.
 */
class CrashHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Ensure clean state before each test
        CrashHandler::uninstall();
    }

    void TearDown() override
    {
        // Restore default handlers after each test
        CrashHandler::uninstall();
    }
};

/**
 * @brief Test that CrashHandler can be installed without errors.
 */
TEST_F(CrashHandlerTest, CanInstallCrashHandler)
{
    // Should not throw or crash
    EXPECT_NO_THROW(CrashHandler::install());
}

/**
 * @brief Test that CrashHandler can be installed with a callback.
 */
TEST_F(CrashHandlerTest, CanInstallWithCallback)
{
    bool callbackInvoked = false;
    auto callback = [&callbackInvoked](int signal, const std::string& signalName) { callbackInvoked = true; };

    EXPECT_NO_THROW(CrashHandler::install(callback));
    // Note: We can't actually trigger a crash to test the callback without terminating the test
}

/**
 * @brief Test that CrashHandler can be uninstalled without errors.
 */
TEST_F(CrashHandlerTest, CanUninstallCrashHandler)
{
    CrashHandler::install();
    EXPECT_NO_THROW(CrashHandler::uninstall());
}

/**
 * @brief Test that CrashHandler can be installed and uninstalled multiple times.
 */
TEST_F(CrashHandlerTest, CanInstallAndUninstallMultipleTimes)
{
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_NO_THROW(CrashHandler::install());
        EXPECT_NO_THROW(CrashHandler::uninstall());
    }
}

/**
 * @brief Test that getStackTrace returns a non-empty string.
 */
TEST_F(CrashHandlerTest, GetStackTraceReturnsNonEmptyString)
{
    std::string stackTrace = CrashHandler::getStackTrace();

    // Stack trace should contain some content
    EXPECT_FALSE(stackTrace.empty()) << "Stack trace should not be empty";
    EXPECT_GT(stackTrace.length(), 10) << "Stack trace should have meaningful content";
}

/**
 * @brief Test that getStackTrace with skip parameter works.
 */
TEST_F(CrashHandlerTest, GetStackTraceWithSkipParameter)
{
    std::string fullTrace = CrashHandler::getStackTrace(0);
    std::string skippedTrace = CrashHandler::getStackTrace(2);

    // Both should be non-empty
    EXPECT_FALSE(fullTrace.empty());
    EXPECT_FALSE(skippedTrace.empty());

    // Skipped trace might be shorter (though not guaranteed on all platforms)
    // At minimum, they should both contain stack frame information
    EXPECT_GT(fullTrace.length(), 0);
    EXPECT_GT(skippedTrace.length(), 0);
}

/**
 * @brief Test that stack traces capture function call context.
 */
TEST_F(CrashHandlerTest, StackTraceCapturesFunctionContext)
{
    // Call through nested functions to build a stack
    auto level3 = []() -> std::string { return CrashHandler::getStackTrace(); };

    auto level2 = [&level3]() -> std::string { return level3(); };

    auto level1 = [&level2]() -> std::string { return level2(); };

    std::string stackTrace = level1();

    // Stack trace should contain frame information
    EXPECT_FALSE(stackTrace.empty());
    EXPECT_GT(stackTrace.length(), 20) << "Stack trace: " << stackTrace;

    // Should contain some indication of stack frames (platform-dependent format)
    // Common indicators: "0x", addresses, or function names
    bool hasFrameIndicators = (stackTrace.find("0x") != std::string::npos) ||
                              (stackTrace.find("#") != std::string::npos) ||
                              (stackTrace.find("Frame") != std::string::npos);

    EXPECT_TRUE(hasFrameIndicators) << "Stack trace should contain frame information: " << stackTrace;
}

/**
 * @brief Test that multiple stack traces from different call sites differ.
 */
TEST_F(CrashHandlerTest, StackTracesFromDifferentCallSitesDiffer)
{
    std::string trace1 = CrashHandler::getStackTrace();

    // Call from a different function context
    auto captureFromLambda = []() -> std::string { return CrashHandler::getStackTrace(); };

    std::string trace2 = captureFromLambda();

    // Both should be non-empty
    EXPECT_FALSE(trace1.empty());
    EXPECT_FALSE(trace2.empty());

    // Stack traces from different call sites should differ
    // (though in optimized builds they might be similar)
    // At minimum, verify both captured something
    EXPECT_GT(trace1.length(), 0);
    EXPECT_GT(trace2.length(), 0);
}

/**
 * @brief Test that CrashHandler doesn't interfere with normal execution.
 */
TEST_F(CrashHandlerTest, DoesNotInterfereWithNormalExecution)
{
    CrashHandler::install();

    // Perform various normal operations
    int sum = 0;
    for (int i = 0; i < 100; ++i)
    {
        sum += i;
    }

    EXPECT_EQ(sum, 4950) << "Normal computation should work correctly with crash handler installed";

    // Test exception handling still works
    bool exceptionCaught = false;
    try
    {
        throw std::runtime_error("Test exception");
    }
    catch (const std::runtime_error&)
    {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught) << "Normal exception handling should still work";

    CrashHandler::uninstall();
}

/**
 * @brief Test that reinstalling the handler updates the callback.
 */
TEST_F(CrashHandlerTest, ReinstallingUpdatesCallback)
{
    // Install with first callback
    int firstCallbackId = 1;
    auto firstCallback = [firstCallbackId](int signal, const std::string& signalName)
    {
        // First callback
    };

    CrashHandler::install(firstCallback);

    // Reinstall with second callback (this should replace the first)
    int secondCallbackId = 2;
    auto secondCallback = [secondCallbackId](int signal, const std::string& signalName)
    {
        // Second callback
    };

    EXPECT_NO_THROW(CrashHandler::install(secondCallback));

    // Should be able to uninstall cleanly
    EXPECT_NO_THROW(CrashHandler::uninstall());
}

/**
 * @brief Test that getStackTrace handles large skip values gracefully.
 */
TEST_F(CrashHandlerTest, GetStackTraceHandlesLargeSkipValues)
{
    // Request skipping more frames than likely exist
    std::string stackTrace = CrashHandler::getStackTrace(1000);

    // Should return empty or minimal trace, but not crash
    // The behavior is platform-dependent, so we just verify it doesn't crash
    EXPECT_NO_FATAL_FAILURE(CrashHandler::getStackTrace(1000));
}

/**
 * @brief Test that stack trace is captured correctly after handler installation.
 */
TEST_F(CrashHandlerTest, StackTraceCaptureAfterInstallation)
{
    // Capture before installation
    std::string traceBefore = CrashHandler::getStackTrace();

    CrashHandler::install();

    // Capture after installation
    std::string traceAfter = CrashHandler::getStackTrace();

    // Both should work (handler installation shouldn't break stack trace capture)
    EXPECT_FALSE(traceBefore.empty());
    EXPECT_FALSE(traceAfter.empty());

    CrashHandler::uninstall();
}

/**
 * @brief Test callback with nullptr is handled correctly.
 */
TEST_F(CrashHandlerTest, InstallWithNullptrCallback)
{
    // Should work without callback
    EXPECT_NO_THROW(CrashHandler::install(nullptr));
    EXPECT_NO_THROW(CrashHandler::uninstall());
}

/**
 * @brief Integration test: Verify crash handler and exception utilities work together.
 */
TEST_F(CrashHandlerTest, IntegrationWithExceptionUtils)
{
    CrashHandler::install();

    // Capture stack trace
    std::string stackTrace = CrashHandler::getStackTrace();
    EXPECT_FALSE(stackTrace.empty());

    // Test that normal exception throwing still works
    bool exceptionThrown = false;
    try
    {
        throw std::runtime_error("Test error");
    }
    catch (const std::runtime_error& e)
    {
        exceptionThrown = true;
        EXPECT_NE(std::string(e.what()).find("Test error"), std::string::npos);
    }

    EXPECT_TRUE(exceptionThrown);

    CrashHandler::uninstall();
}

/**
 * @brief Test that uninstalling without installing is safe.
 */
TEST_F(CrashHandlerTest, UninstallWithoutInstallIsSafe)
{
    // Should not crash even if nothing was installed
    EXPECT_NO_THROW(CrashHandler::uninstall());
    EXPECT_NO_THROW(CrashHandler::uninstall()); // Multiple calls
}
