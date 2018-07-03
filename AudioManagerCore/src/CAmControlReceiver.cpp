/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \file CAmControlReceiver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmControlReceiver.h"
#include <cassert>
#include <stdlib.h>
#include <stdexcept>
#include "audiomanagerconfig.h"
#include "IAmDatabaseHandler.h"
#include "CAmRoutingSender.h"
#include "CAmCommandSender.h"
#include "CAmRouter.h"
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

namespace am {

#define __METHOD_NAME__ std::string (std::string("CAmControlReceiver::") + __func__)

CAmControlReceiver::CAmControlReceiver(IAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmCommandSender *iCommandSender, CAmSocketHandler *iSocketHandler, CAmRouter* iRouter) :
        mDatabaseHandler(iDatabaseHandler),
        mRoutingSender(iRoutingSender),
        mCommandSender(iCommandSender),
        mSocketHandler(iSocketHandler),
        mRouter(iRouter),
        mNodeStateCommunicator(NULL)
{
    assert(mDatabaseHandler!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mCommandSender!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mRouter!=NULL);
}

CAmControlReceiver::~CAmControlReceiver()
{
}

am_Error_e CAmControlReceiver::getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
    return (mRouter->getRoute(onlyfree, sourceID, sinkID, returnList));
}

am_Error_e CAmControlReceiver::connect(am_Handle_s & handle, am_connectionID_t & connectionID, const am_CustomConnectionFormat_t format, const am_sourceID_t sourceID, const am_sinkID_t sinkID)
{
	return (mRoutingSender->asyncConnect(handle, connectionID, sourceID, sinkID, format));
}

am_Error_e CAmControlReceiver::disconnect(am_Handle_s & handle, const am_connectionID_t connectionID)
{
    return (mRoutingSender->asyncDisconnect(handle, connectionID));
}

am_Error_e CAmControlReceiver::crossfade(am_Handle_s & handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_CustomRampType_t rampType, const am_time_t rampTime)
{
    return (mRoutingSender->asyncCrossFade(handle, crossfaderID, hotSource, rampType, rampTime));
}

am_Error_e CAmControlReceiver::setSourceState(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    return (mRoutingSender->asyncSetSourceState(handle, sourceID, state));
}

am_Error_e CAmControlReceiver::setSinkVolume(am_Handle_s & handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time)
{
    return (mRoutingSender->asyncSetSinkVolume(handle, sinkID, volume, ramp, time));
}

am_Error_e CAmControlReceiver::setSourceVolume(am_Handle_s & handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t rampType, const am_time_t time)
{
    return (mRoutingSender->asyncSetSourceVolume(handle, sourceID, volume, rampType, time));
}

am_Error_e CAmControlReceiver::setSinkSoundProperty(am_Handle_s & handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    return (mRoutingSender->asyncSetSinkSoundProperty(handle, sinkID, soundProperty));
}

am_Error_e CAmControlReceiver::setSinkSoundProperties(am_Handle_s & handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    return (mRoutingSender->asyncSetSinkSoundProperties(handle, listSoundProperties, sinkID));
}

am_Error_e CAmControlReceiver::setSourceSoundProperty(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    return (mRoutingSender->asyncSetSourceSoundProperty(handle, sourceID, soundProperty));
}

am_Error_e CAmControlReceiver::setSourceSoundProperties(am_Handle_s & handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    return (mRoutingSender->asyncSetSourceSoundProperties(handle, listSoundProperties, sourceID));
}

am_Error_e CAmControlReceiver::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    return (mRoutingSender->setDomainState(domainID, domainState));
}

am_Error_e CAmControlReceiver::abortAction(const am_Handle_s handle)
{
    return (mRoutingSender->asyncAbort(handle));
}

am_Error_e CAmControlReceiver::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return (mDatabaseHandler->enterDomainDB(domainData, domainID));
}

am_Error_e CAmControlReceiver::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    return (mDatabaseHandler->enterMainConnectionDB(mainConnectionData, connectionID));
}

am_Error_e CAmControlReceiver::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return (mDatabaseHandler->enterSinkDB(sinkData, sinkID));
}

am_Error_e CAmControlReceiver::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return (mDatabaseHandler->enterCrossfaderDB(crossfaderData, crossfaderID));
}

am_Error_e CAmControlReceiver::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return (mDatabaseHandler->enterGatewayDB(gatewayData, gatewayID));
}

am_Error_e CAmControlReceiver::enterConverterDB(const am_Converter_s & converterData, am_converterID_t & converterID)
{
    return (mDatabaseHandler->enterConverterDB(converterData, converterID));
}

am_Error_e CAmControlReceiver::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return (mDatabaseHandler->enterSourceDB(sourceData, sourceID));
}

