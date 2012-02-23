/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file SocketHandler.cpp
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

#include <dbus/DBusWrapper.h>
#include <SocketHandler.h>
#include <config.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include "DLTWrapper.h"

using namespace am;

#define ROOT_INTROSPECT_XML												\
DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE								\
"<node>"																\
"<interface name='org.AudioManager.freedesktop.DBus.Introspectable'>"	\
"<method name='Introspect'>"											\
"	<arg name='xml_data' type='s' direction='out'/>"					\
"</method>"							      								\
"</interface>"															\

DBusWrapper* DBusWrapper::mReference = NULL;

DBusWrapper::DBusWrapper(SocketHandler* socketHandler) :
        pDbusDispatchCallback(this, &DBusWrapper::dbusDispatchCallback), //
        pDbusFireCallback(this, &DBusWrapper::dbusFireCallback), //
        pDbusCheckCallback(this, &DBusWrapper::dbusCheckCallback), //
        pDbusTimerCallback(this, &DBusWrapper::dbusTimerCallback), //
        mDbusConnection(0), //
        mDBusError(), //
        mNodesList(), //
        mListTimerhandlePointer(), //
        mSocketHandler(socketHandler)
{
    assert(mSocketHandler!=0);

    dbus_error_init(&mDBusError);
    logInfo("DBusWrapper::DBusWrapper Opening DBus connection");
    mDbusConnection = dbus_bus_get(DBUS_BUS_SESSION, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::DBusWrapper Error while getting the DBus");
        dbus_error_free(&mDBusError);
    }
    if (NULL == mDbusConnection)
    {
        logError("DBusWrapper::DBusWrapper DBus Connection is null");
    }

    //then we need to adopt the dbus to our mainloop:
    //first, we are old enought to live longer then the connection:
    dbus_connection_set_exit_on_disconnect(mDbusConnection, FALSE);

    //we do not need the manual dispatching, since it is not allowed to call from a different thread. So leave it uncommented:
    //dbus_connection_set_dispatch_status_function

    //add watch functions:
    dbus_bool_t watch = dbus_connection_set_watch_functions(mDbusConnection, addWatch, removeWatch, toogleWatch, this, NULL);
    if (!watch)
    {
        logError("DBusWrapper::DBusWrapper Registering of watch functions failed");
    }

    //add timer functions:
    dbus_bool_t timer = dbus_connection_set_timeout_functions(mDbusConnection, addTimeout, removeTimeout, toggleTimeout, this, NULL);
    if (!timer)
    {
        logError("DBusWrapper::DBusWrapper Registering of timer functions failed");
    }

    //register callback for Introspectio
    mObjectPathVTable.message_function = DBusWrapper::cbRootIntrospection;
    dbus_connection_register_object_path(mDbusConnection, DBUS_SERVICE_OBJECT_PATH, &mObjectPathVTable, this);
    int ret = dbus_bus_request_name(mDbusConnection, DBUS_SERVICE_PREFIX, DBUS_NAME_FLAG_DO_NOT_QUEUE, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::DBusWrapper Name Error",mDBusError.message);
        dbus_error_free(&mDBusError);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
    {
        logError("DBusWrapper::DBusWrapper Wrapper is not the Primary Owner",ret);
        exit(1);
    }
}

DBusWrapper::~DBusWrapper()
{
    //close the connection again
    logInfo("DBusWrapper::~DBusWrapper Closing DBus connection");
    dbus_connection_unref(mDbusConnection);

    //clean up all timerhandles we created but did not delete before
    std::vector<sh_timerHandle_t*>::iterator it = mListTimerhandlePointer.begin();
    for (; it != mListTimerhandlePointer.end(); ++it)
    {
        delete *it;
    }
}

void DBusWrapper::registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata)
{
    logInfo("DBusWrapper::~registerCallback register callback:",path);

    std::string completePath = std::string(DBUS_SERVICE_OBJECT_PATH) + "/" + path;
    dbus_error_init(&mDBusError);
    mDbusConnection = dbus_bus_get(DBUS_BUS_SESSION, &mDBusError);
    dbus_connection_register_object_path(mDbusConnection, completePath.c_str(), vtable, userdata);
    if (dbus_error_is_set(&mDBusError))
    {
        logError("DBusWrapper::registerCallack error: ",mDBusError.message);
        dbus_error_free(&mDBusError);
    }
    mNodesList.push_back(path);
}

DBusHandlerResult DBusWrapper::cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference)
{
    logInfo("DBusWrapper::~cbRootIntrospection called:");

    mReference = (DBusWrapper*) reference;
    std::list<std::string> nodesList = mReference->mNodesList;
    DBusMessage * reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;
    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
    {
        std::list<std::string>::iterator nodeIter = nodesList.begin();
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

        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
}

void DBusWrapper::dbusMainLoop()
{
    logInfo("DBusWrapper::dbusMainLoop Entering MainLoop");

    while (dbus_connection_read_write_dispatch(mDbusConnection, -1))
    {

    }
}

void DBusWrapper::getDBusConnection(DBusConnection *& connection) const
{
    connection = mDbusConnection;
}

dbus_bool_t DBusWrapper::addWatch(DBusWatch *watch, void *userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    return mReference->addWatchDelegate(watch, userData);
}

dbus_bool_t DBusWrapper::addWatchDelegate(DBusWatch * watch, void* userData)
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

    logInfo("DBusWrapper::addWatchDelegate entered new watch, fd=",dbus_watch_get_unix_fd(watch),"event flag=",event);
    am_Error_e error = mSocketHandler->addFDPoll(dbus_watch_get_unix_fd(watch), event, NULL, &pDbusFireCallback, &pDbusCheckCallback, &pDbusDispatchCallback, watch, handle);

    //if everything is alright, add the watch and the handle to our map so we know this relationship
    if (error == E_OK && handle != 0)
    {
        mMapHandleWatch.insert(std::make_pair(watch, handle));
        return true;
    }
    logError("DBusWrapper::addWatchDelegate entering watch failed");
    return (true);
}

