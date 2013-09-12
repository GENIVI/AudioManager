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

void CAmRoutingService::ackConnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackConnect(handle_s, static_cast<am_connectionID_t>(connectionID), static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackDisconnect(org::genivi::am::am_Handle_s handle , org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackDisconnect(handle_s, static_cast<am_connectionID_t>(connectionID), static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
	mpLookpData->removeConnectionLookup(connectionID);
}

void CAmRoutingService::ackSetSinkVolumeChange(org::genivi::am::am_Handle_s handle , org::genivi::am::am_volume_t volume, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSinkVolumeChange(handle_s, volume, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSourceVolumeChange(org::genivi::am::am_Handle_s handle, org::genivi::am::am_volume_t volume, org::genivi::am::am_Error_e error){
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSourceVolumeChange(handle_s, volume, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSourceState(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSourceState(handle_s,static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSinkSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error){
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSinkSoundProperties(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSinkSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSinkSoundProperty(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSourceSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSourceSoundProperties(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSetSourceSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSetSourceSoundProperty(handle_s, static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackCrossFading(org::genivi::am::am_Handle_s handle, org::genivi::am::am_HotSink_e hotSink, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackCrossFading(handle_s, static_cast<am_HotSink_e>(hotSink), static_cast<am_Error_e>(error));
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSourceVolumeTick(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t source, org::genivi::am::am_volume_t volume) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSourceVolumeTick(handle_s, source, volume);
}

void CAmRoutingService::ackSinkVolumeTick(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sink, org::genivi::am::am_volume_t volume) {
	assert(mpIAmRoutingReceive);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	mpIAmRoutingReceive->ackSinkVolumeTick(handle_s, sink, volume);
}

void CAmRoutingService::peekDomain(std::string name, org::genivi::am::am_domainID_t& domainID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->peekDomain(name, domainID));
}

void CAmRoutingService::registerDomain(org::genivi::am::am_Domain_s domainData, std::string returnBusname, std::string returnInterface, org::genivi::am::am_domainID_t& domainID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	assert(mpCAmCAPIWrapper);
	am_Domain_s converted;
	CAmConvertCAPI2AM(domainData, converted);
	converted.busname = CAmLookupData::BUS_NAME;
	am_Error_e resultCode = mpIAmRoutingReceive->registerDomain(converted, domainID);
	error = static_cast<org::genivi::am::am_Error_e>(resultCode);
	if(E_OK==resultCode)
	{
		std::shared_ptr<CommonAPI::Factory> factory = mpCAmCAPIWrapper->factory();
		std::shared_ptr<org::genivi::am::RoutingControlProxy<>> shpSenderProxy = factory->buildProxy<org::genivi::am::RoutingControlProxy>(returnBusname, returnInterface , "local");
		mpLookpData->addDomainLookup(domainID, shpSenderProxy);
	}
}

void CAmRoutingService::deregisterDomain(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->deregisterDomain(domainID));
	if(org::genivi::am::am_Error_e::E_OK==returnError)
		mpLookpData->removeDomainLookup(domainID);
}

void CAmRoutingService::registerGateway(org::genivi::am::am_Gateway_s gatewayData, org::genivi::am::am_gatewayID_t& gatewayID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	am_Gateway_s converted;
	CAmConvertCAPI2AM(gatewayData, converted);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->registerGateway(converted, gatewayID));
}

void CAmRoutingService::deregisterGateway(org::genivi::am::am_gatewayID_t gatewayID, org::genivi::am::am_Error_e& returnError) {
	returnError = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->deregisterGateway(gatewayID));
}

void CAmRoutingService::peekSink(std::string name, org::genivi::am::am_sinkID_t& sinkID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->peekSink(name, sinkID));
}

void CAmRoutingService::registerSink(org::genivi::am::am_Sink_s sinkData, org::genivi::am::am_sinkID_t& sinkID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Sink_s converted;
	CAmConvertCAPI2AM(sinkData, converted);
	am_Error_e result = mpIAmRoutingReceive->registerSink(converted, sinkID);
	error = static_cast<org::genivi::am::am_Error_e>(result);
	if(E_OK==result)
		mpLookpData->addSinkLookup(sinkID, converted.domainID);
}

void CAmRoutingService::deregisterSink(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->deregisterSink(sinkID));
	if(returnError==org::genivi::am::am_Error_e::E_OK)
		mpLookpData->removeSinkLookup(sinkID);
}

void CAmRoutingService::peekSource(std::string name, org::genivi::am::am_sourceID_t& sourceID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->peekSource(name, sourceID));
}

void CAmRoutingService::registerSource(org::genivi::am::am_Source_s sourceData, org::genivi::am::am_sourceID_t& sourceID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Source_s converted;
	CAmConvertCAPI2AM(sourceData, converted);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->registerSource(converted, sourceID));
	if(error==org::genivi::am::am_Error_e::E_OK)
		mpLookpData->addSourceLookup(sourceID, sourceData.domainID);
}

void CAmRoutingService::deregisterSource(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	returnError = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->deregisterSource(sourceID));
	if(returnError==org::genivi::am::am_Error_e::E_OK)
		mpLookpData->removeSourceLookup(sourceID);
}

void CAmRoutingService::registerCrossfader(org::genivi::am::am_Crossfader_s crossfaderData, org::genivi::am::am_crossfaderID_t& crossfaderID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	am_Crossfader_s converted;
	CAmConvertCAPI2AM(crossfaderData, converted);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->registerCrossfader(converted, crossfaderID));
	if(error==org::genivi::am::am_Error_e::E_OK)
		mpLookpData->addCrossfaderLookup(crossfaderID, crossfaderData.sourceID);
}

void CAmRoutingService::deregisterCrossfader(org::genivi::am::am_crossfaderID_t crossfaderID, org::genivi::am::am_Error_e& returnError) {
	assert(mpIAmRoutingReceive);
	returnError = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->deregisterCrossfader(crossfaderID));
	if(returnError==org::genivi::am::am_Error_e::E_OK)
		mpLookpData->removeCrossfaderLookup(crossfaderID);
}

void CAmRoutingService::peekSourceClassID(std::string name, org::genivi::am::am_sourceClass_t& sourceClassID, org::genivi::am::am_Error_e& error) {
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->peekSourceClassID(name, sourceClassID));
}

