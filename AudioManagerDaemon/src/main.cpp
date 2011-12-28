/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file main.cpp
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

/**
 * Please make sure to have read the documentation on genivi.org!
 */

//todo: add debug commandline option to allow to use other than memory database
//todo: make real daemon out of it- systemd conform
//todo: versioning of PluginInterfaces on linux level (.symver stuff)
//todo: all communication like all plugins loaded etc...
//todo: seperate documentation of test from normal project
//todo: check the startup sequence. Dbus shall be activated last...
//todo: there is a bug in the visible flags of sinks and sources. fix it.
//todo: check namespace handling. no use.. in headers
//todo: make sure that iterators have a fixed end to prevent crashed while adding vectors while iterating on critical vectors
//todo: make sure all configurations are tested

#include <config.h>
#include <SocketHandler.h>
#ifdef WITH_DBUS_WRAPPER
#include <dbus/DBusWrapper.h>
#endif
#include "DatabaseHandler.h"
#include "DatabaseObserver.h"
#include "RoutingReceiver.h"
#include "CommandReceiver.h"
#include "ControlReceiver.h"
#include "ControlSender.h"
#include "CommandSender.h"
#include "RoutingSender.h"

#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(DLT_CONTEXT)

using namespace am;

int main(int argc, char *argv[])
{
	DLT_REGISTER_APP("AudioManagerDeamon","AudioManagerDeamon");
	DLT_REGISTER_CONTEXT(DLT_CONTEXT,"Main","Main Context");
	DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("The AudioManager is started "));

	std::vector<std::string> listCommandPluginDirs;
	listCommandPluginDirs.push_back(std::string(DEFAULT_PLUGIN_COMMAND_DIR)); //change this to be modified by the commandline!

	std::vector<std::string> listRoutingPluginDirs;
	listRoutingPluginDirs.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR)); //change this to be modified by the commandline!

	//Instantiate all classes. Keep in same order !
#ifdef WITH_SOCKETHANDLER_LOOP
	SocketHandler iSocketHandler;
#endif

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SOCKETHANDLER_LOOP
	DBusWrapper iDBusWrapper(&iSocketHandler);
#else /*WITH_SOCKETHANDLER_LOOP*/
	DBusWrapper iDBusWrapper;
#endif /*WITH_SOCKETHANDLER_LOOP*/
#endif /*WITH_DBUS_WRAPPER */

	DatabaseHandler iDatabaseHandler(std::string(":memory:"));
	RoutingSender iRoutingSender(listRoutingPluginDirs);
	CommandSender iCommandSender(listCommandPluginDirs);
	ControlSender iControlSender(std::string(CONTROLLER_PLUGIN));
	DatabaseObserver iObserver(&iCommandSender, &iRoutingSender);

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SOCKETHANDLER_LOOP
	CommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iSocketHandler,&iDBusWrapper);
	RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler,&iDBusWrapper);
	ControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler);
#else /*WITH_SOCKETHANDLER_LOOP */
	CommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iDBusWrapper);
	RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iDBusWrapper);
	ControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender);
#endif /*WITH_SOCKETHANDLER_LOOP*/
#else /*WITH_DBUS_WRAPPER*/
	CommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iSocketHandler);
	RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler);
	ControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler);
#endif /*WITH_DBUS_WRAPPER*/

	//since the plugins have been loaded by the *Senders before, we can tell the Controller this:
	iControlSender.hookAllPluginsLoaded();

	//the controller should startup the interfaces - this is just for testing
	iCommandSender.startupInterface(&iCommandReceiver);
	iRoutingSender.startupRoutingInterface(&iRoutingReceiver);
	iRoutingSender.routingInterfacesReady();

#ifdef WITH_SOCKETHANDLER_LOOP
	iSocketHandler.start_listenting();
#endif /*WITH_SOCKETHANDLER_LOOP*/

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SIMPLEDBUS_LOOP
	iDBusWrapper.dbusMainLoop();
#endif/*WITH_SIMPLEDBUS_LOOP*/
#endif /*WITH_DBUS_WRAPPER*/

}




