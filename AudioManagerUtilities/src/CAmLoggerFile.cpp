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
 * \file CAmLoggerFile.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <mutex>
#include <iomanip>
#include "CAmLogWrapper.h"
#include "CAmLoggerFile.h"
#include "CAmTimeUtility.h"

using namespace std;

namespace am
{

#define PADDING_WIDTH 4

pthread_mutex_t gFileMtx = PTHREAD_MUTEX_INITIALIZER;

ostream &operator <<(ostream &out, const class CFileHeader &h)
{
    out << CAmTimeUtility::now() << "[" << setw(PADDING_WIDTH) << left << string(h.mCtx, 0, PADDING_WIDTH) << "] ";
    return out;
}

CAmLogContextFile::CAmLogContextFile(const char *id, const am_LogLevel_e level, const am_LogStatus_e status, ofstream &filestream)
    : mHeader(id)
    , mLogLevel(level)
    , mLogStatus(status)
    , mFilestream(filestream)
{
}

void CAmLogContextFile::append(const int8_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const uint8_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const int16_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const uint16_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const int32_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const uint32_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const uint64_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const int64_t value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const bool value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const char *value)
{
    appendFile(value);
}

void CAmLogContextFile::append(const vector<uint8_t> &data)
{
    mFilestream << data.data() << " ";
}

template<class T>
void CAmLogContextFile::appendFile(T value)
{
    mFilestream << value << " ";
}

bool CAmLogContextFile::configure(const am_LogLevel_e loglevel)
{
    if (LS_OFF || loglevel > mLogLevel)
    {
        return false;
    }

    pthread_mutex_lock(&gFileMtx);
    mFilestream << mHeader;

    return true;
}

bool CAmLogContextFile::checkLogLevel(const am_LogLevel_e logLevel)
{
    return logLevel <= mLogLevel;
}

void CAmLogContextFile::send()
{
    mFilestream << endl;
    pthread_mutex_unlock(&gFileMtx);
}

CAmLoggerFile::CAmLoggerFile(const am_LogStatus_e status, const bool onlyError, const string &filename)
    : mLogStatus(status)
    , mStandardLogLevel(onlyError ? LL_ERROR : LL_INFO)
{
    if (mLogStatus == LS_OFF)
    {
        cout << "Running without Logging support" << endl;
        return;
    }

    mFilestream.open(filename.c_str(), ofstream::out | ofstream::trunc);
    if (!mFilestream.is_open())
    {
        throw runtime_error("Cannot open log file: " + filename);
    }
}

CAmLoggerFile::~CAmLoggerFile()
{
    mFilestream.close();
    unregisterApp();
}

void CAmLoggerFile::unregisterApp()
{
    for (auto &&context : mCtxTable)
    {
        unregisterContext(context.first);
    }
}

void CAmLoggerFile::registerApp(const char *appid, const char *description)
{
    if (mLogStatus == LS_ON)
    {
        mFilestream << mHeader << "Register Application " << string(appid, PADDING_WIDTH) << ", " << description << endl;
    }

    registerContext(DEFAULT_CONTEXT, DEFAULT_DESCRIPTION);
}

IAmLogContext &CAmLoggerFile::registerContext(const char *contextid, const char *description)
{
    return registerContext(contextid, description, mStandardLogLevel, mLogStatus);
}

IAmLogContext &CAmLoggerFile::registerContext(const char *contextid, const char *description,
    const am_LogLevel_e level, const am_LogStatus_e status)
{
    // check, if we already have this context
    for (auto &ctx : mCtxTable)
    {
        if (contextid && strncmp(contextid, ctx.first, PADDING_WIDTH) == 0)
        {
            return *ctx.second;
        }
    }

    // Not in list. Create new
    if (mLogStatus == LS_ON)
    {
        mFilestream << mHeader << "Registering Context " << string(contextid, PADDING_WIDTH) << ", " << description << endl;
    }
    size_t len = (contextid ? strlen(contextid) : 0);
    char *pKey = new char[1 + len];
    strncpy(pKey, contextid, len);
    pKey[len] = '\0';
    auto *pContext = new CAmLogContextFile(contextid, level, status, mFilestream);
    mCtxTable[pKey] = pContext;

    return *pContext;
}

IAmLogContext &CAmLoggerFile::importContext(const char *contextid)
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

void CAmLoggerFile::unregisterContext(const char *contextid)
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
                mFilestream << mHeader << "Context " << string(contextid, PADDING_WIDTH) << "unregistered" << endl;
            }

            return;
        }
    }
}

}
