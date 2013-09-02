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
#include <memory>
#include <assert.h>
#include <algorithm>
#include "CAmRoutingSenderCommon.h"
#include "shared/CAmCommonAPIWrapper.h"
#include "CAmRoutingService.h"


namespace am {

CAmRoutingService::CAmRoutingService():mpCAmCAPIWrapper(NULL), mpIAmRoutingReceive(NULL), mpLookpData(NULL), mNumberDomains(0), mHandle(0), mReady(false) {
	// TODO Auto-generated constructor stub

}

CAmRoutingService::CAmRoutingService(IAmRoutingReceive *aReceiver, CAmLookupData*   aLookpData, CAmCommonAPIWrapper *aCAPIWrapper):
		mpCAmCAPIWrapper(aCAPIWrapper), mpIAmRoutingReceive(aReceiver), mpLookpData(aLookpData), mNumberDomains(0), mHandle(0), mReady(false) {
	// TODO Auto-generated constructor stub

}

CAmRoutingService::~CAmRoutingService() {
	// TODO Auto-generated destructor stub
}

void CAmRoutingService::ackConnect(uint16_t handle, am_gen::am_connectionID_t connectionID, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_CONNECT;
	mpIAmRoutingReceive->ackConnect(handle_s, static_cast<am_connectionID_t>(connectionID), static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackDisconnect(uint16_t handle, am_gen::am_connectionID_t connectionID, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_DISCONNECT;
	mpIAmRoutingReceive->ackDisconnect(handle_s, static_cast<am_connectionID_t>(connectionID), static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
	//todo: Check whether the connection should be removed here!
	mpLookpData->removeConnectionLookup(connectionID);
}

void CAmRoutingService::ackSetSinkVolume(uint16_t handle, am_gen::am_volume_t volume, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSINKVOLUME;
	mpIAmRoutingReceive->ackSetSinkVolumeChange(handle_s, volume, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSourceVolume(uint16_t handle, am_gen::am_volume_t volume, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSOURCEVOLUME;
	mpIAmRoutingReceive->ackSetSourceVolumeChange(handle_s, volume, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSourceState(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSOURCESTATE;
	mpIAmRoutingReceive->ackSetSourceState(handle_s,static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSinkSoundProperties(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSINKSOUNDPROPERTIES;
	mpIAmRoutingReceive->ackSetSinkSoundProperties(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSinkSoundProperty(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSINKSOUNDPROPERTY;
	mpIAmRoutingReceive->ackSetSinkSoundProperty(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSourceSoundProperties(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSOURCESOUNDPROPERTIES;
	mpIAmRoutingReceive->ackSetSourceSoundProperties(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSetSourceSoundProperty(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSOURCESOUNDPROPERTY;
	mpIAmRoutingReceive->ackSetSourceSoundProperty(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackCrossFading(uint16_t handle, am_gen::am_HotSink_e hotSink, am_gen::am_Error_e returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_CROSSFADE;
	mpIAmRoutingReceive->ackCrossFading(handle_s, static_cast<am_HotSink_e>(hotSink), static_cast<am_Error_e>(returnError));
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSourceVolumeTick(uint16_t handle, am_gen::am_sourceID_t source, am_gen::am_volume_t volume) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSOURCEVOLUME;
	mpIAmRoutingReceive->ackSourceVolumeTick(handle_s, source, volume);
}

void CAmRoutingService::ackSinkVolumeTick(uint16_t handle, am_gen::am_sinkID_t sink, am_gen::am_volume_t volume) {
	assert(mpIAmRoutingReceive);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETSINKVOLUME;
	mpIAmRoutingReceive->ackSinkVolumeTick(handle_s, sink, volume);
}

void CAmRoutingService::peekDomain(std::string name, am_gen::am_domainID_t& domainID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->peekDomain(name, domainID));
}

void CAmRoutingService::registerDomain(am_gen::am_Domain_s domainData, std::string returnBusname, std::string, std::string returnInterface, am_gen::am_domainID_t& domainID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	assert(mpCAmCAPIWrapper);
	am_Domain_s converted;
	CAmConvertCAPI2AM(domainData, converted);
	converted.busname = CAmLookupData::BUS_NAME;
	am_Error_e resultCode = mpIAmRoutingReceive->registerDomain(converted, domainID);
	error = static_cast<am_gen::am_Error_e>(resultCode);
	if(E_OK==resultCode)
	{
		std::shared_ptr<CommonAPI::Factory> factory = mpCAmCAPIWrapper->factory();
		std::shared_ptr<RoutingSenderProxy<>> shpSenderProxy = factory->buildProxy<RoutingSenderProxy>(returnBusname, returnInterface , "local");
		mpLookpData->addDomainLookup(domainID, shpSenderProxy);
	}
}

void CAmRoutingService::deregisterDomain(am_gen::am_domainID_t domainID, am_gen::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->deregisterDomain(domainID));
	if(am_gen::am_Error_e::E_OK==returnError)
		mpLookpData->removeDomainLookup(domainID);
}

void CAmRoutingService::registerGateway(am_gen::am_Gateway_s gatewayData, am_gen::am_gatewayID_t& gatewayID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	am_Gateway_s converted;
	CAmConvertCAPI2AM(gatewayData, converted);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->registerGateway(converted, gatewayID));
}

void CAmRoutingService::deregisterGateway(am_gen::am_gatewayID_t gatewayID, am_gen::am_Error_e& returnError) {
	returnError = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->deregisterGateway(gatewayID));
}

void CAmRoutingService::peekSink(std::string name, am_gen::am_sinkID_t& sinkID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->peekSink(name, sinkID));
}

void CAmRoutingService::registerSink(am_gen::sinkData_s sinkData, am_gen::am_sinkID_t& sinkID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Sink_s converted;
	CAmConvertCAPI2AM(sinkData, converted);
	am_Error_e result = mpIAmRoutingReceive->registerSink(converted, sinkID);
	error = static_cast<am_gen::am_Error_e>(result);
	if(E_OK==result)
		mpLookpData->addSinkLookup(sinkID, converted.domainID);
}

void CAmRoutingService::deregisterSink(am_gen::am_sinkID_t sinkID, am_gen::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->deregisterSink(sinkID));
	if(returnError==am_gen::am_Error_e::E_OK)
		mpLookpData->removeSinkLookup(sinkID);
}

void CAmRoutingService::peekSource(std::string name, am_gen::am_sourceID_t& sourceID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->peekSource(name, sourceID));
}

void CAmRoutingService::registerSource(am_gen::sourceData_s sourceData, am_gen::am_sourceID_t& sourceID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Source_s converted;
	CAmConvertCAPI2AM(sourceData, converted);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->registerSource(converted, sourceID));
	if(error==am_gen::am_Error_e::E_OK)
		mpLookpData->addSourceLookup(sourceID, sourceData.domainID);
}

void CAmRoutingService::deregisterSource(am_gen::am_sourceID_t sourceID, am_gen::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->deregisterSource(sourceID));
	if(returnError==am_gen::am_Error_e::E_OK)
		mpLookpData->removeSourceLookup(sourceID);
}

void CAmRoutingService::registerCrossfader(am_gen::crossfaderData_s crossfaderData, am_gen::am_crossfaderID_t& crossfaderID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	am_Crossfader_s converted;
	CAmConvertCAPI2AM(crossfaderData, converted);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->registerCrossfader(converted, crossfaderID));
	if(error==am_gen::am_Error_e::E_OK)
		mpLookpData->addCrossfaderLookup(crossfaderID, crossfaderData.sourceID);
}

void CAmRoutingService::deregisterCrossfader(am_gen::am_crossfaderID_t crossfaderID, am_gen::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	returnError = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->deregisterCrossfader(crossfaderID));
	if(returnError==am_gen::am_Error_e::E_OK)
		mpLookpData->removeCrossfaderLookup(crossfaderID);
}

void CAmRoutingService::peekSourceClassID(std::string name, am_gen::am_sourceClass_t& sourceClassID, am_gen::am_Error_e& error) {
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->peekSourceClassID(name, sourceClassID));
}

void CAmRoutingService::peekSinkClassID(std::string name, am_gen::am_sinkClass_t& sinkClassID, am_gen::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<am_gen::am_Error_e>(mpIAmRoutingReceive->peekSinkClassID(name, sinkClassID));
}

void CAmRoutingService::hookInterruptStatusChange(am_gen::am_sourceID_t sourceID, uint16_t interruptState) {
	assert(mpIAmRoutingReceive);
	mpIAmRoutingReceive->hookInterruptStatusChange(sourceID, static_cast<am_InterruptState_e>(interruptState));
}

void CAmRoutingService::hookDomainRegistrationComplete(am_gen::am_domainID_t domainID) {
	assert(mpIAmRoutingReceive != NULL);
	mpIAmRoutingReceive->hookDomainRegistrationComplete(domainID);
}

void CAmRoutingService::hookSinkAvailablityStatusChange(am_gen::am_sinkID_t sinkID, am_gen::am_Availability_s availability) {
	assert(mpIAmRoutingReceive);
	am_Availability_s am_avialabilty;
	CAmConvertCAPI2AM(availability, am_avialabilty);
	mpIAmRoutingReceive->hookSinkAvailablityStatusChange(sinkID, am_avialabilty);
}

void CAmRoutingService::hookSourceAvailablityStatusChange(am_gen::am_sourceID_t sourceID, am_gen::am_Availability_s availability) {
	assert(mpIAmRoutingReceive);
	am_Availability_s am_availabilty;
	CAmConvertCAPI2AM(availability, am_availabilty);
	mpIAmRoutingReceive->hookSourceAvailablityStatusChange(sourceID, am_availabilty);
}

void CAmRoutingService::hookDomainStateChange(am_gen::am_domainID_t domainID, am_gen::am_DomainState_e domainState) {
	assert(mpIAmRoutingReceive);
	am_DomainState_e am_domainState = static_cast<am_DomainState_e>(domainState);
	mpIAmRoutingReceive->hookDomainStateChange(domainID, am_domainState);
}

void CAmRoutingService::hookTimingInformationChanged(am_gen::am_connectionID_t connectionID, int16_t delay) {
	assert(mpIAmRoutingReceive);
	mpIAmRoutingReceive->hookTimingInformationChanged(connectionID, delay);
}

void CAmRoutingService::sendChangedData(am_gen::am_EarlyData_l earlyData_volumes, am_gen::am_EarlyData_l earlyData_soundproperties) {

	assert(mpIAmRoutingReceive);
	std::vector<am_EarlyData_s> earlyData;
	auto func = [&](const am_gen::am_EarlyData_s &refObject)
	{
			am_EarlyData_s object;
			CAmConvertCAPI2AM(refObject, object);
			earlyData.push_back(object);
	};
	std::for_each(earlyData_volumes.begin(), earlyData_volumes.end(), func);
	std::for_each(earlyData_soundproperties.begin(), earlyData_soundproperties.end(), func);
	mpIAmRoutingReceive->sendChangedData(earlyData);
}

void CAmRoutingService::gotReady(int16_t numberDomains, uint16_t handle)
{
	mReady=true;
    mNumberDomains=numberDomains;
    mHandle=handle;
}

void CAmRoutingService::gotRundown(int16_t numberDomains, uint16_t handle)
{
	mReady=false;
    mNumberDomains=numberDomains;
    mHandle=handle;
}

void CAmRoutingService::confirmRoutingReady(am_gen::am_domainID_t) {
	mpIAmRoutingReceive->confirmRoutingReady(mHandle, E_OK);
    mNumberDomains++;
}

void CAmRoutingService::confirmRoutingRundown(am_gen::am_domainID_t) {
	assert(mpIAmRoutingReceive);
	mpIAmRoutingReceive->confirmRoutingRundown(mHandle, E_OK);
    mNumberDomains--;
}

void CAmRoutingService::updateGateway(am_gen::am_gatewayID_t gatewayID, am_gen::am_ConnectionFormat_L listSourceFormats, am_gen::am_ConnectionFormat_L listSinkFormats, am_gen::bool_L convertionMatrix) {

	assert(mpIAmRoutingReceive);
	std::vector<am_ConnectionFormat_e> destinationSourceConnectionFormats;
	CAmConvertCAPIVector2AM(listSourceFormats, destinationSourceConnectionFormats);

	std::vector<am_ConnectionFormat_e> destinationSinkConnectionFormats;
	CAmConvertCAPIVector2AM(listSinkFormats, destinationSinkConnectionFormats);

	mpIAmRoutingReceive->updateGateway(gatewayID, destinationSourceConnectionFormats, destinationSinkConnectionFormats, convertionMatrix);
}

void CAmRoutingService::updateSink(am_gen::am_sinkID_t sinkID, am_gen::am_sinkClass_t sinkClassID, am_gen::am_SoundProperty_L listSoundProperties, am_gen::am_ConnectionFormat_L listConnectionFormats, am_gen::am_MainSoundProperty_L listMainSoundProperties) {
	assert(mpIAmRoutingReceive);
	std::vector<am_SoundProperty_s> dstListSoundProperties;
    CAmConvertCAPIVector2AM(listSoundProperties, dstListSoundProperties);
    std::vector<am_ConnectionFormat_e> dstListSinkConnectionFormats;
    CAmConvertCAPIVector2AM(listConnectionFormats, dstListSinkConnectionFormats);
    std::vector<am_MainSoundProperty_s> dstListMainSoundProperties;
    CAmConvertCAPIVector2AM(listMainSoundProperties, dstListMainSoundProperties);
    mpIAmRoutingReceive->updateSink( sinkID, sinkClassID, dstListSoundProperties,dstListSinkConnectionFormats,dstListMainSoundProperties);
}

void CAmRoutingService::updateSource(am_gen::am_sourceID_t sourceID, am_gen::am_sourceClass_t sourceClassID, am_gen::am_SoundProperty_L listSoundProperties, am_gen::am_ConnectionFormat_L listConnectionFormats, am_gen::am_MainSoundProperty_L listMainSoundProperties) {
	assert(mpIAmRoutingReceive);
	std::vector<am_SoundProperty_s> dstListSoundProperties;
    CAmConvertCAPIVector2AM(listSoundProperties, dstListSoundProperties);
    std::vector<am_ConnectionFormat_e> dstListSinkConnectionFormats;
    CAmConvertCAPIVector2AM(listConnectionFormats, dstListSinkConnectionFormats);
    std::vector<am_MainSoundProperty_s> dstListMainSoundProperties;
    CAmConvertCAPIVector2AM(listMainSoundProperties, dstListMainSoundProperties);
    mpIAmRoutingReceive->updateSource( sourceID, sourceClassID, dstListSoundProperties,dstListSinkConnectionFormats,dstListMainSoundProperties);
}

void CAmRoutingService::ackSetVolumes(uint16_t handle, am_gen::am_Volumes_l listVolumes, uint16_t error) {
	assert(mpIAmRoutingReceive);
	am_Handle_s handle_s;
	handle_s.handle = handle;
	handle_s.handleType = H_SETVOLUMES;
	std::vector<am_Volumes_s> list;
	CAmConvertCAPIVector2AM(listVolumes, list);
	am_Error_e amError = static_cast<am_Error_e>(error);
	mpIAmRoutingReceive->ackSetVolumes(handle_s, list, amError);
	mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSinkNotificationConfiguration(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s myhandle;
    myhandle.handleType = H_CONNECT;
    myhandle.handle = handle;
    am_Error_e amError = static_cast<am_Error_e>(error);
    mpIAmRoutingReceive->ackSinkNotificationConfiguration(myhandle, amError);
    mpLookpData->removeHandle(handle);
}

void CAmRoutingService::ackSourceNotificationConfiguration(uint16_t handle, uint16_t error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s myhandle;
    myhandle.handleType = H_CONNECT;
    myhandle.handle = handle;
    am_Error_e amError = static_cast<am_Error_e>(error);
    mpIAmRoutingReceive->ackSourceNotificationConfiguration(myhandle, amError);
    mpLookpData->removeHandle(handle);
}

void CAmRoutingService::hookSinkNotificationDataChange(am_gen::am_sinkID_t sinkID, am_gen::notificationPayload_s payload) {
	assert(mpIAmRoutingReceive);
	am_NotificationPayload_s converted;
	CAmConvertCAPI2AM(payload, converted);
	mpIAmRoutingReceive->hookSinkNotificationDataChange(sinkID, converted);
}

void CAmRoutingService::hookSourceNotificationDataChange(am_gen::am_sourceID_t sourceID, am_gen::notificationPayload_s payload) {
	assert(mpIAmRoutingReceive);
	am_NotificationPayload_s converted;
	CAmConvertCAPI2AM(payload, converted);
	mpIAmRoutingReceive->hookSourceNotificationDataChange(sourceID, converted);
}

void CAmRoutingService::getRoutingReadyState(bool& readyState) {
	readyState = mReady;
}

} /* namespace am */