am_Error_e CAmControlReceiver::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    return (mDatabaseHandler->enterSinkClassDB(sinkClass, sinkClassID));
}

am_Error_e CAmControlReceiver::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    return (mDatabaseHandler->enterSourceClassDB(sourceClassID, sourceClass));
}

am_Error_e CAmControlReceiver::enterSystemPropertiesListDB(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
    return (mDatabaseHandler->enterSystemProperties(listSystemProperties));
}

am_Error_e CAmControlReceiver::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID)
{
    return (mDatabaseHandler->changeMainConnectionRouteDB(mainconnectionID, listConnectionID));
}

am_Error_e CAmControlReceiver::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
    return (mDatabaseHandler->changeMainConnectionStateDB(mainconnectionID, connectionState));
}

am_Error_e CAmControlReceiver::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
    return (mDatabaseHandler->changeSinkMainVolumeDB(mainVolume, sinkID));
}

am_Error_e CAmControlReceiver::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
    return (mDatabaseHandler->changeSinkAvailabilityDB(availability, sinkID));
}

am_Error_e CAmControlReceiver::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
    return (mDatabaseHandler->changeDomainStateDB(domainState, domainID));
}

am_Error_e CAmControlReceiver::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
    return (mDatabaseHandler->changeSinkMuteStateDB(muteState, sinkID));
}

am_Error_e CAmControlReceiver::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    return (mDatabaseHandler->changeMainSinkSoundPropertyDB(soundProperty, sinkID));
}

am_Error_e CAmControlReceiver::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    return (mDatabaseHandler->changeMainSourceSoundPropertyDB(soundProperty, sourceID));
}

am_Error_e CAmControlReceiver::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
    return (mDatabaseHandler->changeSourceAvailabilityDB(availability, sourceID));
}

am_Error_e CAmControlReceiver::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
    return (mDatabaseHandler->changeSystemPropertyDB(property));
}

am_Error_e CAmControlReceiver::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
    return (mDatabaseHandler->removeMainConnectionDB(mainConnectionID));
}

am_Error_e CAmControlReceiver::removeSinkDB(const am_sinkID_t sinkID)
{
    return (mDatabaseHandler->removeSinkDB(sinkID));
}

am_Error_e CAmControlReceiver::removeSourceDB(const am_sourceID_t sourceID)
{
    return (mDatabaseHandler->removeSourceDB(sourceID));
}

am_Error_e CAmControlReceiver::removeGatewayDB(const am_gatewayID_t gatewayID)
{
    return (mDatabaseHandler->removeGatewayDB(gatewayID));
}

am_Error_e CAmControlReceiver::removeConverterDB(const am_converterID_t converterID)
{
    return (mDatabaseHandler->removeConverterDB(converterID));
}

am_Error_e CAmControlReceiver::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{
    return (mDatabaseHandler->removeCrossfaderDB(crossfaderID));
}

am_Error_e CAmControlReceiver::removeDomainDB(const am_domainID_t domainID)
{
    return (mDatabaseHandler->removeDomainDB(domainID));
}

am_Error_e CAmControlReceiver::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{
    return (mDatabaseHandler->getSourceClassInfoDB(sourceID, classInfo));
}

am_Error_e CAmControlReceiver::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
    return (mDatabaseHandler->getSinkClassInfoDB(sinkID, sinkClass));
}

am_Error_e CAmControlReceiver::getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s & sinkData) const
{
    return (mDatabaseHandler->getSinkInfoDB(sinkID, sinkData));
}

am_Error_e CAmControlReceiver::getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s & sourceData) const
{
    return (mDatabaseHandler->getSourceInfoDB(sourceID, sourceData));
}

am_Error_e CAmControlReceiver::getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s & mainConnectionData) const
{
    return (mDatabaseHandler->getMainConnectionInfoDB(mainConnectionID, mainConnectionData));
}

am_Error_e CAmControlReceiver::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
    return (mDatabaseHandler->getGatewayInfoDB(gatewayID, gatewayData));
}

am_Error_e CAmControlReceiver::getConverterInfoDB(const am_converterID_t converterID, am_Converter_s & converterData) const
{
    return (mDatabaseHandler->getConverterInfoDB(converterID, converterData));
}


am_Error_e CAmControlReceiver::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
    return (mDatabaseHandler->getCrossfaderInfoDB(crossfaderID, crossfaderData));
}

am_Error_e CAmControlReceiver::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
    return (mDatabaseHandler->getListSinksOfDomain(domainID, listSinkID));
}

am_Error_e CAmControlReceiver::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
    return (mDatabaseHandler->getListSourcesOfDomain(domainID, listSourceID));
}

am_Error_e CAmControlReceiver::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listGatewaysID) const
{
    return (mDatabaseHandler->getListCrossfadersOfDomain(domainID, listGatewaysID));
}

