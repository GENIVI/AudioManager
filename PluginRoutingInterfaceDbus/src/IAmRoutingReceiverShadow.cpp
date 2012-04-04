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


#include <audiomanagertypes.h>
#include <string.h>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include "IAmRoutingReceiverShadow.h"
#include "CAmRoutingSenderDbus.h"
#include "shared/CAmDltWrapper.h"

using namespace am;

DLT_IMPORT_CONTEXT(routingDbus)

/**
 * static ObjectPathTable is needed for DBus Callback handling
 */
static DBusObjectPathVTable gObjectPathVTable;

IAmRoutingReceiverShadow::IAmRoutingReceiverShadow()
: 	mRoutingReceiveInterface(NULL),
  	mDBusWrapper(NULL),
	mFunctionMap(createMap()),
  	mDBUSMessageHandler()
{
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow constructed");
}

IAmRoutingReceiverShadow::~IAmRoutingReceiverShadow()
{
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow destructed");
}


void IAmRoutingReceiverShadow::registerDomain(DBusConnection *conn, DBusMessage *msg)
{
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerDomain called");

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        am_Domain_s mDomain;
        mDomain.domainID = 0;

        char *name = mDBUSMessageHandler.getString();
        char *nodename = mDBUSMessageHandler.getString();
        mDomain.early = mDBUSMessageHandler.getBool();
        mDomain.state = (am_DomainState_e)mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerDomain called, name", name,"nodename", nodename,"mDomain.early", mDomain.early, "mDomain.state", mDomain.state );

        mDomain.name = std::string(name);
        mDomain.nodename = std::string(nodename);
        mDomain.busname = "pulsePlugin";
        mDomain.complete = 0; // will be set once hookdomainregistration event is received

	// pass the data to the controller and the database
        am_Error_e returnCode = mRoutingReceiveInterface->registerDomain(mDomain, mDomain.domainID);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerDomain domain ID, mDomain.domainID", mDomain.domainID );
        mDBUSMessageHandler.initReply(msg);
        mDBUSMessageHandler.append((dbus_uint32_t)mDomain.domainID);
        mDBUSMessageHandler.sendMessage();
        if (returnCode != E_OK)
        {
            log(&routingDbus, DLT_LOG_INFO, "error registering domain" );
        }
}

void IAmRoutingReceiverShadow::registerSource(DBusConnection *conn, DBusMessage *msg)
{
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::RegisterSource called");

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        am_Source_s mSource;

        char *name = mDBUSMessageHandler.getString();
	mSource.sourceID = 0;
        mSource.sourceClassID = mDBUSMessageHandler.getInt32();
        mSource.domainID = mDBUSMessageHandler.getInt32();
        mSource.sourceState = SS_ON;
        mSource.listConnectionFormats.push_back(CF_GENIVI_STEREO);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerSource called, name", name,"mSource.sourceClassID", mSource.sourceClassID,"mSource.domainID", mSource.domainID );

        mSource.name = std::string(name);

        am_Error_e returnCode = mRoutingReceiveInterface->registerSource(mSource, mSource.sourceID);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registersource,  mSource.sourceID", mSource.sourceID );
        mDBUSMessageHandler.initReply(msg);
        mDBUSMessageHandler.append((dbus_uint32_t)mSource.sourceID);
        mDBUSMessageHandler.sendMessage();
        if (returnCode != E_OK)
        {
            log(&routingDbus, DLT_LOG_INFO, "error registering source" );
        }

}

void IAmRoutingReceiverShadow::registerSink(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        am_Sink_s mSink;

        char *name = mDBUSMessageHandler.getString();
	mSink.sinkID = 0;
        mSink.sinkClassID = mDBUSMessageHandler.getInt32();
        mSink.domainID = mDBUSMessageHandler.getInt32();
        mSink.muteState = MS_UNMUTED;
        mSink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerSink called, name", name,"mSink.sinkClassID", mSink.sinkClassID,"mSink.domainID", mSink.domainID );

        mSink.name = std::string(name);

        am_Error_e returnCode = mRoutingReceiveInterface->registerSink(mSink, mSink.sinkID);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registersink, mSink.sinkID", mSink.sinkID );
        mDBUSMessageHandler.initReply(msg);
        mDBUSMessageHandler.append((dbus_uint32_t)mSink.sinkID);
        mDBUSMessageHandler.sendMessage();
        if (returnCode != E_OK)
        {
            log(&routingDbus, DLT_LOG_INFO, "error registering sink" );
        }

}

