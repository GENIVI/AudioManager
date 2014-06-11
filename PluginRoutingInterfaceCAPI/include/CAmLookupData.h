/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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

#ifndef CAMLOOKUPDATA_H_
#define CAMLOOKUPDATA_H_

#include <map>
#include <iostream>
#include <cassert>
#include <memory>
#include <CommonAPI/CommonAPI.h>
#include "audiomanagertypes.h"
#include <org/genivi/am/RoutingControlProxy.h>

#ifdef UNIT_TEST
#include "../test/IAmRoutingSenderBackdoor.h" //we need this for the unit test
#endif

namespace am {

using namespace CommonAPI;

/** A structure holding info for given domain.
 * For every domain a single instance is created which is used by the lookup methods.
 */

struct rs_lookupData_s
{
private:
    bool mIsConnected; //!< bool indicating whether the domain is reachable or not
    std::shared_ptr<org::genivi::am::RoutingControlProxy<> > mSenderProxy; //!< a pointer to the proxy object, which implements the connection out from AudioManager
    CommonAPI::ProxyStatusEvent::Subscription mSubscription;	//!< subscription for the proxy system events
    void onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus); //!< proxy status event callback
public:
    rs_lookupData_s(const std::shared_ptr<org::genivi::am::RoutingControlProxy<> > & aProxy);
    ~rs_lookupData_s();

    /**
     * returns the proxy object.
     */
    std::shared_ptr<org::genivi::am::RoutingControlProxy<>> & getProxy();
    /**
     * returns whether the proxy object is connected or not.
     */
    bool isConnected();
    /**
     * proxy wrapping methods.
     */
    am_Error_e asyncAbort(const am_Handle_s handle, org::genivi::am::RoutingControlProxyBase::AsyncAbortAsyncCallback);
    am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t, const am_sourceID_t, const am_sinkID_t, const am_CustomConnectionFormat_t, org::genivi::am::RoutingControlProxyBase::AsyncConnectAsyncCallback);
    am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t, org::genivi::am::RoutingControlProxyBase::AsyncDisconnectAsyncCallback);
    am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t, const am_volume_t, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkVolumeAsyncCallback);
    am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t , const am_volume_t, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceVolumeAsyncCallback);
    am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t, const am_SourceState_e, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceStateAsyncCallback);
    am_Error_e asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t, const std::vector<am_SoundProperty_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback);
    am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t, const am_SoundProperty_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertyAsyncCallback);
    am_Error_e asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t, const std::vector<am_SoundProperty_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertiesAsyncCallback);
    am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t, const am_SoundProperty_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertyAsyncCallback);
    am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t, const am_HotSink_e, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncCrossFadeAsyncCallback);
    am_Error_e setDomainState(const am_domainID_t, const am_DomainState_e, org::genivi::am::RoutingControlProxyBase::SetDomainStateAsyncCallback);
    am_Error_e asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetVolumesAsyncCallback);
    am_Error_e asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t, const am_NotificationConfiguration_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkNotificationConfigurationAsyncCallback);
    am_Error_e asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t, const am_NotificationConfiguration_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceNotificationConfigurationAsyncCallback);
};


/**
 * The class encapsulate the lookup mechanism and forwards the invocations to the appropriate lookup objects ( proxies ).
 */
class CAmLookupData {

	typedef std::shared_ptr<rs_lookupData_s> RSLookupDataPtr;
	/**
	 * Lookup maps.
	 */
	typedef std::map<am_domainID_t,RSLookupDataPtr> mapDomain_t;
	typedef std::map<am_sinkID_t,RSLookupDataPtr> mapSinks_t;
	typedef std::map<am_sourceID_t,RSLookupDataPtr> mapSources_t;
	typedef std::map<am_connectionID_t,RSLookupDataPtr> mapConnections_t;
	typedef std::map<uint16_t,RSLookupDataPtr> mapHandles_t;
	typedef std::map<am_crossfaderID_t,RSLookupDataPtr> mapCrossfaders_t;

	mapDomain_t mMapDomains;
	mapSinks_t mMapSinks;
	mapSources_t mMapSources;
	mapConnections_t mMapConnections;
	mapHandles_t mMapHandles;
	mapCrossfaders_t mMapCrossfaders;

	/** \brief returns the value for given key if exists.
	 *
	 * @param  key is a search key.
	 * @param  map is a either domain, sink, source, connection, crossfader or handle map.
	 */
	template <typename TKey> static const RSLookupDataPtr getValueForKey(const TKey & key, const std::map<TKey,RSLookupDataPtr> & map);

	/** \brief removes all entries which contains given value.
	 *
	 * @param  value is a search value.
	 * @param  map is a either domain, sink, source, connection, crossfader or handle map.
	 */
	template <typename TKey> static void removeEntriesForValue(const RSLookupDataPtr & value, std::map<TKey,RSLookupDataPtr> & map);

public:
	CAmLookupData();
	virtual ~CAmLookupData();
	/** \brief adds a lookup for given domain.
	 *
	 * @param  domainID is a valid domain id.
	 * @param  aProxy is a proxy object constructed by registerDomain
	 */
	void addDomainLookup(am_domainID_t & domainID,
							std::shared_ptr<org::genivi::am::RoutingControlProxy<>> & aProxy);

