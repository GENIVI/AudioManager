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

#ifndef CAMROUTINGSERVICE_H_
#define CAMROUTINGSERVICE_H_

#include <org/genivi/am/RoutingControlObserverStubDefault.h>
#include "../../include/routing/IAmRoutingReceive.h"
#include "CAmLookupData.h"

namespace am {

class CAmCommonAPIWrapper;

using namespace CommonAPI;

/** Routing interface stub implementation.
 * This class is the routing interface service for the Audio Manager.
 */
class CAmRoutingService: public org::genivi::am::RoutingControlObserverStubDefault {
	CAmCommonAPIWrapper *mpCAmCAPIWrapper; ///< pointer to common-api wrapper
	IAmRoutingReceive* mpIAmRoutingReceive; ///< pointer to the routing receive interface
	CAmLookupData*   mpLookpData;			///< pointer to the plugin's lookup mechanism implementation
    int16_t mNumberDomains;	///< int number of registred domains
    uint16_t mHandle;		///< unsigned current handle
    bool mReady;			///< bool whether the service is in ready state or not
    CAmRoutingService();
public:

	CAmRoutingService(IAmRoutingReceive *aReceiver, CAmLookupData*   aLookpData, CAmCommonAPIWrapper *aCAPIWrapper);
	virtual ~CAmRoutingService();


	/** Stub overwritten methods.
	 *
	 */

	void ackConnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_Error_e error);

	void ackDisconnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_Error_e error);

	void ackSetSinkVolumeChange(org::genivi::am::am_Handle_s handle, org::genivi::am::am_volume_t volume, org::genivi::am::am_Error_e error);

	void ackSetSourceVolumeChange(org::genivi::am::am_Handle_s handle, org::genivi::am::am_volume_t volume, org::genivi::am::am_Error_e error);

	void ackSetSourceState(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackSetSinkSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackSetSinkSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackSetSourceSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackSetSourceSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackCrossFading(org::genivi::am::am_Handle_s handle, org::genivi::am::am_HotSink_e hotSink, org::genivi::am::am_Error_e error);

	void ackSourceVolumeTick(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t source, org::genivi::am::am_volume_t volume);

	void ackSinkVolumeTick(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sink, org::genivi::am::am_volume_t volume);

	void peekDomain(std::string name, org::genivi::am::am_domainID_t& domainID, org::genivi::am::am_Error_e& error);

	void registerDomain(org::genivi::am::am_Domain_s domainData, std::string returnBusname, std::string returnInterface, org::genivi::am::am_domainID_t& domainID, org::genivi::am::am_Error_e& error);

	void deregisterDomain(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_Error_e& returnError);

	void registerGateway(org::genivi::am::am_Gateway_s gatewayData, org::genivi::am::am_gatewayID_t& gatewayID, org::genivi::am::am_Error_e& error);

	void registerConverter(org::genivi::am::am_Converter_s aData, org::genivi::am::am_converterID_t& converterID, org::genivi::am::am_Error_e& error);

	void deregisterGateway(org::genivi::am::am_gatewayID_t gatewayID, org::genivi::am::am_Error_e& returnError);

	void deregisterConverter(org::genivi::am::am_converterID_t converterID, org::genivi::am::am_Error_e& returnError);

	void peekSink(std::string name, org::genivi::am::am_sinkID_t& sinkID, org::genivi::am::am_Error_e& error);

	void registerSink(org::genivi::am::am_Sink_s sinkData, org::genivi::am::am_sinkID_t& sinkID, org::genivi::am::am_Error_e& error);

	void deregisterSink(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_Error_e& returnError);

	void peekSource(std::string name, org::genivi::am::am_sourceID_t& sourceID, org::genivi::am::am_Error_e& error);

	void registerSource(org::genivi::am::am_Source_s sourceData, org::genivi::am::am_sourceID_t& sourceID, org::genivi::am::am_Error_e& error);

	void deregisterSource(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_Error_e& returnError);

	void registerCrossfader(org::genivi::am::am_Crossfader_s crossfaderData, org::genivi::am::am_crossfaderID_t& crossfaderID, org::genivi::am::am_Error_e& error);

	void deregisterCrossfader(org::genivi::am::am_crossfaderID_t crossfaderID, org::genivi::am::am_Error_e& returnError);

	void peekSourceClassID(std::string name, org::genivi::am::am_sourceClass_t& sourceClassID, org::genivi::am::am_Error_e& error);

	void peekSinkClassID(std::string name, org::genivi::am::am_sinkClass_t& sinkClassID, org::genivi::am::am_Error_e& error);

	void hookInterruptStatusChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_InterruptState_e InterruptState);

	void hookDomainRegistrationComplete(org::genivi::am::am_domainID_t domainID);

	void hookSinkAvailablityStatusChange(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_Availability_s availability);

	void hookSourceAvailablityStatusChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_Availability_s availability);

	void hookDomainStateChange(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_DomainState_e domainState);

	void hookTimingInformationChanged(org::genivi::am::am_connectionID_t connectionID, int16_t delay);

	void sendChangedData(org::genivi::am::am_EarlyData_L earlyData);

	void updateGateway(org::genivi::am::am_gatewayID_t gatewayID, org::genivi::am::am_ConnectionFormat_L listSourceFormats, org::genivi::am::am_ConnectionFormat_L listSinkFormats, org::genivi::am::am_Convertion_L convertionMatrix, org::genivi::am::am_Error_e& error);

	void updateConverter(org::genivi::am::am_converterID_t converterID, org::genivi::am::am_ConnectionFormat_L listSourceFormats, org::genivi::am::am_ConnectionFormat_L listSinkFormats, org::genivi::am::am_Convertion_L convertionMatrix, org::genivi::am::am_Error_e& error);

	void updateSink(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_sinkClass_t sinkClassID, org::genivi::am::am_SoundProperty_L listSoundProperties, org::genivi::am::am_ConnectionFormat_L listConnectionFormats, org::genivi::am::am_MainSoundProperty_L listMainSoundProperties, org::genivi::am::am_Error_e& error);

	void updateSource(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sourceClass_t sourceClassID, org::genivi::am::am_SoundProperty_L listSoundProperties, org::genivi::am::am_ConnectionFormat_L listConnectionFormats, org::genivi::am::am_MainSoundProperty_L listMainSoundProperties, org::genivi::am::am_Error_e& error);

	void ackSetVolumes(org::genivi::am::am_Handle_s handle , org::genivi::am::am_Volumes_L listVolumes, org::genivi::am::am_Error_e error);

	void ackSinkNotificationConfiguration (org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void ackSourceNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Error_e error);

	void hookSinkNotificationDataChange(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_NotificationPayload_s payload);

	void hookSourceNotificationDataChange(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_NotificationPayload_s payload);

	void confirmRoutingRundown(std::string domainName);

	void gotRundown(int16_t numberDomains, uint16_t handle);

	void setHandle(uint16_t handle) {mHandle=handle;}

};

} /* namespace am */
#endif /* CAMROUTINGSERVICE_H_ */
