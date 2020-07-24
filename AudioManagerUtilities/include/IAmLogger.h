/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, ADIT GmbH
 *
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2015
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
 * \file IAmLogger.h
 * For further information see http://www.genivi.org/.
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <pthread.h>
#include <stdint.h>
#include <sstream>
#include <vector>
#include <cassert>
#include "audiomanagertypes.h"


namespace am
{

#define DEFAULT_CONTEXT     "DEF"
#define DEFAULT_DESCRIPTION "default Context registered by DLT Logger Class"

#define DEFAULT_LOG_SERVICE static_cast<am_LogService_e>(0)

enum am_LogService_e
{
    LOG_SERVICE_DLT,
    LOG_SERVICE_STDOUT,
    LOG_SERVICE_FILE,
    LOG_SERVICE_OFF
};

enum am_LogLevel_e
{
    LL_OFF     = 0x00, /**< Log level off */
    LL_FATAL   = 0x01, /**< fatal system error */
    LL_ERROR   = 0x02, /**< error with impact to correct functionality */
    LL_WARN    = 0x03, /**< warning, correct behavior could not be ensured */
    LL_INFO    = 0x04, /**< informational */
    LL_DEBUG   = 0x05, /**< debug  */
    LL_VERBOSE = 0x06  /**< highest grade of information */
};

enum am_LogStatus_e
{
    LS_OFF = 0x00,
    LS_ON  = 0x01
};

class IAmLogContext
{
    // enable cooperation with legacy class CAmDltWrapper
    friend class CAmDltWrapper;

public:
    virtual ~IAmLogContext() {}

    /**
     * logs given loglevel and given values
     * @param loglevel
     * @param ...
     */
    template<typename... TArgs>
    void off(const TArgs & ... args)
    {
        this->log(LL_OFF, args...);
    }

    template<typename... TArgs>
    void fatal(const TArgs & ... args)
    {
        this->log(LL_FATAL, args...);
    }

    template<typename... TArgs>
    void error(const TArgs & ... args)
    {
        this->log(LL_ERROR, args...);
    }

    template<typename... TArgs>
    void warn(const TArgs & ... args)
    {
        this->log(LL_WARN, args...);
    }

    template<typename... TArgs>
    void info(const TArgs & ... args)
    {
        this->log(LL_INFO, args...);
    }

    template<typename... TArgs>
    void debug(const TArgs & ... args)
    {
        this->log(LL_DEBUG, args...);
    }

    template<typename... TArgs>
    void verbose(const TArgs & ... args)
    {
        this->log(LL_VERBOSE, args...);
    }

    template<typename... TArgs>
    void log(const am_LogLevel_e loglevel, const TArgs & ... args)
    {
        if (!this->configure(loglevel))
        {
            return;
        }

        this->append(args...);
        this->send();
    }

    virtual bool checkLogLevel(const am_LogLevel_e logLevel) = 0;

    virtual bool configure(const am_LogLevel_e loglevel) = 0;

private:
    virtual void send() = 0;

    virtual void append(const int8_t value)               = 0;
    virtual void append(const uint8_t value)              = 0;
    virtual void append(const int16_t value)              = 0;
    virtual void append(const uint16_t value)             = 0;
    virtual void append(const int32_t value)              = 0;
    virtual void append(const uint32_t value)             = 0;
    virtual void append(const uint64_t value)             = 0;
    virtual void append(const int64_t value)              = 0;
    virtual void append(const bool value)                 = 0;
    virtual void append(const std::vector<uint8_t> &data) = 0;
    virtual void append(const char *value)                = 0;

    template<typename T = std::string &>
    void append(const std::string &value)
    {
        this->append(value.c_str());
    }

    template<typename T>
    void append_enum(const T &value, const T &tmax, const std::vector<const char *> &text)
    {
        assert(tmax == text.size());
        try
        {
            this->append(text.at(value));
        }
        catch (const std::out_of_range &)
        {
            this->append(__PRETTY_FUNCTION__);
            this->append(static_cast<int32_t>(value));
            this->append("out of range!");
        }
    }

