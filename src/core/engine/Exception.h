// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

/* Exception class for engine errors. */

#pragma once

#include "model/EngineError.h"

#include <exception>
#include <string>

namespace pkt::core
{

class Exception : public std::exception
{
  public:
    Exception(const char* sourcefile, int sourceline, EngineError error, int osErrorCode);
    Exception(const char* sourcefile, int sourceline, EngineError error);

    virtual ~Exception() throw();

    int getErrorId() const { return static_cast<const int>(myErrorId); }
    int getOsErrorCode() const { return myOsErrorCode; }

    virtual const char* what() const throw();

  private:
    EngineError myErrorId;
    int myOsErrorCode;

    std::string myMsg;
};
} // namespace pkt::core
