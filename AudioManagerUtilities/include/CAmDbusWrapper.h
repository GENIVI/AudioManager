/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  \file CAmDbusWrapper.h
 *  For further information see http://www.genivi.org/.
 */

/**
 * todo add removeCallback ...
 */

#ifndef DBUSWRAPPER_H_
#define DBUSWRAPPER_H_

#include <dbus/dbus.h>
#include <string>
#include <list>
#include <map>
#include "audiomanagerconfig.h"
#include "CAmSocketHandler.h"

namespace am
{

/**
 * This wraps dbus and provides everything needed to anyone who wants to use dbus (including plugins). Works on the basis of CAmSocketHandler
 */
class CAmDbusWrapper
{
public:
    CAmDbusWrapper(CAmSocketHandler* socketHandler,DBusBusType type=DBUS_BUS_SESSION,
            const std::string& prefix = DBUS_SERVICE_PREFIX, const std::string& objectPath = DBUS_SERVICE_OBJECT_PATH);
    virtual ~CAmDbusWrapper();

    void registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata, const std::string& prefix = DBUS_SERVICE_OBJECT_PATH);
    void registerSignalWatch(DBusHandleMessageFunction handler, const std::string& rule, void* userdata);
    void getDBusConnection(DBusConnection*& connection) const;

    static dbus_bool_t addWatch(DBusWatch *watch, void *userData);
    static void removeWatch(DBusWatch *watch, void *userData);
    static void toogleWatch(DBusWatch *watch, void *userData);

    static dbus_bool_t addTimeout(DBusTimeout *timeout, void* userData);
    static void removeTimeout(DBusTimeout *timeout, void* userData);
    static void toggleTimeout(DBusTimeout *timeout, void* userData);

    void dbusPrepareCallback(const sh_pollHandle_t handle, void* userData);
    TAmShPollPrepare<CAmDbusWrapper> pDbusPrepareCallback;

    bool dbusDispatchCallback(const sh_pollHandle_t handle, void* userData);
    TAmShPollDispatch<CAmDbusWrapper> pDbusDispatchCallback;

    void dbusFireCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
    TAmShPollFired<CAmDbusWrapper> pDbusFireCallback;

    bool dbusCheckCallback(const sh_pollHandle_t handle, void* userData);
    TAmShPollCheck<CAmDbusWrapper> pDbusCheckCallback;

    void dbusTimerCallback(sh_timerHandle_t handle, void* userData);
    TAmShTimerCallBack<CAmDbusWrapper> pDbusTimerCallback;

private:
    static CAmDbusWrapper* mpReference; //!< reference to the dbus instance
    static DBusHandlerResult cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference);
    dbus_bool_t addWatchDelegate(DBusWatch * watch, void* userData);
    void removeWatchDelegate(DBusWatch *watch, void *userData);
    void toogleWatchDelegate(DBusWatch *watch, void *userData);
    dbus_bool_t addTimeoutDelegate(DBusTimeout *timeout, void* userData);
    void removeTimeoutDelegate(DBusTimeout *timeout, void* userData);
    void toggleTimeoutDelegate(DBusTimeout *timeout, void* userData);
    DBusObjectPathVTable mObjectPathVTable; //!< the vpathtable
    DBusConnection* mpDbusConnection; //!< pointer to the dbus connection used
    DBusError mDBusError; //!< dbuserror
    std::vector<std::string> mListNodes; //!< holds a list of all nodes of the dbus
    std::vector<sh_timerHandle_t*> mpListTimerhandles; //!< pointer to the timer handles
    CAmSocketHandler *mpSocketHandler; //!< pointer to the sockethandler
    std::map<DBusWatch*, sh_pollHandle_t> mMapHandleWatch; //!< map to the handle watches
    DBusBusType mDbusType;
};

}

#endif /* DBUSWRAPPER_H_ */
