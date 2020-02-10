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
 * \file CAmLoggerFile.h
 * For further information see http://www.genivi.org/.
 */

#ifndef LOGGERFILE_H_
#define LOGGERFILE_H_

#include "IAmLogger.h"
#include <map>
#include <string>
#include <string.h>
#include <fstream>

namespace am
{

class CFileHeader
{
public:
    CFileHeader(const char *ctx = "LOG")
        : mCtx(ctx) {}
    friend std ::ostream &operator <<(std::ostream &out, const CFileHeader &h);

    const char *mCtx;
};

class CAmLogContextFile : public IAmLogContext
{
public:
    CAmLogContextFile(const char *id, const am_LogLevel_e level, const am_LogStatus_e status, std::ofstream &filestream);
    virtual ~CAmLogContextFile() {}

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
    void appendFile(T value);

private:
    CFileHeader    mHeader;
    am_LogLevel_e  mLogLevel;
    am_LogStatus_e mLogStatus;
    std::ofstream &mFilestream;
};

class CAmLoggerFile : public IAmLogger
{
public:
    CAmLoggerFile(const am_LogStatus_e status, const bool onlyError = false, const std::string &filename = "");
    ~CAmLoggerFile();

    /* IAmLogger */
    void registerApp(const char *appid, const char *description) override;
    void unregisterApp() override;
    IAmLogContext &registerContext(const char *contextid, const char *description) override;
    IAmLogContext &registerContext(const char *contextid, const char *description,
        const am_LogLevel_e level, const am_LogStatus_e status) override;
    IAmLogContext &importContext(const char *contextid = NULL) override;
    void unregisterContext(const char *contextid) override;

private:
    void print(std::string str);

private:
    CFileHeader          mHeader;
    const am_LogStatus_e mLogStatus;
    const am_LogLevel_e  mStandardLogLevel;
    std::ofstream        mFilestream;
    std::map<const char *, CAmLogContextFile *> mCtxTable;
};

}

#endif /* LOGGERFILE_H_ */
