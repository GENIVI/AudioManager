/*
 * RoutingSender.h
 *
 *  Created on: Oct 26, 2011
 *      Author: christian
 */

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "pluginTemplate.h"
#include "routing/RoutingSendInterface.h"
#include <map>

using namespace am;

class RoutingSender {
public:
	RoutingSender();
	virtual ~RoutingSender();
	/**
	 * @author Christian
	 * this adds the domain to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
	 * This must be done whenever a domain is registered.
	 */
	am_Error_e addDomainLookup(const am_Domain_s& domainData);
	/**
	 * @author Christian
	 * this adds the Source to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
	 * This must be done whenever a Source is registered.
	 */
	am_Error_e addSourceLookup(const am_Source_s& sourceData);
	/**
	 * @author Christian
	 * this adds the Sink to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
	 * This must be done whenever a Sink is registered.
	 */
	am_Error_e addSinkLookup(const am_Sink_s& sinkData);
	/**
	 * @author Christian
	 * this adds the Crossfader to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
	 * This must be done whenever a Crossfader is registered.
	 */
	am_Error_e addCrossfaderLookup(const am_Crossfader_s& crossfaderData);
	/**
	 * @author Christian
	 * this removes the Domain to the lookup table of the Router. This must be done everytime a domain is deregistered.
	 */
	am_Error_e removeDomainLookup(const am_domainID_t domainID);
	/**
	 * @author Christian
	 * this removes the Source to the lookup table of the Router. This must be done everytime a source is deregistered.
	 */
	am_Error_e removeSourceLookup(const am_sourceID_t sourceID);
	/**
	 * @author Christian
	 * this removes the Sink to the lookup table of the Router. This must be done everytime a sink is deregistered.
	 */
	am_Error_e removeSinkLookup(const am_sinkID_t sinkID);
	/**
	 * @author Christian
	 * this removes the Crossfader to the lookup table of the Router. This must be done everytime a crossfader is deregistered.
	 */
	am_Error_e removeCrossfaderLookup(const am_crossfaderID_t crossfaderID);
	void startupRoutingInterface(RoutingReceiveInterface* routingreceiveinterface) ;
	void routingInterfacesReady() ;
	void routingInterfacesRundown() ;
	am_Error_e asyncAbort(const am_Handle_s handle) ;
	am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) ;
	am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID) ;
	am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
	am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time) ;
	am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	am_Error_e returnBusName(std::string& BusName) const ;
private:

	struct InterfaceNamePairs
	{
		RoutingSendInterface* routingInterface;
		std::string busName;
	};

    typedef std::map<am_domainID_t, RoutingSendInterface*> DomainInterfaceMap;
    typedef std::map<am_sinkID_t, RoutingSendInterface*> SinkInterfaceMap;
    typedef std::map<am_sourceID_t, RoutingSendInterface*> SourceInterfaceMap;
    typedef std::map<am_crossfaderID_t, RoutingSendInterface*> CrossfaderInterfaceMap;
    typedef std::map<am_connectionID_t, RoutingSendInterface*> ConnectionInterfaceMap;

	std::vector<InterfaceNamePairs> mListInterfaces;
    DomainInterfaceMap mMapDomainInterface;
    SinkInterfaceMap mMapSinkInterface;
    SourceInterfaceMap mMapSourceInterface;
    CrossfaderInterfaceMap mMapCrossfaderInterface;
    ConnectionInterfaceMap mMapConnectionInterface;
};

#endif /* ROUTINGSENDER_H_ */
