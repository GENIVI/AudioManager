/*
 * main.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

//todo: add debug commandline option to allow to use other than memory database
//todo: make real daemon out of it- systemd conform

#include "control/ControlReceiveInterface.h"
#include "RoutingReceiver.h"
#include "CommandReceiver.h"
#include "ControlReceiver.h"
#include "DatabaseHandler.h"
#include "ControlSender.h"
#include "CommandSender.h"
#include "RoutingSender.h"
#include "DBusWrapper.h"
#include <dbus/dbus.h>
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(AudioManager)

#define UNIT_TEST 1

using namespace am;

int main(int argc, char *argv[])
{
	DLT_REGISTER_APP("AudioManagerDeamon","AudioManagerDeamon");
	DLT_REGISTER_CONTEXT(AudioManager,"Main","Main Context");
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("The AudioManager is started "));

	DatabaseHandler iDatabaseHandler;
	DBusWrapper iDBusWrapper;
	RoutingSender iRoutingSender;
	CommandSender iCommandSender;
	ControlSender iControlSender;
	Observer iObserver(&iCommandSender, &iRoutingSender);
	CommandReceiver iCommandReceiver(&iDatabaseHandler,&iDBusWrapper,&iControlSender);
	RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender);
	ControlReceiver iControlReceiver(&iDatabaseHandler, &iRoutingSender);

	iCommandSender.startupInterface(&iCommandReceiver);

	iDBusWrapper.dbusMainLoop();

}




