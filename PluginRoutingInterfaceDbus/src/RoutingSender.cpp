/*
 * RoutingSender.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#include "RoutingSender.h"


extern "C" RoutingSendInterface* PluginRoutingInterfaceDbusFactory() {
    return (new DbusRoutingSender());
}

extern "C" void destroyRoutingPluginInterfaceDbus(RoutingSendInterface* routingSendInterface) {
    delete routingSendInterface;
}

DbusRoutingSender::DbusRoutingSender()
{
}



DbusRoutingSender::~DbusRoutingSender()
{
}



void DbusRoutingSender::startupRoutingInterface(RoutingReceiveInterface *routingreceiveinterface)
{
}



void DbusRoutingSender::routingInterfacesReady()
{
}



void DbusRoutingSender::routingInterfacesRundown()
{
}



am_Error_e DbusRoutingSender::asyncAbort(const am_Handle_s handle)
{
}



am_Error_e DbusRoutingSender::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
}



am_Error_e DbusRoutingSender::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
}



am_Error_e DbusRoutingSender::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
}



am_Error_e DbusRoutingSender::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
}



am_Error_e DbusRoutingSender::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
}



am_Error_e DbusRoutingSender::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID)
{
}


am_Error_e DbusRoutingSender::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
}



am_Error_e DbusRoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
}



am_Error_e DbusRoutingSender::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_SoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
}



am_Error_e DbusRoutingSender::returnBusName(std::string & BusName) const
{
}






