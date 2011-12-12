/*
 * ControlSender.cpp
 *
 *  Created on: Dec 9, 2011
 *      Author: christian
 */

#include "ControlSender.h"
#include "control/ControlSendInterface.h"
#include "control/ControlReceiveInterface.h"
#include "pluginTemplate.h"
#include <assert.h>

std::string controlPluginFile= "/home/christian/workspace/gitserver/build/plugins/control/libPluginControlInterface.so";

ControlSender::ControlSender()
{
	ControlSendInterface* (*createFunc)();
	createFunc = getCreateFunction<ControlSendInterface*()>(controlPluginFile,mlibHandle);

	assert(createFunc!=NULL);

	mController = createFunc();
}

ControlSender::~ControlSender()
{
	dlclose(mlibHandle);
}

void ControlSender::hookAllPluginsLoaded()
{
	mController->hookAllPluginsLoaded();
}



am_Error_e ControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
	return mController->hookUserConnectionRequest(sourceID,sinkID,mainConnectionID);
}



am_Error_e ControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
	return mController->hookUserDisconnectionRequest(connectionID);
}



am_Error_e ControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
	return mController->hookUserSetMainSinkSoundProperty(sinkID,soundProperty);
}



am_Error_e ControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
	return mController->hookUserSetMainSourceSoundProperty(sourceID,soundProperty);
}



am_Error_e ControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
	return mController->hookUserSetSystemProperty(property);
}



am_Error_e ControlSender::hookUserVolumeChange(const am_sinkID_t sinkID, const am_mainVolume_t newVolume)
{
	return mController->hookUserVolumeChange(sinkID,newVolume);
}



am_Error_e ControlSender::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
	return mController->hookUserVolumeStep(sinkID,increment);
}



am_Error_e ControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
	return mController->hookUserSetSinkMuteState(sinkID,muteState);
}



am_Error_e ControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	return mController->hookSystemRegisterDomain(domainData,domainID);
}



am_Error_e ControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
	return mController->hookSystemDeregisterDomain(domainID);
}



void ControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
	return mController->hookSystemDomainRegistrationComplete(domainID);
}



am_Error_e ControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
	return mController->hookSystemRegisterSink(sinkData,sinkID);
}



am_Error_e ControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
	return mController->hookSystemDeregisterSink(sinkID);
}



am_Error_e ControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
	return mController->hookSystemRegisterSource(sourceData,sourceID);
}



am_Error_e ControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
	return mController->hookSystemDeregisterSource(sourceID);
}



am_Error_e ControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
	return mController->hookSystemRegisterGateway(gatewayData,gatewayID);
}



am_Error_e ControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
	return mController->hookSystemDeregisterGateway(gatewayID);
}



am_Error_e ControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
	return mController->hookSystemRegisterCrossfader(crossfaderData,crossfaderID);
}



am_Error_e ControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
	return mController->hookSystemDeregisterCrossfader(crossfaderID);
}



void ControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
	return mController->hookSystemSinkVolumeTick(handle,sinkID,volume);
}



void ControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
	return mController->hookSystemSourceVolumeTick(handle,sourceID,volume);
}



void ControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
	return mController->hookSystemInterruptStateChange(sourceID,interruptState);
}



void ControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
	return mController->hookSystemSinkAvailablityStateChange(sinkID,availability);
}



void ControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
	return mController->hookSystemSourceAvailablityStateChange(sourceID,availability);
}



void ControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
	return mController->hookSystemDomainStateChange(domainID,state);
}



void ControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
	return mController->hookSystemReceiveEarlyData(data);
}



void ControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
	return mController->hookSystemSpeedChange(speed);
}



void ControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
	return mController->hookSystemTimingInformationChanged(mainConnectionID,time);
}



void ControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
	return mController->cbAckConnect(handle,errorID);
}



void ControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
	return mController->cbAckDisconnect(handle,errorID);
}



void ControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
	return mController->cbAckCrossFade(handle,hostsink,error);
}



void ControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
	return mController->cbAckSetSinkVolumeChange(handle,volume,error);
}



void ControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
	return mController->cbAckSetSourceVolumeChange(handle,voulme,error);
}



void ControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
	return mController->cbAckSetSourceState(handle,error);
}



void ControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
	return mController->cbAckSetSourceSoundProperty(handle,error);
}



am_Error_e ControlSender::startupController(ControlReceiveInterface *controlreceiveinterface)
{
	return mController->startupController(controlreceiveinterface);
}

am_Error_e ControlSender::stopController()
{
	return mController->stopController();
}

void ControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
	return mController->cbAckSetSinkSoundProperty(handle,error);
}