void IAmRoutingReceiverShadow::registerGateway(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        am_Gateway_s mGateway;
	mGateway.gatewayID = 0;

        char *name = mDBUSMessageHandler.getString();
        char *sink = mDBUSMessageHandler.getString();
        char *source = mDBUSMessageHandler.getString();
        char *domainsource = mDBUSMessageHandler.getString();
        char *domainsink = mDBUSMessageHandler.getString();
        char *controldomain = mDBUSMessageHandler.getString();
	mGateway.listSinkFormats.push_back(CF_GENIVI_STEREO);
	mGateway.listSourceFormats.push_back(CF_GENIVI_STEREO);
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::registerGateway called, name", name,"sink", sink,"source", source );
        log(&routingDbus, DLT_LOG_INFO, "domainsource", domainsource,"domainsink", domainsink,"controldomain", controldomain );

        mGateway.name = std::string(name);
        mGateway.sinkID = 101;
        mGateway.sourceID = 101;
        mGateway.domainSourceID = 1;
        mGateway.domainSinkID = 1;
        mGateway.controlDomainID = 1;
	mGateway.convertionMatrix.push_back(true);

        am_Error_e returnCode = mRoutingReceiveInterface->registerGateway(mGateway, mGateway.gatewayID);
        mDBUSMessageHandler.initReply(msg);
        mDBUSMessageHandler.append((dbus_uint32_t)mGateway.gatewayID);
        mDBUSMessageHandler.sendMessage();
        if (returnCode != E_OK)
        {
            log(&routingDbus, DLT_LOG_INFO, "error registering gateway" );
        }

}

void IAmRoutingReceiverShadow::hookDomainRegistrationComplete(DBusConnection *conn, DBusMessage *msg)
{
        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int domainID = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::hookDomainRegistrationComplete called, domainID", domainID);
        mRoutingReceiveInterface->hookDomainRegistrationComplete((am_domainID_t)domainID);

}

void IAmRoutingReceiverShadow::ackConnect(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);
        int handle = mDBUSMessageHandler.getInt32();
        int conid = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();

        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackConnect called, handle", handle, "conid", conid, "error", error);

        am_Handle_s myhandle;
        myhandle.handleType = H_CONNECT;
        myhandle.handle = handle;
        am_connectionID_t mconnectionid = (am_connectionID_t)conid;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackConnect(myhandle,mconnectionid, merror);
}



void IAmRoutingReceiverShadow::ackDisconnect(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int conid = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackDisconnect called, handle", handle, "conid", conid, "error", error);

        am_Handle_s myhandle;
        myhandle.handleType = H_DISCONNECT;
        myhandle.handle = handle;
        am_connectionID_t mconnectionid = conid;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackDisconnect(myhandle,mconnectionid, merror);

}

void IAmRoutingReceiverShadow::ackSetSinkVolume(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int volume = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSetSinkVolume called, handle", handle, "error", error, "volume", volume);

        am_Handle_s myhandle;
        myhandle.handleType = H_SETSINKVOLUME;
        myhandle.handle = handle;
        am_volume_t mvolume = volume;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackSetSinkVolumeChange(myhandle,mvolume, merror);

}

void IAmRoutingReceiverShadow::ackSinkVolumeTick(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int sinkID = mDBUSMessageHandler.getInt32();
        int volume = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSinkVolumeTick called, handle", handle, "sinkID", sinkID, "volume", volume);

        am_Handle_s myhandle;
        myhandle.handleType = H_SETSINKVOLUME;
        myhandle.handle = handle;
        am_volume_t mvolume = volume;
        mRoutingReceiveInterface->ackSinkVolumeTick(myhandle,sinkID, mvolume);

}

void IAmRoutingReceiverShadow::ackSourceVolumeTick(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int sourceID = mDBUSMessageHandler.getInt32();
        int volume = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSourceVolumeTick called, handle", handle, "sourceID", sourceID, "volume", volume);

        am_Handle_s myhandle;
        myhandle.handleType = H_SETSINKVOLUME;
        myhandle.handle = handle;
        am_volume_t mvolume = volume;
        mRoutingReceiveInterface->ackSourceVolumeTick(myhandle,sourceID, mvolume);

}

void IAmRoutingReceiverShadow::ackSetSourceVolume(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int volume = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSetSourceVolume called, handle", handle, "volume", volume, "error", error);

        am_Handle_s myhandle;
        myhandle.handleType = H_SETSOURCEVOLUME;
        myhandle.handle = handle;
        am_volume_t mvolume = volume;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackSetSourceVolumeChange(myhandle,mvolume, merror);

}

void IAmRoutingReceiverShadow::ackSetSinkSoundProperty(DBusConnection *conn, DBusMessage *msg)
{

        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSetSinkSoundProperty called, handle", handle, "error", error);

        am_Handle_s myhandle;
        myhandle.handleType = H_SETSINKSOUNDPROPERTY;
        myhandle.handle = handle;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackSetSinkSoundProperty(myhandle, merror);

}

void IAmRoutingReceiverShadow::ackSetSourceSoundProperty(DBusConnection *conn, DBusMessage *msg)
{
        (void) conn;
        assert(mRoutingReceiveInterface!=NULL);

        mDBUSMessageHandler.initReceive(msg);

        int handle = mDBUSMessageHandler.getInt32();
        int error = mDBUSMessageHandler.getInt32();
        log(&routingDbus, DLT_LOG_INFO, "IAmRoutingReceiverShadow::ackSetSourceSoundProperty called, handle", handle, "error", error);


        am_Handle_s myhandle;
        myhandle.handleType = H_SETSOURCESOUNDPROPERTY;
        myhandle.handle = handle;
        am_Error_e merror = (am_Error_e) error;
        mRoutingReceiveInterface->ackSetSourceSoundProperty(myhandle, merror);

}

