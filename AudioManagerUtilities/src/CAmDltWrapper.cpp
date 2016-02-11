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

namespace am
{

CAmDltWrapper* CAmDltWrapper::mpDLTWrapper = NULL;
pthread_mutex_t CAmDltWrapper::mMutex = PTHREAD_MUTEX_INITIALIZER;

CAmDltWrapper *CAmDltWrapper::instance(const bool enableNoDLTDebug)
{
    if (!mpDLTWrapper)
        mpDLTWrapper = new CAmDltWrapper(enableNoDLTDebug);
#ifndef WITH_DLT
    if(enableNoDLTDebug)
        mpDLTWrapper->enableNoDLTDebug(true);
#endif        
    return (mpDLTWrapper);
}

void CAmDltWrapper::unregisterContext(DltContext & handle)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
    {
		dlt_unregister_context(&handle);
    }
#else
    (void) handle;
#endif
}

void CAmDltWrapper::deinit()
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
    {
		unregisterContext(mDltContext);
    }
#endif
}

CAmDltWrapper::CAmDltWrapper(const bool enableNoDLTDebug=true) :
        mDltContext(), //
        mDltContextData(), //
		mEnableNoDLTDebug(enableNoDLTDebug)
{
    (void) enableNoDLTDebug;
#ifndef WITH_DLT
    std::cout << "\e[0;34m[DLT]\e[0;30m\tRunning without DLT-support" << std::endl;
#endif
}

void CAmDltWrapper::registerApp(const char *appid, const char *description)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_register_app(appid, description);
		//register a default context
		dlt_register_context(&mDltContext, "def", "default Context registered by DLTWrapper CLass");
	}
#else
    (void) appid;
    (void) description;
#endif
}

void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_register_context(&handle, contextid, description);
	}
#else
    strncpy(handle.contextID,contextid,4);

    // store only the first contextID
    if(0 == strlen(mDltContext.contextID))
    {
        memcpy(&mDltContext.contextID,contextid,4);
        const size_t str_len = strlen(description);
        if(str_len < 2000)
        {
            mDltContextData.context_description = new char[str_len + 1];
            (void) strcpy(mDltContextData.context_description,description);
        }
        mDltContext.log_level_user = DLT_DEFAULT_LOG_LEVEL;
    }
    handle.log_level_user = DLT_DEFAULT_LOG_LEVEL;
    std::cout << "\e[0;34m[DLT]\e[0;30m\tRegistering Context " << contextid << " , " << description << std::endl;

#endif
}

void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char * description,
        const DltLogLevelType level, const DltTraceStatusType status)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_register_context_ll_ts(&handle, contextid, description, level, status);
	}
#else
    strncpy(handle.contextID,contextid,4);

    // store only the first contextID
    if(0 == strlen(mDltContext.contextID))
    {
        memcpy(&mDltContext.contextID,contextid,4);
        const size_t str_len = strlen(description);
        if(str_len < 2000)
        {
            mDltContextData.context_description = new char[str_len + 1];
            (void) strcpy(mDltContextData.context_description,description);
        }
        mDltContext.log_level_user = level;
    }
    handle.log_level_user = level;
    std::cout << "\e[0;34m[DLT]\e[0;30m\tRegistering Context " << contextid << " , " << description << std::endl;

#endif
}

bool CAmDltWrapper::init(DltLogLevelType loglevel, DltContext* context)
{
    (void) loglevel;
    pthread_mutex_lock(&mMutex);
    if (!context)
        context = &mDltContext;
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
		if(dlt_user_log_write_start(context, &mDltContextData, loglevel) <= 0)
#else
    if((mEnableNoDLTDebug == false) || (loglevel > context->log_level_user))
#endif
		{
			pthread_mutex_unlock(&mMutex);
			return false;
		}
#ifndef WITH_DLT
    std::cout << "\e[0;34m[" << context->contextID << "]\e[0;30m\t";
#endif
    return true;
}

void CAmDltWrapper::send()
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_finish(&mDltContextData);
	}
#else
    if(mEnableNoDLTDebug)
        std::cout << mDltContextData.buffer.str().c_str() << std::endl;

    mDltContextData.buffer.str("");
    mDltContextData.buffer.clear();
#endif
    pthread_mutex_unlock(&mMutex);
}

void CAmDltWrapper::append(const int8_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_int8(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint8_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_uint8(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int16_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_int16(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint16_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_uint16(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int32_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_int32(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint32_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_uint32(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const std::string& value)
{
	if (mEnableNoDLTDebug)
		append(value.c_str());
}

void CAmDltWrapper::append(const bool value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int64_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_int64(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint64_t value)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_uint64(&mDltContextData, value);
	}
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const std::vector<uint8_t> & data)
{
#ifdef WITH_DLT
	if (mEnableNoDLTDebug)
	{
		dlt_user_log_write_raw(&mDltContextData,(void*)data.data(),data.size());
	}
#else
	mDltContextData.buffer << data.data();
#endif
}

#ifndef WITH_DLT
template<class T> void CAmDltWrapper::appendNoDLT(T value)
{
    mDltContextData.buffer << value;
}

void CAmDltWrapper::enableNoDLTDebug(const bool enableNoDLTDebug)
{
    mEnableNoDLTDebug = enableNoDLTDebug;
}
#endif

CAmDltWrapper::~CAmDltWrapper()
{
    if (mpDLTWrapper && mEnableNoDLTDebug)
    {
        mpDLTWrapper->unregisterContext(mDltContext);
        delete mpDLTWrapper;
    }
}
}