void DBusWrapper::removeWatch(DBusWatch *watch, void *userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    mReference->removeWatchDelegate(watch, userData);
}

void DBusWrapper::removeWatchDelegate(DBusWatch *watch, void *userData)
{
    (void) userData;
    std::map<DBusWatch*, sh_pollHandle_t>::iterator iterator = mMapHandleWatch.begin();
    iterator = mMapHandleWatch.find(watch);
    if (iterator != mMapHandleWatch.end())
        mSocketHandler->removeFDPoll(iterator->second);
    logInfo("DBusWrapper::removeWatch removed watch with handle",iterator->second);
    mMapHandleWatch.erase(iterator);
}

void DBusWrapper::toogleWatch(DBusWatch *watch, void *userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    mReference->toogleWatchDelegate(watch, userData);
}

void DBusWrapper::toogleWatchDelegate(DBusWatch *watch, void *userData)
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
        mSocketHandler->updateEventFlags(iterator->second, event);
    logInfo("DBusWrapper::toogleWatchDelegate watch was toggeled");
}

dbus_bool_t DBusWrapper::addTimeout(DBusTimeout *timeout, void* userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    return mReference->addTimeoutDelegate(timeout, userData);
}

dbus_bool_t DBusWrapper::addTimeoutDelegate(DBusTimeout *timeout, void* userData)
{
    if (!dbus_timeout_get_enabled(timeout))
        return false;

    //calculate the timeout in timeval
    timespec pollTimeout;
    int localTimeout = dbus_timeout_get_interval(timeout);
    pollTimeout.tv_sec = localTimeout / 1000;
    pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;

    //prepare handle and callback. new is eval, but there is no other choice because we need the pointer!
    sh_timerHandle_t* handle = new sh_timerHandle_t;
    mListTimerhandlePointer.push_back(handle);
    shTimerCallBack* buffer = &pDbusTimerCallback;

    //add the timer to the pollLoop
    mSocketHandler->addTimer(pollTimeout, buffer, *handle, timeout);

    //save the handle with dbus context
    dbus_timeout_set_data(timeout, handle, NULL);

    //save timeout in Socket context
    userData = timeout;
    logInfo("DBusWrapper::addTimeoutDelegate a timeout was added");
    return true;
}

void DBusWrapper::removeTimeout(DBusTimeout *timeout, void* userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    mReference->removeTimeoutDelegate(timeout, userData);
}

void DBusWrapper::removeTimeoutDelegate(DBusTimeout *timeout, void* userData)
{
    (void) userData;
    //get the pointer to the handle and remove the timer
    sh_timerHandle_t* handle = (sh_timerHandle_t*) dbus_timeout_get_data(timeout);
    mSocketHandler->removeTimer(*handle);

    //now go throught the timerlist and remove the pointer, free memory
    std::vector<sh_timerHandle_t*>::iterator it = mListTimerhandlePointer.begin();
    for (; it != mListTimerhandlePointer.end(); ++it)
    {
        if (*it == handle)
        {
            mListTimerhandlePointer.erase(it);
            break;
        }
    }
    delete handle;
    logInfo("DBusWrapper::removeTimeoutDelegate a timeout was removed");
}

void DBusWrapper::toggleTimeout(DBusTimeout *timeout, void* userData)
{
    mReference = (DBusWrapper*) userData;
    assert(mReference!=0);
    mReference->toggleTimeoutDelegate(timeout, userData);
}

bool am::DBusWrapper::dbusDispatchCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    bool returnVal = true;
    dbus_connection_ref(mDbusConnection);
    if (dbus_connection_dispatch(mDbusConnection) == DBUS_DISPATCH_COMPLETE)
        returnVal = false;
    dbus_connection_unref(mDbusConnection);
//    logInfo("DBusWrapper::dbusDispatchCallback was called");
    return returnVal;
}

bool am::DBusWrapper::dbusCheckCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    bool returnVal = false;
    dbus_connection_ref(mDbusConnection);
    if (dbus_connection_get_dispatch_status(mDbusConnection) == DBUS_DISPATCH_DATA_REMAINS)
        returnVal = true;
    dbus_connection_unref(mDbusConnection);
//    logInfo("DBusWrapper::dbusCheckCallback was called");
    return returnVal;
}

void am::DBusWrapper::dbusFireCallback(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
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

    dbus_connection_ref(mDbusConnection);
    dbus_watch_handle(watch, flags);
    dbus_connection_unref(mDbusConnection);
//    logInfo("DBusWrapper::dbusFireCallback was called");
}

void DBusWrapper::toggleTimeoutDelegate(DBusTimeout *timeout, void* userData)
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
        mSocketHandler->restartTimer(*handle, pollTimeout);
    }
    else
    {
        mSocketHandler->stopTimer(*handle);
    }logInfo("DBusWrapper::toggleTimeoutDelegate was called");
}

void DBusWrapper::dbusTimerCallback(sh_timerHandle_t handle, void *userData)
{
    assert(userData!=NULL);
    if (dbus_timeout_get_enabled((DBusTimeout*) userData))
    {
        timespec ts;
        ts.tv_nsec = -1;
        ts.tv_sec = -1;
        mSocketHandler->restartTimer(handle, ts);
    }
    dbus_timeout_handle((DBusTimeout*) userData);
    logInfo("DBusWrapper::dbusTimerCallback was called");
}

