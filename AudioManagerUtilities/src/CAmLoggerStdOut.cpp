/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2017, ADIT GmbH
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
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2017
 * \author Mattia Guerra, mguerra@de.adit-jv.com ADIT 2017
 * \author Martin Koch, mkoch@de.adit-jv.com ADIT 2020
 *
 * \file CAmLoggerStdOut.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <mutex>
#include <iomanip>
#include "CAmLogWrapper.h"
#include "CAmLoggerStdOut.h"
#include "CAmTimeUtility.h"

using namespace std;

namespace am
{

#define PADDING_WIDTH 4

pthread_mutex_t gStdOutMtx = PTHREAD_MUTEX_INITIALIZER;

ostream &operator <<(ostream &out, const class CStdOutHeader &h)
{
    out << CAmTimeUtility::now() << h.mCc << "[" << setw(PADDING_WIDTH) << left << string(h.mCtx, 0, PADDING_WIDTH) << "] " << CC_RESET;
    return out;
}

CAmLogContextStdOut::CAmLogContextStdOut(const char *id, const am_LogLevel_e level, const am_LogStatus_e status)
    : mHeader(id, CC_GREEN)
    , mLogLevel(level)
    , mLogStatus(status)
{
}

void CAmLogContextStdOut::changeLogLS(const am_LogLevel_e level, const am_LogStatus_e status)
{
    mLogLevel  = level;
    mLogStatus = status;
}

void CAmLogContextStdOut::append(const int8_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const uint8_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const int16_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const uint16_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const int32_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const uint32_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const uint64_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const int64_t value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const bool value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const char *value)
{
    appendStdOut(value);
}

void CAmLogContextStdOut::append(const vector<uint8_t> &data)
{
    cout << data.data() << " ";
}

template<class T>
void CAmLogContextStdOut::appendStdOut(T value)
{
    cout << value << " ";
}

bool CAmLogContextStdOut::configure(const am_LogLevel_e loglevel)
{
    if ((mLogStatus == LS_OFF) || (loglevel > mLogLevel))
    {
        return false;
    }

    pthread_mutex_lock(&gStdOutMtx);

    switch (loglevel)
    {
    case LL_ERROR:
        cout << mHeader << CC_RED;
        break;
    case LL_WARN:
        cout << mHeader << CC_YELLOW;
        break;
    default:
        cout << mHeader << CC_RESET;
        break;
    }

    return true;
}

bool CAmLogContextStdOut::checkLogLevel(const am_LogLevel_e logLevel)
{
    return ((mLogStatus == LS_ON) && (logLevel <= mLogLevel));
}

void CAmLogContextStdOut::send()
{
    // NOTE: The color is set in the configure function
    cout << CC_RESET << endl;
    pthread_mutex_unlock(&gStdOutMtx);
}

CAmLoggerStdOut::CAmLoggerStdOut(const am_LogStatus_e status, const bool onlyError)
    : mLogStatus(status)
    , mStandardLogLevel(onlyError ? LL_ERROR : LL_INFO)
{
    if (mLogStatus == LS_OFF)
    {
        cout << mHeader << "Running without Logging support" << endl;
    }
}

CAmLoggerStdOut::~CAmLoggerStdOut()
{
    unregisterApp();
}

void CAmLoggerStdOut::unregisterApp()
{
    for (auto &&context : mCtxTable)
    {
        unregisterContext(context.first);
    }
}

void CAmLoggerStdOut::registerApp(const char *appid, const char *description)
{
    if (mLogStatus == LS_ON)
    {
        cout << mHeader << "Register Application " << string(appid, PADDING_WIDTH) << ", " << description << endl;
    }

    registerContext(DEFAULT_CONTEXT, DEFAULT_DESCRIPTION);
}

IAmLogContext &CAmLoggerStdOut::registerContext(const char *contextid, const char *description)
{
    return registerContext(contextid, description, mStandardLogLevel, mLogStatus);
}

IAmLogContext &CAmLoggerStdOut::registerContext(const char *contextid, const char *description,
    const am_LogLevel_e level, const am_LogStatus_e status)
{
    // check, if we already have this context
    for (auto &ctx : mCtxTable)
    {
        if (contextid && strncmp(contextid, ctx.first, PADDING_WIDTH) == 0)
        {
            ctx.second->changeLogLS(level, status);
            return *ctx.second;
        }
    }

    // Not in list. Create new
    if (mLogStatus == LS_ON)
    {
        cout << mHeader << "Registering Context " << string(contextid, PADDING_WIDTH) << ", " << description << endl;
    }
    size_t len = (contextid ? strlen(contextid) : 0);
    char *pKey = new char[1 + len];
    strncpy(pKey, contextid, len);
    pKey[len] = '\0';
    auto *pContext = new CAmLogContextStdOut(contextid, level, status);
    pContext->changeLogLS(level, status);
    mCtxTable[pKey] = pContext;

    return *pContext;
}

IAmLogContext &CAmLoggerStdOut::importContext(const char *contextid)
{
    // check, if we have this context
    contextid = (contextid ? contextid : DEFAULT_CONTEXT);
    for (auto &ctx : mCtxTable)
    {
        if (contextid && strncmp(ctx.first, contextid, PADDING_WIDTH) == 0)
        {
            return *ctx.second;
        }
    }

    // no match. Fall back to default context
    return importContext(DEFAULT_CONTEXT);
}

void CAmLoggerStdOut::unregisterContext(const char *contextid)
{
    for (auto it = mCtxTable.begin(); it != mCtxTable.end(); ++it)
    {
        if (contextid && strncmp(contextid, it->first, PADDING_WIDTH) == 0)
        {
            delete it->second;
            const char *key = it->first;
            mCtxTable.erase(it);
            delete key;

            if (mLogStatus == LS_ON)
            {
                cout << mHeader << string(contextid, PADDING_WIDTH) << " unregistered" << endl;
            }

            return;
        }
    }
}

}
