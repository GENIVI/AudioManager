/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginDBus
 *
 * \file RoutingSend.h
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
 */

#ifndef BUS_INTERFACE_H_
#define BUS_INTERFACE_H_

#include "headers.h"
#include "routinginterface.h"

/**Implementation of the interface
 *
 */
class DbusInterface: public RoutingSendInterface {

public:
	DbusInterface();
	virtual ~DbusInterface();

    void startup_interface(RoutingReceiveInterface * audioman);
	void return_BusName(char * BusName);
	connection_t connect(source_t source, sink_t sink, connection_t connID);
	bool disconnect(connection_t connectionID);
	volume_t setSinkVolume(volume_t volume, sink_t sink);
	volume_t setSourceVolume(volume_t volume, source_t source);
	bool muteSource(source_t sourceID);
	bool muteSink(sink_t sinkID);
	bool unmuteSource(source_t sourceID);
	bool unmuteSink(sink_t sinkID);
	void system_ready();

private:
	RoutingReceiveInterface *m_audioman;
	AudioManagerInterface* m_DbusInterface;
	DBusConnection* m_conn;
	char* m_busname;
	char* m_path;

};


#endif /* BUS_INTERFACE_H_ */