DBusHandlerResult IAmRoutingReceiverShadow::receiveCallback(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	assert(conn!=NULL);
	assert(msg!=NULL);
	assert(user_data!=NULL);
        IAmRoutingReceiverShadow* reference=(IAmRoutingReceiverShadow*) user_data;
	return (reference->receiveCallbackDelegate(conn,msg));
}

void IAmRoutingReceiverShadow::sendIntrospection(DBusConnection *conn, DBusMessage *msg)
{
	assert(conn!=NULL);
	assert(msg!=NULL);
    DBusMessage* reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;

    // create a reply from the message
    reply = dbus_message_new_method_return(msg);
    //std::ifstream in(ROUTING_XML_FILE);
    //assert(in!=NULL);
    std::ifstream in("RoutingReceiver.xml", std::ifstream::in);
    if (!in)
    {
        logError("IAmCommandReceiverShadow::sendIntrospection could not load xml file");
        throw std::runtime_error("IAmCommandReceiverShadow::sendIntrospection Could not load introspecton XML");
    }
    std::string introspect((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const char* string = introspect.c_str();
    log(&routingDbus, DLT_LOG_INFO, introspect.c_str());

    // add the arguments to the reply
    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
    {
        log(&routingDbus, DLT_LOG_INFO, "DBUS handler Out Of Memory!");
    }

    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &serial))
    {
        log(&routingDbus, DLT_LOG_INFO, "DBUS handler Out Of Memory!");
    }
    dbus_connection_flush(conn);

    // free the reply
    dbus_message_unref(reply);
}

DBusHandlerResult IAmRoutingReceiverShadow::receiveCallbackDelegate(DBusConnection *conn, DBusMessage *msg)
{

	if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
		sendIntrospection(conn,msg);
		return (DBUS_HANDLER_RESULT_HANDLED);
	}

	functionMap_t::iterator iter = mFunctionMap.begin();
	std::string k(dbus_message_get_member(msg));
        log(&routingDbus, DLT_LOG_INFO, k.c_str());
        iter=mFunctionMap.find(k);
    if (iter != mFunctionMap.end())
    {
    	std::string p(iter->first);
    	CallBackMethod cb=iter->second;
    	(this->*cb)(conn,msg);
    	return (DBUS_HANDLER_RESULT_HANDLED);
    }

	return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

void IAmRoutingReceiverShadow::setRoutingReceiver(IAmRoutingReceive*& receiver)
{
	assert(receiver!=NULL);
        mRoutingReceiveInterface=receiver;

        gObjectPathVTable.message_function=IAmRoutingReceiverShadow::receiveCallback;

	DBusConnection* connection;
        mRoutingReceiveInterface->getDBusConnectionWrapper(mDBusWrapper);
	assert(mDBusWrapper!=NULL);

	mDBusWrapper->getDBusConnection(connection);
	assert(connection!=NULL);
	mDBUSMessageHandler.setDBusConnection(connection);

        std::string path(ROUTING_NODE);
	mDBusWrapper->registerCallback(&gObjectPathVTable,path,this);

}

IAmRoutingReceiverShadow::functionMap_t IAmRoutingReceiverShadow::createMap()
{
	functionMap_t m;
        m["ackConnect"]=&IAmRoutingReceiverShadow::ackConnect ;
        m["ackDisconnect"]=&IAmRoutingReceiverShadow::ackDisconnect ;
        m["ackSetSinkVolume"]=&IAmRoutingReceiverShadow::ackSetSinkVolume ;
        m["ackSetSourceVolume"]=&IAmRoutingReceiverShadow::ackSetSourceVolume ;
        m["ackSinkVolumeTick"]=&IAmRoutingReceiverShadow::ackSinkVolumeTick ;
        m["ackSourceVolumeTick"]=&IAmRoutingReceiverShadow::ackSourceVolumeTick ;
        m["ackSetSinkSoundProperty"]=&IAmRoutingReceiverShadow::ackSetSinkSoundProperty ;
        m["ackSetSourceSoundProperty"]=&IAmRoutingReceiverShadow::ackSetSourceSoundProperty ;

        m["registerDomain"]=&IAmRoutingReceiverShadow::registerDomain ;
        m["registerSource"]=&IAmRoutingReceiverShadow::registerSource ;
        m["registerSink"]=&IAmRoutingReceiverShadow::registerSink ;
        m["registerGateway"]=&IAmRoutingReceiverShadow::registerGateway ;

        m["hookDomainRegistrationComplete"]=&IAmRoutingReceiverShadow::hookDomainRegistrationComplete;
	return (m);
}



