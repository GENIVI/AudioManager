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

#include <cassert>
#include <map>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "shared/CAmDltWrapper.h"
#include "CAmRoutingSenderCAPI.h"
#include "CAmRoutingSenderCommon.h"




namespace am
{
DLT_DECLARE_CONTEXT(ctxCommandCAPI)


extern "C" IAmRoutingSend* PluginRoutingInterfaceCAPIFactory()
{
    CAmDltWrapper::instance()->registerContext(ctxCommandCAPI, "DRS", "Common-API Plugin");
    return (new CAmRoutingSenderCAPI(Am_CAPI));
}

extern "C" void destroyPluginRoutingInterfaceCAPI(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

const char * CAmRoutingSenderCAPI::ROUTING_INTERFACE_SERVICE = "local:org.genivi.audiomanager.routinginterface:org.genivi.audiomanager";

CAmRoutingSenderCAPI::CAmRoutingSenderCAPI() :
                mIsServiceStarted(false),
                mLookupData(),
				mpCAmCAPIWrapper(NULL), //
				mpIAmRoutingReceive(NULL),
				mService()
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "RoutingSender constructed");
}

CAmRoutingSenderCAPI::CAmRoutingSenderCAPI(CAmCommonAPIWrapper *aWrapper) :
                mIsServiceStarted(false),
                mLookupData(),
                mpCAmCAPIWrapper(aWrapper), //
                mpIAmRoutingReceive(NULL),
                mService()
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CommandSenderCAPI constructor called");
    assert(mpCAmCAPIWrapper!=NULL);
}

CAmRoutingSenderCAPI::~CAmRoutingSenderCAPI()
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "RoutingSender deallocate");
    CAmDltWrapper::instance()->unregisterContext(ctxCommandCAPI);
    tearDownInterface(mpIAmRoutingReceive);
}

am_Error_e CAmRoutingSenderCAPI::startService(IAmRoutingReceive* pIAmRoutingReceive)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__);
	if(!mIsServiceStarted)
	{
		assert(pIAmRoutingReceive);
		mService = std::make_shared<CAmRoutingService>(pIAmRoutingReceive, &mLookupData, mpCAmCAPIWrapper);
	    mService->setRoutingReadyAttribute(org::genivi::am::am_RoutingReady_e::RR_UNKNOWN);
		//Registers the service
		if( false == mpCAmCAPIWrapper->registerStub(mService, CAmRoutingSenderCAPI::ROUTING_INTERFACE_SERVICE) )
		{
			return (E_NOT_POSSIBLE);
		}
		mIsServiceStarted = true;
	}
    return (E_OK);
}

am_Error_e CAmRoutingSenderCAPI::startupInterface(IAmRoutingReceive* pIAmRoutingReceive)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__);
    mpIAmRoutingReceive = pIAmRoutingReceive;
    return startService(mpIAmRoutingReceive);
}

am_Error_e CAmRoutingSenderCAPI::tearDownInterface(IAmRoutingReceive*)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__);
    if(mpCAmCAPIWrapper)
    {
    	if(mIsServiceStarted)
    	{
    		mIsServiceStarted = false;
			mpCAmCAPIWrapper->unregisterStub(CAmRoutingSenderCAPI::ROUTING_INTERFACE_SERVICE);
			mService.reset();
    	}
   		return (E_OK);
    }
    return (E_NOT_POSSIBLE);
}

void CAmRoutingSenderCAPI::getInterfaceVersion(std::string & version) const
{
    version = RoutingSendVersion;
}

void CAmRoutingSenderCAPI::setRoutingReady(const uint16_t handle)
{
	assert(mpIAmRoutingReceive);
	mService->setHandle(handle);

    log(&ctxCommandCAPI, DLT_LOG_INFO, "sending routingReady signal");
    mService->setRoutingReadyAttribute(org::genivi::am::am_RoutingReady_e::RR_READY);
}

