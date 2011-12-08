/*
 * RoutingReceiver.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "RoutingReceiver.h"



RoutingReceiver::RoutingReceiver()
{
}



RoutingReceiver::~RoutingReceiver()
{
}



void RoutingReceiver::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
}



void RoutingReceiver::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
}



void RoutingReceiver::ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
}



void RoutingReceiver::ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
}



void RoutingReceiver::ackSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
}



void RoutingReceiver::ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



void RoutingReceiver::ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



void RoutingReceiver::ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error)
{
}



void RoutingReceiver::ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
}



void RoutingReceiver::ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
}



am_Error_e RoutingReceiver::peekDomain(const std::string & name, am_domainID_t & domainID)
{
}



am_Error_e RoutingReceiver::registerDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
}



am_Error_e RoutingReceiver::deregisterDomain(const am_domainID_t domainID)
{
}



am_Error_e RoutingReceiver::registerGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
}



am_Error_e RoutingReceiver::deregisterGateway(const am_gatewayID_t gatewayID)
{
}



am_Error_e RoutingReceiver::peekSink(const std::string& name, am_sinkID_t & sinkID)
{
}



am_Error_e RoutingReceiver::registerSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
}



am_Error_e RoutingReceiver::deregisterSink(const am_sinkID_t sinkID)
{
}



am_Error_e RoutingReceiver::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
}



am_Error_e RoutingReceiver::registerSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
}



am_Error_e RoutingReceiver::deregisterSource(const am_sourceID_t sourceID)
{
}



am_Error_e RoutingReceiver::registerCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
}



am_Error_e RoutingReceiver::deregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
}



void RoutingReceiver::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
}



void RoutingReceiver::hookDomainRegistrationComplete(const am_domainID_t domainID)
{
}



void RoutingReceiver::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
}



void RoutingReceiver::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
}



void RoutingReceiver::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
}



void RoutingReceiver::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
}



am_Error_e RoutingReceiver::sendChangedData(const std::vector<am_EarlyData_s> & earlyData)
{
}



am_Error_e RoutingReceiver::getDBusConnectionWrapper(DBusWrapper *dbusConnectionWrapper) const
{
}



am_Error_e RoutingReceiver::registerDbusNode(const std::string & nodeName)
{
}

