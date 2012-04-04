/**
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *
 *  \author: Sampreeth Ramavana
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

#include "routing/IAmRoutingSend.h"
#include <dbus/dbus.h>
#include <dlt/dlt.h>
#include <map>
#include "shared/CAmDbusWrapper.h"
#include "CAmDbusMessageHandler.h"

namespace am {

class IAmRoutingReceiverShadow;

typedef void (IAmRoutingReceiverShadow::*CallBackMethod)(DBusConnection *connection, DBusMessage *message);

/**
 * receives the DBus Callbacks, marhsalls and demarshalls the parameters and calls CommandReceive
 */
class IAmRoutingReceiverShadow {
public:
        IAmRoutingReceiverShadow();
        virtual ~IAmRoutingReceiverShadow();
        void ackConnect(DBusConnection *conn, DBusMessage *msg) ;
        void ackDisconnect(DBusConnection *conn, DBusMessage *msg) ;
        void ackSetSinkVolume(DBusConnection *conn, DBusMessage *msg);
        void ackSetSourceVolume(DBusConnection *conn, DBusMessage *msg);
        void ackSinkVolumeTick(DBusConnection *conn, DBusMessage *msg);
        void ackSourceVolumeTick(DBusConnection *conn, DBusMessage *msg);
        void ackSetSinkSoundProperty(DBusConnection *conn, DBusMessage *msg);
        void ackSetSourceSoundProperty(DBusConnection *conn, DBusMessage *msg);

        void registerDomain(DBusConnection *conn, DBusMessage *msg) ;
        void registerSource(DBusConnection *conn, DBusMessage *msg) ;
        void registerSink(DBusConnection *conn, DBusMessage *msg) ;
        void registerGateway(DBusConnection *conn, DBusMessage *msg) ;
        void hookDomainRegistrationComplete(DBusConnection *conn, DBusMessage *msg);

	/**
	 * sets the pointer to the CommandReceiveInterface and registers Callback
	 * @param receiver
	 */
        void setRoutingReceiver(IAmRoutingReceive*& receiver);
private:
        IAmRoutingReceive* mRoutingReceiveInterface;
        CAmDbusWrapper* mDBusWrapper;
	typedef std::map<std::string,CallBackMethod> functionMap_t;
	functionMap_t mFunctionMap;
        CAmDbusMessageHandler mDBUSMessageHandler;

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
	void sendIntrospection(DBusConnection* conn, DBusMessage* msg) ;

	/**
	 * creates the function map needed to combine DBus messages and function adresses
	 * @return the map
	 */
	functionMap_t createMap();
};

}

#endif /* _IAMROUTINGRECEIVERSHADOW_H_ */
