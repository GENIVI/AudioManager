/*
 * RoutingSender.h
 *
 *  Created on: Oct 26, 2011
 *      Author: christian
 */

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "routing/RoutingSendInterface.h"

#ifdef UNIT_TEST //this is needed to test RoutingSender
#include "../test/RoutingInterfaceBackdoor.h"
#endif

#include <map>
#include <set>

using namespace am;

class RoutingSender {
public:
	RoutingSender();
	virtual ~RoutingSender();

	/**
	 * removes a handle from the list
	 * @param handle to be removed
	 * @return E_OK in case of success
	 */
	am_Error_e removeHandle(const am_Handle_s& handle);

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
	am_Error_e asyncAbort(const am_Handle_s& handle) ;
	am_Error_e asyncConnect(am_Handle_s& handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) ;
	am_Error_e asyncDisconnect(am_Handle_s& handle, const am_connectionID_t connectionID) ;
	am_Error_e asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
	am_Error_e asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time) ;
	am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	am_Error_e getListHandles(std::vector<am_Handle_s> & listHandles) const ;

	struct InterfaceNamePairs
	{
		RoutingSendInterface* routingInterface;
		std::string busName;
	};

	//todo: maybe this would be valuable information for the controller...
	class am_handleData_c
	{
	public:
		union
		{
			am_sinkID_t sinkID;
			am_sourceID_t sourceID;
			am_crossfaderID_t crossfaderID;
			am_connectionID_t connectionID;
		};
		union
		{
			am_SoundProperty_s soundPropery;
			am_SourceState_e sourceState;
			am_volume_t volume;
			am_HotSink_e hotSink;
		};

	};


#ifdef UNIT_TEST //this is needed to test RoutingSender
	friend class RoutingInterfaceBackdoor;
#endif

am_handleData_c returnHandleData(am_Handle_s handle);

private:

	struct comparator
	{
		bool operator()(const am_Handle_s& a, const am_Handle_s& b) const
		{
			return (a.handle < b.handle);
		}
	};

	am_Handle_s createHandle(const am_handleData_c& handleData, const am_Handle_e type);
	void unloadLibraries(void);

    typedef std::map<am_domainID_t, RoutingSendInterface*> DomainInterfaceMap;
    typedef std::map<am_sinkID_t, RoutingSendInterface*> SinkInterfaceMap;
    typedef std::map<am_sourceID_t, RoutingSendInterface*> SourceInterfaceMap;
    typedef std::map<am_crossfaderID_t, RoutingSendInterface*> CrossfaderInterfaceMap;
    typedef std::map<am_connectionID_t, RoutingSendInterface*> ConnectionInterfaceMap;
    typedef std::map<uint16_t, RoutingSendInterface*> HandleInterfaceMap;
    typedef std::map<am_Handle_s,am_handleData_c,comparator> HandlesMap;

    int16_t mHandleCount;
	std::vector<InterfaceNamePairs> mListInterfaces;
	std::vector<void*> mListLibraryHandles;
    ConnectionInterfaceMap mMapConnectionInterface;
    CrossfaderInterfaceMap mMapCrossfaderInterface;
    DomainInterfaceMap mMapDomainInterface;
    SinkInterfaceMap mMapSinkInterface;
    SourceInterfaceMap mMapSourceInterface;
    HandleInterfaceMap mMapHandleInterface;
    HandlesMap mlistActiveHandles;
};

#endif /* ROUTINGSENDER_H_ */
