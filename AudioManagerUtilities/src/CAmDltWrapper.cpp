/**
 *  SPDX license identifier: MPL-2.0
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 *
 * \file CAmDltWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */


#include "CAmDltWrapper.h"
#include <string>
#include <iostream>
#include <string.h>
#include <chrono>
#include <ctime>
#include <unistd.h>

namespace am
{
CAmDltWrapper* CAmDltWrapper::mpDLTWrapper = NULL;
pthread_mutex_t CAmDltWrapper::mMutex = PTHREAD_MUTEX_INITIALIZER;

const std::vector<const char*> CAmDltWrapper::mStr_error = 
{
		"E_OK",
		"E_UNKNOWN",
		"E_OUT_OF_RANGE",
		"E_NOT_USED",
		"E_DATABASE_ERROR",
		"E_ALREADY_EXISTS",
		"E_NO_CHANGE",
		"E_NOT_POSSIBLE",
		"E_NON_EXISTENT",
		"E_ABORTED",
		"E_WRONG_FORMAT",
		"E_COMMUNICATION",
		"E_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_sourceState = 
{
		"SS_UNKNNOWN",
		"SS_ON",
		"SS_OFF",
		"SS_PAUSED",
		"SS_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_MuteState = 
{
		"MS_UNKNOWN" ,
		"MS_MUTED" ,
		"MS_UNMUTED" ,
		"MS_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_DomainState = 
{
		"DS_UNKNOWN",
		"DS_CONTROLLED",
		"DS_INDEPENDENT_STARTUP",
		"DS_INDEPENDENT_RUNDOWN",
		"DS_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_ConnectionState = 
{
		"CS_UNKNOWN",
		"CS_CONNECTING",
		"CS_CONNECTED",
		"CS_DISCONNECTING",
		"CS_DISCONNECTED",
		"CS_SUSPENDED",
		"CS_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_Availability = 
{
		"A_UNKNOWN",
		"A_AVAILABLE",
		"A_UNAVAILABLE",
		"A_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_Interrupt = 
{
		"IS_UNKNOWN",
		"IS_OFF",
		"IS_INTERRUPTED",
		"IS_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_Handle = 
{
		"H_UNKNOWN",
		"H_CONNECT",
		"H_DISCONNECT",
		"H_SETSOURCESTATE",
		"H_SETSINKVOLUME",
		"H_SETSOURCEVOLUME",
		"H_SETSINKSOUNDPROPERTY",
		"H_SETSOURCESOUNDPROPERTY",
		"H_SETSINKSOUNDPROPERTIES",
		"H_SETSOURCESOUNDPROPERTIES",
		"H_CROSSFADE",
		"H_SETVOLUMES",
		"H_SETSINKNOTIFICATION",
		"H_SETSOURCENOTIFICATION",
		"H_MAX"
};

const std::vector<const char*> CAmDltWrapper::mStr_NotificationStatus = 
{
		"NS_UNKNOWN",
		"NS_OFF",
		"NS_PERIODIC",
		"NS_MINIMUM",
		"NS_MAXIMUM",
		"NS_CHANGE",
		"NS_MAX"
};



std::string CAmDltWrapper::now()
{
	std::time_t t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	struct tm * timeinfo(localtime(&t));
	char buffer[80];
	std::strftime(buffer,80,"%D %T ",timeinfo);
	return (std::string(buffer));
}

CAmDltWrapper* CAmDltWrapper::instanctiateOnce(const char *appid, const char * description, const bool debugEnabled, const logDestination logDest, const std::string Filename,bool onlyError)
{
	if (!mpDLTWrapper)
	{
		mpDLTWrapper = new CAmDltWrapper(appid,description,debugEnabled,logDest,Filename,onlyError);
	}   
	return (mpDLTWrapper);
}

CAmDltWrapper* CAmDltWrapper::instance()
{
	if (!mpDLTWrapper)
	{
		// an application seems not to use our CAmDltWrapper class therefore create default
		std::ostringstream description;
		description << "PID=" << getpid() << " _=" << getenv("_");
		mpDLTWrapper = new CAmDltWrapper("AMDL", description.str().c_str());
		std::cerr << "Application doesn't call CAmDltWrapper::instanciateOnce!!!" << std::endl;
		std::cerr << "-> CAmDltWrapper::instance registers DLT application [ AMDL | " << description.str() << " ]" << std::endl;
	}
	return mpDLTWrapper;
}

bool CAmDltWrapper::getEnabled()
{
	return (mDebugEnabled);
}

bool CAmDltWrapper::initNoDlt(DltLogLevelType loglevel, DltContext* context)
{
	if (mlogDestination==logDestination::COMMAND_LINE)
	{
		if (!context)
		{
			switch (loglevel)
			{
				case DLT_LOG_OFF :
				case DLT_LOG_FATAL : 
				case DLT_LOG_ERROR :
					mNoDltContextData.buffer << "\033[0;31m"<<"[DEF] [Erro] \033[0m";
					mLogOn=true;
					break;
				case DLT_LOG_WARN :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;33m"<<"[DEF] [Warn] \033[0m";	
					}
					else
						mLogOn=false;
					break;
				case DLT_LOG_INFO :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;36m"<<"[DEF] [Info] \033[0m";
					}
					else
						mLogOn=false;
					break;	
				default:
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;32m"<<"[DEF] [Defa] \033[0m";	
					}
					else
						mLogOn=false;
			}
		}
		else
		{
			std::string con(mMapContext.at(context));
			switch (loglevel)
			{
				case DLT_LOG_OFF :
				case DLT_LOG_FATAL : 
				case DLT_LOG_ERROR :
					mNoDltContextData.buffer << "\033[0;31m["<<con<<"] [Erro] \033[0m";
					mLogOn=true;
					break;
				case DLT_LOG_WARN :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;33m["<<con<<"] [Warn] \033[0m";	
					}
					else
						mLogOn=false;
					break;
				case DLT_LOG_INFO :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;36m["<<con<<"]  [Info] \033[0m";						
					}
					else
						mLogOn=false;

					break;	
				default:
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "\033[0;32m["<<con<<"]  [Defa] \033[0m";	
					}
					else
						mLogOn=false;	
			}
		}
		return true;
	}
	else
	{
		if (!context)
		{
			switch (loglevel)
			{
				case DLT_LOG_OFF :
				case DLT_LOG_FATAL : 
				case DLT_LOG_ERROR :
					mNoDltContextData.buffer <<"[DEF] [Erro] ";
					mLogOn=true;
					 break;
				case DLT_LOG_WARN :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer <<"[DEF] [Warn] ";	
					}
					else
						mLogOn=false;
					break;
				case DLT_LOG_INFO :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer <<"[DEF] [Info] ";
					}
					else
						mLogOn=false;					
					break;	
				default:
					if (!mOnlyError)
					{
						mNoDltContextData.buffer <<"[DEF] [Defa] ";						
					}
					else
						mLogOn=false;					

			}
		}
		else
		{
			std::string con(mMapContext.at(context));
			switch (loglevel)
			{
				case DLT_LOG_OFF :
				case DLT_LOG_FATAL : 
				case DLT_LOG_ERROR :
					mNoDltContextData.buffer << "["<<con<<"] [Erro] ";
					mLogOn=true;
					 break;
				case DLT_LOG_WARN :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "["<<con<<"] [Warn] ";								
					}
					else
						mLogOn=false;							
					break;
				case DLT_LOG_INFO :
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "["<<con<<"] [Info] ";												
					}
					else
						mLogOn=false;						

					break;	
				default:
					if (!mOnlyError)
					{
						mNoDltContextData.buffer << "["<<con<<"] [Defa] ";														
					}
					else
						mLogOn=false;					
			}
		}
		return true;		
	}
}
	
#ifdef WITH_DLT	

	CAmDltWrapper::CAmDltWrapper(const char *appid, const char * description, const bool debugEnabled, const logDestination logDest, const std::string Filename,bool onlyError) :
		mDebugEnabled(debugEnabled), //
		mlogDestination(logDest), //
		mFilename(NULL), //
		mOnlyError(onlyError), //
		mLogOn(true)
	{
		if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
		{
			dlt_register_app(appid, description);
			//register a default context
			dlt_register_context(&mDltContext, "DEF", "Default Context registered by DLTWrapper Class");	
		}
		else if (mDebugEnabled)
		{
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering AppID " << appid << " , " << description << "\033[0m"<< std::endl;
			else
			{
				mFilename.open(Filename, std::ofstream::out | std::ofstream::trunc);
				if (!mFilename.is_open())
				{
					throw std::runtime_error("Cannot open file for logging");
				}
				mFilename << now() << "[DLT] Registering AppID " << appid << " , " << description << std::endl;
			}
		}
	}
	
	CAmDltWrapper::~CAmDltWrapper()
	{
		if (mpDLTWrapper && mDebugEnabled && mlogDestination==logDestination::DAEMON)
		{
			mpDLTWrapper->unregisterContext(mDltContext);
			delete mpDLTWrapper;
		}
		else if (mpDLTWrapper && mDebugEnabled && mlogDestination==logDestination::COMMAND_LINE)
		{
			mFilename.close();
		}
	}

	void CAmDltWrapper::unregisterContext(DltContext & handle)
	{
		if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
		{
			dlt_unregister_context(&handle);
		}
	}

	void CAmDltWrapper::deinit()
	{
		if (mDebugEnabled)
		{
			unregisterContext(mDltContext);
		}
	}
	
	void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
	{
		if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
		{
			dlt_register_context(&handle, contextid, description);
		}
		else if (mDebugEnabled)
		{
			mMapContext.emplace(&handle,std::string(contextid));
			
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
			else
				mFilename << now() << "[DLT] Registering Context " << contextid << " , " << description << std::endl;
		}
	}
	
	void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char * description,const DltLogLevelType level, const DltTraceStatusType status)
	{
		if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
		{
			dlt_register_context_ll_ts(&handle, contextid, description, level, status);
		}
		else if (mDebugEnabled)
		{
			mMapContext.emplace(&handle,std::string(contextid));
			
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
			else
				mFilename << now() << " [DLT] Registering Context " << contextid << " , " << description << std::endl;
		}
	}
	
	bool CAmDltWrapper::init(DltLogLevelType loglevel, DltContext* context)
	{
		pthread_mutex_lock(&mMutex);
		if (mlogDestination==logDestination::DAEMON)
		{
			if (!context)
				context = &mDltContext;

			if(dlt_user_log_write_start(context, &mDltContextData, loglevel) <= 0)
			{
				pthread_mutex_unlock(&mMutex);
				return false;
			}
		}
		else 
		{
			initNoDlt(loglevel,context);
		}
		return true;
	}

	void CAmDltWrapper::send()
	{
		if (mlogDestination==logDestination::DAEMON)
		{
			dlt_user_log_write_finish(&mDltContextData);
		}
		else
		{
			if (mlogDestination==logDestination::COMMAND_LINE && mLogOn)
				std::cout << mNoDltContextData.buffer.str().c_str() << std::endl;
			else if (mLogOn)
				mFilename << now() << mNoDltContextData.buffer.str().c_str() << std::endl;	
					
			mNoDltContextData.buffer.str("");
			mNoDltContextData.buffer.clear();
		}
		pthread_mutex_unlock(&mMutex);
	}
	
	void CAmDltWrapper::append(const int8_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_int8(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const uint8_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_uint8(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const int16_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_int16(&mDltContextData, value);
		else
			appendNoDLT(value);			
	}

	void CAmDltWrapper::append(const uint16_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_uint16(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const int32_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_int32(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const uint32_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_uint32(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const std::string& value)
	{
		append(value.c_str());
	}

	void CAmDltWrapper::append(const bool value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const int64_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_int64(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const uint64_t value)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_uint64(&mDltContextData, value);
		else
			appendNoDLT(value);
	}

	void CAmDltWrapper::append(const std::vector<uint8_t> & data)
	{
		if (mlogDestination==logDestination::DAEMON)
			dlt_user_log_write_raw(&mDltContextData,(void*)data.data(),data.size());
		else
			mNoDltContextData.buffer << data.data();
	}
}

#else //------------------------------------------------------------------------------------------------- no DLT !

	CAmDltWrapper::CAmDltWrapper(const char *appid, const char * description, const bool debugEnabled, const logDestination logDest, const std::string Filename,bool onlyError) :
		mDebugEnabled(debugEnabled), //
		mlogDestination(logDest), //
		mFilename(NULL), //
		mOnlyError(onlyError), //
		mLogOn(true)
	{
		if (logDest==logDestination::DAEMON)
		{
			std::cout << "\033[0;31m[DLT] Cannot Use Daemon Logging, active in CMAKE! Using CommandLine\033[0m"<< std::endl;	
			mlogDestination=logDestination::COMMAND_LINE;
		}
		if (mDebugEnabled)
		{
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering AppID " << appid << " , " << description << "\033[0m"<< std::endl;
			else
			{
				mFilename.open(Filename, std::ofstream::out | std::ofstream::trunc);
				if (!mFilename.is_open())
				{
					throw std::runtime_error("Cannot open file for logging");
				}
				mFilename << now() << "[DLT] Registering AppID " << appid << " , " << description << std::endl;
			}
		}
	}
	
	CAmDltWrapper::~CAmDltWrapper()
	{
		if (mpDLTWrapper && mDebugEnabled && mlogDestination==logDestination::COMMAND_LINE)
		{
			mFilename.close();
		}
	}
	
	void CAmDltWrapper::unregisterContext(DltContext & handle)
	{}

	void CAmDltWrapper::deinit()
	{}
	
	void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
	{
		if (mDebugEnabled)
		{
			mMapContext.emplace(&handle,std::string(contextid));
			
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
			else
				mFilename << now() << "[DLT] Registering Context " << contextid << " , " << description << std::endl;
		}
	}
	
	void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char * description,const DltLogLevelType level, const DltTraceStatusType status)
	{
		if (mDebugEnabled)
		{
			mMapContext.emplace(&handle,std::string(contextid));
			
			if (mlogDestination==logDestination::COMMAND_LINE)
				std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
			else
				mFilename << now() << " [DLT] Registering Context " << contextid << " , " << description << std::endl;
		}
	}
	
	bool CAmDltWrapper::init(DltLogLevelType loglevel, DltContext* context)
	{
		pthread_mutex_lock(&mMutex);
		initNoDlt(loglevel,context);
	}
	
	void CAmDltWrapper::send()
	{
		if (mlogDestination==logDestination::COMMAND_LINE && mLogOn)
			std::cout << mNoDltContextData.buffer.str().c_str() << std::endl;
		else if (mLogOn)
			mFilename << now() << mNoDltContextData.buffer.str().c_str() << std::endl;	
				
		mNoDltContextData.buffer.str("");
		mNoDltContextData.buffer.clear();
		pthread_mutex_unlock(&mMutex);
	}
	
	void CAmDltWrapper::append(const int8_t value)
	{
		appendNoDLT(value);
	}

	void CAmDltWrapper::append(const uint8_t value)
	{
		appendNoDLT(value);
	}

	void CAmDltWrapper::append(const int16_t value)
	{
		appendNoDLT(value);			
	}

	void CAmDltWrapper::append(const uint16_t value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const int32_t value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const uint32_t value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const std::string& value)
	{
		append(value.c_str());
	}

	void CAmDltWrapper::append(const bool value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const int64_t value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const uint64_t value)
	{
		appendNoDLT(value);	
	}

	void CAmDltWrapper::append(const std::vector<uint8_t> & data)
	{
		mNoDltContextData.buffer << data.data();
	}
}
#endif //WITH_DLT


