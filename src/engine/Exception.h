/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
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

#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <exception>
#include <string>

class Exception : public std::exception
{
public:

	Exception(const char *sourcefile, int sourceline, int errorId, int osErrorCode);
	Exception(const char *sourcefile, int sourceline, int errorId);

	virtual ~Exception() throw();

	int GetErrorId() const {
		return m_errorId;
	}
	int GetOsErrorCode() const {
		return m_osErrorCode;
	}

	virtual const char *what() const throw();

private:
	int m_errorId;
	int m_osErrorCode;

	std::string m_msg;
};

#endif
