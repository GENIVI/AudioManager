/**
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *  Copyright (c) 2012 BMW
 *
 *  \author Sampreeth Ramavana
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
 *  For further information see http://www.genivi.org/.
 */

#ifndef _IAMROUTINGRECEIVERSHADOW_H_
#define _IAMROUTINGRECEIVERSHADOW_H_

#include <dbus/dbus.h>
#include <map>
#include "CAmDbusMessageHandler.h"

namespace am {

class CAmRoutingSenderDbus;
class IAmRoutingReceive;
class CAmDbusWrapper;

/**
 * receives the DBus Callbacks, marhsalls and demarshalls the parameters and calls CommandReceive
 */
class IAmRoutingReceiverShadowDbus
{
public:
    IAmRoutingReceiverShadowDbus(CAmRoutingSenderDbus* pRoutingSenderDbus);
    virtual ~IAmRoutingReceiverShadowDbus();
    void ackConnect(DBusConnection *conn, DBusMessage *msg);
    void ackDisconnect(DBusConnection *conn, DBusMessage *msg);
    void ackSetSinkVolume(DBusConnection *conn, DBusMessage *msg);
    void ackSetSourceVolume(DBusConnection *conn, DBusMessage *msg);
    void ackSetSourceState(DBusConnection *conn, DBusMessage *msg);
    void ackSinkVolumeTick(DBusConnection *conn, DBusMessage *msg);
    void ackSourceVolumeTick(DBusConnection *conn, DBusMessage *msg);
    void ackSetSinkSoundProperty(DBusConnection *conn, DBusMessage *msg);
    void ackSetSourceSoundProperty(DBusConnection *conn, DBusMessage *msg);
    void ackSetSinkSoundProperties(DBusConnection *conn, DBusMessage *msg);
    void ackSetSourceSoundProperties(DBusConnection *conn, DBusMessage *msg);
    void ackCrossFading(DBusConnection *conn, DBusMessage *msg);
    void registerDomain(DBusConnection *conn, DBusMessage *msg);
    void registerSource(DBusConnection *conn, DBusMessage *msg);
    void registerSink(DBusConnection *conn, DBusMessage *msg);
    void registerGateway(DBusConnection *conn, DBusMessage *msg);
    void peekDomain(DBusConnection *conn, DBusMessage *msg);
    void deregisterDomain(DBusConnection *conn, DBusMessage *msg);
    void deregisterGateway(DBusConnection *conn, DBusMessage *msg);
    void peekSink(DBusConnection *conn, DBusMessage *msg);
    void deregisterSink(DBusConnection *conn, DBusMessage *msg);
    void peekSource(DBusConnection *conn, DBusMessage *msg);
    void deregisterSource(DBusConnection *conn, DBusMessage *msg);
    void registerCrossfader(DBusConnection *conn, DBusMessage *msg);
    void deregisterCrossfader(DBusConnection *conn, DBusMessage *msg);
    void peekSourceClassID(DBusConnection *conn, DBusMessage *msg);
    void peekSinkClassID(DBusConnection *conn, DBusMessage *msg);
    void hookInterruptStatusChange(DBusConnection *conn, DBusMessage *msg);
    void hookDomainRegistrationComplete(DBusConnection *conn, DBusMessage *msg);
    void hookSinkAvailablityStatusChange(DBusConnection *conn, DBusMessage *msg);
    void hookSourceAvailablityStatusChange(DBusConnection *conn, DBusMessage *msg);
    void hookDomainStateChange(DBusConnection *conn, DBusMessage *msg);
    void hookTimingInformationChanged(DBusConnection *conn, DBusMessage *msg);
    void sendChangedData(DBusConnection *conn, DBusMessage *msg);
    void confirmRoutingReady(DBusConnection *conn, DBusMessage *msg);
    void confirmRoutingRundown(DBusConnection *conn, DBusMessage *msg);

    /**
     * sets the pointer to the CommandReceiveInterface and registers Callback
     * @param receiver
     */
    void setRoutingReceiver(IAmRoutingReceive*& receiver);

    void gotReady(int16_t numberDomains, uint16_t handle);
    void gotRundown(int16_t numberDomains, uint16_t handle);

private:
    typedef void (IAmRoutingReceiverShadowDbus::*CallBackMethod)(DBusConnection *connection, DBusMessage *message);
    IAmRoutingReceive* mRoutingReceiveInterface;
    CAmDbusWrapper* mDBusWrapper;
    CAmRoutingSenderDbus* mpRoutingSenderDbus;

    typedef std::map<std::string, CallBackMethod> functionMap_t;
    functionMap_t mFunctionMap;
    CAmRoutingDbusMessageHandler mDBUSMessageHandler;
    int16_t mNumberDomains;
    uint16_t mHandle;

    /**
     * receives a callback whenever the path of the plugin is called
     */
    static DBusHandlerResult receiveCallback(DBusConnection *conn, DBusMessage *msg, void *user_data);

    /**
     * dynamic delegate that handles the Callback of the static receiveCallback
     * @param conn DBus connection
     * @param msg DBus message
     * @param user_data pointer to instance of IAmRoutingReceiverShadow
     * @return
     */
    DBusHandlerResult receiveCallbackDelegate(DBusConnection *conn, DBusMessage *msg);

    /**
     * sends out introspectiondata read from an xml file.
     * @param conn
     * @param msg
     */
    void sendIntrospection(DBusConnection* conn, DBusMessage* msg);

    /**
     * creates the function map needed to combine DBus messages and function adresses
     * @return the map
     */
    functionMap_t createMap();
};

}

#endif /* _IAMROUTINGRECEIVERSHADOW_H_ */
