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

#include <org/genivi/audiomanager/RoutingInterfaceStubDefault.h>
#include "../../include/routing/IAmRoutingReceive.h"
#include "CAmLookupData.h"

namespace am {

class CAmCommonAPIWrapper;

using namespace CommonAPI;
using namespace org::genivi::audiomanager;

/** Routing interface stub implementation.
 * This class is the routing interface service for the Audio Manager.
 */
class CAmRoutingService: public RoutingInterfaceStubDefault {
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

	virtual void ackConnect(uint16_t handle, am_gen::am_connectionID_t connectionID, uint16_t error);

	virtual void ackDisconnect(uint16_t handle, am_gen::am_connectionID_t connectionID, uint16_t error);

	virtual void ackSetSinkVolume(uint16_t handle, am_gen::am_volume_t volume, uint16_t error);

	virtual void ackSetSourceVolume(uint16_t handle, am_gen::am_volume_t volume, uint16_t error);

	virtual void ackSetSourceState(uint16_t handle, uint16_t error);

	virtual void ackSetSinkSoundProperties(uint16_t handle, uint16_t error);

	virtual void ackSetSinkSoundProperty(uint16_t handle, uint16_t error);

	virtual void ackSetSourceSoundProperties(uint16_t handle, uint16_t error);

	virtual void ackSetSourceSoundProperty(uint16_t handle, uint16_t error);

	virtual void ackCrossFading(uint16_t handle, am_gen::am_HotSink_e hotSink, am_gen::am_Error_e returnError);

	virtual void ackSourceVolumeTick(uint16_t handle, am_gen::am_sourceID_t source, am_gen::am_volume_t volume);

	virtual void ackSinkVolumeTick(uint16_t handle, am_gen::am_sinkID_t sink, am_gen::am_volume_t volume);

	virtual void peekDomain(std::string name, am_gen::am_domainID_t& domainID, am_gen::am_Error_e& error);

	virtual void registerDomain(am_gen::am_Domain_s domainData, std::string returnBusname, std::string returnPath, std::string returnInterface, am_gen::am_domainID_t& domainID, am_gen::am_Error_e& error);

	virtual void deregisterDomain(am_gen::am_domainID_t domainID, am_gen::am_Error_e& returnError);

	virtual void registerGateway(am_gen::am_Gateway_s gatewayData, am_gen::am_gatewayID_t& gatewayID, am_gen::am_Error_e& error);

	virtual void deregisterGateway(am_gen::am_gatewayID_t gatewayID, am_gen::am_Error_e& returnError);

	virtual void peekSink(std::string name, am_gen::am_sinkID_t& sinkID, am_gen::am_Error_e& error);

	virtual void registerSink(am_gen::sinkData_s sinkData, am_gen::am_sinkID_t& sinkID, am_gen::am_Error_e& error);

	virtual void deregisterSink(am_gen::am_sinkID_t sinkID, am_gen::am_Error_e& returnError);

	virtual void peekSource(std::string name, am_gen::am_sourceID_t& sourceID, am_gen::am_Error_e& error);

	virtual void registerSource(am_gen::sourceData_s sourceData, am_gen::am_sourceID_t& sourceID, am_gen::am_Error_e& error);

	virtual void deregisterSource(am_gen::am_sourceID_t sourceID, am_gen::am_Error_e& returnError);

	virtual void registerCrossfader(am_gen::crossfaderData_s crossfaderData, am_gen::am_crossfaderID_t& crossfaderID, am_gen::am_Error_e& error);

	virtual void deregisterCrossfader(am_gen::am_crossfaderID_t crossfaderID, am_gen::am_Error_e& returnError);

	virtual void peekSourceClassID(std::string name, am_gen::am_sourceClass_t& sourceClassID, am_gen::am_Error_e& error);

	virtual void peekSinkClassID(std::string name, am_gen::am_sinkClass_t& sinkClassID, am_gen::am_Error_e& error);

	virtual void hookInterruptStatusChange(am_gen::am_sourceID_t sourceID, uint16_t interruptState);

	virtual void hookDomainRegistrationComplete(am_gen::am_domainID_t domainID);

	virtual void hookSinkAvailablityStatusChange(am_gen::am_sinkID_t sinkID, am_gen::am_Availability_s availability);

	virtual void hookSourceAvailablityStatusChange(am_gen::am_sourceID_t sourceID, am_gen::am_Availability_s availability);

	virtual void hookDomainStateChange(am_gen::am_domainID_t domainID, am_gen::am_DomainState_e domainState);

	virtual void hookTimingInformationChanged(am_gen::am_connectionID_t connectionID, int16_t delay);

	virtual void sendChangedData(am_gen::am_EarlyData_l earlyData_volumes, am_gen::am_EarlyData_l earlyData_soundproperties);

	virtual void confirmRoutingReady(am_gen::am_domainID_t domainID);

	virtual void confirmRoutingRundown(am_gen::am_domainID_t domainID);

	virtual void updateGateway(am_gen::am_gatewayID_t gatewayID, am_gen::am_ConnectionFormat_L listSourceFormats, am_gen::am_ConnectionFormat_L listSinkFormats, am_gen::bool_L convertionMatrix);

	virtual void updateSink(am_gen::am_sinkID_t sinkID, am_gen::am_sinkClass_t sinkClassID, am_gen::am_SoundProperty_L listSoundProperties, am_gen::am_ConnectionFormat_L listConnectionFormats, am_gen::am_MainSoundProperty_L listMainSoundProperties);

	virtual void updateSource(am_gen::am_sourceID_t sourceID, am_gen::am_sourceClass_t sourceClassID, am_gen::am_SoundProperty_L listSoundProperties, am_gen::am_ConnectionFormat_L listConnectionFormats, am_gen::am_MainSoundProperty_L listMainSoundProperties);

	virtual void ackSetVolumes(uint16_t handle, am_gen::am_Volumes_l listVolumes, uint16_t error);

	virtual void ackSinkNotificationConfiguration(uint16_t handle, uint16_t error);

	virtual void ackSourceNotificationConfiguration(uint16_t handle, uint16_t error);

	virtual void hookSinkNotificationDataChange(am_gen::am_sinkID_t sinkID, am_gen::notificationPayload_s payload);

	virtual void hookSourceNotificationDataChange(am_gen::am_sourceID_t sourceID, am_gen::notificationPayload_s payload);

	virtual void getRoutingReadyState(bool& readyState);

	void gotReady(int16_t numberDomains, uint16_t handle);
	void gotRundown(int16_t numberDomains, uint16_t handle);
};

} /* namespace am */
#endif /* CAMROUTINGSERVICE_H_ */
