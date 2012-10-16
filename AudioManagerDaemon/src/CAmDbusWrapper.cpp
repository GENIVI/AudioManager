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
 * \file CAmDbusWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "shared/CAmDbusWrapper.h"
#include <config.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSocketHandler.h"

namespace am
{

/**
 * introspectio header
 */
#define ROOT_INTROSPECT_XML												\
DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE								\
"<node>"																\
"<interface name='org.AudioManager.freedesktop.DBus.Introspectable'>"	\
"<method name='Introspect'>"											\
"	<arg name='xml_data' type='s' direction='out'/>"					\
"</method>"							      								\
"</interface>"															\

CAmDbusWrapper* CAmDbusWrapper::mpReference = NULL;

CAmDbusWrapper::CAmDbusWrapper(CAmSocketHandler* socketHandler, DBusBusType type) :
        pDbusDispatchCallback(this, &CAmDbusWrapper::dbusDispatchCallback), //
        pDbusFireCallback(this, &CAmDbusWrapper::dbusFireCallback), //
        pDbusCheckCallback(this, &CAmDbusWrapper::dbusCheckCallback), //
        pDbusTimerCallback(this, &CAmDbusWrapper::dbusTimerCallback), //
        mpDbusConnection(0), //
        mDBusError(), //
        mListNodes(), //
        mpListTimerhandles(), //
        mpSocketHandler(socketHandler), //
        mDbusType(type)
{
    assert(mpSocketHandler!=0);

    dbus_error_init(&mDBusError);

    if (!dbus_threads_init_default())
        logError("CAmDbusWrapper::CAmDbusWrapper threads init call failed");
    logInfo("DBusWrapper::DBusWrapper Opening DBus connection");
    mpDbusConnection = dbus_bus_get(mDbusType, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::DBusWrapper Error while getting the DBus");
        dbus_error_free(&mDBusError);
    }
    if (NULL == mpDbusConnection)
    {
        logError("DBusWrapper::DBusWrapper DBus Connection is null");
    }

    //then we need to adopt the dbus to our mainloop:
    //first, we are old enought to live longer then the connection:
    dbus_connection_set_exit_on_disconnect(mpDbusConnection, FALSE);

    //we do not need the manual dispatching, since it is not allowed to call from a different thread. So leave it uncommented:
    //dbus_connection_set_dispatch_status_function

    //add watch functions:
    dbus_bool_t watch = dbus_connection_set_watch_functions(mpDbusConnection, addWatch, removeWatch, toogleWatch, this, NULL);
    if (!watch)
    {
        logError("DBusWrapper::DBusWrapper Registering of watch functions failed");
    }

    //add timer functions:
    dbus_bool_t timer = dbus_connection_set_timeout_functions(mpDbusConnection, addTimeout, removeTimeout, toggleTimeout, this, NULL);
    if (!timer)
    {
        logError("DBusWrapper::DBusWrapper Registering of timer functions failed");
    }

    //register callback for Introspectio
    mObjectPathVTable.message_function = CAmDbusWrapper::cbRootIntrospection;
    logInfo("dbusconnection ",mpDbusConnection);
    dbus_connection_register_object_path(mpDbusConnection, DBUS_SERVICE_OBJECT_PATH, &mObjectPathVTable, this);
    int ret = dbus_bus_request_name(mpDbusConnection, DBUS_SERVICE_PREFIX, DBUS_NAME_FLAG_DO_NOT_QUEUE, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::DBusWrapper Name Error", mDBusError.message);
        dbus_error_free(&mDBusError);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
    {
        logError("DBusWrapper::DBusWrapper Wrapper is not the Primary Owner ! Another instance already running?");
        throw std::runtime_error("DBusWrapper::DBusWrapper Wrapper is not the Primary Owner ! Another instance already running?");
    }
}

CAmDbusWrapper::~CAmDbusWrapper()
{
    //close the connection again
    logInfo("DBusWrapper::~DBusWrapper Closing DBus connection");
    dbus_connection_unref(mpDbusConnection);

    //clean up all timerhandles we created but did not delete before
    std::vector<sh_timerHandle_t*>::iterator it = mpListTimerhandles.begin();
    for (; it != mpListTimerhandles.end(); ++it)
    {
        delete *it;
    }
}

/**
 * registers a callback that is entered as path below the main path.
 * The configuration of the mainpath is done via DBusConfiguration.h
 * @param vtable the vtable that holds a pointer to the callback that is called when the path is called from the dbus
 * @param path the name of the path
 * @param userdata pointer to the class that will handle the callback
 */
void CAmDbusWrapper::registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata)
{
    logInfo("DBusWrapper::registerCallback register callback:", path);

    std::string completePath = std::string(DBUS_SERVICE_OBJECT_PATH) + "/" + path;
    dbus_error_init(&mDBusError);
    mpDbusConnection = dbus_bus_get(mDbusType, &mDBusError);
    dbus_connection_register_object_path(mpDbusConnection, completePath.c_str(), vtable, userdata);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::registerCallack error: ", mDBusError.message);
        dbus_error_free(&mDBusError);
    }
    mListNodes.push_back(path);
}

