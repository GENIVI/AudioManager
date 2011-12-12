/*
 * Observer.cpp
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#include "Observer.h"
#include "CommandSender.h"
#include <assert.h>

Observer::Observer(CommandSender *iCommandSender, RoutingSender *iRoutingSender)
	:mCommandSender(iCommandSender),
	 mRoutingSender(iRoutingSender)
{
	assert(mCommandSender!=0);
	assert(mRoutingSender!=0);
	mCommandSender->cbCommunicationReady();
}

Observer::~Observer() {
	// TODO Auto-generated destructor stub
}

void Observer::newSink(am_Sink_s sink)
{
	mRoutingSender->addSinkLookup(sink);
	mCommandSender->cbNumberOfSinksChanged();
}



void Observer::newSource(am_Source_s source)
{
	mRoutingSender->addSourceLookup(source);
	mCommandSender->cbNumberOfSourcesChanged();
}



void Observer::newDomain(am_Domain_s domain)
{
	mRoutingSender->addDomainLookup(domain);
}



void Observer::newGateway(am_Gateway_s gateway)
{

}



void Observer::newCrossfader(am_Crossfader_s crossfader)
{
	mRoutingSender->addCrossfaderLookup(crossfader);
}



void Observer::removedSink(am_sinkID_t sinkID)
{
	mRoutingSender->removeSinkLookup(sinkID);
	mCommandSender->cbNumberOfSinksChanged();
}



void Observer::removedSource(am_sourceID_t sourceID)
{
	mRoutingSender->removeSourceLookup(sourceID);
	mCommandSender->cbNumberOfSourcesChanged();
}



void Observer::removeDomain(am_domainID_t domainID)
{
	mRoutingSender->removeDomainLookup(domainID);
}



void Observer::removeGateway(am_gatewayID_t gatewayID)
{
}



void Observer::removeCrossfader(am_crossfaderID_t crossfaderID)
{
	mRoutingSender->removeCrossfaderLookup(crossfaderID);
}


void Observer::numberOfMainConnectionsChanged()
{
	mCommandSender->cbNumberOfMainConnectionsChanged();
}

void Observer::numberOfSinkClassesChanged()
{
	mCommandSender->cbNumberOfSinkClassesChanged();
}



void Observer::numberOfSourceClassesChanged()
{
	mCommandSender->cbNumberOfSourceClassesChanged();
}



void Observer::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
	mCommandSender->cbMainConnectionStateChanged(connectionID,connectionState);
}



void Observer::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty)
{
	mCommandSender->cbMainSinkSoundPropertyChanged(sinkID,SoundProperty);
}



void Observer::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
	mCommandSender->cbMainSourceSoundPropertyChanged(sourceID,SoundProperty);
}



void Observer::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
	mCommandSender->cbSinkAvailabilityChanged(sinkID,availability);
}



void Observer::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
	mCommandSender->cbSourceAvailabilityChanged(sourceID,availability);
}



void Observer::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
	mCommandSender->cbVolumeChanged(sinkID,volume);
}



void Observer::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
	mCommandSender->cbSinkMuteStateChanged(sinkID,muteState);
}



void Observer::systemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
	mCommandSender->cbSystemPropertyChanged(SystemProperty);
}



void Observer::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
	mCommandSender->cbTimingInformationChanged(mainConnection,time);
	//todo:inform the controller via controlsender
}