	/** \brief removes given handle from the list.
	 *
	 */
	void removeHandle(am_Handle_s handle);

	/** \brief adds a lookup for given source in a given domain.
	 *
	 * @param  sourceID is a valid source id.
	 * @param  domainID is a valid domain id
	 */
    void addSourceLookup(am_sourceID_t sourceID, am_domainID_t domainID);

	/** \brief adds a lookup for given sink in a given domain.
	 *
	 * @param  sinkID is a valid sink id.
	 * @param  domainID is a valid domain id
	 */
    void addSinkLookup(am_sinkID_t sinkID, am_domainID_t domainID);

	/** \brief adds a lookup for given crossfader in the domain wherein the given source belongs to.
	 *
	 * @param  crossfaderID is a valid crossfader id.
	 * @param  soucreID is a valid source id
	 */
    void addCrossfaderLookup(am_crossfaderID_t crossfaderID, am_sourceID_t soucreID);

	/** \brief removes a given domain lookup.
	 */
    void removeDomainLookup(am_domainID_t domainID);

	/** \brief removes a given source lookup.
	 */
    void removeSourceLookup(am_sourceID_t sourceID);

	/** \brief removes a given sink lookup.
	 */
    void removeSinkLookup(am_sinkID_t sinkID);

	/** \brief removes a given crossfader lookup.
	 */
    void removeCrossfaderLookup(am_crossfaderID_t crossfaderID);

	/** \brief removes a given connection lookup.
	 */
    void removeConnectionLookup(am_connectionID_t connectionID);

    size_t numberOfDomains() { return mMapDomains.size(); }

    static const char * BUS_NAME;

    /**
     * Wrapping methods.
     */
    am_Error_e asyncAbort(const am_Handle_s, org::genivi::am::RoutingControlProxyBase::AsyncAbortAsyncCallback);
    am_Error_e asyncConnect(const am_Handle_s , const am_connectionID_t, const am_sourceID_t, const am_sinkID_t, const am_CustomConnectionFormat_t, org::genivi::am::RoutingControlProxyBase::AsyncConnectAsyncCallback);
    am_Error_e asyncDisconnect(const am_Handle_s , const am_connectionID_t, org::genivi::am::RoutingControlProxyBase::AsyncDisconnectAsyncCallback);
    am_Error_e asyncSetSinkVolume(const am_Handle_s , const am_sinkID_t, const am_volume_t, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkVolumeAsyncCallback);
    am_Error_e asyncSetSourceVolume(const am_Handle_s , const am_sourceID_t , const am_volume_t, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceVolumeAsyncCallback);
    am_Error_e asyncSetSourceState(const am_Handle_s , const am_sourceID_t, const am_SourceState_e, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceStateAsyncCallback);
    am_Error_e asyncSetSinkSoundProperties(const am_Handle_s , const am_sinkID_t, const std::vector<am_SoundProperty_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback);
    am_Error_e asyncSetSinkSoundProperty(const am_Handle_s , const am_sinkID_t, const am_SoundProperty_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertyAsyncCallback);
    am_Error_e asyncSetSourceSoundProperties(const am_Handle_s , const am_sourceID_t, const std::vector<am_SoundProperty_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertiesAsyncCallback);
    am_Error_e asyncSetSourceSoundProperty(const am_Handle_s , const am_sourceID_t, const am_SoundProperty_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertyAsyncCallback);
    am_Error_e asyncCrossFade(const am_Handle_s , const am_crossfaderID_t, const am_HotSink_e, const am_CustomRampType_t, const am_time_t, org::genivi::am::RoutingControlProxyBase::AsyncCrossFadeAsyncCallback);
    am_Error_e setDomainState(const am_domainID_t, const am_DomainState_e, org::genivi::am::RoutingControlProxyBase::SetDomainStateAsyncCallback);
    am_Error_e asyncSetVolumes(const am_Handle_s , const std::vector<am_Volumes_s>&, org::genivi::am::RoutingControlProxyBase::AsyncSetVolumesAsyncCallback);
    am_Error_e asyncSetSinkNotificationConfiguration(const am_Handle_s , const am_sinkID_t, const am_NotificationConfiguration_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSinkNotificationConfigurationAsyncCallback);
    am_Error_e asyncSetSourceNotificationConfiguration(const am_Handle_s , const am_sourceID_t, const am_NotificationConfiguration_s&, org::genivi::am::RoutingControlProxyBase::AsyncSetSourceNotificationConfigurationAsyncCallback);
#ifdef UNIT_TEST
    friend class IAmRoutingSenderBackdoor;
#endif
};

} /* namespace am */
#endif /* CAMLOOKUPDATA_H_ */