void CAmRoutingService::peekSinkClassID(std::string name, org::genivi::am::am_sinkClass_t& sinkClassID, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	error = static_cast<org::genivi::am::am_Error_e>(mpIAmRoutingReceive->peekSinkClassID(name, sinkClassID));
}

void CAmRoutingService::hookInterruptStatusChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_InterruptState_e InterruptState) {
	assert(mpIAmRoutingReceive);
	mpIAmRoutingReceive->hookInterruptStatusChange(sourceID, static_cast<am_InterruptState_e>(InterruptState));
}

void CAmRoutingService::hookDomainRegistrationComplete(org::genivi::am::am_domainID_t domainID) {
	assert(mpIAmRoutingReceive != NULL);
	mpIAmRoutingReceive->hookDomainRegistrationComplete(domainID);
}

void CAmRoutingService::hookSinkAvailablityStatusChange(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_Availability_s availability) {
	assert(mpIAmRoutingReceive);
	am_Availability_s am_avialabilty;
	CAmConvertCAPI2AM(availability, am_avialabilty);
	mpIAmRoutingReceive->hookSinkAvailablityStatusChange(sinkID, am_avialabilty);
}

void CAmRoutingService::hookSourceAvailablityStatusChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_Availability_s availability) {
	assert(mpIAmRoutingReceive);
	am_Availability_s am_availabilty;
	CAmConvertCAPI2AM(availability, am_availabilty);
	mpIAmRoutingReceive->hookSourceAvailablityStatusChange(sourceID, am_availabilty);
}

void CAmRoutingService::hookDomainStateChange(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_DomainState_e domainState) {
	assert(mpIAmRoutingReceive);
	am_DomainState_e am_domainState = static_cast<am_DomainState_e>(domainState);
	mpIAmRoutingReceive->hookDomainStateChange(domainID, am_domainState);
}

void CAmRoutingService::hookTimingInformationChanged(org::genivi::am::am_connectionID_t connectionID, int16_t delay) {
	assert(mpIAmRoutingReceive);
	mpIAmRoutingReceive->hookTimingInformationChanged(connectionID, delay);
}

void CAmRoutingService::sendChangedData(org::genivi::am::am_EarlyData_L earlyData) {

	assert(mpIAmRoutingReceive);
	std::vector<am_EarlyData_s> dest;
	CAmConvertCAPIVector2AM(earlyData,dest);
	mpIAmRoutingReceive->sendChangedData(dest);
}

void CAmRoutingService::updateGateway(org::genivi::am::am_gatewayID_t gatewayID, org::genivi::am::am_ConnectionFormat_L listSourceFormats, org::genivi::am::am_ConnectionFormat_L listSinkFormats, org::genivi::am::am_Convertion_L convertionMatrix, org::genivi::am::am_Error_e& error) {

	assert(mpIAmRoutingReceive);
	std::vector<am_ConnectionFormat_e> destinationSourceConnectionFormats;
	CAmConvertCAPIVector2AM(listSourceFormats, destinationSourceConnectionFormats);

	std::vector<am_ConnectionFormat_e> destinationSinkConnectionFormats;
	CAmConvertCAPIVector2AM(listSinkFormats, destinationSinkConnectionFormats);

	mpIAmRoutingReceive->updateGateway(gatewayID, destinationSourceConnectionFormats, destinationSinkConnectionFormats, convertionMatrix);
}

