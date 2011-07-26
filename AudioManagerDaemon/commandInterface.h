/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DBusCommandInterface.h
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

#ifndef COMMANDINTERFACE_H
#define COMMANDINTERFACE_H

#include "audioManagerIncludes.h"


#define SERVICEINTERFACE "org.Genivi.ControllerInterface"

class CommandSendInterface;

/**
 * The interface towards the Controlling Instace (e.g HMI).
 * It handles the communication towards the HMI. It also implements some
 * Application logic that needs to be triggered to execute the actions demanded by
 * the Controller.
 * \todo: make a clear seperation between hmi Interface and Application Logic
 */
class CommandReceiveInterface  {

public:
	virtual void startupInterface()=0;
	virtual connection_t connect(source_t source, sink_t sink)=0;
	virtual connection_t disconnect(source_t source, source_t sink)=0;
	virtual std::list<ConnectionType> getListConnections()=0;
	virtual std::list<SinkType> getListSinks()=0;
	virtual std::list<SourceType> getListSources()=0;
	virtual genInt_t interruptRequest(const std::string &SourceName, const std::string &SinkName)=0;
	virtual interrupt_t interruptResume(interrupt_t interrupt)=0;
	virtual volume_t setVolume(sink_t sink, volume_t volume)=0;
};

class CommandSendInterface {
public:

	/**
	 * Callback that is called when the number of connections change
	 */
	virtual void cbConnectionChanged()=0;
	/**
	 * Callback that is called when the number of sinks change
	 */
	virtual void cbNumberOfSinksChanged()=0;
	/**
	 * Callback that is called when the number of sources change
	 */
	virtual void cbNumberOfSourcesChanged()=0;
};

#endif /* COMMANDINTERFACE_H */
