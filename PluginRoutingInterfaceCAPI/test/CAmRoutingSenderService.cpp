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

#include  <stdint.h>
#include  <limits.h>
#include "shared/CAmCommonAPIWrapper.h"
#include "shared/CAmDltWrapper.h"
#include "CAmRoutingSenderService.h"


namespace am {

#define CAPI_SENDER_ADDRESS 	"local:" CAPI_SENDER_INTERFACE ":" CAPI_SENDER_INSTANCE
#define CAPI_ROUTING_ADDRESS 	"local:" CAPI_ROUTING_INTERFACE ":" CAPI_ROUTING_INSTANCE

const char * CAmRoutingSenderService::ROUTING_SENDER_SERVICE = CAPI_SENDER_ADDRESS;
const char * CAmRoutingSenderService::ROUTING_INTERFACE_SERVICE = CAPI_ROUTING_ADDRESS;

CAmRoutingSenderService::CAmRoutingSenderService(CAmCommonAPIWrapper * aWrapper):
		mDomainData(), mAbortedHandle(), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(aWrapper), mRoutingInterfaceProxy(NULL)
{
	mAbortedHandle.handle=UINT_MAX;
}

CAmRoutingSenderService::CAmRoutingSenderService():
		mDomainData(), mAbortedHandle(), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(NULL), mRoutingInterfaceProxy(NULL)
{

}

CAmRoutingSenderService::CAmRoutingSenderService(CAmCommonAPIWrapper * aWrapper, std::shared_ptr<org::genivi::am::RoutingControlObserverProxy<> > aProxy):
		mDomainData(), mAbortedHandle(), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(aWrapper), mRoutingInterfaceProxy(aProxy)
{
	mAbortedHandle.handle=UINT_MAX;
	mRoutingInterfaceProxy->getProxyStatusEvent().subscribe(std::bind(&CAmRoutingSenderService::onServiceStatusEvent,this,std::placeholders::_1));
    //mRoutingInterfaceProxy->getSetRoutingReadyEvent().subscribe(std::bind(&CAmRoutingSenderService::onRoutingReadyEvent(),this));
    //mRoutingInterfaceProxy->getSetRoutingRundownEvent().subscribe(std::bind(&CAmRoutingSenderService::onRoutingReadyRundown,this));
}

CAmRoutingSenderService::~CAmRoutingSenderService() {
	mpWrapper = NULL;
	if(mRoutingInterfaceProxy)
		mRoutingInterfaceProxy.reset();
}

void CAmRoutingSenderService::onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus)
{
	logInfo(__PRETTY_FUNCTION__);
	mIsDomainRegistred = false;
	mIsServiceAvailable = false;
	mIsReady = false;
    std::stringstream  avail;
    avail  << "(" << static_cast<int>(serviceStatus) << ")";
    logInfo("Domain test service status changed to ", avail.str());
    std::cout << std::endl << "Domain test service status changed to " << avail.str() << std::endl;
    if(serviceStatus==CommonAPI::AvailabilityStatus::AVAILABLE)
    {
    	mIsServiceAvailable = true;
    	CommonAPI::CallStatus callStatus;
    	org::genivi::am::am_RoutingReady_e readyAttr;
    	mRoutingInterfaceProxy->getRoutingReadyAttribute().getValue(callStatus,readyAttr);
    	if (callStatus!=CommonAPI::CallStatus::SUCCESS)
    		logError(__PRETTY_FUNCTION__,"Could not get RoutingReady");
    	else
    		mIsReady = readyAttr == org::genivi::am::am_RoutingReady_e::RR_READY ? true : false;
    }
}

void CAmRoutingSenderService::onRoutingReadyEvent()
{
	logInfo(__PRETTY_FUNCTION__);
	mIsReady = true;
}

void CAmRoutingSenderService::onRoutingReadyRundown()
{
    logInfo(__PRETTY_FUNCTION__);
    mIsReady = true;
    mIsDomainRegistred = false;
}