    template<typename T = am_Error_e>
    void append(const am_Error_e value)
    {
        this->append_enum(value, E_MAX, std::vector<const char *> {
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
                "E_COMMUNICATION"
            }
            );
    }

    template<typename T = am_SourceState_e>
    void append(const am_SourceState_e value)
    {
        this->append_enum(value, SS_MAX, std::vector<const char *> {
                "SS_UNKNNOWN",
                "SS_ON",
                "SS_OFF",
                "SS_PAUSED"
            }
            );
    }

    template<typename T = am_MuteState_e>
    void append(const am_MuteState_e value)
    {
        this->append_enum(value, MS_MAX, std::vector<const char *> {
                "MS_UNKNOWN",
                "MS_MUTED",
                "MS_UNMUTED"
            }
            );
    }

    template<typename T = am_DomainState_e>
    void append(const am_DomainState_e value)
    {
        this->append_enum(value, DS_MAX, std::vector<const char *> {
                "DS_UNKNOWN",
                "DS_CONTROLLED",
                "DS_INDEPENDENT_STARTUP",
                "DS_INDEPENDENT_RUNDOWN"
            }
            );
    }

    template<typename T = am_ConnectionState_e>
    void append(const am_ConnectionState_e value)
    {
        this->append_enum(value, CS_MAX, std::vector<const char *> {
                "CS_UNKNOWN",
                "CS_CONNECTING",
                "CS_CONNECTED",
                "CS_DISCONNECTING",
                "CS_DISCONNECTED",
                "CS_SUSPENDED"
            }
            );
    }

    template<typename T = am_Availability_e>
    void append(const am_Availability_e value)
    {
        this->append_enum(value, A_MAX, std::vector<const char *> {
                "A_UNKNOWN",
                "A_AVAILABLE",
                "A_UNAVAILABLE"
            }
            );
    }

    template<typename T = am_InterruptState_e>
    void append(const am_InterruptState_e value)
    {
        this->append_enum(value, IS_MAX, std::vector<const char *> {
                "IS_UNKNOWN",
                "IS_OFF",
                "IS_INTERRUPTED"
            }
            );
    }

    template<typename T = am_Handle_e>
    void append(const am_Handle_e value)
    {
        this->append_enum(value, H_MAX, std::vector<const char *> {
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
                "H_TRANSFERCONNECTION"
            }
            );
    }

    template<typename T = am_Handle_s>
    void append(const am_Handle_s value)
    {
        this->append(value.handle);
        this->append(value.handleType);
    }

    // Template to print unknown pointer types with their address
    template<typename T>
    void append(T *value)
    {
        std::ostringstream ss;
        ss << "0x" << std::hex << (uint64_t)value;
        this->append(ss.str().c_str());
    }

    // Template to print unknown types
    template<typename T>
    void append(T value)
    {
        std::ostringstream ss;
        ss << std::dec << value;
        this->append(ss.str().c_str());
    }

    // Template parameter pack to generate recursive code
    void append(void) {}
    template<typename T, typename... TArgs>
    void append(const T &value, const TArgs & ... args)
    {
        this->append(value);
        this->append(args...);
    }

};

class IAmLogger
{
public:
    virtual ~IAmLogger() {}
    virtual void registerApp(const char *appid, const char *description) = 0;
    virtual void unregisterApp() = 0;
    virtual IAmLogContext &registerContext(const char *contextid, const char *description) = 0;
    virtual IAmLogContext &registerContext(const char *contextid, const char *description,
        const am_LogLevel_e level, const am_LogStatus_e status) = 0;
    virtual IAmLogContext &importContext(const char *contextid = NULL) = 0;
    virtual void unregisterContext(const char *contextid)      = 0;

    template<typename T, typename... TArgs>
    void logToDefaultContext(const am_LogLevel_e loglevel, const T &value, const TArgs & ... args)
    {
        this->importContext().log(loglevel, value, args...);
    }

};

}

#endif // LOGGER_H_