am_Error_e CAmControlReceiver::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
    return (mDatabaseHandler->getListGatewaysOfDomain(domainID, listGatewaysID));
}

am_Error_e CAmControlReceiver::getListConvertersOfDomain(const am_domainID_t domainID,std::vector<am_converterID_t>& listConverterID) const
{
	return (mDatabaseHandler->getListConvertersOfDomain(domainID,listConverterID));
}

am_Error_e CAmControlReceiver::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
    return (mDatabaseHandler->getListMainConnections(listMainConnections));
}

am_Error_e CAmControlReceiver::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
    return (mDatabaseHandler->getListDomains(listDomains));
}

am_Error_e CAmControlReceiver::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
    return (mDatabaseHandler->getListConnections(listConnections));
}

am_Error_e CAmControlReceiver::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
    return (mDatabaseHandler->getListSinks(listSinks));
}

am_Error_e CAmControlReceiver::getListSources(std::vector<am_Source_s> & listSources) const
{
    return (mDatabaseHandler->getListSources(listSources));
}

am_Error_e CAmControlReceiver::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    return (mDatabaseHandler->getListSourceClasses(listSourceClasses));
}

am_Error_e CAmControlReceiver::getListHandles(std::vector<am_Handle_s> & listHandles) const
{
    return (mRoutingSender->getListHandles(listHandles));
}

am_Error_e CAmControlReceiver::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
    return (mDatabaseHandler->getListCrossfaders(listCrossfaders));
}

am_Error_e CAmControlReceiver::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
    return (mDatabaseHandler->getListGateways(listGateways));
}

am_Error_e CAmControlReceiver::getListConverters(std::vector<am_Converter_s>& listConverters) const
{
    return (mDatabaseHandler->getListConverters(listConverters));
}

am_Error_e CAmControlReceiver::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    return (mDatabaseHandler->getListSinkClasses(listSinkClasses));
}

am_Error_e CAmControlReceiver::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
    return (mDatabaseHandler->getListSystemProperties(listSystemProperties));
}

am_Error_e CAmControlReceiver::changeSinkClassInfoDB(const am_SinkClass_s & classInfo)
{
    return (mDatabaseHandler->changeSinkClassInfoDB(classInfo));
}

am_Error_e CAmControlReceiver::changeSourceClassInfoDB(const am_SourceClass_s & classInfo)
{
    return(mDatabaseHandler->changeSourceClassInfoDB(classInfo));
}

am_Error_e CAmControlReceiver::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
    return (mDatabaseHandler->removeSinkClassDB(sinkClassID));
}

am_Error_e CAmControlReceiver::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
    return (mDatabaseHandler->removeSourceClassDB(sourceClassID));
}

void CAmControlReceiver::setCommandReady()
{
    logVerbose(__METHOD_NAME__);
    mCommandSender->setCommandReady();
}

void CAmControlReceiver::setRoutingReady()
{
	logVerbose(__METHOD_NAME__);
    mRoutingSender->setRoutingReady();
}

void CAmControlReceiver::confirmControllerReady(const am_Error_e error)
{
	if (error!=E_OK)
		logError(__METHOD_NAME__,"controller reported error", error);
}

void CAmControlReceiver::confirmControllerRundown(const am_Error_e error)
{
	if (error!=E_OK)
	{
		logError(__METHOD_NAME__,"exited with error ",error);
		//we might be blocked here -> so lets better exit right away
		throw std::runtime_error("controller Confirmed with error");
	}

	logVerbose (__METHOD_NAME__,"will exit now");

	//end the mainloop here...
	mSocketHandler->exit_mainloop();
}

am_Error_e CAmControlReceiver::getSocketHandler(CAmSocketHandler *& socketHandler)
{
    socketHandler = mSocketHandler;
    return (E_OK);
}

void CAmControlReceiver::setCommandRundown()
{
	logInfo(__METHOD_NAME__);
    mCommandSender->setCommandRundown();
}

void CAmControlReceiver::setRoutingRundown()
{
    logInfo(__METHOD_NAME__);
    mRoutingSender->setRoutingRundown();
}

void CAmControlReceiver::getInterfaceVersion(std::string & version) const
{
    version = ControlVersion;
}

am_Error_e CAmControlReceiver::changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    return (mDatabaseHandler->changeSourceDB(sourceID,sourceClassID,listSoundProperties,listConnectionFormats,listMainSoundProperties));
}

am_Error_e CAmControlReceiver::changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    return (mDatabaseHandler->changeSinkDB(sinkID,sinkClassID,listSoundProperties,listConnectionFormats,listMainSoundProperties));
}