void CAmRoutingSenderService::setAbortHandle(org::genivi::am::am_Handle_s handle)
{
	mAbortedHandle=handle;
}

void CAmRoutingSenderService::registerDomain()
{
	if( mIsDomainRegistred || !mIsServiceAvailable || !mIsReady )
		return;
	mIsDomainRegistred = true;
	logInfo(__PRETTY_FUNCTION__,"start registering Domain...");
    org::genivi::am::am_Error_e error;
    mDomainData.name = "TestDomain";
    mDomainData.busname ="TestDomain";
    mDomainData.complete = true;
    mDomainData.domainID = 0;
    mDomainData.early = false;
    mDomainData.nodename = "Test";
    mDomainData.state = org::genivi::am::am_DomainState_e::DS_CONTROLLED;
    CommonAPI::CallStatus callStatus;
    mRoutingInterfaceProxy->registerDomain(mDomainData,
    									   CAPI_SENDER_INSTANCE,
    									   CAPI_SENDER_PATH,
    									   CAPI_SENDER_INTERFACE,
    									   callStatus,
    									   mDomainData.domainID,
    									   error);
    logInfo("Domain: got domainID", mDomainData.domainID);
}

org::genivi::am::am_Error_e CAmRoutingSenderService::errorForHandle(const org::genivi::am::am_Handle_s & handle)
{
	org::genivi::am::am_Error_e error = org::genivi::am::am_Error_e::E_OK;
	if(handle==mAbortedHandle && mAbortedHandle.handle!=UINT_MAX)
	{
		error = org::genivi::am::am_Error_e::E_ABORTED;
		mAbortedHandle.handle = UINT_MAX;
	}
	return error;
}

void CAmRoutingSenderService::asyncSetSourceState(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SourceState_e sourceState) {
	logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceState(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::setDomainState(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_DomainState_e domainState, org::genivi::am::am_Error_e& error) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	error = org::genivi::am::am_Error_e::E_OK;
	mRoutingInterfaceProxy->hookDomainStateChange(domainID, domainState, callStatus);
}

void CAmRoutingSenderService::asyncSetSourceVolume(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_volume_t volume, org::genivi::am::am_RampType_pe ramp, org::genivi::am::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceVolumeChange(handle, volume, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkVolume(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_volume_t volume, org::genivi::am::am_RampType_pe ramp, org::genivi::am::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkVolumeChange(handle, volume, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncConnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_ConnectionFormat_pe connectionFormat) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackConnect(handle, connectionID, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncDisconnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackDisconnect(handle, connectionID, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncAbort(org::genivi::am::am_Handle_s handle) {
   logInfo(__FUNCTION__, " called");
   mAbortedHandle = handle;
}

void CAmRoutingSenderService::asyncSetSinkSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_SoundProperty_L listSoundProperties) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkSoundProperties(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_SoundProperty_s soundProperty) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkSoundProperty(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SoundProperty_L listSoundProperties) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceSoundProperties(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SoundProperty_s soundProperty) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceSoundProperty(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncCrossFade(org::genivi::am::am_Handle_s handle, org::genivi::am::am_crossfaderID_t crossfaderID, org::genivi::am::am_HotSink_e hotSink, org::genivi::am::am_RampType_pe rampType, org::genivi::am::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackCrossFading(handle, hotSink, (org::genivi::am::am_Error_e)errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetVolumes(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Volumes_L volumes) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetVolumes(handle, volumes, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_NotificationConfiguration_s notificationConfiguration) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSinkNotificationConfiguration(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_NotificationConfiguration_s notificationConfiguration) {
   logInfo(__FUNCTION__, " called");
   CommonAPI::CallStatus callStatus;
   mRoutingInterfaceProxy->ackSourceNotificationConfiguration(handle, errorForHandle(handle), callStatus);
}

} /* namespace org */