/**
 * internal callback for the root introspection
 * @param conn
 * @param msg
 * @param reference
 * @return
 */
DBusHandlerResult CAmDbusWrapper::cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference)
{
    logInfo("DBusWrapper::~cbRootIntrospection called:");

    mpReference = (CAmDbusWrapper*) reference;
    std::vector<std::string> nodesList = mpReference->mListNodes;
    DBusMessage * reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;
    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
    {
        std::vector<std::string>::iterator nodeIter = nodesList.begin();
        const char *xml = ROOT_INTROSPECT_XML;
        std::stringstream introspect;
        introspect << std::string(xml);
        for (; nodeIter != nodesList.end(); ++nodeIter)
        {
            introspect << "<node name='" << nodeIter->c_str() << "'/>";
        }
        introspect << "</node>";

        reply = dbus_message_new_method_return(msg);
        std::string s = introspect.str();
        const char* string = s.c_str();

        // add the arguments to the reply
        dbus_message_iter_init_append(reply, &args);
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
        {
            logError("DBusWrapper::~cbRootIntrospection DBUS Out Of Memory!");
        }

        // send the reply && flush the connection
        if (!dbus_connection_send(conn, reply, &serial))
        {
            logError("DBusWrapper::~cbRootIntrospection DBUS Out Of Memory!");
        }
        dbus_connection_flush(conn);
        // free the reply
        dbus_message_unref(reply);

        return (DBUS_HANDLER_RESULT_HANDLED);
    }
    else
    {
        return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
    }
}

/**
 * returns the dbus connection
 * @param connection pointer to the connection
 */
void CAmDbusWrapper::getDBusConnection(DBusConnection *& connection) const
{
    connection = mpDbusConnection;
}

dbus_bool_t CAmDbusWrapper::addWatch(DBusWatch *watch, void *userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    return (mpReference->addWatchDelegate(watch, userData));
}

dbus_bool_t CAmDbusWrapper::addWatchDelegate(DBusWatch * watch, void* userData)
{
    (void) userData;
    int16_t event = 0;
    sh_pollHandle_t handle = 0;
    uint flags = dbus_watch_get_flags(watch);

    /* no watch flags for disabled watches */
    if (dbus_watch_get_enabled(watch))
    {
        if (flags & DBUS_WATCH_READABLE)
            event |= POLLIN;
        if (flags & DBUS_WATCH_WRITABLE)
            event |= POLLOUT;
    }

    logInfo("DBusWrapper::addWatchDelegate entered new watch, fd=", dbus_watch_get_unix_fd(watch), "event flag=", event);
    am_Error_e error = mpSocketHandler->addFDPoll(dbus_watch_get_unix_fd(watch), event, NULL, &pDbusFireCallback, &pDbusCheckCallback, &pDbusDispatchCallback, watch, handle);

    //if everything is alright, add the watch and the handle to our map so we know this relationship
    if (error == E_OK && handle != 0)
    {
        mMapHandleWatch.insert(std::make_pair(watch, handle));
        return (true);
    }
    logError("DBusWrapper::addWatchDelegate entering watch failed");
    return (true);
}

void CAmDbusWrapper::removeWatch(DBusWatch *watch, void *userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    mpReference->removeWatchDelegate(watch, userData);
}

void CAmDbusWrapper::removeWatchDelegate(DBusWatch *watch, void *userData)
{
    (void) userData;
    std::map<DBusWatch*, sh_pollHandle_t>::iterator iterator = mMapHandleWatch.begin();
    iterator = mMapHandleWatch.find(watch);
    if (iterator != mMapHandleWatch.end())
    {
        mpSocketHandler->removeFDPoll(iterator->second);
        logInfo("DBusWrapper::removeWatch removed watch with handle", iterator->second);
        mMapHandleWatch.erase(iterator);
    }
    else
    {
        logError("DBusWrapper::removeWatch could not find handle !");
    }
}

void CAmDbusWrapper::toogleWatch(DBusWatch *watch, void *userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    mpReference->toogleWatchDelegate(watch, userData);
}

void CAmDbusWrapper::toogleWatchDelegate(DBusWatch *watch, void *userData)
{
    (void) userData;
    int16_t event = 0;
    dbus_watch_get_unix_fd(watch);
    uint flags = dbus_watch_get_flags(watch);
    /* no watch flags for disabled watches */
    if (dbus_watch_get_enabled(watch))
    {
        if (flags & DBUS_WATCH_READABLE)
            event |= POLLIN;
        if (flags & DBUS_WATCH_WRITABLE)
            event |= POLLOUT;
    }
    std::map<DBusWatch*, sh_pollHandle_t>::iterator iterator = mMapHandleWatch.begin();
    iterator = mMapHandleWatch.find(watch);
    if (iterator != mMapHandleWatch.end())
        mpSocketHandler->updateEventFlags(iterator->second, event);
    logInfo("DBusWrapper::toogleWatchDelegate watch was toggeled");
}

dbus_bool_t CAmDbusWrapper::addTimeout(DBusTimeout *timeout, void* userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    return (mpReference->addTimeoutDelegate(timeout, userData));
}

