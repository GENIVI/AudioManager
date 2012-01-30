/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file ControlReceiver.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#include <config.h>
#include <SocketHandler.h>
#include "ControlReceiver.h"
#include "DatabaseHandler.h"
#include "RoutingSender.h"
#include "CommandSender.h"
#include "Router.h"
#include "DLTWrapper.h"
#include <cassert>

using namespace am;

ControlReceiver::ControlReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, CommandSender *iCommandSender, SocketHandler *iSocketHandler, Router* iRouter) :
        mDatabaseHandler(iDatabaseHandler), //
        mRoutingSender(iRoutingSender), //
        mCommandSender(iCommandSender), //
        mSocketHandler(iSocketHandler), //
        mRouter(iRouter)
{
    assert(mDatabaseHandler!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mCommandSender!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mRouter!=NULL);
}

ControlReceiver::ControlReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, CommandSender *iCommandSender, Router* iRouter) :
        mDatabaseHandler(iDatabaseHandler), //
        mRoutingSender(iRoutingSender), //
        mCommandSender(iCommandSender), //
        mRouter(iRouter)
{
    assert(mDatabaseHandler!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mCommandSender!=NULL);
    assert(mRouter!=NULL);
}

ControlReceiver::~ControlReceiver()
{
}

am_Error_e ControlReceiver::getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
    mRouter->getRoute(onlyfree, sourceID, sinkID, returnList);
    return E_NOT_USED;
}

am_Error_e ControlReceiver::connect(am_Handle_s & handle, am_connectionID_t & connectionID, const am_ConnectionFormat_e format, const am_sourceID_t sourceID, const am_sinkID_t sinkID)
{
    logInfo("ControlReceiver::connect got called, connectionFormat=", format, "sourceID=", sourceID, "sinkID=", sinkID);

    am_Connection_s tempConnection;
    tempConnection.sinkID = sinkID;
    tempConnection.sourceID = sourceID;
    tempConnection.connectionFormat = format;
    tempConnection.connectionID = 0;

    //todo: enter function to find out what happends if the same connection is in the course of being build up.
    if (mDatabaseHandler->existConnection(tempConnection))
        return E_ALREADY_EXISTS; //todo:enter the correct connectionID here?

    mDatabaseHandler->enterConnectionDB(tempConnection, connectionID);
    return mRoutingSender->asyncConnect(handle, connectionID, sourceID, sinkID, format);
}

am_Error_e ControlReceiver::disconnect(am_Handle_s & handle, const am_connectionID_t connectionID)
{
    logInfo("ControlReceiver::disconnect got called, connectionID=", connectionID);

    if (!mDatabaseHandler->existConnectionID(connectionID))
        return E_NON_EXISTENT; //todo: check with EA model and correct
    return mRoutingSender->asyncDisconnect(handle, connectionID);
}

am_Error_e ControlReceiver::crossfade(am_Handle_s & handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_RampType_e rampType, const am_time_t rampTime)
{
    logInfo("ControlReceiver::crossfade got called, hotSource=",hotSource,"crossfaderID=",crossfaderID,"rampType=",rampType,"rampTime=",rampTime);

    if (!mDatabaseHandler->existcrossFader(crossfaderID))
        return E_NON_EXISTENT;
    return mRoutingSender->asyncCrossFade(handle, crossfaderID, hotSource, rampType, rampTime);
}

am_Error_e ControlReceiver::setSourceState(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    logInfo("ControlReceiver::setSourceState got called, sourceID=",sourceID,"state=",state);

    am_SourceState_e sourceState;
    if (mDatabaseHandler->getSoureState(sourceID, sourceState) != E_OK)
        return E_UNKNOWN;
    if (sourceState == state)
        return E_NO_CHANGE;
    return mRoutingSender->asyncSetSourceState(handle, sourceID, state);
}

am_Error_e ControlReceiver::setSinkVolume(am_Handle_s & handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    logInfo("ControlReceiver::setSinkVolume got called, sinkID=",sinkID,"volume=",volume,"ramp=",ramp, "time=",time);

    am_volume_t tempVolume;
    if (mDatabaseHandler->getSinkVolume(sinkID, tempVolume) != E_OK)
        return E_UNKNOWN;
    if (tempVolume == volume)
        return E_NO_CHANGE;
    return mRoutingSender->asyncSetSinkVolume(handle, sinkID, volume, ramp, time);
}

