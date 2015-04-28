/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmLog.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmLog.h"


void CAmLog::CAmFileLogger::generateLogFilename(std::string &result)
{
	static uint32_t logFileID = 1;
	time_t rawtime;
	time (&rawtime);

	std::ostringstream stream;
	stream << DEFAULT_LOG_FOLDER << DEFAULT_LOGFILE_PREFIX << logFileID << "_" << rawtime << DEFAULT_LOGFILE_EXT;
	logFileID++;
	result =  stream.str();
}

CAmLog::CAmFileLogger::~CAmFileLogger()
{
	if (mOutputStream)
	{
		std::ofstream* of = static_cast<std::ofstream*>(mOutputStream);
		of->close();
		DEL(mOutputStream)
	}
}

CAmLog::CAmLog(const eCAmLogType type ):mLogType(type)
{
	instantiateLogger(type);
}

CAmLog::CAmLog():mLogType(eCAmLogStdout)
{
	instantiateLogger((const eCAmLogType)eCAmLogStdout);
}

CAmLog::~CAmLog()
{
	releaseLogger();
}

void CAmLog::releaseLogger()
{
	if(mLogger)
		DEL(mLogger)
}

void CAmLog::instantiateLogger( const eCAmLogType type)
{
	if( eCAmLogStdout == type )
		mLogger = new CAmStdOutLogger();
	else if( eCAmLogFile == type )
	{
		std::string filename("");
		CAmLog::CAmFileLogger::generateLogFilename(filename);
		mLogger = new CAmFileLogger(filename);
	}
}

CAmLog *CAmLog::getDefaultLog()
{
	static CAmLog theInstance;
	return &theInstance;
}

void CAmLog::setLogType( const eCAmLogType type)
{
	if(mLogType!=type)
	{
		mLogType = type;
		releaseLogger();
		instantiateLogger(type);
	}
}

eCAmLogType CAmLog::getLogType() const
{
	return mLogType;
}
