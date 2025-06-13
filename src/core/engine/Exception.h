/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

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

    int GetErrorId() const { return static_cast<const int>(m_errorId); }
    int GetOsErrorCode() const { return m_osErrorCode; }

    virtual const char* what() const throw();

  private:
    EngineError m_errorId;
    int m_osErrorCode;

    std::string m_msg;
};
} // namespace pkt::core