am_Error_e ControlReceiver::setSourceVolume(am_Handle_s & handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e rampType, const am_time_t time)
{
    logInfo("ControlReceiver::setSourceVolume got called, sourceID=",sourceID,"volume=",volume,"ramp=",rampType,"time=",time);

    am_volume_t tempVolume;
    if (mDatabaseHandler->getSourceVolume(sourceID, tempVolume) != E_OK)
        return E_UNKNOWN;
    if (tempVolume == volume)
        return E_NO_CHANGE;
    return mRoutingSender->asyncSetSourceVolume(handle, sourceID, volume, rampType, time);
}

am_Error_e ControlReceiver::setSinkSoundProperty(am_Handle_s & handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    logInfo("ControlReceiver::setSinkSoundProperty got called, sinkID=",sinkID,"soundProperty.Type=",soundProperty.type,"soundProperty.value=",soundProperty.value);

    uint16_t value;
    if (mDatabaseHandler->getSinkSoundPropertyValue(sinkID, soundProperty.type, value) != E_OK)
        return E_UNKNOWN;
    if (value == soundProperty.value)
        return E_NO_CHANGE;
    return mRoutingSender->asyncSetSinkSoundProperty(handle, sinkID, soundProperty);
}

am_Error_e ControlReceiver::setSinkSoundProperties(am_Handle_s & handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    logInfo("ControlReceiver::setSinkSoundProperties got called, sinkID=",sinkID);

    uint16_t value;
    bool noChange = true;
    std::vector<am_SoundProperty_s>::const_iterator it = listSoundProperties.begin();
    for (; it != listSoundProperties.end(); ++it)
    {
        if (mDatabaseHandler->getSinkSoundPropertyValue(sinkID, it->type, value) != E_OK)
            return (E_UNKNOWN);
        if (value != it->value)
            noChange = false;
    }
    if (noChange)
        return (E_NO_CHANGE);
    return (mRoutingSender->asyncSetSinkSoundProperties(handle, listSoundProperties, sinkID));
}

am_Error_e ControlReceiver::setSourceSoundProperty(am_Handle_s & handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    logInfo("ControlReceiver::setSourceSoundProperty got called, sourceID=",sourceID,"soundProperty.Type=",soundProperty.type,"soundProperty.value=",soundProperty.value);

    uint16_t value;
    if (mDatabaseHandler->getSourceSoundPropertyValue(sourceID, soundProperty.type, value) != E_OK)
        return E_UNKNOWN;
    if (value == soundProperty.value)
        return E_NO_CHANGE;
    return mRoutingSender->asyncSetSourceSoundProperty(handle, sourceID, soundProperty);
}

am_Error_e ControlReceiver::setSourceSoundProperties(am_Handle_s & handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    logInfo("ControlReceiver::setSourceSoundProperties got called, sourceID=",sourceID);

    uint16_t value;
    bool noChange = true;
    std::vector<am_SoundProperty_s>::const_iterator it = listSoundProperties.begin();
    for (; it != listSoundProperties.end(); ++it)
    {
        if (mDatabaseHandler->getSourceSoundPropertyValue(sourceID, it->type, value) != E_OK)
            return (E_UNKNOWN);
        if (value != it->value)
            noChange = false;
    }
    if (noChange)
        return (E_NO_CHANGE);
    return (mRoutingSender->asyncSetSourceSoundProperties(handle, listSoundProperties, sourceID));
}

am_Error_e ControlReceiver::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    logInfo("ControlReceiver::setDomainState got called, domainID=",domainID,"domainState=",domainState);

    am_DomainState_e tempState = DS_MIN;
    if (mDatabaseHandler->getDomainState(domainID, tempState) != E_OK)
        return E_UNKNOWN;
    if (tempState == domainState)
        return E_NO_CHANGE;
    return mRoutingSender->setDomainState(domainID, domainState);
}

am_Error_e ControlReceiver::abortAction(const am_Handle_s handle)
{
    logInfo("ControlReceiver::abortAction got called, handle.type=",handle.handle,"handle.handleType=",handle.handleType);

    return mRoutingSender->asyncAbort(handle);
}

am_Error_e ControlReceiver::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return mDatabaseHandler->enterDomainDB(domainData, domainID);
}

am_Error_e ControlReceiver::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    return mDatabaseHandler->enterMainConnectionDB(mainConnectionData, connectionID);
}

am_Error_e ControlReceiver::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return mDatabaseHandler->enterSinkDB(sinkData, sinkID);
}

am_Error_e ControlReceiver::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return mDatabaseHandler->enterCrossfaderDB(crossfaderData, crossfaderID);
}

