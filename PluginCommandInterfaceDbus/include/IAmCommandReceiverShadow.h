/**
 *  Copyright (c) 2012 BMW
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
 *  For further information see http://www.genivi.org/.
 */

#ifndef COMMANDRECEIVERSHADOW_H_
#define COMMANDRECEIVERSHADOW_H_



#include <dbus/dbus.h>
#include <map>
#include "config.h"
#include "command/IAmCommandReceive.h"
#include "CAmDbusMessageHandler.h"
#include "shared/CAmDbusWrapper.h"

namespace am
{

class IAmCommandReceiverShadow;

typedef void (IAmCommandReceiverShadow::*CallBackMethod)(DBusConnection *connection, DBusMessage *message);

/**
 * receives the DBus Callbacks, marhsalls and demarshalls the parameters and calls CommandReceive
 */
class IAmCommandReceiverShadow
{
public:
    IAmCommandReceiverShadow();
    virtual ~IAmCommandReceiverShadow();
    void connect(DBusConnection *conn, DBusMessage *msg);
    void disconnect(DBusConnection *conn, DBusMessage *msg);
    void setVolume(DBusConnection *conn, DBusMessage *msg);
    void volumeStep(DBusConnection *conn, DBusMessage *msg);
    void setSinkMuteState(DBusConnection *conn, DBusMessage *msg);
    void setMainSinkSoundProperty(DBusConnection *conn, DBusMessage *msg);
    void setMainSourceSoundProperty(DBusConnection *conn, DBusMessage *msg);
    void setSystemProperty(DBusConnection *conn, DBusMessage *msg);
    void getListMainConnections(DBusConnection *conn, DBusMessage *msg);
    void getListMainSinks(DBusConnection *conn, DBusMessage *msg);
    void getListMainSources(DBusConnection *conn, DBusMessage *msg);
    void getListMainSinkSoundProperties(DBusConnection *conn, DBusMessage *msg);
    void getListMainSourceSoundProperties(DBusConnection *conn, DBusMessage *msg);
    void getListSourceClasses(DBusConnection *conn, DBusMessage *msg);
    void getListSinkClasses(DBusConnection *conn, DBusMessage *msg);
    void getListSystemProperties(DBusConnection *conn, DBusMessage *msg);
    void getTimingInformation(DBusConnection *conn, DBusMessage *msg);

    /**
     * sets the pointer to the CommandReceiveInterface and registers Callback
     * @param receiver
     */
    void setCommandReceiver(IAmCommandReceive*& receiver);
private:
    typedef std::map<std::string, CallBackMethod> functionMap_t;
    functionMap_t mFunctionMap;
    CAmDbusMessageHandler mDBUSMessageHandler;
    IAmCommandReceive* mpIAmCommandReceive;
    CAmDbusWrapper* mpCAmDbusWrapper;

    /**
     * receives a callback whenever the path of the plugin is called
     */
    static DBusHandlerResult receiveCallback(DBusConnection *conn, DBusMessage *msg, void *user_data);

    /**
     * dynamic delegate that handles the Callback of the static receiveCallback
     * @param conn DBus connection
     * @param msg DBus message
     * @param user_data pointer to instance of CommandReceiverShadow
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

#endif /* COMMANDRECEIVERSHADOW_H_ */
