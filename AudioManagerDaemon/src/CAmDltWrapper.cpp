/**
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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmDltWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */


#include "shared/CAmDltWrapper.h"
#include <string>
#include <sstream>
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
    dlt_unregister_context(&handle);
#else
    (void) handle;
#endif
}

void CAmDltWrapper::deinit()
{
#ifdef WITH_DLT
    unregisterContext(mDltContext);
#endif
}

CAmDltWrapper::CAmDltWrapper(const bool enableNoDLTDebug) :
#ifndef WITH_DLT
        mEnableNoDLTDebug(enableNoDLTDebug),
#endif
        mDltContext(), //
        mDltContextData()
{
    (void) enableNoDLTDebug;
#ifndef WITH_DLT
    std::cout << "\e[0;34m[DLT]\e[0;30m\tRunning without DLT-support" << std::endl;
#endif
}

void CAmDltWrapper::registerApp(const char *appid, const char *description)
{
#ifdef WITH_DLT
    dlt_register_app(appid, description);
    //register a default context
    dlt_register_context(&mDltContext, "def", "default Context registered by DLTWrapper CLass");
#else
    (void) appid;
    (void) description;
#endif
}

void CAmDltWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
{
#ifdef WITH_DLT
    dlt_register_context(&handle, contextid, description);
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
    }

    std::cout << "\e[0;34m[DLT]\e[0;30m\tRegistering Context " << contextid << " , " << description << std::endl;

#endif
}

void CAmDltWrapper::init(DltLogLevelType loglevel, DltContext* context)
{
    (void) loglevel;
    pthread_mutex_lock(&mMutex);
    if (!context)
        context = &mDltContext;
#ifdef WITH_DLT
    dlt_user_log_write_start(context, &mDltContextData, loglevel);
#else
    if(mEnableNoDLTDebug)
        std::cout << "\e[0;34m[" << context->contextID << "]\e[0;30m\t";
#endif

}

void CAmDltWrapper::send()
{
#ifdef WITH_DLT
    dlt_user_log_write_finish(&mDltContextData);
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
    dlt_user_log_write_int8(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint8_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint8(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int16_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int16(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint16_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint16(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int32_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int32(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint32_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint32(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const char*& value)
{
#ifdef WITH_DLT
    dlt_user_log_write_string(&mDltContextData, value);
#else
    mDltContextData.buffer << value;
#endif
}

void CAmDltWrapper::append(const std::string& value)
{
#ifdef WITH_DLT
    dlt_user_log_write_string(&mDltContextData, value.c_str());
#else
    mDltContextData.buffer << value;
#endif
}

void CAmDltWrapper::append(const bool value)
{
#ifdef WITH_DLT
    dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const int64_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int64(&mDltContextData, value);
#else
    appendNoDLT(value);
#endif
}

void CAmDltWrapper::append(const uint64_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint64(&mDltContextData, value);
#else
    appendNoDLT(value);
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
    if (mpDLTWrapper)
    {
        mpDLTWrapper->unregisterContext(mDltContext);
        delete mpDLTWrapper;
    }
}
}