am_Error_e CAmControlReceiver::changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    return (mDatabaseHandler->changeGatewayDB(gatewayID,listSourceConnectionFormats,listSinkConnectionFormats,convertionMatrix));
}

am_Error_e CAmControlReceiver::changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    return (mDatabaseHandler->changeConverterDB(converterID,listSourceConnectionFormats,listSinkConnectionFormats,convertionMatrix));
}

am_Error_e CAmControlReceiver::setVolumes(am_Handle_s& handle, const std::vector<am_Volumes_s>& listVolumes)
{
    return (mRoutingSender->asyncSetVolumes(handle,listVolumes));
}

am_Error_e CAmControlReceiver::setSinkNotificationConfiguration(am_Handle_s& handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    return (mRoutingSender->asyncSetSinkNotificationConfiguration(handle,sinkID,notificationConfiguration));
}

am_Error_e CAmControlReceiver::setSourceNotificationConfiguration(am_Handle_s& handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
	return (mRoutingSender->asyncSetSourceNotificationConfiguration(handle,sourceID,notificationConfiguration));
}

void CAmControlReceiver::sendMainSinkNotificationPayload(const am_sinkID_t sinkID, const am_NotificationPayload_s& notificationPayload)
{
	logVerbose(__METHOD_NAME__,"sinkID=",sinkID,"type=",notificationPayload.type,"value=",notificationPayload.value);
    mCommandSender->cbSinkNotification(sinkID,notificationPayload);
}

void CAmControlReceiver::sendMainSourceNotificationPayload(const am_sourceID_t sourceID, const am_NotificationPayload_s& notificationPayload)
{
	logVerbose(__METHOD_NAME__,"sourceID=",sourceID,"type=",notificationPayload.type,"value=",notificationPayload.value);
    mCommandSender->cbSourceNotification(sourceID,notificationPayload);
}

am_Error_e CAmControlReceiver::changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
    logVerbose(__METHOD_NAME__,"sinkID", sinkID);
    return (mDatabaseHandler->changeMainSinkNotificationConfigurationDB(sinkID,mainNotificationConfiguration));
}

am_Error_e CAmControlReceiver::changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
    logVerbose(__METHOD_NAME__,"sourceID", sourceID);
    return (mDatabaseHandler->changeMainSourceNotificationConfigurationDB(sourceID,mainNotificationConfiguration));
}

am_Error_e CAmControlReceiver::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundproperties) const
{
	logVerbose(__METHOD_NAME__,"sinkID", sinkID);
	return (mDatabaseHandler->getListMainSinkSoundProperties(sinkID,listSoundproperties));
}

am_Error_e CAmControlReceiver::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSoundproperties) const
{
	 logVerbose(__METHOD_NAME__,"sourceID", sourceID);
	 return (mDatabaseHandler->getListMainSourceSoundProperties(sourceID, listSoundproperties));
}

am_Error_e CAmControlReceiver::getListSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
	logVerbose(__METHOD_NAME__,"sinkID", sinkID);
	return (mDatabaseHandler->getListSinkSoundProperties(sinkID,listSoundproperties));
}

am_Error_e CAmControlReceiver::getListSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
	 logVerbose(__METHOD_NAME__,"sourceID", sourceID);
	 return (mDatabaseHandler->getListSourceSoundProperties(sourceID, listSoundproperties));
}

am_Error_e CAmControlReceiver::getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{
	logVerbose(__METHOD_NAME__,"sinkID", sinkID);
	return (mDatabaseHandler->getMainSinkSoundPropertyValue(sinkID,propertyType,value));
}

am_Error_e CAmControlReceiver::getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const
{
	logVerbose(__METHOD_NAME__,"sinkID", sinkID);
	return (mDatabaseHandler->getSinkSoundPropertyValue(sinkID,propertyType,value));
}

am_Error_e CAmControlReceiver::getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{
	logVerbose(__METHOD_NAME__,"sourceID", sourceID);
	return (mDatabaseHandler->getMainSourceSoundPropertyValue(sourceID,propertyType,value));
}

am_Error_e CAmControlReceiver::getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const
{
	 logVerbose(__METHOD_NAME__,"sourceID", sourceID);
	 return (mDatabaseHandler->getSourceSoundPropertyValue(sourceID,propertyType,value));
}

am_Error_e CAmControlReceiver::resyncConnectionState(const am_domainID_t domainID,std::vector<am_Connection_s>& listOfExistingConnections)
{
	logInfo(__METHOD_NAME__,"domainID", domainID);
	return (mRoutingSender->resyncConnectionState(domainID,listOfExistingConnections));
}

am_Error_e CAmControlReceiver::removeHandle(const am_Handle_s handle)
{
	logInfo(__METHOD_NAME__,"handle", handle.handle);
	return (mRoutingSender->removeHandle(handle));
}

}

