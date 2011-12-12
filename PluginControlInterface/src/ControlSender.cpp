/*
 * ControlSender.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#include "ControlSender.h"

extern "C" ControlSendInterface* PluginControlInterfaceFactory() {
    return (new ControlSenderPlugin());
}

extern "C" void destroyControlPluginInterface(ControlSendInterface* controlSendInterface) {
    delete controlSendInterface;
}

ControlSenderPlugin::ControlSenderPlugin() {
	// TODO Auto-generated constructor stub

}

ControlSenderPlugin::~ControlSenderPlugin() {
	// TODO Auto-generated destructor stub
}

//am_Error_e ControlSendInterface::startupController(ControlReceiveInterface* controlreceiveinterface)
//{
//}



am_Error_e ControlSenderPlugin::stopController()
{
}



void ControlSenderPlugin::hookAllPluginsLoaded()
{
}



am_Error_e ControlSenderPlugin::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
}



am_Error_e ControlSenderPlugin::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
}



am_Error_e ControlSenderPlugin::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
}



am_Error_e ControlSenderPlugin::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
}



am_Error_e ControlSenderPlugin::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
}



am_Error_e ControlSenderPlugin::hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume)
{
}



am_Error_e ControlSenderPlugin::hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment)
{
}



am_Error_e ControlSenderPlugin::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
}



am_Error_e ControlSenderPlugin::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
}



am_Error_e ControlSenderPlugin::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
}



void ControlSenderPlugin::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
}



am_Error_e ControlSenderPlugin::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
}



am_Error_e ControlSenderPlugin::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
}



am_Error_e ControlSenderPlugin::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
}



am_Error_e ControlSenderPlugin::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
}



am_Error_e ControlSenderPlugin::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
}



am_Error_e ControlSenderPlugin::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
}



am_Error_e ControlSenderPlugin::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
}



am_Error_e ControlSenderPlugin::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
}



void ControlSenderPlugin::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
}



void ControlSenderPlugin::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
}



void ControlSenderPlugin::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
}



void ControlSenderPlugin::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
}



void ControlSenderPlugin::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
}



void ControlSenderPlugin::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
}



void ControlSenderPlugin::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
}



void ControlSenderPlugin::hookSystemSpeedChange(const am_speed_t speed)
{
}



void ControlSenderPlugin::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
}



void ControlSenderPlugin::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
}



void ControlSenderPlugin::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
}



void ControlSenderPlugin::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
}



void ControlSenderPlugin::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
}



void ControlSenderPlugin::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
}



void ControlSenderPlugin::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
}



void ControlSenderPlugin::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



void ControlSenderPlugin::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
}



