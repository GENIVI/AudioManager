/*
 * RoutingReceiver.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "RoutingReceiver.h"
#include <assert.h>



RoutingReceiver::RoutingReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, ControlSender *iControlSender)
	:mDatabaseHandler(iDatabaseHandler),
	 mRoutingSender(iRoutingSender),
	 mControlSender(iControlSender)
{
	assert(mDatabaseHandler!=0);
	assert(mRoutingSender!=0);
	assert(mControlSender!=0);
}



RoutingReceiver::~RoutingReceiver()
{
}



void RoutingReceiver::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
	mRoutingSender->removeHandle(handle);
	if (error==E_OK)
	{
		mDatabaseHandler->changeConnectionFinal(connectionID);
	}
	else
	{
		mDatabaseHandler->removeConnection(connectionID);
	}
	return mControlSender->cbAckConnect(handle,error);
}



void RoutingReceiver::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
	mRoutingSender->removeHandle(handle);
	if (error==E_OK)
	{
		mDatabaseHandler->removeConnection(connectionID);
	}
	return mControlSender->cbAckConnect(handle,error);
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
	return mDatabaseHandler->peekDomain(name,domainID);

}


am_Error_e RoutingReceiver::registerDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	return mControlSender->hookSystemRegisterDomain(domainData,domainID);
}



am_Error_e RoutingReceiver::deregisterDomain(const am_domainID_t domainID)
{
	return mControlSender->hookSystemDeregisterDomain(domainID);
}



am_Error_e RoutingReceiver::registerGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
	return mControlSender->hookSystemRegisterGateway(gatewayData,gatewayID);
}



am_Error_e RoutingReceiver::deregisterGateway(const am_gatewayID_t gatewayID)
{
	return mControlSender->hookSystemDeregisterGateway(gatewayID);
}



am_Error_e RoutingReceiver::peekSink(const std::string& name, am_sinkID_t & sinkID)
{
	return mDatabaseHandler->peekSink(name,sinkID);
}



am_Error_e RoutingReceiver::registerSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
	return mControlSender->hookSystemRegisterSink(sinkData,sinkID);
}



am_Error_e RoutingReceiver::deregisterSink(const am_sinkID_t sinkID)
{
	return mControlSender->hookSystemDeregisterSink(sinkID);
}



am_Error_e RoutingReceiver::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
	return mDatabaseHandler->peekSource(name,sourceID);
}



am_Error_e RoutingReceiver::registerSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
	return mControlSender->hookSystemRegisterSource(sourceData,sourceID);
}



am_Error_e RoutingReceiver::deregisterSource(const am_sourceID_t sourceID)
{
	return mControlSender->hookSystemDeregisterSource(sourceID);
}



am_Error_e RoutingReceiver::registerCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
	return mControlSender->hookSystemRegisterCrossfader(crossfaderData,crossfaderID);
}



am_Error_e RoutingReceiver::deregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
	return mControlSender->hookSystemDeregisterCrossfader(crossfaderID);
}



void RoutingReceiver::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
	return mControlSender->hookSystemInterruptStateChange(sourceID,interruptState);
}



void RoutingReceiver::hookDomainRegistrationComplete(const am_domainID_t domainID)
{
	 mControlSender->hookSystemDomainRegistrationComplete(domainID);
}




void RoutingReceiver::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
	mControlSender->hookSystemSinkAvailablityStateChange(sinkID,availability);
}



void RoutingReceiver::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
	mControlSender->hookSystemSourceAvailablityStateChange(sourceID,availability);
}



void RoutingReceiver::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
	mControlSender->hookSystemDomainStateChange(domainID,domainState);
}



void RoutingReceiver::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
	mDatabaseHandler->changeConnectionTimingInformation(connectionID,delay);
}



am_Error_e RoutingReceiver::sendChangedData(const std::vector<am_EarlyData_s> & earlyData)
{
	mControlSender->hookSystemReceiveEarlyData(earlyData);
	return E_OK;
	//todo: change return type to void in EA model
}



am_Error_e RoutingReceiver::peekSinkClassID(const std::string & name, am_sourceClass_t & sourceClassID)
{
	//todo: implement
}

am_Error_e RoutingReceiver::peekSourceClassID(const std::string & name, am_sinkClass_t & sinkClassID)
{
	//todo: implement
}

am_Error_e RoutingReceiver::getDBusConnectionWrapper(DBusWrapper *dbusConnectionWrapper) const
{
	//todo: return DbusWrapper
}

