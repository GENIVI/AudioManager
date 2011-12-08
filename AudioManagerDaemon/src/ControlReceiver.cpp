/*
 * ContronlReceiver.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "ControlReceiver.h"

ControlReceiver::ControlReceiver(DatabaseHandler* iDatabaseHandler) : mDatabaseHandler(iDatabaseHandler)
{
}

ControlReceiver::~ControlReceiver()
{
}

am_Error_e ControlReceiver::getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
}



am_Error_e ControlReceiver::connect(am_Handle_s & handle, am_connectionID_t & connectionID, const am_ConnectionFormat_e format, const am_sourceID_t sourceID, const am_sinkID_t sinkID)
{
}



am_Error_e ControlReceiver::disconnect(am_Handle_s & handle, const am_connectionID_t connectionID)
{
}



am_Error_e ControlReceiver::crossfade(am_Handle_s & handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_RampType_e rampType, const am_time_t rampTime)
{
}



am_Error_e ControlReceiver::setSourceState(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
}



am_Error_e ControlReceiver::setSinkVolume(am_Handle_s & handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
}



am_Error_e ControlReceiver::setSourceVolume(am_Handle_s & handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e rampType, const am_time_t time)
{
}



am_Error_e ControlReceiver::setSinkSoundProperty(am_Handle_s & handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
}



am_Error_e ControlReceiver::setSourceSoundProperty(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
}



am_Error_e ControlReceiver::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
}



am_Error_e ControlReceiver::abortAction(const am_Handle_s handle)
{
}



am_Error_e ControlReceiver::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	return mDatabaseHandler->enterDomainDB(domainData,domainID);
}



am_Error_e ControlReceiver::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
	return mDatabaseHandler->enterMainConnectionDB(mainConnectionData,connectionID);
}



am_Error_e ControlReceiver::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
	return mDatabaseHandler->enterSinkDB(sinkData,sinkID);
}



am_Error_e ControlReceiver::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
	return mDatabaseHandler->enterCrossfaderDB(crossfaderData,crossfaderID);
}



am_Error_e ControlReceiver::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
	return mDatabaseHandler->enterGatewayDB(gatewayData,gatewayID);
}



am_Error_e ControlReceiver::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
	return mDatabaseHandler->enterSourceDB(sourceData,sourceID);
}

am_Error_e ControlReceiver::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
	return mDatabaseHandler->enterSinkClassDB(sinkClass,sinkClassID);
}

am_Error_e ControlReceiver::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
	return mDatabaseHandler->enterSourceClassDB(sourceClassID,sourceClass);
}


am_Error_e ControlReceiver::enterSystemPropertiesListDB(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
	return mDatabaseHandler->enterSystemProperties(listSystemProperties);
}


am_Error_e ControlReceiver::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const am_Route_s & route)
{
	return mDatabaseHandler->changeMainConnectionRouteDB(mainconnectionID,route);
}



am_Error_e ControlReceiver::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
	return mDatabaseHandler->changeMainConnectionStateDB(mainconnectionID,connectionState);
}



am_Error_e ControlReceiver::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
	return mDatabaseHandler->changeSinkMainVolumeDB(mainVolume,sinkID);
}



am_Error_e ControlReceiver::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
	return mDatabaseHandler->changeSinkAvailabilityDB(availability,sinkID);
}



am_Error_e ControlReceiver::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
	return mDatabaseHandler->changDomainStateDB(domainState,domainID);
}



am_Error_e ControlReceiver::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
	return mDatabaseHandler->changeSinkMuteStateDB(muteState,sinkID);
}



am_Error_e ControlReceiver::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
	return mDatabaseHandler->changeMainSinkSoundPropertyDB(soundProperty,sinkID);
}



am_Error_e ControlReceiver::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
	return mDatabaseHandler->changeMainSinkSoundPropertyDB(soundProperty,sourceID);
}



am_Error_e ControlReceiver::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
	return mDatabaseHandler->changeSourceAvailabilityDB(availability,sourceID);
}



am_Error_e ControlReceiver::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
	return mDatabaseHandler->changeSystemPropertyDB(property);
}



am_Error_e ControlReceiver::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
	return mDatabaseHandler->removeMainConnectionDB(mainConnectionID);
}



am_Error_e ControlReceiver::removeSinkDB(const am_sinkID_t sinkID)
{
	return mDatabaseHandler->removeSinkDB(sinkID);
}



am_Error_e ControlReceiver::removeSourceDB(const am_sourceID_t sourceID)
{
	return mDatabaseHandler->removeSourceDB(sourceID);
}



am_Error_e ControlReceiver::removeGatewayDB(const am_gatewayID_t gatewayID)
{
	return mDatabaseHandler->removeGatewayDB(gatewayID);
}



am_Error_e ControlReceiver::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{
	return mDatabaseHandler->removeCrossfaderDB(crossfaderID);
}



am_Error_e ControlReceiver::removeDomainDB(const am_domainID_t domainID)
{
	return mDatabaseHandler->removeDomainDB(domainID);
}



am_Error_e ControlReceiver::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{
	return mDatabaseHandler->getSourceClassInfoDB(sourceID,classInfo);
}


am_Error_e ControlReceiver::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
	return mDatabaseHandler->getSinkClassInfoDB(sinkID,sinkClass);
}



am_Error_e ControlReceiver::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
	return mDatabaseHandler->getGatewayInfoDB(gatewayID,gatewayData);
}



am_Error_e ControlReceiver::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
	return mDatabaseHandler->getCrossfaderInfoDB(crossfaderID,crossfaderData);
}



am_Error_e ControlReceiver::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
	return mDatabaseHandler->getListSinksOfDomain(domainID,listSinkID);
}



am_Error_e ControlReceiver::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
	return mDatabaseHandler->getListSourcesOfDomain(domainID,listSourceID);
}



am_Error_e ControlReceiver::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listGatewaysID) const
{
	return mDatabaseHandler->getListCrossfadersOfDomain(domainID,listGatewaysID);
}



am_Error_e ControlReceiver::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
	return mDatabaseHandler->getListGatewaysOfDomain(domainID,listGatewaysID);
}



am_Error_e ControlReceiver::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
	return mDatabaseHandler->getListMainConnections(listMainConnections);
}



am_Error_e ControlReceiver::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
	return mDatabaseHandler->getListDomains(listDomains);
}



am_Error_e ControlReceiver::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
	return mDatabaseHandler->getListConnections(listConnections);
}



am_Error_e ControlReceiver::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
	return mDatabaseHandler->getListSinks(listSinks);
}



am_Error_e ControlReceiver::getListSources(std::vector<am_Source_s> & listSources) const
{
	return mDatabaseHandler->getListSources(listSources);
}



am_Error_e ControlReceiver::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
	return mDatabaseHandler->getListSourceClasses(listSourceClasses);
}



am_Error_e ControlReceiver::getListHandles(std::vector<am_Handle_s> & listHandles) const
{
	//todo: implement getListHandles
}



am_Error_e ControlReceiver::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
	return mDatabaseHandler->getListCrossfaders(listCrossfaders);
}



am_Error_e ControlReceiver::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
	return mDatabaseHandler->getListGateways(listGateways);
}



am_Error_e ControlReceiver::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
	return mDatabaseHandler->getListSinkClasses(listSinkClasses);
}


am_Error_e ControlReceiver::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
	return mDatabaseHandler->getListSystemProperties(listSystemProperties);
}


void ControlReceiver::setRoutingReady()
{
}


am_Error_e ControlReceiver::changeSinkClassInfoDB(const am_SinkClass_s & classInfo)
{
}

am_Error_e ControlReceiver::changeSourceClassInfoDB(const am_SourceClass_s & classInfo)
{
}

am_Error_e ControlReceiver::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
}

am_Error_e ControlReceiver::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
}

void ControlReceiver::setCommandReady()
{
}



