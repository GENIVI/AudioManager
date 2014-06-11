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

#include <algorithm>
#include "CAmLookupData.h"
#include "CAmRoutingSenderCommon.h"
#include "shared/CAmDltWrapper.h"


namespace am {

const char * CAmLookupData::BUS_NAME = "CAPIRoutingPlugin";

/**
 * rs_lookupData_s
 */

rs_lookupData_s::rs_lookupData_s(const std::shared_ptr<org::genivi::am::RoutingControlProxy<> > & aProxy):mSenderProxy(aProxy)
{
	logInfo(__PRETTY_FUNCTION__);
	mIsConnected = mSenderProxy->isAvailable();
	mSubscription = mSenderProxy->getProxyStatusEvent().subscribe(std::bind(&rs_lookupData_s::onServiceStatusEvent,this,std::placeholders::_1));
}

rs_lookupData_s::~rs_lookupData_s()
{
	mSenderProxy->getProxyStatusEvent().unsubscribe(mSubscription);
	mSenderProxy.reset();
}

std::shared_ptr<org::genivi::am::RoutingControlProxy<>> & rs_lookupData_s::getProxy()
{
	return mSenderProxy;
}

bool rs_lookupData_s::isConnected()
{
	return mIsConnected;
}

am_Error_e rs_lookupData_s::asyncAbort(const am_Handle_s handle, org::genivi::am::RoutingControlProxyBase::AsyncAbortAsyncCallback callback)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncAbortAsync(myHandle, callback);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncConnect(const am_Handle_s handle,
											const am_connectionID_t connectionID,
											const am_sourceID_t sourceID,
											const am_sinkID_t sinkID,
											const am_CustomConnectionFormat_t connectionFormat,
											org::genivi::am::RoutingControlProxyBase::AsyncConnectAsyncCallback callback)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncConnectAsync(myHandle,
										static_cast<org::genivi::am::am_connectionID_t>(connectionID),
										static_cast<org::genivi::am::am_sourceID_t>(sourceID),
										static_cast<org::genivi::am::am_sinkID_t>(sinkID),
										static_cast<org::genivi::am::am_CustomConnectionFormat_t>(connectionFormat),
										callback);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncDisconnect(const am_Handle_s handle,
												const am_connectionID_t connectionID,
												org::genivi::am::RoutingControlProxyBase::AsyncDisconnectAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncDisconnectAsync(myHandle,
										   static_cast<org::genivi::am::am_connectionID_t>(connectionID),
										   cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSinkVolume(	const am_Handle_s handle,
													const am_sinkID_t sinkID,
													const am_volume_t volume,
													const am_CustomRampType_t ramp,
													const am_time_t time,
													org::genivi::am::RoutingControlProxyBase::AsyncSetSinkVolumeAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSinkVolumeAsync(myHandle,
										      static_cast<org::genivi::am::am_sinkID_t>(sinkID),
										      static_cast<org::genivi::am::am_volume_t>(volume),
										      static_cast<org::genivi::am::am_CustomRampType_t>(ramp),
										      static_cast<org::genivi::am::am_time_t>(time),
										      cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSourceVolume(const am_Handle_s handle,
														const am_sourceID_t sourceID,
														const am_volume_t volume,
														const am_CustomRampType_t ramp,
														const am_time_t time,
														org::genivi::am::RoutingControlProxyBase::AsyncSetSourceVolumeAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSourceVolumeAsync(myHandle,
												static_cast<org::genivi::am::am_sourceID_t>(sourceID),
												static_cast<org::genivi::am::am_volume_t>(volume),
												static_cast<org::genivi::am::am_CustomRampType_t>(ramp),
												static_cast<org::genivi::am::am_time_t>(time),
												cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSourceState(	const am_Handle_s handle,
															const am_sourceID_t sourceID,
															const am_SourceState_e state,
															org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSourceStateAsync(myHandle,
												static_cast<org::genivi::am::am_sourceID_t>(sourceID),
												static_cast<org::genivi::am::am_SourceState_e>(state),
												cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSinkSoundProperties(	const am_Handle_s handle,
																const am_sinkID_t sinkID,
																const std::vector<am_SoundProperty_s>& listSoundProperties,
																org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_SoundProperty_L lsp;
		CAmConvertAMVector2CAPI(listSoundProperties, lsp);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSinkSoundPropertiesAsync(myHandle,
														static_cast<org::genivi::am::am_sinkID_t>(sinkID),
														lsp,
														cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSinkSoundProperty(	const am_Handle_s handle,
															const am_sinkID_t sinkID,
															const am_SoundProperty_s& soundProperty,
															org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertyAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_SoundProperty_s converted;
		CAmConvertAM2CAPI(soundProperty, converted);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSinkSoundPropertyAsync(myHandle,
													 static_cast<org::genivi::am::am_sinkID_t>(sinkID),
													 converted,
													 cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSourceSoundProperties(const am_Handle_s handle,
																const am_sourceID_t sourceID,
																const std::vector<am_SoundProperty_s>& listSoundProperties,
																org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertiesAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_SoundProperty_L lsp;
		CAmConvertAMVector2CAPI(listSoundProperties, lsp);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSourceSoundPropertiesAsync(myHandle,
														static_cast<org::genivi::am::am_sourceID_t>(sourceID),
														lsp,
														cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSourceSoundProperty(const am_Handle_s handle,
																const am_sourceID_t sourceID,
																const am_SoundProperty_s& soundProperty,
																org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertyAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_SoundProperty_s converted;
		CAmConvertAM2CAPI(soundProperty, converted);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSourceSoundPropertyAsync(	myHandle,
														static_cast<org::genivi::am::am_sourceID_t>(sourceID),
														converted,
														cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}


am_Error_e rs_lookupData_s::asyncCrossFade(const am_Handle_s handle,
												const am_crossfaderID_t crossfaderID,
												const am_HotSink_e hotSink,
												const am_CustomRampType_t rampType,
												const am_time_t time,
												org::genivi::am::RoutingControlProxyBase::AsyncCrossFadeAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncCrossFadeAsync(myHandle,
										  static_cast<org::genivi::am::am_crossfaderID_t>(crossfaderID),
										  static_cast<org::genivi::am::am_HotSink_e>(hotSink),
										  static_cast<org::genivi::am::am_CustomRampType_t>(rampType),
										  static_cast<org::genivi::am::am_time_t>(time),
										  cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState, org::genivi::am::RoutingControlProxyBase::SetDomainStateAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		mSenderProxy->setDomainStateAsync(static_cast<org::genivi::am::am_domainID_t>(domainID),
										  static_cast<org::genivi::am::am_DomainState_e>(domainState),
										  cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetVolumes(const am_Handle_s handle,
												const std::vector<am_Volumes_s>& volumes ,
												org::genivi::am::RoutingControlProxyBase::AsyncSetVolumesAsyncCallback cb )
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_Volumes_L list;
		CAmConvertAMVector2CAPI(volumes, list);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetVolumesAsync(myHandle,
								      list,
									   cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSinkNotificationConfiguration(const am_Handle_s handle,
																		 const am_sinkID_t sinkID,
																		 const am_NotificationConfiguration_s& notificationConfiguration,
																		 org::genivi::am::RoutingControlProxyBase::AsyncSetSinkNotificationConfigurationAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_NotificationConfiguration_s converted;
		CAmConvertAM2CAPI(notificationConfiguration, converted);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSinkNotificationConfigurationAsync(myHandle,
										  	  	  	  	    static_cast<org::genivi::am::am_sinkID_t>(sinkID),
										  	  	  	  	    converted,
										  	  	  	  	   	cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

am_Error_e rs_lookupData_s::asyncSetSourceNotificationConfiguration(const am_Handle_s handle,
																			const am_sourceID_t sourceID,
																			const am_NotificationConfiguration_s& notificationConfiguration,
																			org::genivi::am::RoutingControlProxyBase::AsyncSetSourceNotificationConfigurationAsyncCallback cb)
{
	logInfo(__PRETTY_FUNCTION__, " [ isConnected : ", isConnected(), " ]");
	if(isConnected())
	{
		org::genivi::am::am_NotificationConfiguration_s converted;
		CAmConvertAM2CAPI(notificationConfiguration, converted);
		org::genivi::am::am_Handle_s myHandle;
		CAmConvertAM2CAPI(handle,myHandle);
		mSenderProxy->asyncSetSourceNotificationConfigurationAsync( myHandle,
																static_cast<org::genivi::am::am_sourceID_t>(sourceID),
																converted,
																cb);
		return (E_OK);
	}
	return (E_UNKNOWN);
}

void rs_lookupData_s::onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus)
{
	logInfo(__PRETTY_FUNCTION__, " status : ", (int)serviceStatus );
	mIsConnected = (serviceStatus==CommonAPI::AvailabilityStatus::AVAILABLE);
}

/**
 * CAmLookupData
 */

CAmLookupData::CAmLookupData():mMapDomains(), mMapSinks(), mMapSources(),mMapConnections(),mMapHandles(), mMapCrossfaders() {
	// TODO Auto-generated constructor stub

}

CAmLookupData::~CAmLookupData() {
	// TODO Auto-generated destructor stub
}

void CAmLookupData::addDomainLookup(am_domainID_t & domainID,
										std::shared_ptr<org::genivi::am::RoutingControlProxy<>> & aProxy)
{
	logInfo(__PRETTY_FUNCTION__, " [ domainID : ", domainID, " ]");
	RSLookupDataPtr lookupData = std::make_shared<rs_lookupData_s>(aProxy);
	mMapDomains.insert(std::make_pair(domainID, lookupData));
}

void CAmLookupData::removeHandle(am_Handle_s handle)
{
	mMapHandles.erase(handle.handle);
}

void CAmLookupData::addSourceLookup(am_sourceID_t sourceID, am_domainID_t domainID)
{
	logInfo(__PRETTY_FUNCTION__, " [ domainID : ", domainID, " ]", " [ sourceID : ", sourceID, " ]");
    mapDomain_t::iterator iter(mMapDomains.begin());
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
        mMapSources.insert(std::make_pair(sourceID, iter->second));
    }
}

void CAmLookupData::addSinkLookup(am_sinkID_t sinkID, am_domainID_t domainID)
{
	logInfo(__PRETTY_FUNCTION__, " [ domainID : ", domainID, " ]", " [ sinkID : ", sinkID, " ]");
    mapDomain_t::iterator iter(mMapDomains.begin());
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
        mMapSinks.insert(std::make_pair(sinkID, iter->second));
    }
}

void CAmLookupData::addCrossfaderLookup(am_crossfaderID_t crossfaderID, am_sourceID_t soucreID)
{
	logInfo(__PRETTY_FUNCTION__, " [ crossfaderID : ", crossfaderID, " ]", " [ soucreID : ", soucreID, " ]");
    mapSources_t::iterator iter(mMapSources.begin());
    iter = mMapSources.find(soucreID);
    if (iter != mMapSources.end())
    {
    	mMapCrossfaders.insert(std::make_pair(crossfaderID, iter->second));
    }
}

void CAmLookupData::removeDomainLookup(am_domainID_t domainID)
{
    mapDomain_t::iterator iter(mMapDomains.begin());
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
    	CAmLookupData::removeEntriesForValue(iter->second, mMapSources);
    	CAmLookupData::removeEntriesForValue(iter->second, mMapSinks);
    	CAmLookupData::removeEntriesForValue(iter->second, mMapCrossfaders);
    	CAmLookupData::removeEntriesForValue(iter->second, mMapHandles);
    	CAmLookupData::removeEntriesForValue(iter->second, mMapConnections);
		mMapDomains.erase(domainID);
    }
}

void CAmLookupData::removeSourceLookup(am_sourceID_t sourceID)
{
    mMapSources.erase(sourceID);
}

void CAmLookupData::removeSinkLookup(am_sinkID_t sinkID)
{
    mMapSinks.erase(sinkID);
}

void CAmLookupData::removeCrossfaderLookup(am_crossfaderID_t crossfaderID)
{
	mMapCrossfaders.erase(crossfaderID);
}

void CAmLookupData::removeConnectionLookup(am_connectionID_t connectionID)
{
	mMapConnections.erase(connectionID);
}

template <typename TKey> void  CAmLookupData::removeEntriesForValue(const RSLookupDataPtr & value, std::map<TKey,RSLookupDataPtr> & map)
{
	typename std::map<TKey,RSLookupDataPtr>::iterator it = map.begin();
	while ( it != map.end() )
	{
		if (it->second == value)
		{
			typename std::map<TKey,RSLookupDataPtr>::iterator it_tmp = it;
			it++;
			map.erase(it_tmp);
		}
		else
			++it;
	}
}

template <typename TKey> const CAmLookupData::RSLookupDataPtr  CAmLookupData::getValueForKey(const TKey & key, const std::map<TKey,CAmLookupData::RSLookupDataPtr> & map)
{
	mapHandles_t::const_iterator iter = map.find(key);
	if (iter != map.end() )
	{
		return iter->second;
	}
	return NULL;
}

am_Error_e CAmLookupData::asyncAbort(const am_Handle_s handle, org::genivi::am::RoutingControlProxyBase::AsyncAbortAsyncCallback callback)
{
	RSLookupDataPtr result = getValueForKey(handle.handle, mMapHandles);
    if(result)
   		return result->asyncAbort(handle, callback);
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncConnect(const am_Handle_s handle,
											const am_connectionID_t connectionID,
											const am_sourceID_t sourceID,
											const am_sinkID_t sinkID,
											const am_CustomConnectionFormat_t connectionFormat,
											org::genivi::am::RoutingControlProxyBase::AsyncConnectAsyncCallback callback)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
    logInfo(__PRETTY_FUNCTION__, " [sourceID:", sourceID, "]", "[sinkID:", sinkID, "]");
    if(result)
    {
    	logInfo(" [isConnected:", result->isConnected(), "]", "[address:", result->getProxy()->getAddress(), "]");

        mMapConnections.insert(std::make_pair(connectionID, result));
        mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncConnect(handle, connectionID, sourceID, sinkID, connectionFormat, callback);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncDisconnect(const am_Handle_s handle,
												const am_connectionID_t connectionID,
												org::genivi::am::RoutingControlProxyBase::AsyncDisconnectAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(connectionID, mMapConnections);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncDisconnect(handle, connectionID, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSinkVolume(	const am_Handle_s handle,
													const am_sinkID_t sinkID,
													const am_volume_t volume,
													const am_CustomRampType_t ramp,
													const am_time_t time,
													org::genivi::am::RoutingControlProxyBase::AsyncSetSinkVolumeAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sinkID, mMapSinks);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSinkVolume(handle, sinkID, volume, ramp, time, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSourceVolume(const am_Handle_s handle,
													const am_sourceID_t sourceID,
													const am_volume_t volume,
													const am_CustomRampType_t ramp,
													const am_time_t time,
													org::genivi::am::RoutingControlProxyBase::AsyncSetSourceVolumeAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSourceVolume(handle, sourceID, volume, ramp, time, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSourceState(const am_Handle_s handle,
													const am_sourceID_t sourceID,
													const am_SourceState_e state,
													org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSourceState(handle, sourceID, state, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSinkSoundProperties(	const am_Handle_s handle,
																const am_sinkID_t sinkID,
																const std::vector<am_SoundProperty_s>& listSoundProperties,
																org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertiesAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sinkID, mMapSinks);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSinkSoundProperties(handle, sinkID, listSoundProperties, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSinkSoundProperty(	const am_Handle_s handle,
															const am_sinkID_t sinkID,
															const am_SoundProperty_s& soundProperty,
															org::genivi::am::RoutingControlProxyBase::AsyncSetSinkSoundPropertyAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sinkID, mMapSinks);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSinkSoundProperty(handle, sinkID, soundProperty, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSourceSoundProperties(const am_Handle_s handle,
																const am_sourceID_t sourceID,
																const std::vector<am_SoundProperty_s>& listSoundProperties,
																org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertiesAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSourceSoundProperties(handle, sourceID, listSoundProperties, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSourceSoundProperty(const am_Handle_s handle,
															const am_sourceID_t sourceID,
															const am_SoundProperty_s& soundProperty,
															org::genivi::am::RoutingControlProxyBase::AsyncSetSourceSoundPropertyAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSourceSoundProperty(handle, sourceID, soundProperty, cb);
    }
    return (E_UNKNOWN);
}


am_Error_e CAmLookupData::asyncCrossFade(const am_Handle_s handle,
												const am_crossfaderID_t crossfaderID,
												const am_HotSink_e hotSink,
												const am_CustomRampType_t rampType,
												const am_time_t time,
												org::genivi::am::RoutingControlProxyBase::AsyncCrossFadeAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(crossfaderID, mMapCrossfaders);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncCrossFade(handle, crossfaderID, hotSink, rampType, time, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState, org::genivi::am::RoutingControlProxyBase::SetDomainStateAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(domainID, mMapDomains);
    if(result)
   		return result->setDomainState(domainID, domainState, cb);
    return (E_UNKNOWN);
}


am_Error_e CAmLookupData::asyncSetVolumes(const am_Handle_s handle,
												const std::vector<am_Volumes_s>& volumes ,
												org::genivi::am::RoutingControlProxyBase::AsyncSetVolumesAsyncCallback cb )
{

	if(volumes.size())
	{
		am_Volumes_s volumeItem = volumes.at(0);
		RSLookupDataPtr result = NULL;
		if(volumeItem.volumeType == VT_SINK)
			result = CAmLookupData::getValueForKey(volumeItem.volumeID.sink, mMapSinks);
		else if(volumeItem.volumeType == VT_SOURCE)
			result = CAmLookupData::getValueForKey(volumeItem.volumeID.source, mMapSources);
	    if(result)
	    {
	    	mMapHandles.insert(std::make_pair(+handle.handle, result));
	   		return result->asyncSetVolumes(handle, volumes, cb);
	    }
	}
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSinkNotificationConfiguration(const am_Handle_s handle,
																	 const am_sinkID_t sinkID,
																	 const am_NotificationConfiguration_s& notificationConfiguration,
																	 org::genivi::am::RoutingControlProxyBase::AsyncSetSinkNotificationConfigurationAsyncCallback cb)
{
    RSLookupDataPtr result = CAmLookupData::getValueForKey(sinkID, mMapSinks);
    if(result)
    {
    	mMapHandles.insert(std::make_pair(+handle.handle, result));
   		return result->asyncSetSinkNotificationConfiguration(handle, sinkID, notificationConfiguration, cb);
    }
    return (E_UNKNOWN);
}

am_Error_e CAmLookupData::asyncSetSourceNotificationConfiguration(const am_Handle_s handle,
																		const am_sourceID_t sourceID,
																		const am_NotificationConfiguration_s& notificationConfiguration,
																		org::genivi::am::RoutingControlProxyBase::AsyncSetSourceNotificationConfigurationAsyncCallback cb)
{
	RSLookupDataPtr result = CAmLookupData::getValueForKey(sourceID, mMapSources);
	if(result)
	{
		mMapHandles.insert(std::make_pair(+handle.handle, result));
		return result->asyncSetSourceNotificationConfiguration(handle, sourceID, notificationConfiguration, cb);
	}
	return (E_UNKNOWN);
}

} /* namespace am */
