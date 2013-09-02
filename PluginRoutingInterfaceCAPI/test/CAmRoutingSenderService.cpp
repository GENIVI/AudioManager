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
		mDomainData(), mAbortedHandle(UINT_MAX), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(aWrapper), mRoutingInterfaceProxy(NULL)
{

}

CAmRoutingSenderService::CAmRoutingSenderService():
		mDomainData(), mAbortedHandle(UINT_MAX), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(NULL), mRoutingInterfaceProxy(NULL) {
	// TODO Auto-generated constructor stub

}

CAmRoutingSenderService::CAmRoutingSenderService(CAmCommonAPIWrapper * aWrapper, std::shared_ptr<RoutingInterfaceProxy<> > aProxy):
		mDomainData(), mAbortedHandle(UINT_MAX), mIsDomainRegistred(false), mIsServiceAvailable(0), mIsReady(0), mpWrapper(aWrapper), mRoutingInterfaceProxy(aProxy)
{
	mRoutingInterfaceProxy->getProxyStatusEvent().subscribe(std::bind(&CAmRoutingSenderService::onServiceStatusEvent,this,std::placeholders::_1));
    mRoutingInterfaceProxy->getSetRoutingReadyEvent().subscribe(std::bind(&CAmRoutingSenderService::onRoutingReadyEvent,this));
    mRoutingInterfaceProxy->getSetRoutingRundownEvent().subscribe(std::bind(&CAmRoutingSenderService::onRoutingReadyRundown,this));
}

CAmRoutingSenderService::~CAmRoutingSenderService() {
	mpWrapper = NULL;
	if(mRoutingInterfaceProxy)
		mRoutingInterfaceProxy.reset();
}

void CAmRoutingSenderService::onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus)
{
	logInfo(__PRETTY_FUNCTION__);
    std::stringstream  avail;
    avail  << "(" << static_cast<int>(serviceStatus) << ")";
    logInfo("Domain test service status changed to ", avail.str());
    std::cout << std::endl << "Domain test service status changed to " << avail.str() << std::endl;
    if(serviceStatus==CommonAPI::AvailabilityStatus::AVAILABLE)
    {
    	mIsServiceAvailable = true;
    	CommonAPI::CallStatus callStatus;
    	mRoutingInterfaceProxy->getRoutingReadyState(callStatus, mIsReady);
    }
    else
    {
    	mIsDomainRegistred = false;
    	mIsServiceAvailable = false;
    	mIsReady = false;
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

void CAmRoutingSenderService::setAbortHandle(uint16_t handle)
{
	mAbortedHandle = handle;
}

void CAmRoutingSenderService::registerDomain()
{
	if( mIsDomainRegistred || !mIsServiceAvailable || !mIsReady )
		return;
	mIsDomainRegistred = true;
	logInfo(__PRETTY_FUNCTION__,"start registering Domain...");
    am_gen::am_Error_e error;
    mDomainData.name = "TestDomain";
    mDomainData.busname ="TestDomain";
    mDomainData.complete = true;
    mDomainData.domainID = 0;
    mDomainData.early = false;
    mDomainData.nodename = "Test";
    mDomainData.state = am_gen::am_DomainState_e::DS_CONTROLLED;
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

uint16_t CAmRoutingSenderService::errorForHandle(const uint16_t & handle)
{
	uint16_t error = E_OK;
	if(handle==mAbortedHandle && mAbortedHandle!=UINT_MAX)
	{
		error = (uint16_t)am_gen::am_Error_e::E_ABORTED;
		mAbortedHandle = UINT_MAX;
	}
	return error;
}

void CAmRoutingSenderService::asyncSetSourceState(uint16_t handle, am_gen::am_sourceID_t sourceID, am_gen::am_SourceState_e sourceState) {
	logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceState(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::setDomainState(am_gen::am_domainID_t domainID, am_gen::am_DomainState_e domainState, am_gen::am_Error_e& error) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	error = am_gen::am_Error_e::E_OK;
	mRoutingInterfaceProxy->hookDomainStateChange(domainID, domainState, callStatus);
}

void CAmRoutingSenderService::asyncSetSourceVolume(am_gen::am_handle_t handle, am_gen::am_sourceID_t sourceID, am_gen::am_volume_t volume, am_gen::am_RampType_e ramp, am_gen::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceVolume(handle, volume, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkVolume(am_gen::am_handle_t handle, am_gen::am_sinkID_t sinkID, am_gen::am_volume_t volume, am_gen::am_RampType_e ramp, am_gen::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkVolume(handle, volume, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncConnect(am_gen::am_handle_t handle, am_gen::am_connectionID_t connectionID, am_gen::am_sourceID_t sourceID, am_gen::am_sinkID_t sinkID, am_gen::am_ConnectionFormat_e connectionFormat) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackConnect(handle, connectionID, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncDisconnect(am_gen::am_handle_t handle, am_gen::am_connectionID_t connectionID) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackDisconnect(handle, connectionID, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncAbort(am_gen::am_handle_t handle, am_gen::am_Error_e& error) {
   logInfo(__FUNCTION__, " called");
   mAbortedHandle = handle;
   error = am_gen::am_Error_e::E_OK;
}

void CAmRoutingSenderService::asyncSetSinkSoundProperties(am_gen::am_handle_t handle, am_gen::am_sinkID_t sinkID, am_gen::am_SoundProperty_L listSoundProperties) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkSoundProperties(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkSoundProperty(am_gen::am_handle_t handle, am_gen::am_sinkID_t sinkID, am_gen::am_SoundProperty_s soundProperty) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSinkSoundProperty(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceSoundProperties(am_gen::am_handle_t handle, am_gen::am_sourceID_t sourceID, am_gen::am_SoundProperty_L listSoundProperties) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceSoundProperties(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceSoundProperty(am_gen::am_handle_t handle, am_gen::am_sourceID_t sourceID, am_gen::am_SoundProperty_s soundProperty) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetSourceSoundProperty(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncCrossFade(am_gen::am_handle_t handle, am_gen::am_crossfaderID_t crossfaderID, am_gen::am_HotSink_e hotSink, am_gen::am_RampType_e rampType, am_gen::am_time_t time) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackCrossFading(handle, hotSink, (am_gen::am_Error_e)errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetVolumes(am_gen::am_handle_t handle, am_gen::am_Volumes_l volumes) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSetVolumes(handle, volumes, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSinkNotificationConfiguration(am_gen::am_handle_t handle, am_gen::am_sinkID_t sinkID, am_gen::am_NotificationConfiguration_s notificationConfiguration) {
   logInfo(__FUNCTION__, " called");
	CommonAPI::CallStatus callStatus;
	mRoutingInterfaceProxy->ackSinkNotificationConfiguration(handle, errorForHandle(handle), callStatus);
}

void CAmRoutingSenderService::asyncSetSourceNotificationConfiguration(am_gen::am_handle_t handle, am_gen::am_sourceID_t sourceID, am_gen::am_NotificationConfiguration_s notificationConfiguration) {
   logInfo(__FUNCTION__, " called");
   CommonAPI::CallStatus callStatus;
   mRoutingInterfaceProxy->ackSourceNotificationConfiguration(handle, errorForHandle(handle), callStatus);
}

} /* namespace org */
