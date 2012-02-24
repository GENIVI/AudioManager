/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file DLTWrapper.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */


#include "DLTWrapper.h"
#include <cassert>

DLTWrapper* DLTWrapper::mDLTWrapper = NULL;

DLTWrapper *DLTWrapper::instance()
{
    if (!mDLTWrapper)
        mDLTWrapper = new DLTWrapper;
    return mDLTWrapper;
}

void DLTWrapper::unregisterContext(DltContext & handle)
{
#ifdef WITH_DLT
    dlt_unregister_context(&handle);
#endif
}

DLTWrapper::DLTWrapper() :
        mDltContext(), //
        mDltContextData()
{
}

void DLTWrapper::registerApp(const char *appid, const char *description)
{
#ifdef WITH_DLT
    dlt_register_app(appid, description);
    //register a default context
    dlt_register_context(&mDltContext, "def", "default Context registered by DLTWrapper CLass");
#endif
}

void DLTWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
{
#ifdef WITH_DLT
    dlt_register_context(&handle, contextid, description);
#endif
}

void DLTWrapper::init(DltLogLevelType loglevel, DltContext* context)
{
    if (!context)
        context = &mDltContext;
#ifdef WITH_DLT
    dlt_user_log_write_start(context, &mDltContextData, loglevel);
#endif

}

void DLTWrapper::send()
{
#ifdef WITH_DLT
    dlt_user_log_write_finish(&mDltContextData);
#endif
}

void DLTWrapper::append(const int8_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int8(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const uint8_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint8(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const int16_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int16(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const uint16_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint16(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const int32_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_int32(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const uint32_t value)
{
#ifdef WITH_DLT
    dlt_user_log_write_uint32(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const char*& value)
{
#ifdef WITH_DLT
    dlt_user_log_write_string(&mDltContextData, value);
#endif
}

void DLTWrapper::append(const std::string& value)
{
#ifdef WITH_DLT
    dlt_user_log_write_string(&mDltContextData, value.c_str());
#endif
}

void DLTWrapper::append(const bool value)
{
#ifdef WITH_DLT
    dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
#endif
}

DLTWrapper::~DLTWrapper()
{
    if (mDLTWrapper)
        delete mDLTWrapper;
}

