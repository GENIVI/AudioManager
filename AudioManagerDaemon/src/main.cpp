/*
 * main.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

//todo: add debug commandline option to allow to use other than memory database
#include "RoutingReceiver.h"
#include "CommandReceiver.h"
#include "ControlReceiver.h"
#include "DatabaseHandler.h"
#include "control/ControlSendInterface.h"
#include "DBusWrapper.h"
#include "ControlLoader.h"
#include "CommandSender.h"
#include "RoutingSender.h"
#include <dbus/dbus.h>
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(AudioManager);

using namespace am;

int main(int argc, char *argv[])
{
	DLT_REGISTER_APP("AudioManagerDeamon","AudioManagerDeamon");
	DLT_REGISTER_CONTEXT(AudioManager,"Main","Main Context");
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("The AudioManager is started "));

	DatabaseHandler iDatabaseHandler;
	DBusWrapper iDBusWrapper;
	CommandReceiver iCommandReceiver(&iDatabaseHandler,&iDBusWrapper);
	RoutingReceiver iRoutingReceiver;
	ControlReceiver iControlReceiver(&iDatabaseHandler);
	RoutingSender iRoutingSender;
	CommandSender iCommandSender;

	am_Connection_s lowCon;
	am_connectionID_t cID;
	lowCon.connectionID=0;
	lowCon.sinkID=2;
	lowCon.sourceID=3;
	lowCon.connectionFormat=CF_ANALOG;
	lowCon.delay=-1;
	iDatabaseHandler.enterConnectionDB(lowCon,cID);

	am_RoutingElement_s re;
	re.connectionFormat=CF_ANALOG;
	re.domainID=1;
	re.sinkID=2;
	re.sourceID=3;

	am_MainConnection_s con;
	am_mainConnectionID_t mainC;
	con.connectionID=0;
	con.connectionState=CS_CONNECTING;
	con.route.sinkID=2;
	con.route.sourceID=3;
	con.route.route.push_back(re);
	iControlReceiver.enterMainConnectionDB(con,mainC);
	//ControlLoader iControlLoader;
//	ControlSendInterface* iControlSender =iControlLoader.returnControl();


	iCommandSender.startupInterface(&iCommandReceiver);

	iDBusWrapper.dbusMainLoop();

}




