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
 * \file CAmLoggerStdOut.h
 * For further information see http://www.genivi.org/.
 */

#ifndef LOGGERSTDOUT_H_
#define LOGGERSTDOUT_H_

#include "IAmLogger.h"
#include <map>
#include <string>
#include <string.h>

namespace am
{

static const std::string CC_BLACK   = "\x1b[0;30m";
static const std::string CC_RED     = "\x1b[0;31m";
static const std::string CC_GREEN   = "\x1b[0;32m";
static const std::string CC_YELLOW  = "\x1b[0;33m";
static const std::string CC_BLUE    = "\x1b[0;34m";
static const std::string CC_MAGENTA = "\x1b[0;35m";
static const std::string CC_CYAN    = "\x1b[0;36m";
static const std::string CC_WHITE   = "\x1b[0;37m";
static const std::string CC_RESET   = "\x1b[0;39m";

class CStdOutHeader
{
public:
    CStdOutHeader(const char *ctx = "LOG", const std::string &color = CC_BLUE)
        : mCtx(ctx)
        , mCc(color) {}
    friend std ::ofstream &operator <<(std::ofstream &out, const CStdOutHeader &h);

    const char        *mCtx;
    const std::string &mCc;
};

class CAmLogContextStdOut : public IAmLogContext
{
public:
    CAmLogContextStdOut(const char *id, const am_LogLevel_e level, const am_LogStatus_e status);
    virtual ~CAmLogContextStdOut() {}

    void changeLogLS(const am_LogLevel_e level, const am_LogStatus_e status);

    /* IAmLogContext */
    bool checkLogLevel(const am_LogLevel_e logLevel) override;

private:
    /* IAmLogContext */
    bool configure(const am_LogLevel_e loglevel) override;
    void send() override;
    void append(const int8_t value) override;
    void append(const uint8_t value) override;
    void append(const int16_t value) override;
    void append(const uint16_t value) override;
    void append(const int32_t value) override;
    void append(const uint32_t value) override;
    void append(const uint64_t value) override;
    void append(const int64_t value) override;
    void append(const bool value) override;
    void append(const std::vector<uint8_t> &data) override;
    void append(const char *value) override;

    template<class T>
    void appendStdOut(T value);

private:
    CStdOutHeader  mHeader;
    am_LogLevel_e  mLogLevel;
    am_LogStatus_e mLogStatus;
};

class CAmLoggerStdOut : public IAmLogger
{
public:
    CAmLoggerStdOut(const am_LogStatus_e status, const bool onlyError = false);
    ~CAmLoggerStdOut();

    /* IAmLogger */
    void registerApp(const char *appid, const char *description) override;
    void unregisterApp() override;
    IAmLogContext &registerContext(const char *contextid, const char *description) override;
    IAmLogContext &registerContext(const char *contextid, const char *description,
        const am_LogLevel_e level, const am_LogStatus_e status) override;
    IAmLogContext &importContext(const char *contextid = NULL) override;
    void unregisterContext(const char *contextid) override;

private:
    CStdOutHeader        mHeader;
    const am_LogStatus_e mLogStatus;
    const am_LogLevel_e  mStandardLogLevel;
    std::map<const char *, CAmLogContextStdOut *> mCtxTable;
};

}

#endif /* LOGGERSTDOUT_H_ */
