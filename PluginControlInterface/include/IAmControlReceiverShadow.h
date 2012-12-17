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

#ifndef IAMCONTROLRECEIVERSHADOW_H_
#define IAMCONTROLRECEIVERSHADOW_H_

#include "audiomanagertypes.h"
#include "shared/CAmSerializer.h"

namespace am
{
class IAmControlReceive;
class CAmSocketHandler;

/**
 * shadow class that used CAmSerializer to make threadsafe calls to the IAmControlReceive interface.
 */
class IAmControlReceiverShadow
{
public:
    IAmControlReceiverShadow(IAmControlReceive* iReceiveInterface, CAmSocketHandler* iSocketHandler);
    ~IAmControlReceiverShadow();
    am_Error_e getRoute(bool onlyfree, am_sourceID_t sourceID, am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);
    am_Error_e connect(am_Handle_s& handle, am_connectionID_t& connectionID, am_ConnectionFormat_e format, am_sourceID_t sourceID, am_sinkID_t sinkID);
    am_Error_e disconnect(am_Handle_s& handle, am_connectionID_t connectionID);
    am_Error_e crossfade(am_Handle_s& handle, am_HotSink_e hotSource, am_crossfaderID_t crossfaderID, am_RampType_e rampType, am_time_t rampTime);
    am_Error_e abortAction(am_Handle_s handle);
    am_Error_e setSourceState(am_Handle_s& handle, am_sourceID_t sourceID, am_SourceState_e state);
    am_Error_e setSinkVolume(am_Handle_s& handle, am_sinkID_t sinkID, am_volume_t volume, am_RampType_e ramp, am_time_t time);
    am_Error_e setSourceVolume(am_Handle_s& handle, am_sourceID_t sourceID, am_volume_t volume, am_RampType_e rampType, am_time_t time);
    am_Error_e setSinkSoundProperties(am_Handle_s& handle, am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& soundProperty);
    am_Error_e setSinkSoundProperty(am_Handle_s& handle, am_sinkID_t sinkID, am_SoundProperty_s& soundProperty);
    am_Error_e setSourceSoundProperties(am_Handle_s& handle, am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& soundProperty);
    am_Error_e setSourceSoundProperty(am_Handle_s& handle, am_sourceID_t sourceID, am_SoundProperty_s& soundProperty);
    am_Error_e setDomainState(am_domainID_t domainID, am_DomainState_e domainState);
    am_Error_e enterDomainDB(am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e enterMainConnectionDB(am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID);
    am_Error_e enterSinkDB(am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e enterCrossfaderDB(am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e enterGatewayDB(am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e enterSourceDB(am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e enterSinkClassDB(am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID);
    am_Error_e enterSourceClassDB(am_sourceClass_t& sourceClassID, am_SourceClass_s& sourceClass);
    am_Error_e changeSinkClassInfoDB(am_SinkClass_s& sinkClass);
    am_Error_e changeSourceClassInfoDB(am_SourceClass_s& sourceClass);
    am_Error_e enterSystemPropertiesListDB(std::vector<am_SystemProperty_s>& listSystemProperties);
    am_Error_e changeMainConnectionRouteDB(am_mainConnectionID_t mainconnectionID, std::vector<am_connectionID_t>& listConnectionID);
    am_Error_e changeMainConnectionStateDB(am_mainConnectionID_t mainconnectionID, am_ConnectionState_e connectionState);
    am_Error_e changeSinkMainVolumeDB(am_mainVolume_t mainVolume, am_sinkID_t sinkID);
    am_Error_e changeSinkAvailabilityDB(am_Availability_s& availability, am_sinkID_t sinkID);
    am_Error_e changDomainStateDB(am_DomainState_e domainState, am_domainID_t domainID);
    am_Error_e changeSinkMuteStateDB(am_MuteState_e muteState, am_sinkID_t sinkID);
    am_Error_e changeMainSinkSoundPropertyDB(am_MainSoundProperty_s& soundProperty, am_sinkID_t sinkID);
    am_Error_e changeMainSourceSoundPropertyDB(am_MainSoundProperty_s& soundProperty, am_sourceID_t sourceID);
    am_Error_e changeSourceAvailabilityDB(am_Availability_s& availability, am_sourceID_t sourceID);
    am_Error_e changeSystemPropertyDB(am_SystemProperty_s& property);
    am_Error_e removeMainConnectionDB(am_mainConnectionID_t mainConnectionID);
    am_Error_e removeSinkDB(am_sinkID_t sinkID);
    am_Error_e removeSourceDB(am_sourceID_t sourceID);
    am_Error_e removeGatewayDB(am_gatewayID_t gatewayID);
    am_Error_e removeCrossfaderDB(am_crossfaderID_t crossfaderID);
    am_Error_e removeDomainDB(am_domainID_t domainID);
    am_Error_e removeSinkClassDB(am_sinkClass_t sinkClassID);
    am_Error_e removeSourceClassDB(am_sourceClass_t sourceClassID);
    am_Error_e getSourceClassInfoDB(am_sourceID_t sourceID, am_SourceClass_s& classInfo);
    am_Error_e getSinkClassInfoDB(am_sinkID_t sinkID, am_SinkClass_s& sinkClass);
    am_Error_e getSinkInfoDB(am_sinkID_t sinkID, am_Sink_s& sinkData);
    am_Error_e getSourceInfoDB(am_sourceID_t sourceID, am_Source_s& sourceData);
    am_Error_e getGatewayInfoDB(am_gatewayID_t gatewayID, am_Gateway_s& gatewayData);
    am_Error_e getCrossfaderInfoDB(am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData);
    am_Error_e getMainConnectionInfoDB(am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData);
    am_Error_e getListSinksOfDomain(am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID);
    am_Error_e getListSourcesOfDomain(am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID);
    am_Error_e getListCrossfadersOfDomain(am_domainID_t domainID, std::vector<am_crossfaderID_t>& listCrossfadersID);
    am_Error_e getListGatewaysOfDomain(am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID);
    am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections);
    am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains);
    am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections);
    am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks);
    am_Error_e getListSources(std::vector<am_Source_s>& listSources);
    am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses);
    am_Error_e getListHandles(std::vector<am_Handle_s>& listHandles);
    am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders);
    am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways);
    am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses);
    am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties);
    void setCommandReady();
    void setCommandRundown();
    void setRoutingReady();
    void setRoutingRundown();
    void confirmControllerReady(am_Error_e error);
    void confirmControllerRundown(am_Error_e error);
    am_Error_e getSocketHandler(CAmSocketHandler*& socketHandler);
private:
    IAmControlReceive *mpIAmControlReceiver;
    CAmSerializer mCAmSerializer;

};

} /* namespace am */
#endif /* IAMCONTROLRECEIVERSHADOW_H_ */
