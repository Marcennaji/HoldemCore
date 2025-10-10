// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "model/EngineError.h"

#include <exception>
#include <string>

namespace pkt::core
{

/**
 * @brief Exception class for poker engine errors.
 * 
 * This class provides detailed error reporting for engine-related failures,
 * including source file location, error codes, and descriptive messages.
 * Used throughout the poker engine to handle exceptional conditions.
 */
class Exception : public std::exception
{
  public:
    Exception(const char* sourcefile, int sourceline, EngineError error, int osErrorCode);
    Exception(const char* sourcefile, int sourceline, EngineError error);

    virtual ~Exception() throw();

    int getErrorId() const { return static_cast<const int>(m_errorId); }
    int getOsErrorCode() const { return m_osErrorCode; }

    virtual const char* what() const throw();

  private:
    EngineError m_errorId;
    int m_osErrorCode;

    std::string m_msg;
};
} // namespace pkt::core