void CAmRoutingService::updateSink(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_sinkClass_t sinkClassID, org::genivi::am::am_SoundProperty_L listSoundProperties, org::genivi::am::am_ConnectionFormat_L listConnectionFormats, org::genivi::am::am_MainSoundProperty_L listMainSoundProperties, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	std::vector<am_SoundProperty_s> dstListSoundProperties;
    CAmConvertCAPIVector2AM(listSoundProperties, dstListSoundProperties);
    std::vector<am_ConnectionFormat_e> dstListSinkConnectionFormats;
    CAmConvertCAPIVector2AM(listConnectionFormats, dstListSinkConnectionFormats);
    std::vector<am_MainSoundProperty_s> dstListMainSoundProperties;
    CAmConvertCAPIVector2AM(listMainSoundProperties, dstListMainSoundProperties);
    mpIAmRoutingReceive->updateSink( sinkID, sinkClassID, dstListSoundProperties,dstListSinkConnectionFormats,dstListMainSoundProperties);
}

void CAmRoutingService::updateSource(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sourceClass_t sourceClassID, org::genivi::am::am_SoundProperty_L listSoundProperties, org::genivi::am::am_ConnectionFormat_L listConnectionFormats, org::genivi::am::am_MainSoundProperty_L listMainSoundProperties, org::genivi::am::am_Error_e& error) {
	assert(mpIAmRoutingReceive);
	std::vector<am_SoundProperty_s> dstListSoundProperties;
    CAmConvertCAPIVector2AM(listSoundProperties, dstListSoundProperties);
    std::vector<am_ConnectionFormat_e> dstListSinkConnectionFormats;
    CAmConvertCAPIVector2AM(listConnectionFormats, dstListSinkConnectionFormats);
    std::vector<am_MainSoundProperty_s> dstListMainSoundProperties;
    CAmConvertCAPIVector2AM(listMainSoundProperties, dstListMainSoundProperties);
    mpIAmRoutingReceive->updateSource( sourceID, sourceClassID, dstListSoundProperties,dstListSinkConnectionFormats,dstListMainSoundProperties);
}

void CAmRoutingService::ackSetVolumes(org::genivi::am::am_Handle_s handle , org::genivi::am::am_Volumes_L listVolumes, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
	std::vector<am_Volumes_s> list;
	CAmConvertCAPIVector2AM(listVolumes, list);
	am_Error_e amError = static_cast<am_Error_e>(error);
	mpIAmRoutingReceive->ackSetVolumes(handle_s, list, amError);
	mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSinkNotificationConfiguration (org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
    am_Error_e amError = static_cast<am_Error_e>(error);
    mpIAmRoutingReceive->ackSinkNotificationConfiguration(handle_s, amError);
    mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::ackSourceNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error) {
	assert(mpIAmRoutingReceive);
	assert(mpLookpData);
	am_Handle_s handle_s;
	CAmConvertCAPI2AM(handle,handle_s);
    am_Error_e amError = static_cast<am_Error_e>(error);
    mpIAmRoutingReceive->ackSourceNotificationConfiguration(handle_s, amError);
    mpLookpData->removeHandle(handle_s);
}

void CAmRoutingService::hookSinkNotificationDataChange(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_NotificationPayload_s payload) {
	assert(mpIAmRoutingReceive);
	am_NotificationPayload_s converted;
	CAmConvertCAPI2AM(payload, converted);
	mpIAmRoutingReceive->hookSinkNotificationDataChange(sinkID, converted);
}

void CAmRoutingService::hookSourceNotificationDataChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_NotificationPayload_s payload) {
	assert(mpIAmRoutingReceive);
	am_NotificationPayload_s converted;
	CAmConvertCAPI2AM(payload, converted);
	mpIAmRoutingReceive->hookSourceNotificationDataChange(sourceID, converted);
}

void CAmRoutingService::confirmRoutingRundown(std::string domainName)
{
	mNumberDomains--;
	if (mNumberDomains==0)
		mpIAmRoutingReceive->confirmRoutingRundown(mHandle,E_OK);
}

void CAmRoutingService::gotRundown(int16_t numberDomains, uint16_t handle)
{
	mReady=false;
    mNumberDomains=numberDomains;
    mHandle=handle;
}

} /* namespace am */
