/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, ADIT GmbH
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
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2017
 * \author Mattia Guerra, mguerra@de.adit-jv.com ADIT 2017
 * \author Martin Koch, mkoch@de.adit-jv.com ADIT 2020
 *
 * \file CAmLoggerDlt.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmLogWrapper.h"
#include "CAmLoggerDlt.h"

namespace am
{

pthread_mutex_t gDltMtx = PTHREAD_MUTEX_INITIALIZER;

CAmLogContextDlt::CAmLogContextDlt(const char *id)
    : mId(id)
{
}

DltContext *CAmLogContextDlt::getHandle()
{
    return &mDltContext;
}

void CAmLogContextDlt::append(const int8_t value)
{
    dlt_user_log_write_int8(&mDltContextData, value);
}

void CAmLogContextDlt::append(const uint8_t value)
{
    dlt_user_log_write_uint8(&mDltContextData, value);
}

void CAmLogContextDlt::append(const int16_t value)
{
    dlt_user_log_write_int16(&mDltContextData, value);
}

void CAmLogContextDlt::append(const uint16_t value)
{
    dlt_user_log_write_uint16(&mDltContextData, value);
}

void CAmLogContextDlt::append(const int32_t value)
{
    dlt_user_log_write_int32(&mDltContextData, value);
}

void CAmLogContextDlt::append(const uint32_t value)
{
    dlt_user_log_write_uint32(&mDltContextData, value);
}

void CAmLogContextDlt::append(const bool value)
{
    dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
}

void CAmLogContextDlt::append(const int64_t value)
{
    dlt_user_log_write_int64(&mDltContextData, value);
}

void CAmLogContextDlt::append(const uint64_t value)
{
    dlt_user_log_write_uint64(&mDltContextData, value);
}

void CAmLogContextDlt::append(const std::vector<uint8_t> &data)
{
    dlt_user_log_write_raw(&mDltContextData, (void *)data.data(), data.size());
}

void CAmLogContextDlt::append(const char *value)
{
    dlt_user_log_write_string(&mDltContextData, value);
}

bool CAmLogContextDlt::checkLogLevel(const am_LogLevel_e logLevel)
{
#ifdef DLT_IS_LOG_LEVEL_ENABLED
    return (dlt_user_is_logLevel_enabled(&mDltContext, static_cast<DltLogLevelType>(logLevel)) == DLT_RETURN_TRUE);
#else
    (void)logLevel;
    return true;
#endif
}

bool CAmLogContextDlt::configure(const am_LogLevel_e loglevel)
{
    pthread_mutex_lock(&gDltMtx);

    /* leave in case we are allowed to send messages */
    if (dlt_user_log_write_start(&mDltContext, &mDltContextData, static_cast<DltLogLevelType>(loglevel)) > 0)
    {
        return true;
    }

    pthread_mutex_unlock(&gDltMtx);
    return false;
}

void CAmLogContextDlt::send()
{
    dlt_user_log_write_finish(&mDltContextData);
    pthread_mutex_unlock(&gDltMtx);
}

CAmLoggerDlt::~CAmLoggerDlt()
{
    unregisterApp();
}

void CAmLoggerDlt::registerApp(const char *appid, const char *description)
{
    dlt_register_app(appid, description);
    registerContext(DEFAULT_CONTEXT, DEFAULT_DESCRIPTION);
}

void CAmLoggerDlt::unregisterApp()
{
    for (auto &&context : mCtxTable)
    {
        dlt_unregister_context(context.second->getHandle());
        delete context.second;
        delete context.first;
    }
    mCtxTable.clear();

    dlt_unregister_app();
}

IAmLogContext &CAmLoggerDlt::registerContext(const char *contextid, const char *description)
{
    auto &&context = createContext(contextid);
    dlt_register_context(context.getHandle(), contextid, description);
    return context;
}

IAmLogContext &CAmLoggerDlt::registerContext(const char *contextid, const char *description, const am_LogLevel_e level, const am_LogStatus_e status)
{
    auto &&context = createContext(contextid);
    dlt_register_context_ll_ts(context.getHandle(), contextid, description, static_cast<DltLogLevelType>(level), static_cast<DltTraceStatusType>(status));
    return context;
}

IAmLogContext &CAmLoggerDlt::importContext(const char *contextid)
{
    // check, if we have this context
    contextid = (contextid ? contextid : DEFAULT_CONTEXT);
    for (auto &ctx : mCtxTable)
    {
        if (contextid && strncmp(ctx.first, contextid, DLT_ID_SIZE) == 0)
        {
            return *ctx.second;
        }
    }

    // no match. Fall back to default context
    return importContext(DEFAULT_CONTEXT);
}

void CAmLoggerDlt::unregisterContext(const char *contextid)
{
    for (auto it = mCtxTable.begin(); it != mCtxTable.end(); ++it)
    {
        if (contextid && strncmp(contextid, it->first, DLT_ID_SIZE) == 0)
        {
            dlt_unregister_context(it->second->getHandle());
            delete it->second;

            const char *pKey = it->first;
            mCtxTable.erase(it);
            delete pKey;

            return;
        }
    }
}

CAmLogContextDlt &CAmLoggerDlt::createContext(const char *contextid)
{
    // check, if we already have this context
    for (auto &ctx : mCtxTable)
    {
        if (contextid && strncmp(contextid, ctx.first, DLT_ID_SIZE) == 0)
        {
            return *ctx.second;
        }
    }

    // Not in list. Create new
    size_t len = (contextid ? strlen(contextid) : 0);
    char *pKey = new char[1 + len];
    strncpy(pKey, contextid, len);
    pKey[len] = '\0';
    auto *pContext = new CAmLogContextDlt(contextid);
    mCtxTable[pKey] = pContext;

    return *pContext;
}

}
