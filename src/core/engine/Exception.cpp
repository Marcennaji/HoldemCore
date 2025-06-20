// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Exception.h"
#include <sstream>

namespace pkt::core
{

using namespace std;

Exception::Exception(const char* sourcefile, int sourceline, EngineError errorId, int osErrorCode)
    : myErrorId(errorId), myOsErrorCode(osErrorCode)
{
    ostringstream msgStream;
    msgStream << sourcefile << " (" << sourceline << "): Error " << static_cast<const int>(errorId);
    if (osErrorCode)
    {
        msgStream << " (system error " << osErrorCode << ")";
    }
    myMsg = msgStream.str();
}

Exception::Exception(const char* sourcefile, int sourceline, EngineError errorId) : myErrorId(errorId)
{
    ostringstream msgStream;
    msgStream << sourcefile << " (" << sourceline << "): Error " << static_cast<const int>(errorId);
    myMsg = msgStream.str();
}

Exception::~Exception() throw() = default;

const char* Exception::what() const throw()
{
    return myMsg.c_str();
}
} // namespace pkt::core