am_Error_e ControlReceiver::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return mDatabaseHandler->enterGatewayDB(gatewayData, gatewayID);
}

am_Error_e ControlReceiver::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return mDatabaseHandler->enterSourceDB(sourceData, sourceID);
}

am_Error_e ControlReceiver::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    return mDatabaseHandler->enterSinkClassDB(sinkClass, sinkClassID);
}

am_Error_e ControlReceiver::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    return mDatabaseHandler->enterSourceClassDB(sourceClassID, sourceClass);
}

am_Error_e ControlReceiver::enterSystemPropertiesListDB(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
    return mDatabaseHandler->enterSystemProperties(listSystemProperties);
}

am_Error_e ControlReceiver::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const am_Route_s & route)
{
    return mDatabaseHandler->changeMainConnectionRouteDB(mainconnectionID, route);
}

am_Error_e ControlReceiver::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
    return mDatabaseHandler->changeMainConnectionStateDB(mainconnectionID, connectionState);
}

am_Error_e ControlReceiver::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
    return mDatabaseHandler->changeSinkMainVolumeDB(mainVolume, sinkID);
}

am_Error_e ControlReceiver::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
    return mDatabaseHandler->changeSinkAvailabilityDB(availability, sinkID);
}

am_Error_e ControlReceiver::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
    return mDatabaseHandler->changDomainStateDB(domainState, domainID);
}

am_Error_e ControlReceiver::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
    return mDatabaseHandler->changeSinkMuteStateDB(muteState, sinkID);
}

am_Error_e ControlReceiver::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    return mDatabaseHandler->changeMainSinkSoundPropertyDB(soundProperty, sinkID);
}

am_Error_e ControlReceiver::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    return mDatabaseHandler->changeMainSinkSoundPropertyDB(soundProperty, sourceID);
}

am_Error_e ControlReceiver::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
    return mDatabaseHandler->changeSourceAvailabilityDB(availability, sourceID);
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
    return mDatabaseHandler->getSourceClassInfoDB(sourceID, classInfo);
}

am_Error_e ControlReceiver::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
    return mDatabaseHandler->getSinkClassInfoDB(sinkID, sinkClass);
}

am_Error_e ControlReceiver::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
    return mDatabaseHandler->getGatewayInfoDB(gatewayID, gatewayData);
}

am_Error_e ControlReceiver::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
    return mDatabaseHandler->getCrossfaderInfoDB(crossfaderID, crossfaderData);
}

am_Error_e ControlReceiver::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
    return mDatabaseHandler->getListSinksOfDomain(domainID, listSinkID);
}

am_Error_e ControlReceiver::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
    return mDatabaseHandler->getListSourcesOfDomain(domainID, listSourceID);
}

am_Error_e ControlReceiver::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listGatewaysID) const
{
    return mDatabaseHandler->getListCrossfadersOfDomain(domainID, listGatewaysID);
}

am_Error_e ControlReceiver::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
    return mDatabaseHandler->getListGatewaysOfDomain(domainID, listGatewaysID);
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
    return mRoutingSender->getListHandles(listHandles);
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

am_Error_e ControlReceiver::changeSinkClassInfoDB(const am_SinkClass_s & classInfo)
{
    return mDatabaseHandler->changeSinkClassInfoDB(classInfo);
}

am_Error_e ControlReceiver::changeSourceClassInfoDB(const am_SourceClass_s & classInfo)
{
    return mDatabaseHandler->changeSourceClassInfoDB(classInfo);
}

am_Error_e ControlReceiver::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
    return mDatabaseHandler->removeSinkClassDB(sinkClassID);
}

am_Error_e ControlReceiver::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
    return mDatabaseHandler->removeSourceClassDB(sourceClassID);
}

void ControlReceiver::setCommandReady()
{
    logInfo("ControlReceiver::setCommandReady got called");
    mCommandSender->cbCommunicationReady();
}

void ControlReceiver::setRoutingReady()
{
    logInfo("ControlReceiver::setRoutingReady got called");
    mRoutingSender->routingInterfacesReady();
}

am_Error_e ControlReceiver::getSocketHandler(SocketHandler *& socketHandler)
{
#ifdef WITH_SOCKETHANDLER_LOOP
    socketHandler = mSocketHandler;
    return E_OK;
#else
    return E_UNKNOWN;
#endif
}

uint16_t ControlReceiver::getInterfaceVersion() const
{
    return ControlReceiveVersion;
}