dbus_bool_t CAmDbusWrapper::addTimeoutDelegate(DBusTimeout *timeout, void* userData)
{
    (void)userData;

    if (!dbus_timeout_get_enabled(timeout))
        return (false);

    //calculate the timeout in timeval
    timespec pollTimeout;
    int localTimeout = dbus_timeout_get_interval(timeout);
    pollTimeout.tv_sec = localTimeout / 1000;
    pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;

    //prepare handle and callback. new is eval, but there is no other choice because we need the pointer!
    sh_timerHandle_t* handle = new sh_timerHandle_t;
    mpListTimerhandles.push_back(handle);

    //add the timer to the pollLoop
    mpSocketHandler->addTimer(pollTimeout, &pDbusTimerCallback, *handle, timeout);

    //save the handle with dbus context
    dbus_timeout_set_data(timeout, handle, NULL);

    //save timeout in Socket context
    userData = timeout;
    logInfo("DBusWrapper::addTimeoutDelegate a timeout was added, timeout",localTimeout," handle ", *handle);
    return (true);
}

void CAmDbusWrapper::removeTimeout(DBusTimeout *timeout, void* userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    mpReference->removeTimeoutDelegate(timeout, userData);
}

void CAmDbusWrapper::removeTimeoutDelegate(DBusTimeout *timeout, void* userData)
{
    (void) userData;
    //get the pointer to the handle and remove the timer
    sh_timerHandle_t* handle = (sh_timerHandle_t*) dbus_timeout_get_data(timeout);
    mpSocketHandler->removeTimer(*handle);

    //now go throught the timerlist and remove the pointer, free memory
    std::vector<sh_timerHandle_t*>::iterator it = mpListTimerhandles.begin();
    for (; it != mpListTimerhandles.end(); ++it)
    {
        if (*it == handle)
        {
            mpListTimerhandles.erase(it);
            break;
        }
    }
    delete handle;
    logInfo("DBusWrapper::removeTimeoutDelegate a timeout was removed");
}

void CAmDbusWrapper::toggleTimeout(DBusTimeout *timeout, void* userData)
{
    mpReference = (CAmDbusWrapper*) userData;
    assert(mpReference!=0);
    mpReference->toggleTimeoutDelegate(timeout, userData);
}

bool am::CAmDbusWrapper::dbusDispatchCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    bool returnVal = true;
    dbus_connection_ref(mpDbusConnection);
    if (dbus_connection_dispatch(mpDbusConnection) == DBUS_DISPATCH_COMPLETE)
        returnVal = false;
    dbus_connection_unref(mpDbusConnection);
    //logInfo("DBusWrapper::dbusDispatchCallback was called");
    return (returnVal);
}

bool am::CAmDbusWrapper::dbusCheckCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    bool returnVal = false;
    dbus_connection_ref(mpDbusConnection);
    if (dbus_connection_get_dispatch_status(mpDbusConnection) == DBUS_DISPATCH_DATA_REMAINS)
        returnVal = true;
    dbus_connection_unref(mpDbusConnection);
    //logInfo("DBusWrapper::dbusCheckCallback was called");
    return (returnVal);
}

void am::CAmDbusWrapper::dbusFireCallback(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    assert(userData!=NULL);
    uint flags = 0;

    if (pollfd.revents & POLLIN)
        flags |= DBUS_WATCH_READABLE;
    if (pollfd.revents & POLLOUT)
        flags |= DBUS_WATCH_WRITABLE;
    if (pollfd.revents & POLLHUP)
        flags |= DBUS_WATCH_HANGUP;
    if (pollfd.revents & POLLERR)
        flags |= DBUS_WATCH_ERROR;

    DBusWatch *watch = (DBusWatch*) userData;

    dbus_connection_ref(mpDbusConnection);
    dbus_watch_handle(watch, flags);
    dbus_connection_unref(mpDbusConnection);
    //logInfo("DBusWrapper::dbusFireCallback was called");
}

void CAmDbusWrapper::toggleTimeoutDelegate(DBusTimeout *timeout, void* userData)
{
    (void) userData;
    //get the pointer to the handle and remove the timer
    sh_timerHandle_t* handle = (sh_timerHandle_t*) dbus_timeout_get_data(timeout);

    //stop or restart?
    if (dbus_timeout_get_enabled(timeout))
    {
        //calculate the timeout in timeval
        timespec pollTimeout;
        int localTimeout = dbus_timeout_get_interval(timeout);
        pollTimeout.tv_sec = localTimeout / 1000;
        pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;
        mpSocketHandler->updateTimer(*handle, pollTimeout);
    }
    else
    {
        mpSocketHandler->stopTimer(*handle);
    }
    logInfo("DBusWrapper::toggleTimeoutDelegate was called");
}

void CAmDbusWrapper::dbusTimerCallback(sh_timerHandle_t handle, void *userData)
{
    logInfo("DBusWrapper::dbusTimerCallback was called");
    assert(userData!=NULL);
    if (dbus_timeout_get_enabled((DBusTimeout*) userData))
    {
        mpSocketHandler->restartTimer(handle);
    }
    dbus_timeout_handle((DBusTimeout*) userData);
}
}

