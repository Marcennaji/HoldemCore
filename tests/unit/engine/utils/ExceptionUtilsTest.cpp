// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "core/engine/utils/ExceptionUtils.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

using namespace pkt::core::utils;

/**
 * @brief Test fixture for ExceptionUtils functionality.
 */
class ExceptionUtilsTest : public ::testing::Test
{
  protected:
    /**
     * @brief Helper to extract file name from path (for cross-platform compatibility).
     */
    static std::string extractFileName(const std::string& path)
    {
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            return path.substr(pos + 1);
        }
        return path;
    }
};

/**
 * @brief Test that throwRuntimeError includes source location in message.
 */
TEST_F(ExceptionUtilsTest, ThrowRuntimeErrorIncludesSourceLocation)
{
    try
    {
        throwRuntimeError("Test error message");
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // Verify the message contains expected components
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("Test error message"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find(" in "), std::string::npos) << "Message: " << message;

        // Verify line number is present (should be a number followed by " in ")
        EXPECT_TRUE(message.find(":") != std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that throwInvalidArgument includes source location in message.
 */
TEST_F(ExceptionUtilsTest, ThrowInvalidArgumentIncludesSourceLocation)
{
    try
    {
        throwInvalidArgument("Invalid parameter");
        FAIL() << "Expected std::invalid_argument to be thrown";
    }
    catch (const std::invalid_argument& e)
    {
        std::string message(e.what());

        // Verify the message contains expected components
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("Invalid parameter"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find(" in "), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that throwLogicError includes source location in message.
 */
TEST_F(ExceptionUtilsTest, ThrowLogicErrorIncludesSourceLocation)
{
    try
    {
        throwLogicError("Logic error occurred");
        FAIL() << "Expected std::logic_error to be thrown";
    }
    catch (const std::logic_error& e)
    {
        std::string message(e.what());

        // Verify the message contains expected components
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("Logic error occurred"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find(" in "), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that throwWithLocation works with custom exception types.
 */
TEST_F(ExceptionUtilsTest, ThrowWithLocationSupportsCustomExceptions)
{
    try
    {
        throwWithLocation<std::overflow_error>("Overflow happened");
        FAIL() << "Expected std::overflow_error to be thrown";
    }
    catch (const std::overflow_error& e)
    {
        std::string message(e.what());

        // Verify correct exception type and message
        EXPECT_NE(message.find("Overflow happened"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that throwWithLocation captures line numbers correctly.
 *
 * Two consecutive throws should have different line numbers in their messages.
 */
TEST_F(ExceptionUtilsTest, ThrowWithLocationCapturesDistinctLineNumbers)
{
    std::string firstMessage;
    std::string secondMessage;

    try
    {
        throwRuntimeError("First error"); // Line X
    }
    catch (const std::runtime_error& e)
    {
        firstMessage = e.what();
    }

    try
    {
        throwRuntimeError("Second error"); // Line X+N (different from first)
    }
    catch (const std::runtime_error& e)
    {
        secondMessage = e.what();
    }

    // Extract line numbers from messages
    auto extractLineNumber = [](const std::string& msg) -> int
    {
        size_t colonPos = msg.find(".cpp:");
        if (colonPos != std::string::npos)
        {
            size_t start = colonPos + 5; // Length of ".cpp:"
            size_t end = msg.find(" ", start);
            if (end != std::string::npos)
            {
                std::string lineStr = msg.substr(start, end - start);
                return std::stoi(lineStr);
            }
        }
        return -1;
    };

    int firstLine = extractLineNumber(firstMessage);
    int secondLine = extractLineNumber(secondMessage);

    EXPECT_GT(firstLine, 0) << "First message: " << firstMessage;
    EXPECT_GT(secondLine, 0) << "Second message: " << secondMessage;
    EXPECT_NE(firstLine, secondLine) << "Line numbers should be different\nFirst: " << firstMessage
                                     << "\nSecond: " << secondMessage;
}

/**
 * @brief Test that exception messages include function names.
 */
TEST_F(ExceptionUtilsTest, ExceptionMessagesIncludeFunctionNames)
{
    auto throwFromLambda = []() { throwRuntimeError("Lambda error"); };

    try
    {
        throwFromLambda();
        FAIL() << "Expected exception from lambda";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // The message should include some reference to the function context
        EXPECT_NE(message.find(" in "), std::string::npos) << "Message: " << message;
        // Function name will vary by compiler, but " in " separator should be present
    }
}

/**
 * @brief Test that empty error messages are handled correctly.
 */
TEST_F(ExceptionUtilsTest, HandlesEmptyErrorMessages)
{
    try
    {
        throwRuntimeError("");
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // Even with empty message, should still have location info
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find(":"), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that special characters in error messages are preserved.
 */
TEST_F(ExceptionUtilsTest, PreservesSpecialCharactersInMessages)
{
    const std::string specialMessage = "Error: \"value\" = 100% <invalid>";

    try
    {
        throwRuntimeError(specialMessage);
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // Verify special characters are preserved
        EXPECT_NE(message.find(specialMessage), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("\"value\""), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("100%"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("<invalid>"), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that multi-line error messages are handled correctly.
 */
TEST_F(ExceptionUtilsTest, HandlesMultiLineErrorMessages)
{
    const std::string multiLineMessage = "Error on line 1\nLine 2 with details\nLine 3 with more info";

    try
    {
        throwRuntimeError(multiLineMessage);
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // Verify all lines are present
        EXPECT_NE(message.find("Error on line 1"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("Line 2 with details"), std::string::npos) << "Message: " << message;
        EXPECT_NE(message.find("Line 3 with more info"), std::string::npos) << "Message: " << message;
    }
}

/**
 * @brief Test that very long error messages are handled correctly.
 */
TEST_F(ExceptionUtilsTest, HandlesVeryLongErrorMessages)
{
    std::string longMessage(1000, 'A'); // 1000 'A' characters

    try
    {
        throwRuntimeError(longMessage);
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());

        // Verify the long message is preserved and location is still present
        EXPECT_NE(message.find(longMessage), std::string::npos);
        EXPECT_NE(message.find("ExceptionUtilsTest.cpp"), std::string::npos) << "Message length: " << message.size();
    }
}

/**
 * @brief Test exception inheritance hierarchy is preserved.
 */
TEST_F(ExceptionUtilsTest, PreservesExceptionHierarchy)
{
    // Test that exceptions can be caught by base class
    try
    {
        throwInvalidArgument("Test");
        FAIL() << "Expected exception";
    }
    catch (const std::logic_error& e)
    {
        // invalid_argument derives from logic_error
        SUCCEED() << "Caught as logic_error: " << e.what();
    }

    try
    {
        throwRuntimeError("Test");
        FAIL() << "Expected exception";
    }
    catch (const std::exception& e)
    {
        // runtime_error derives from exception
        SUCCEED() << "Caught as std::exception: " << e.what();
    }
}

/**
 * @brief Test that throwWithLocation with default template parameter works.
 */
TEST_F(ExceptionUtilsTest, ThrowWithLocationDefaultTemplateParameter)
{
    try
    {
        // Should default to std::runtime_error
        throwWithLocation("Default exception type");
        FAIL() << "Expected std::runtime_error to be thrown";
    }
    catch (const std::runtime_error& e)
    {
        std::string message(e.what());
        EXPECT_NE(message.find("Default exception type"), std::string::npos) << "Message: " << message;
        SUCCEED() << "Default template parameter works correctly";
    }
    catch (...)
    {
        FAIL() << "Should throw std::runtime_error by default";
    }
}
