/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file routinginterface.h
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

#ifndef ROUTINGINTERFACE_H_
#define ROUTINGINTERFACE_H_

#include "dataTypes.h"

class RoutingSendInterface;

/** Routing Receive sendInterface description.
 * This class implements everything from RoutingAdapter -> Audiomanager
 */
class RoutingReceiveInterface {
public:
	/** destructor*/
	virtual ~RoutingReceiveInterface() {
	}

	/** Registers a Domain at the Audiomanager.
	 *  \return a unique ID of the domain	*/
	virtual domain_t registerDomain(char* name, /**< the name of the domain. Must be unique in the system*/
	char* busname, /**< the name of the bus that is used laster to talk to the domain*/
	char* node, /**< the nonde of the bus*/
	bool earlymode /**< true if the domain is in earlymode*/
	)=0;

	/** Registers a Gateway at the Audiomanager
	 * \return a unique ID of the gateway	*/
	virtual gateway_t registerGateway(char* name, /**< the name of the gateway. Should be unique in the system*/
	char* sink, /**< the name of the sink (on the emitting side)*/
	char* source, /**< the name of the source (on the receiving side)*/
	char *domainSource, /**< the domain of the source*/
	char* domainSink, /**< the domain of the sink*/
	char* controlDomain /**< the controlling domain*/
	)=0;

	/** registers a Sink at the Audiomanager
	 * \return a unique ID of the sink.*/
	virtual sink_t registerSink(char* name, /**< the name of the sink to be registered. Sink names must be unique within a Domain*/
	char* sinkclass, /**< the name of the class. Must be valid otherwise registration fails*/
	char* domain /**< the domain name of the sink*/
	)=0;

	/** registers a Source at the Audiomanager *
	 * \return unique ID of the source	*/
	virtual source_t registerSource(char* name, /**< the name of the source to be registered. Source names must be unique wihin the Domain*/
	char* audioclass, /**< the name of the class. If not existend, default will be used.*/
	char* domain /**< the domain of the sink*/
	)=0;

	/** just get the ID of a domain without registering it. This is used to register Gateways.
	 * During the time of registration it is unclear if the other Domain already exists. This function will either
	 * return the already existing ID or reserve an ID with is then used later when the domain is registered. *
	 * \return the unique Id of the domain.	*/
	virtual domain_t peekDomain(char* name /**< the name of the domain*/
	)=0;

	/**Acknowledgement of a connect. This function shall be called when a connect event is finished
	 *
	 * @param handle the the handle for the connection
	 * @param error	reads GEN_OK on success, other errors in case of problems
	 */
	virtual void ackConnect(genHandle_t handle, genError_t error)=0;

};

/** Routing Send sendInterface
 *  This class implements everything from Audiomanager -> RoutingAdapter
 */
class RoutingSendInterface {
public:
	/** destructor*/
	virtual ~RoutingSendInterface() {
	}

	/** starts up the interface. In the implementations, here is the best place for
	 * init routines.
	 */
	virtual void startup_interface(RoutingReceiveInterface * action /**< hands over the handle to the receive object. */
	)=0;

	/** connect a source to a sink
	 *  \return the unique ID of the connection.
	 */
	virtual connection_t connect(source_t source, /**< the ID of the source*/
	sink_t sink, /**< the ID of the sink*/
	connection_t con_ID /**< the ID of the connection*/
	)=0;

	/** disconnect a connection
	 * 	\return true on success
	 */
	virtual bool disconnect(connection_t connection_ID /**< the ID of the connection*/
	)=0;

	/** this method is used to retrieve the busname during startup of the plugin.
	 * 	Needs to be implemented
	 */
	virtual void return_BusName(char* BusName /**< pointer to the Busname that needs to be returned*/
	)=0;

	/** this method is used to set the volume of a sink
	 *  \return returns the new value or -1 on error or impossible.
	 *  It is not mandatory that a Plugin implements this feature.
	 */
	virtual volume_t setSinkVolume(volume_t volume, /**< new volume */
	sink_t sinkID /**< sinkID to change */
	)=0;

	/** this method is used to set the volume of a source
	 *  \return returns the new value or -1 on error or impossible.
	 *  It is not mandatory that a Plugin implements this feature.
	 */
	virtual volume_t setSourceVolume(volume_t volume, /**< new volume */
	source_t sourceID /**< sourceID to change */
	)=0;

	/** this method is used to mute a source
	 *
	 * \return true if succeeded
	 * \todo add error codes to answers
	 */
	virtual bool muteSource(source_t sourceID /**< SourceID to be muted */
	)=0;

	/** this method is used to mute a sink
	 *
	 * \return true if succeeded
	 * \todo add error codes to answers
	 */
	virtual bool muteSink(sink_t sinkID /**< SinkID to be muted */
	)=0;

	/** this method is used to unmute a source
	 *
	 * \return true if succeeded
	 * \todo add error codes to answers
	 */
	virtual bool unmuteSource(source_t sourceID /**< SourceID to be unmuted */
	)=0;

	/** this method is used to unmute a sink
	 *
	 * \return true if succeeded
	 * \todo add error codes to answers
	 */
	virtual bool unmuteSink(sink_t sinkID /**< SinkID to be unmuted */
	)=0;

	/** signal that tells the plugin that the system is ready. Is used to trigger a registration of Domains, etc..*/
	void slot_system_ready();
};

/** Routing sendInterface Factory.
 * This is used for the Qt Plugin mechanism. The factory creates and returns a pointer to the
 * RoutingSendInterface.
 */
class RoutingInterfaceFactory {
public:
	virtual ~RoutingInterfaceFactory() {
	}

	/** returns an Instance of RoutingSendInterface.
	 * \return pointer to the instance.
	 */
	virtual RoutingSendInterface* returnInstance()=0;
};


#endif /* ROUTINGINTERFACE_H_ */
