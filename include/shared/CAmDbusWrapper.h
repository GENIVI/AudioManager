/** Copyright (c) 2012 GENIVI Alliance
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, BMW
 *
 *  \section license
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef DBUSWRAPPER_H_
#define DBUSWRAPPER_H_

#include <config.h>
#include <dbus/dbus.h>
#include <string>
#include <list>
#include "shared/CAmSocketHandler.h"

namespace am
{
/**
 * This wraps dbus and provides everything needed to anyone who wants to use dbus (including plugins)
 */
class CAmDbusWrapper
{
public:
    CAmDbusWrapper(CAmSocketHandler* socketHandler);
    virtual ~CAmDbusWrapper();

    /**
     * registers a callback that is entered as path below the main path.
     * The configuration of the mainpath is done via DBusConfiguration.h
     * @param vtable the vtable that holds a pointer to the callback that is called when the path is called from the dbus
     * @param path the name of the path
     * @param userdata pointer to the class that will handle the callback
     */
    void registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata);

    /**
     * returns the dbus connection
     * @param connection pointer to the connection
     */
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
    static CAmDbusWrapper* mpReference;
    static DBusHandlerResult cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference);
    dbus_bool_t addWatchDelegate(DBusWatch * watch, void* userData);
    void removeWatchDelegate(DBusWatch *watch, void *userData);
    void toogleWatchDelegate(DBusWatch *watch, void *userData);
    dbus_bool_t addTimeoutDelegate(DBusTimeout *timeout, void* userData);
    void removeTimeoutDelegate(DBusTimeout *timeout, void* userData);
    void toggleTimeoutDelegate(DBusTimeout *timeout, void* userData);
    DBusObjectPathVTable mObjectPathVTable;
    DBusConnection* mpDbusConnection;
    DBusError mDBusError;
    std::list<std::string> mListNodes;
    std::vector<sh_timerHandle_t*> mpListTimerhandles;
    CAmSocketHandler *mpSocketHandler;
    std::map<DBusWatch*, sh_pollHandle_t> mMapHandleWatch;
};

}

#endif /* DBUSWRAPPER_H_ */
