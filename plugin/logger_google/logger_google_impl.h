/*
	Copyright (c) 2016 Tencent.  See the AUTHORS file for names 
	of contributors.
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
	
*/

#pragma once

#include "phxpaxos/log.h"
#include "phxpaxos_plugin/logger_google.h"
#include <string>

namespace phxpaxos
{

class LoggerGoogleImpl : public LoggerGoogle
{
public:
	LoggerGoogleImpl(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel);
	~LoggerGoogleImpl();

	void LogError(const char * pcFormat, ...);

	void LogWarning(const char * pcFormat, ...);
	
	void LogInfo(const char * pcFormat, ...);
	
	void LogVerbose(const char * pcFormat, ...);
};
	
}
