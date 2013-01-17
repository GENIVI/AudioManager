/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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

#ifndef MOCKCONTROLINTERFACE_H_
#define MOCKCONTROLINTERFACE_H_

#include "control/IAmControlReceive.h"


namespace am {

class MockIAmControlReceive : public IAmControlReceive {
 public:
    MOCK_CONST_METHOD1(getInterfaceVersion,
        void(std::string& version));
    MOCK_METHOD4(getRoute,
        am_Error_e(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList));
    MOCK_METHOD5(connect,
        am_Error_e(am_Handle_s& handle, am_connectionID_t& connectionID, const am_ConnectionFormat_e format, const am_sourceID_t sourceID, const am_sinkID_t sinkID));
    MOCK_METHOD2(disconnect,
        am_Error_e(am_Handle_s& handle, const am_connectionID_t connectionID));
    MOCK_METHOD5(crossfade,
        am_Error_e(am_Handle_s& handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_RampType_e rampType, const am_time_t rampTime));
    MOCK_METHOD1(abortAction,
        am_Error_e(const am_Handle_s handle));
    MOCK_METHOD3(setSourceState,
        am_Error_e(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state));
    MOCK_METHOD5(setSinkVolume,
        am_Error_e(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time));
    MOCK_METHOD5(setSourceVolume,
        am_Error_e(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e rampType, const am_time_t time));
    MOCK_METHOD3(setSinkSoundProperties,
        am_Error_e(am_Handle_s& handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& soundProperty));
    MOCK_METHOD3(setSinkSoundProperty,
        am_Error_e(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty));
    MOCK_METHOD3(setSourceSoundProperties,
        am_Error_e(am_Handle_s& handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& soundProperty));
    MOCK_METHOD3(setSourceSoundProperty,
        am_Error_e(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty));
    MOCK_METHOD2(setDomainState,
        am_Error_e(const am_domainID_t domainID, const am_DomainState_e domainState));
    MOCK_METHOD2(enterDomainDB,
        am_Error_e(const am_Domain_s& domainData, am_domainID_t& domainID));
    MOCK_METHOD2(enterMainConnectionDB,
        am_Error_e(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID));
    MOCK_METHOD2(enterSinkDB,
        am_Error_e(const am_Sink_s& sinkData, am_sinkID_t& sinkID));
    MOCK_METHOD2(enterCrossfaderDB,
        am_Error_e(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID));
    MOCK_METHOD2(enterGatewayDB,
        am_Error_e(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID));
    MOCK_METHOD2(enterSourceDB,
        am_Error_e(const am_Source_s& sourceData, am_sourceID_t& sourceID));
    MOCK_METHOD2(enterSinkClassDB,
        am_Error_e(const am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID));
    MOCK_METHOD2(enterSourceClassDB,
        am_Error_e(am_sourceClass_t& sourceClassID, const am_SourceClass_s& sourceClass));
    MOCK_METHOD1(changeSinkClassInfoDB,
        am_Error_e(const am_SinkClass_s& sinkClass));
    MOCK_METHOD1(changeSourceClassInfoDB,
        am_Error_e(const am_SourceClass_s& sourceClass));
    MOCK_METHOD1(enterSystemPropertiesListDB,
        am_Error_e(const std::vector<am_SystemProperty_s>& listSystemProperties));
    MOCK_METHOD2(changeMainConnectionRouteDB,
        am_Error_e(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID));
    MOCK_METHOD2(changeMainConnectionStateDB,
        am_Error_e(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState));
    MOCK_METHOD2(changeSinkMainVolumeDB,
        am_Error_e(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID));
    MOCK_METHOD2(changeSinkAvailabilityDB,
        am_Error_e(const am_Availability_s& availability, const am_sinkID_t sinkID));
    MOCK_METHOD2(changDomainStateDB,
        am_Error_e(const am_DomainState_e domainState, const am_domainID_t domainID));
    MOCK_METHOD2(changeSinkMuteStateDB,
        am_Error_e(const am_MuteState_e muteState, const am_sinkID_t sinkID));
    MOCK_METHOD2(changeMainSinkSoundPropertyDB,
        am_Error_e(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID));
    MOCK_METHOD2(changeMainSourceSoundPropertyDB,
        am_Error_e(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID));
    MOCK_METHOD2(changeSourceAvailabilityDB,
        am_Error_e(const am_Availability_s& availability, const am_sourceID_t sourceID));
    MOCK_METHOD1(changeSystemPropertyDB,
        am_Error_e(const am_SystemProperty_s& property));
    MOCK_METHOD1(removeMainConnectionDB,
        am_Error_e(const am_mainConnectionID_t mainConnectionID));
    MOCK_METHOD1(removeSinkDB,
        am_Error_e(const am_sinkID_t sinkID));
    MOCK_METHOD1(removeSourceDB,
        am_Error_e(const am_sourceID_t sourceID));
    MOCK_METHOD1(removeGatewayDB,
        am_Error_e(const am_gatewayID_t gatewayID));
    MOCK_METHOD1(removeCrossfaderDB,
        am_Error_e(const am_crossfaderID_t crossfaderID));
    MOCK_METHOD1(removeDomainDB,
        am_Error_e(const am_domainID_t domainID));
    MOCK_METHOD1(removeSinkClassDB,
        am_Error_e(const am_sinkClass_t sinkClassID));
    MOCK_METHOD1(removeSourceClassDB,
        am_Error_e(const am_sourceClass_t sourceClassID));
    MOCK_CONST_METHOD2(getSourceClassInfoDB,
        am_Error_e(const am_sourceID_t sourceID, am_SourceClass_s& classInfo));
    MOCK_CONST_METHOD2(getSinkClassInfoDB,
        am_Error_e(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass));
    MOCK_CONST_METHOD2(getSinkInfoDB,
        am_Error_e(const am_sinkID_t sinkID, am_Sink_s& sinkData));
    MOCK_CONST_METHOD2(getSourceInfoDB,
        am_Error_e(const am_sourceID_t sourceID, am_Source_s& sourceData));
    MOCK_CONST_METHOD2(getGatewayInfoDB,
        am_Error_e(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData));
    MOCK_CONST_METHOD2(getCrossfaderInfoDB,
        am_Error_e(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData));
    MOCK_CONST_METHOD2(getMainConnectionInfoDB,
        am_Error_e(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData));
    MOCK_CONST_METHOD2(getListSinksOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID));
    MOCK_CONST_METHOD2(getListSourcesOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID));
    MOCK_CONST_METHOD2(getListCrossfadersOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listCrossfadersID));
    MOCK_CONST_METHOD2(getListGatewaysOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID));
    MOCK_CONST_METHOD1(getListMainConnections,
        am_Error_e(std::vector<am_MainConnection_s>& listMainConnections));
    MOCK_CONST_METHOD1(getListDomains,
        am_Error_e(std::vector<am_Domain_s>& listDomains));
    MOCK_CONST_METHOD1(getListConnections,
        am_Error_e(std::vector<am_Connection_s>& listConnections));
    MOCK_CONST_METHOD1(getListSinks,
        am_Error_e(std::vector<am_Sink_s>& listSinks));
    MOCK_CONST_METHOD1(getListSources,
        am_Error_e(std::vector<am_Source_s>& listSources));
    MOCK_CONST_METHOD1(getListSourceClasses,
        am_Error_e(std::vector<am_SourceClass_s>& listSourceClasses));
    MOCK_CONST_METHOD1(getListHandles,
        am_Error_e(std::vector<am_Handle_s>& listHandles));
    MOCK_CONST_METHOD1(getListCrossfaders,
        am_Error_e(std::vector<am_Crossfader_s>& listCrossfaders));
    MOCK_CONST_METHOD1(getListGateways,
        am_Error_e(std::vector<am_Gateway_s>& listGateways));
    MOCK_CONST_METHOD1(getListSinkClasses,
        am_Error_e(std::vector<am_SinkClass_s>& listSinkClasses));
    MOCK_CONST_METHOD1(getListSystemProperties,
        am_Error_e(std::vector<am_SystemProperty_s>& listSystemProperties));
    MOCK_METHOD0(setCommandReady,
        void());
    MOCK_METHOD0(setCommandRundown,
        void());
    MOCK_METHOD0(setRoutingReady,
        void());
    MOCK_METHOD0(setRoutingRundown,
        void());
    MOCK_METHOD1(confirmControllerReady,
        void(const am_Error_e error));
    MOCK_METHOD1(confirmControllerRundown,
        void(const am_Error_e error));
    MOCK_METHOD1(getSocketHandler,
        am_Error_e(CAmSocketHandler*& socketHandler));
    MOCK_METHOD5(changeSourceDB,
        am_Error_e(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_ConnectionFormat_e>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties));
    MOCK_METHOD5(changeSinkDB,
        am_Error_e(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_ConnectionFormat_e>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties));
    MOCK_METHOD4(changeGatewayDB,
        am_Error_e(const am_gatewayID_t gatewayID, const std::vector<am_ConnectionFormat_e>& listSourceConnectionFormats, const std::vector<am_ConnectionFormat_e>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix));
    MOCK_METHOD2(setVolumes,
        am_Error_e(am_Handle_s& handle, const std::vector<am_Volumes_s>& listVolumes));
    MOCK_METHOD3(setSinkNotificationConfiguration,
        am_Error_e(am_Handle_s& handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration));
    MOCK_METHOD3(setSourceNotificationConfiguration,
        am_Error_e(am_Handle_s& handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration));
    MOCK_METHOD2(sendMainSinkNotificationPayload,
        void(const am_sinkID_t sinkID, const am_NotificationPayload_s& notificationPayload));
    MOCK_METHOD2(sendMainSourceNotificationPayload,
        void(const am_sourceID_t sourceID, const am_NotificationPayload_s& notificationPayload));
    MOCK_METHOD2(changeMainSinkNotificationConfigurationDB,
        am_Error_e(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration));
    MOCK_METHOD2(changeMainSourceNotificationConfigurationDB,
        am_Error_e(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration));
    MOCK_METHOD1(getRestartReasonPropertyNSM,
        am_Error_e(NsmRestartReason_e& restartReason));
    MOCK_METHOD1(getShutdownReasonPropertyNSM,
        am_Error_e(NsmShutdownReason_e& ShutdownReason));
    MOCK_METHOD1(getRunningReasonPropertyNSM,
        am_Error_e(NsmRunningReason_e& nsmRunningReason));
    MOCK_METHOD1(getNodeStateNSM,
        NsmErrorStatus_e(NsmNodeState_e& nsmNodeState));
    MOCK_METHOD3(getSessionStateNSM,
        NsmErrorStatus_e(const std::string& sessionName, const NsmSeat_e seatID, NsmSessionState_e& sessionState));
    MOCK_METHOD1(getApplicationModeNSM,
        NsmErrorStatus_e(NsmApplicationMode_e& applicationMode));
    MOCK_METHOD2(registerShutdownClientNSM,
        NsmErrorStatus_e(const uint32_t shutdownMode, const uint32_t timeoutMs));
    MOCK_METHOD1(unRegisterShutdownClientNSM,
        NsmErrorStatus_e(const uint32_t shutdownMode));
    MOCK_METHOD1(getInterfaceVersionNSM,
        am_Error_e(uint32_t& version));
    MOCK_METHOD2(sendLifecycleRequestCompleteNSM,
        NsmErrorStatus_e(const uint32_t RequestId, const NsmErrorStatus_e status));

};

} // namespace am
#endif
