/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "IAmRoutingReceiverShadow.h"
#include <cassert>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <string>
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSerializer.h"

using namespace am;

IAmRoutingReceiverShadow::IAmRoutingReceiverShadow(IAmRoutingReceive* iReceiveInterface, CAmSocketHandler* iSocketHandler) :
        mSocketHandler(iSocketHandler), //
        mRoutingReceiveInterface(iReceiveInterface), //
        mSerializer(iSocketHandler)
{

}

IAmRoutingReceiverShadow::~IAmRoutingReceiverShadow()
{
}

void IAmRoutingReceiverShadow::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_connectionID_t, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackConnect, handle, connectionID, error);
}

void IAmRoutingReceiverShadow::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_connectionID_t, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackDisconnect, handle, connectionID, error);
}

void IAmRoutingReceiverShadow::ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_volume_t, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSetSinkVolumeChange, handle, volume, error);
}

void IAmRoutingReceiverShadow::ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_volume_t, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSetSourceVolumeChange, handle, volume, error);
}

void IAmRoutingReceiverShadow::ackSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSetSourceState, handle, error);
}

void IAmRoutingReceiverShadow::ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSetSinkSoundProperty, handle, error);
}

void IAmRoutingReceiverShadow::ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSetSourceSoundProperty, handle, error);
}

void IAmRoutingReceiverShadow::ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_HotSink_e, const am_Error_e>(mRoutingReceiveInterface, &IAmRoutingReceive::ackCrossFading, handle, hotSink, error);
}

void IAmRoutingReceiverShadow::ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_sourceID_t, const am_volume_t>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSourceVolumeTick, handle, sourceID, volume);
}

void IAmRoutingReceiverShadow::ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_Handle_s, const am_sinkID_t, const am_volume_t>(mRoutingReceiveInterface, &IAmRoutingReceive::ackSinkVolumeTick, handle, sinkID, volume);
}

void IAmRoutingReceiverShadow::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_sinkID_t, const am_InterruptState_e>(mRoutingReceiveInterface, &IAmRoutingReceive::hookInterruptStatusChange, sourceID, interruptState);
}

void IAmRoutingReceiverShadow::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_sinkID_t, const am_Availability_s&>(mRoutingReceiveInterface, &IAmRoutingReceive::hookSinkAvailablityStatusChange, sinkID, availability);
}

void IAmRoutingReceiverShadow::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_sourceID_t, const am_Availability_s&>(mRoutingReceiveInterface, &IAmRoutingReceive::hookSourceAvailablityStatusChange, sourceID, availability);
}

void IAmRoutingReceiverShadow::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_domainID_t, const am_DomainState_e>(mRoutingReceiveInterface, &IAmRoutingReceive::hookDomainStateChange, domainID, domainState);
}

void IAmRoutingReceiverShadow::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    mSerializer.asyncCall<IAmRoutingReceive, const am_connectionID_t, const am_timeSync_t>(mRoutingReceiveInterface, &IAmRoutingReceive::hookTimingInformationChanged, connectionID, delay);
}

am_Error_e IAmRoutingReceiverShadow::registerDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    am_Error_e error (E_UNKNOWN);
    am_Domain_s domainDataCopy(domainData);
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, const am_Domain_s&,am_domainID_t&, am_Domain_s, am_domainID_t>(mRoutingReceiveInterface, &IAmRoutingReceive::registerDomain, error, domainDataCopy, domainID);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::registerGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    am_Error_e error (E_UNKNOWN);
    am_Gateway_s gatewayDataCopy(gatewayData);
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, const am_Gateway_s&, am_gatewayID_t&, am_Gateway_s, am_gatewayID_t>(mRoutingReceiveInterface,&IAmRoutingReceive::registerGateway, error, gatewayDataCopy, gatewayID);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::registerSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    am_Error_e error (E_UNKNOWN);
    am_Sink_s sinkDataCopy(sinkData);
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, const am_Sink_s&, am_sinkID_t&, am_Sink_s, am_sinkID_t>(mRoutingReceiveInterface,&IAmRoutingReceive::registerSink, error, sinkDataCopy, sinkID);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::deregisterSink(const am_sinkID_t sinkID)
{
    am_Error_e error;
    am_sinkID_t s(sinkID); //no const values allowed in syncCalls due to reference !
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, am_sinkID_t>(mRoutingReceiveInterface, &IAmRoutingReceive::deregisterSink, error, s);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::registerSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    am_Error_e error (E_UNKNOWN);
    am_Source_s sourceDataCopy(sourceData);
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, const am_Source_s&, am_sourceID_t&, am_Source_s, am_sourceID_t>(mRoutingReceiveInterface,&IAmRoutingReceive::registerSource, error, sourceDataCopy, sourceID);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::deregisterSource(const am_sourceID_t sourceID)
{
    am_Error_e error;
    am_sourceID_t s(sourceID); //no const values allowed in syncCalls due to reference !
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, am_sinkID_t>(mRoutingReceiveInterface, &IAmRoutingReceive::deregisterSource, error, s);
    return (error);
}

am_Error_e am::IAmRoutingReceiverShadow::registerCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    am_Error_e error (E_UNKNOWN);
    am_Crossfader_s crossfaderDataCopy(crossfaderData);
    mSerializer.syncCall<IAmRoutingReceive, am_Error_e, const am_Crossfader_s&, am_crossfaderID_t&, am_Crossfader_s, am_crossfaderID_t>(mRoutingReceiveInterface,&IAmRoutingReceive::registerCrossfader, error, crossfaderDataCopy, crossfaderID);
    return (error);
}

void am::IAmRoutingReceiverShadow::confirmRoutingReady(uint16_t starupHandle)
{
    mSerializer.asyncCall<IAmRoutingReceive,uint16_t>(mRoutingReceiveInterface,&IAmRoutingReceive::confirmRoutingReady,starupHandle);
}

void am::IAmRoutingReceiverShadow::confirmRoutingRundown(uint16_t rundownHandle)
{
    mSerializer.asyncCall<IAmRoutingReceive,uint16_t>(mRoutingReceiveInterface,&IAmRoutingReceive::confirmRoutingRundown,rundownHandle);
}





