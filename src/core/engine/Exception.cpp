// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

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
