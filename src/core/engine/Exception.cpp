/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "Exception.h"
#include <sstream>

namespace pkt::core
{

using namespace std;

Exception::Exception(const char* sourcefile, int sourceline, EngineError errorId, int osErrorCode)
    : m_errorId(errorId), m_osErrorCode(osErrorCode)
{
    ostringstream msgStream;
    msgStream << sourcefile << " (" << sourceline << "): Error " << static_cast<const int>(errorId);
    if (osErrorCode)
        msgStream << " (system error " << osErrorCode << ")";
    m_msg = msgStream.str();
}

Exception::Exception(const char* sourcefile, int sourceline, EngineError errorId) : m_errorId(errorId)
{
    ostringstream msgStream;
    msgStream << sourcefile << " (" << sourceline << "): Error " << static_cast<const int>(errorId);
    m_msg = msgStream.str();
}

Exception::~Exception() throw()
{
}

const char* Exception::what() const throw()
{
    return m_msg.c_str();
}
} // namespace pkt::core
