/*
 * IAmControlReceiverShadow.cpp
 *
 *  Created on: Mar 2, 2012
 *      Author: christian
 */

#include "IAmControlReceiverShadow.h"
#include "control/IAmControlReceive.h"

namespace am
{

IAmControlReceiverShadow::IAmControlReceiverShadow(IAmControlReceive *iReceiveInterface, CAmSocketHandler *iSocketHandler) :
        mpIAmControlReceiver(iReceiveInterface), //
        mCAmSerializer(iSocketHandler)
{
}

IAmControlReceiverShadow::~IAmControlReceiverShadow()
{

}

am_Error_e IAmControlReceiverShadow::getRoute(bool onlyfree, am_sourceID_t sourceID, am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const bool, const am_sourceID_t, const am_sinkID_t, std::vector<am_Route_s> &, bool, am_sourceID_t, am_sinkID_t, std::vector<am_Route_s> >(mpIAmControlReceiver, &IAmControlReceive::getRoute, error, onlyfree, sourceID, sinkID, returnList);
    return (error);
}

am_Error_e IAmControlReceiverShadow::connect(am_Handle_s & handle, am_connectionID_t & connectionID, am_ConnectionFormat_e format, am_sourceID_t sourceID, am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, am_connectionID_t &, const am_ConnectionFormat_e, const am_sourceID_t, const am_sinkID_t, am_Handle_s, am_connectionID_t, am_ConnectionFormat_e, am_sourceID_t, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::connect, error, handle, connectionID, format, sourceID, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::disconnect(am_Handle_s & handle, am_connectionID_t connectionID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_connectionID_t, am_Handle_s, am_connectionID_t>(mpIAmControlReceiver, &IAmControlReceive::disconnect, error, handle, connectionID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::crossfade(am_Handle_s & handle, am_HotSink_e hotSource, am_crossfaderID_t crossfaderID, am_RampType_e rampType, am_time_t rampTime)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_HotSink_e, const am_crossfaderID_t, const am_RampType_e, const am_time_t, am_Handle_s, am_HotSink_e, am_crossfaderID_t, am_RampType_e, am_time_t>(mpIAmControlReceiver, &IAmControlReceive::crossfade, error, handle, hotSource, crossfaderID, rampType, rampTime);
    return (error);
}

am_Error_e IAmControlReceiverShadow::abortAction(am_Handle_s handle)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Handle_s, am_Handle_s>(mpIAmControlReceiver, &IAmControlReceive::abortAction, error, handle);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSourceState(am_Handle_s & handle, am_sourceID_t sourceID, am_SourceState_e state)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s&, const am_sourceID_t, const am_SourceState_e, am_Handle_s, am_sourceID_t, am_SourceState_e>(mpIAmControlReceiver, &IAmControlReceive::setSourceState, error, handle, sourceID, state);
    return (error);

}

am_Error_e IAmControlReceiverShadow::setSinkVolume(am_Handle_s & handle, am_sinkID_t sinkID, am_volume_t volume, am_RampType_e ramp, am_time_t time)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s&, const am_sinkID_t, const am_volume_t, const am_RampType_e, const am_time_t, am_Handle_s, am_sinkID_t, am_volume_t, am_RampType_e, am_time_t>(mpIAmControlReceiver, &IAmControlReceive::setSinkVolume, error, handle, sinkID, volume, ramp, time);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSourceVolume(am_Handle_s & handle, am_sourceID_t sourceID, am_volume_t volume, am_RampType_e rampType, am_time_t time)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s&, const am_sourceID_t, const am_volume_t, const am_RampType_e, const am_time_t, am_Handle_s, am_sourceID_t, am_volume_t, am_RampType_e, am_time_t>(mpIAmControlReceiver, &IAmControlReceive::setSourceVolume, error, handle, sourceID, volume, rampType, time);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSinkSoundProperties(am_Handle_s & handle, am_sinkID_t sinkID, std::vector<am_SoundProperty_s> & soundProperty)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_sinkID_t, const std::vector<am_SoundProperty_s> &, am_Handle_s, am_sinkID_t, std::vector<am_SoundProperty_s> >(mpIAmControlReceiver, &IAmControlReceive::setSinkSoundProperties, error, handle, sinkID, soundProperty);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSinkSoundProperty(am_Handle_s & handle, am_sinkID_t sinkID, am_SoundProperty_s & soundProperty)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_sinkID_t, const am_SoundProperty_s &, am_Handle_s, am_sinkID_t, am_SoundProperty_s>(mpIAmControlReceiver, &IAmControlReceive::setSinkSoundProperty, error, handle, sinkID, soundProperty);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSourceSoundProperties(am_Handle_s & handle, am_sourceID_t sourceID, std::vector<am_SoundProperty_s> & soundProperty)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_sourceID_t, const std::vector<am_SoundProperty_s> &, am_Handle_s, am_sourceID_t, std::vector<am_SoundProperty_s> >(mpIAmControlReceiver, &IAmControlReceive::setSourceSoundProperties, error, handle, sourceID, soundProperty);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setSourceSoundProperty(am_Handle_s & handle, am_sourceID_t sourceID, am_SoundProperty_s & soundProperty)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_Handle_s &, const am_sourceID_t, const am_SoundProperty_s &, am_Handle_s, am_sourceID_t, am_SoundProperty_s>(mpIAmControlReceiver, &IAmControlReceive::setSourceSoundProperty, error, handle, sourceID, soundProperty);
    return (error);
}

am_Error_e IAmControlReceiverShadow::setDomainState(am_domainID_t domainID, am_DomainState_e domainState)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, const am_DomainState_e, am_domainID_t, am_DomainState_e>(mpIAmControlReceiver, &IAmControlReceive::setDomainState, error, domainID, domainState);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterDomainDB(am_Domain_s & domainData, am_domainID_t & domainID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Domain_s&, am_domainID_t &, am_Domain_s, am_domainID_t>(mpIAmControlReceiver, &IAmControlReceive::enterDomainDB, error, domainData, domainID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterMainConnectionDB(am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_MainConnection_s &, am_mainConnectionID_t&, am_MainConnection_s, am_mainConnectionID_t>(mpIAmControlReceiver, &IAmControlReceive::enterMainConnectionDB, error, mainConnectionData, connectionID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterSinkDB(am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Sink_s &, am_sinkID_t&, am_Sink_s, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::enterSinkDB, error, sinkData, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterCrossfaderDB(am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Crossfader_s &, am_crossfaderID_t &, am_Crossfader_s, am_crossfaderID_t>(mpIAmControlReceiver, &IAmControlReceive::enterCrossfaderDB, error, crossfaderData, crossfaderID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterGatewayDB(am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Gateway_s &, am_gatewayID_t &, am_Gateway_s, am_gatewayID_t>(mpIAmControlReceiver, &IAmControlReceive::enterGatewayDB, error, gatewayData, gatewayID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterSourceDB(am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Source_s &, am_sourceID_t &, am_Source_s, am_sourceID_t>(mpIAmControlReceiver, &IAmControlReceive::enterSourceDB, error, sourceData, sourceID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterSinkClassDB(am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_SinkClass_s &, am_sinkClass_t &, am_SinkClass_s, am_sinkClass_t>(mpIAmControlReceiver, &IAmControlReceive::enterSinkClassDB, error, sinkClass, sinkClassID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterSourceClassDB(am_sourceClass_t & sourceClassID, am_SourceClass_s & sourceClass)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_sourceClass_t &, const am_SourceClass_s &, am_sourceClass_t, am_SourceClass_s>(mpIAmControlReceiver, &IAmControlReceive::enterSourceClassDB, error, sourceClassID, sourceClass);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSinkClassInfoDB(am_SinkClass_s & sinkClass)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_SinkClass_s &, am_SinkClass_s>(mpIAmControlReceiver, &IAmControlReceive::changeSinkClassInfoDB, error, sinkClass);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSourceClassInfoDB(am_SourceClass_s & sourceClass)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_SourceClass_s &, am_SourceClass_s>(mpIAmControlReceiver, &IAmControlReceive::changeSourceClassInfoDB, error, sourceClass);
    return (error);
}

am_Error_e IAmControlReceiverShadow::enterSystemPropertiesListDB(std::vector<am_SystemProperty_s> & listSystemProperties)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const std::vector<am_SystemProperty_s> &, std::vector<am_SystemProperty_s> >(mpIAmControlReceiver, &IAmControlReceive::enterSystemPropertiesListDB, error, listSystemProperties);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeMainConnectionRouteDB(am_mainConnectionID_t mainconnectionID, std::vector<am_connectionID_t> & listConnectionID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_mainConnectionID_t, const std::vector<am_connectionID_t> &, am_mainConnectionID_t, std::vector<am_connectionID_t> >(mpIAmControlReceiver, &IAmControlReceive::changeMainConnectionRouteDB, error, mainconnectionID, listConnectionID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeMainConnectionStateDB(am_mainConnectionID_t mainconnectionID, am_ConnectionState_e connectionState)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_mainConnectionID_t, const am_ConnectionState_e, am_mainConnectionID_t, am_ConnectionState_e>(mpIAmControlReceiver, &IAmControlReceive::changeMainConnectionStateDB, error, mainconnectionID, connectionState);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSinkMainVolumeDB(am_mainVolume_t mainVolume, am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_mainVolume_t, const am_sinkID_t, am_mainVolume_t, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::changeSinkMainVolumeDB, error, mainVolume, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_MainConnection_s> &, std::vector<am_MainConnection_s> >(mpIAmControlReceiver, &IAmControlReceive::getListMainConnections, error, listMainConnections);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListDomains(std::vector<am_Domain_s> & listDomains)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Domain_s> &, std::vector<am_Domain_s> >(mpIAmControlReceiver, &IAmControlReceive::getListDomains, error, listDomains);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListConnections(std::vector<am_Connection_s> & listConnections)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Connection_s>&, std::vector<am_Connection_s> >(mpIAmControlReceiver, &IAmControlReceive::getListConnections, error, listConnections);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSinks(std::vector<am_Sink_s> & listSinks)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Sink_s> &, std::vector<am_Sink_s> >(mpIAmControlReceiver, &IAmControlReceive::getListSinks, error, listSinks);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSources(std::vector<am_Source_s> & listSources)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Source_s> &, std::vector<am_Source_s> >(mpIAmControlReceiver, &IAmControlReceive::getListSources, error, listSources);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_SourceClass_s> &, std::vector<am_SourceClass_s> >(mpIAmControlReceiver, &IAmControlReceive::getListSourceClasses, error, listSourceClasses);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListHandles(std::vector<am_Handle_s> & listHandles)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Handle_s> &, std::vector<am_Handle_s> >(mpIAmControlReceiver, &IAmControlReceive::getListHandles, error, listHandles);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Crossfader_s> &, std::vector<am_Crossfader_s> >(mpIAmControlReceiver, &IAmControlReceive::getListCrossfaders, error, listCrossfaders);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListGateways(std::vector<am_Gateway_s> & listGateways)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_Gateway_s>&, std::vector<am_Gateway_s> >(mpIAmControlReceiver, &IAmControlReceive::getListGateways, error, listGateways);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_SinkClass_s> &, std::vector<am_SinkClass_s> >(mpIAmControlReceiver, &IAmControlReceive::getListSinkClasses, error, listSinkClasses);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, std::vector<am_SystemProperty_s> &, std::vector<am_SystemProperty_s> >(mpIAmControlReceiver, &IAmControlReceive::getListSystemProperties, error, listSystemProperties);
    return (error);
}

void IAmControlReceiverShadow::setCommandReady()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::setCommandReady);
}

void IAmControlReceiverShadow::setCommandRundown()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::setCommandRundown);
}

void IAmControlReceiverShadow::setRoutingReady()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::setRoutingReady);
}

void IAmControlReceiverShadow::setRoutingRundown()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::setRoutingRundown);
}

void IAmControlReceiverShadow::confirmControllerReady()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::confirmControllerReady);
}

void IAmControlReceiverShadow::confirmControllerRundown()
{
    mCAmSerializer.asyncCall<IAmControlReceive>(mpIAmControlReceiver, &IAmControlReceive::confirmControllerRundown);
}

am_Error_e IAmControlReceiverShadow::getSocketHandler(CAmSocketHandler *& socketHandler)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, CAmSocketHandler*&, CAmSocketHandler*>(mpIAmControlReceiver, &IAmControlReceive::getSocketHandler, error, socketHandler);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSinkAvailabilityDB(am_Availability_s& availability, am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Availability_s&, const am_sinkID_t, am_Availability_s, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::changeSinkAvailabilityDB, error, availability, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changDomainStateDB(am_DomainState_e domainState, am_domainID_t domainID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_DomainState_e, const am_domainID_t, am_DomainState_e, am_domainID_t>(mpIAmControlReceiver, &IAmControlReceive::changDomainStateDB, error, domainState, domainID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSinkMuteStateDB(am_MuteState_e muteState, am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, am_MuteState_e, const am_sinkID_t, am_MuteState_e, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::changeSinkMuteStateDB, error, muteState, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeMainSinkSoundPropertyDB(am_MainSoundProperty_s& soundProperty, am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_MainSoundProperty_s&, const am_sinkID_t, am_MainSoundProperty_s, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::changeMainSinkSoundPropertyDB, error, soundProperty, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeMainSourceSoundPropertyDB(am_MainSoundProperty_s& soundProperty, am_sourceID_t sourceID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_MainSoundProperty_s&, const am_sourceID_t, am_MainSoundProperty_s, am_sourceID_t>(mpIAmControlReceiver, &IAmControlReceive::changeMainSourceSoundPropertyDB, error, soundProperty, sourceID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSourceAvailabilityDB(am_Availability_s& availability, am_sourceID_t sourceID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_Availability_s&, const am_sourceID_t, am_Availability_s, am_sourceID_t>(mpIAmControlReceiver, &IAmControlReceive::changeSourceAvailabilityDB, error, availability, sourceID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::changeSystemPropertyDB(am_SystemProperty_s& property)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_SystemProperty_s&, am_SystemProperty_s>(mpIAmControlReceiver, &IAmControlReceive::changeSystemPropertyDB, error, property);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeMainConnectionDB(am_mainConnectionID_t mainConnectionID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_mainConnectionID_t, am_mainConnectionID_t>(mpIAmControlReceiver, &IAmControlReceive::removeMainConnectionDB, error, mainConnectionID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeSinkDB(am_sinkID_t sinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sinkID_t, am_sinkID_t>(mpIAmControlReceiver, &IAmControlReceive::removeSinkDB, error, sinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeSourceDB(am_sourceID_t sourceID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sourceID_t, am_sourceID_t>(mpIAmControlReceiver, &IAmControlReceive::removeSourceDB, error, sourceID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeGatewayDB(am_gatewayID_t gatewayID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_gatewayID_t, am_gatewayID_t>(mpIAmControlReceiver, &IAmControlReceive::removeGatewayDB, error, gatewayID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeCrossfaderDB(am_crossfaderID_t crossfaderID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_crossfaderID_t, am_crossfaderID_t>(mpIAmControlReceiver, &IAmControlReceive::removeCrossfaderDB, error, crossfaderID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeDomainDB(am_domainID_t domainID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, am_domainID_t>(mpIAmControlReceiver, &IAmControlReceive::removeDomainDB, error, domainID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeSinkClassDB(am_sinkClass_t sinkClassID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sinkClass_t, am_sinkClass_t>(mpIAmControlReceiver, &IAmControlReceive::removeSinkClassDB, error, sinkClassID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::removeSourceClassDB(am_sourceClass_t sourceClassID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sourceClass_t, am_sourceClass_t>(mpIAmControlReceiver, &IAmControlReceive::removeSourceClassDB, error, sourceClassID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getSourceClassInfoDB(am_sourceID_t sourceID, am_SourceClass_s& classInfo)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sourceID_t, am_SourceClass_s&, am_sourceID_t, am_SourceClass_s>(mpIAmControlReceiver, &IAmControlReceive::getSourceClassInfoDB, error, sourceID, classInfo);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getSinkClassInfoDB(am_sinkID_t sinkID, am_SinkClass_s& sinkClass)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sinkID_t, am_SinkClass_s&, am_sinkID_t, am_SinkClass_s>(mpIAmControlReceiver, &IAmControlReceive::getSinkClassInfoDB, error, sinkID, sinkClass);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getSinkInfoDB(am_sinkID_t sinkID, am_Sink_s& sinkData)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sinkID_t, am_Sink_s&, am_sinkID_t, am_Sink_s>(mpIAmControlReceiver, &IAmControlReceive::getSinkInfoDB, error, sinkID, sinkData);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getSourceInfoDB(am_sourceID_t sourceID, am_Source_s& sourceData)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_sourceID_t, am_Source_s&, am_sourceID_t, am_Source_s>(mpIAmControlReceiver, &IAmControlReceive::getSourceInfoDB, error, sourceID, sourceData);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getGatewayInfoDB(am_gatewayID_t gatewayID, am_Gateway_s& gatewayData)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_gatewayID_t, am_Gateway_s&, am_gatewayID_t, am_Gateway_s>(mpIAmControlReceiver, &IAmControlReceive::getGatewayInfoDB, error, gatewayID, gatewayData);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getCrossfaderInfoDB(am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_crossfaderID_t, am_Crossfader_s&, am_crossfaderID_t, am_Crossfader_s>(mpIAmControlReceiver, &IAmControlReceive::getCrossfaderInfoDB, error, crossfaderID, crossfaderData);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getMainConnectionInfoDB(am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_mainConnectionID_t, am_MainConnection_s&, am_mainConnectionID_t, am_MainConnection_s>(mpIAmControlReceiver, &IAmControlReceive::getMainConnectionInfoDB, error, mainConnectionID, mainConnectionData);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSinksOfDomain(am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, std::vector<am_sinkID_t>&, am_domainID_t, std::vector<am_sinkID_t> >(mpIAmControlReceiver, &IAmControlReceive::getListSinksOfDomain, error, domainID, listSinkID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListSourcesOfDomain(am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, std::vector<am_sourceID_t>&, am_domainID_t, std::vector<am_sourceID_t> >(mpIAmControlReceiver, &IAmControlReceive::getListSourcesOfDomain, error, domainID, listSourceID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListCrossfadersOfDomain(am_domainID_t domainID, std::vector<am_crossfaderID_t>& listCrossfadersID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, std::vector<am_crossfaderID_t>&, am_domainID_t, std::vector<am_crossfaderID_t> >(mpIAmControlReceiver, &IAmControlReceive::getListCrossfadersOfDomain, error, domainID, listCrossfadersID);
    return (error);
}

am_Error_e IAmControlReceiverShadow::getListGatewaysOfDomain(am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID)
{
    am_Error_e error;
    mCAmSerializer.syncCall<IAmControlReceive, am_Error_e, const am_domainID_t, std::vector<am_gatewayID_t>&, am_domainID_t, std::vector<am_gatewayID_t> >(mpIAmControlReceiver, &IAmControlReceive::getListGatewaysOfDomain, error, domainID, listGatewaysID);
    return (error);
}

} /* namespace am */
