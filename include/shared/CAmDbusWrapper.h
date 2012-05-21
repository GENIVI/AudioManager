/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include "config.h"
#include "CAmSocketHandler.h"

namespace am
{

/**
 * This wraps dbus and provides everything needed to anyone who wants to use dbus (including plugins). Works on the basis of CAmSocketHandler
 */
class CAmDbusWrapper
{
public:
    CAmDbusWrapper(CAmSocketHandler* socketHandler,DBusBusType type=DBUS_BUS_SESSION);
    virtual ~CAmDbusWrapper();

    void registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata);
    void getDBusConnection(DBusConnection*& connection) const;

    static dbus_bool_t addWatch(DBusWatch *watch, void *userData);
    static void removeWatch(DBusWatch *watch, void *userData);
    static void toogleWatch(DBusWatch *watch, void *userData);

    static dbus_bool_t addTimeout(DBusTimeout *timeout, void* userData);
    static void removeTimeout(DBusTimeout *timeout, void* userData);
    static void toggleTimeout(DBusTimeout *timeout, void* userData);

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
