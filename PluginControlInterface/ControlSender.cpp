/*
 * ControlSender.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#include "ControlSender.h"

extern "C" ControlSendInterface* PluginControlInterfaceFactory() {
    return new ControlSender();
}

extern "C" void destroyControlPluginInterface(ControlSendInterface* controlSendInterface) {
    delete controlSendInterface;
}

ControlSender::ControlSender() {
	// TODO Auto-generated constructor stub

}

ControlSender::~ControlSender() {
	// TODO Auto-generated destructor stub
}

am_Error_e ControlSendInterface::startupController(ControlReceiveInterface* controlreceiveinterface)
{
}



am_Error_e ControlSender::stopController()
{
}



void ControlSender::hookAllPluginsLoaded()
{
}



am_Error_e ControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
}



am_Error_e ControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
}



am_Error_e ControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
}



am_Error_e ControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
}



am_Error_e ControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
}



am_Error_e ControlSender::hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume)
{
}



am_Error_e ControlSender::hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment)
{
}



am_Error_e ControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
}



am_Error_e ControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
}



am_Error_e ControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
}



void ControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
}



am_Error_e ControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
}



am_Error_e ControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
}



am_Error_e ControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
}



am_Error_e ControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
}



am_Error_e ControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
}



am_Error_e ControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
}



am_Error_e ControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
}



am_Error_e ControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
}



void ControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
}



void ControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
}



void ControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
}



void ControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
}



void ControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
}



void ControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
}



void ControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
}



void ControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
}



void ControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
}



void ControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
}



void ControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
}



void ControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
}



void ControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
}



void ControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
}



void ControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
}



void ControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



void ControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