void CAmRoutingSenderCAPI::setRoutingRundown(const uint16_t handle)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__);
	assert(mpIAmRoutingReceive);
	mService->setRoutingReadyAttribute(org::genivi::am::am_RoutingReady_e::RR_RUNDOWN);
	mService->gotRundown(mLookupData.numberOfDomains(),handle);
}

am_Error_e CAmRoutingSenderCAPI::asyncAbort(const am_Handle_s handle)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncAbort called");
   	return mLookupData.asyncAbort(handle,[](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncConnect called");
   	return mLookupData.asyncConnect(handle,connectionID, sourceID, sinkID, connectionFormat, [&,handle,connectionID](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackConnect(dst, connectionID, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncDisconnect called");
	return mLookupData.asyncDisconnect(handle,connectionID, [&, handle, connectionID](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackDisconnect(dst, connectionID, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSinkVolume called");
	return mLookupData.asyncSetSinkVolume(handle,sinkID, volume, ramp, time, [&, handle, volume](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSinkVolumeChange(dst, volume, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSourceVolume called");
	return mLookupData.asyncSetSourceVolume(handle,sourceID, volume, ramp, time, [&, handle, volume](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSourceVolumeChange(dst, volume, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSourceState called");
	return mLookupData.asyncSetSourceState(handle,sourceID, state,[&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSourceState(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSinkSoundProperties called");
	return mLookupData.asyncSetSinkSoundProperties(handle,sinkID, listSoundProperties, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSinkSoundProperties(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSinkSoundProperty called");
	return mLookupData.asyncSetSinkSoundProperty(handle, sinkID, soundProperty, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSinkSoundProperty(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSourceSoundProperties called");
	return mLookupData.asyncSetSourceSoundProperties(handle, sourceID, listSoundProperties, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSourceSoundProperties(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSourceSoundProperty called");
	return mLookupData.asyncSetSourceSoundProperty(handle, sourceID, soundProperty, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSetSourceSoundProperty(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t time)
{
	return mLookupData.asyncCrossFade(handle, crossfaderID, hotSink, rampType, time, [&, handle, hotSink](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackCrossFading(dst, (org::genivi::am::am_HotSink_e)hotSink, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::setDomainState called");
	return mLookupData.setDomainState(domainID, domainState, [](const CommonAPI::CallStatus& callStatus, org::genivi::am::am_Error_e error){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus),"Error",static_cast<am_Error_e>(error));
	});
}

am_Error_e CAmRoutingSenderCAPI::returnBusName(std::string& BusName) const
{
    BusName = CAmLookupData::BUS_NAME;
    return (E_OK);
}

am_Error_e CAmRoutingSenderCAPI::asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& volumes)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetVolumes called");
	return mLookupData.asyncSetVolumes(handle, volumes, [&, handle, volumes](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			org::genivi::am::am_Volumes_L list;
			CAmConvertAMVector2CAPI(volumes, list);
			mService->ackSetVolumes(dst, list, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& nc)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSinkNotificationConfiguration called");
	return mLookupData.asyncSetSinkNotificationConfiguration(handle, sinkID, nc, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSinkNotificationConfiguration(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

am_Error_e CAmRoutingSenderCAPI::asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& nc)
{
	log(&ctxCommandCAPI, DLT_LOG_INFO, "CAmRoutingSenderCAPI::asyncSetSourceNotificationConfiguration called");
	return mLookupData.asyncSetSourceNotificationConfiguration(handle, sourceID, nc, [&, handle](const CommonAPI::CallStatus& callStatus){
		log(&ctxCommandCAPI, DLT_LOG_INFO, __PRETTY_FUNCTION__, "Response with call status:", static_cast<int16_t>(callStatus));
		if (callStatus != CommonAPI::CallStatus::SUCCESS)
		{
			org::genivi::am::am_Handle_s dst;
			CAmConvertAM2CAPI(handle, dst);
			mService->ackSourceNotificationConfiguration(dst, org::genivi::am::am_Error_e::E_NON_EXISTENT);
		}
	});
}

}

