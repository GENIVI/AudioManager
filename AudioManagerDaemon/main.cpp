/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file main.cpp
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 *
 */

#include "audioManagerIncludes.h"
/**
 * \todo: write some documentation about Plugin mechanism
 *
 */
//put here all plugins you want to use with the Routing Interface
//Q_IMPORT_PLUGIN(RoutingPlugin)
//Q_IMPORT_PLUGIN(RoutingJackPlugin)
//put here all plugins that you want to use with the hooks. No more modification needed (besides adoption of the CMakeList) !
//Q_IMPORT_PLUGIN(TestPlugin)
DLT_DECLARE_CONTEXT(AudioManager);

int main(int argc, char *argv[]) {

	//of course, we need some logging :-)
	DLT_REGISTER_APP("AudioManagerDeamon","AudioManagerDeamon");
	DLT_REGISTER_CONTEXT(AudioManager,"Main","Main Context");
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("The AudioManager is started "));


	//Here are our Main Classes
	DataBaseHandler dhandler;
	RoutingReceiver breceiver;
	Bushandler bushandler;
	Router router;
	HookHandler hookhandler;
	AudioManagerCore core;
	//DBusCommandInterface commandIface;

	//meet and greet: register all the classes @ each other
	hookhandler.registerAudioManagerCore(&core);
	core.registerBushandler(&bushandler);
	core.registerDatabasehandler(&dhandler);
	core.registerRouter(&router);
	core.registerHookEngine(&hookhandler);
	core.registerReceiver(&breceiver);
	//core.registerCommandInterface(&commandIface);
	router.registerDatabasehandler(&dhandler);
	//commandIface.registerDatabasehandler(&dhandler);

	//commandIface.registerAudioManagerCore(&core);
	breceiver.register_Databasehandler(&dhandler);
	bushandler.registerReceiver(&breceiver);

	/**
	 * \todo: we do not have to knock down the database whole the time - this can be done different
	 */DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("create tables for database"));
	dhandler.create_tables();

	/**
	 * \todo replace this static implementation with an XML file that is parsed
	 */
	dhandler.insert_into_Source_Class_table("default", 10, false, false);
	dhandler.insert_into_Source_Class_table("nav", 10, true, true);
	dhandler.insert_into_Source_Class_table("ta", 10, true, false);
	dhandler.insert_into_Source_Class_table("rad", 10, false, false);
	dhandler.insert_into_Sink_Class_table("default");

	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("load hook plugins"));
	hookhandler.loadHookPlugins();

	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("load bus plugins"));
	bushandler.load_Bus_plugins();
	bushandler.StartupInterfaces();
	//commandIface.startupInterface();
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Init phase is over, everything up and running"));

	while (1) {
		sleep(2000);
	}
}

