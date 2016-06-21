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

#include "logger_google_impl.h"
#include <stdarg.h>

#include <glog/logging.h>
#include <stdio.h>
#include <string>

using namespace std;

namespace phxpaxos
{

LoggerGoogleImpl :: LoggerGoogleImpl(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel)
{
    google::InitGoogleLogging(sModuleName.c_str());
    FLAGS_log_dir = sLogPath;
    FLAGS_stderrthreshold = google :: FATAL;
    switch( iLogLevel )
    {
        case 1: 
            FLAGS_minloglevel = google::ERROR;
            break;  
        case 2: 
            FLAGS_minloglevel = google::WARNING;
            break;  
        case 3: 
            FLAGS_minloglevel = google::INFO;
            break;  
    }

	LogError("%s", "init_glog_warning_file");
	LogWarning("%s", "init_glog_error_file");
	LogInfo("%s", "init_glog_error_file");
	LogVerbose("%s", "init_glog_info_file");
}

LoggerGoogleImpl :: ~LoggerGoogleImpl()
{
}

void LoggerGoogleImpl :: LogError(const char * pcFormat, ...)
{
	char sBuf[1024] = {0};
	string newFormat = "\033[41;37m " + string(pcFormat) + " \033[0m";
	va_list args;
	va_start(args, pcFormat);
	vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
	va_end(args);

	LOG(ERROR) << sBuf;
}

void LoggerGoogleImpl :: LogWarning(const char * pcFormat, ...)
{
	char sBuf[1024] = {0};
	string newFormat = "\033[44;37m " + string(pcFormat) + " \033[0m";
	va_list args;
	va_start(args, pcFormat);
	vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
	va_end(args);

	LOG(WARNING) << sBuf;
}

void LoggerGoogleImpl :: LogInfo(const char * pcFormat, ...)
{
	char sBuf[1024] = {0};
	string newFormat = "\033[45;37m " + string(pcFormat) + " \033[0m";
	va_list args;
	va_start(args, pcFormat);
	vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
	va_end(args);

	LOG(INFO) << sBuf;
}

void LoggerGoogleImpl :: LogVerbose(const char * pcFormat, ...)
{
	char sBuf[1024] = {0};
	va_list args;
	va_start(args, pcFormat);
	vsnprintf(sBuf, sizeof(sBuf), pcFormat, args);
	va_end(args);

	LOG(INFO) << sBuf;
}

}


