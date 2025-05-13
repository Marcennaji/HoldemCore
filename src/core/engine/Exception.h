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

/* Exception class for engine errors. */

#pragma once

#include "EngineError.h"

#include <exception>
#include <string>

class Exception : public std::exception
{
public:

	Exception(const char *sourcefile, int sourceline, EngineError error, int osErrorCode);
	Exception(const char *sourcefile, int sourceline, EngineError error);

	virtual ~Exception() throw();

	int GetErrorId() const {
		return static_cast<const int>(m_errorId);
	}
	int GetOsErrorCode() const {
		return m_osErrorCode;
	}

	virtual const char *what() const throw();

private:
	EngineError m_errorId;
	int m_osErrorCode;

	std::string m_msg;
};

