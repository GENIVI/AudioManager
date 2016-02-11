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
 * \file CAmLog.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef CAMLOG_H_
#define CAMLOG_H_

#include <iostream>
#include <iosfwd>
#include <stdio.h>
#include <stdexcept>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <assert.h>

/**
 * Implements a basic logging mechanism that can be used to print debug information into a file or to the console.
 * It can be used either as singleton through the appropriate method getDefaultLog() or as independent instantiated object.
 * The default initializer sets the console as output for newly created objects.
 * Example: CAmLogger << "Text"; //to print out through the singleton object directly to the console
 */

#define DEFAULT_LOG_FOLDER 		"/tmp/"
#define DEFAULT_LOGFILE_PREFIX "am_dump_"
#define DEFAULT_LOGFILE_EXT 	".log"

#define DEL( aPointer ) delete aPointer, aPointer = NULL;

/* */
typedef enum { eCAmLogNone = 0, eCAmLogStdout = 1, eCAmLogFile = 2 } eCAmLogType;

class CAmLog
{
private:
	/**
	 * Private classes which usually own (wrap) a stream object. They are responsible for creating and deleting it.
	 */
	class CAmLogger
	{
	protected:
		std::ostream* mOutputStream;
	public:
		CAmLogger ():mOutputStream(NULL) {};
		virtual ~CAmLogger () { };
		virtual void log(const std::string& _s)
	    {
	        (*mOutputStream) << _s;
	        mOutputStream->flush();
	    }
	    template <class T>
	    CAmLogger & operator << (const T & t)
	    {
	    	(*mOutputStream) << t;
	    	return (*this);
	    }
	};

	class CAmFileLogger : public CAmLogger
	{
		std::string mFilename;
	public:
		static void generateLogFilename(std::string &result);
		explicit CAmFileLogger(const std::string& _s) : CAmLogger()
		{
			mFilename = _s;
			mOutputStream = new std::ofstream(mFilename.c_str());
		}
		~CAmFileLogger();
	};

	class CAmStdOutLogger : public CAmLogger
	{
	public:
		CAmStdOutLogger()
		{
			mOutputStream = &std::cout;
		}
	};

private:
	eCAmLogType mLogType;
    CAmLogger* mLogger;

protected:
    void releaseLogger();
    void instantiateLogger( const eCAmLogType type);
public:
    CAmLog(const eCAmLogType type );
    CAmLog();
    ~CAmLog();

    static CAmLog *getDefaultLog();

    void setLogType( const eCAmLogType type);
    eCAmLogType getLogType() const;

    template <class T>
    CAmLog & operator << (const T & t)
    {
    	assert(mLogger!=NULL);
    	(*mLogger) << t;
    	return (*this);
    }
 };

#define CAmLogger (*CAmLog::getDefaultLog())


#endif /* CAMLOG_H_ */
