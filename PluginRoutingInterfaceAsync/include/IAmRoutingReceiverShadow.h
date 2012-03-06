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

#ifndef ROUTINGRECEIVERASYNCSHADOW_H_
#define ROUTINGRECEIVERASYNCSHADOW_H_

#include "routing/IAmRoutingReceive.h"
#include "shared/CAmSerializer.h"
#include "shared/CAmSocketHandler.h"

namespace am
{

/**
 * Threadsafe shadow of the RoutingReceiverInterface
 * Register and deregister Functions are sychronous so they do not show up here...
 */
class IAmRoutingReceiverShadow
{
public:
    IAmRoutingReceiverShadow(IAmRoutingReceive* iReceiveInterface,CAmSocketHandler* iSocketHandler);
    virtual ~IAmRoutingReceiverShadow();
    void ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceState(const am_Handle_s handle, const am_Error_e error);
    void ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error);
    void ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume);
    void ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume);
    void hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState);
    void hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState);
    void hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay);
    am_Error_e registerDomain(const am_Domain_s& domainData, am_domainID_t& domainID) ;
    am_Error_e registerGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) ;
    am_Error_e registerSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID) ;
    am_Error_e deregisterSink(const am_sinkID_t sinkID) ;
    am_Error_e registerSource(const am_Source_s& sourceData, am_sourceID_t& sourceID) ;
    am_Error_e deregisterSource(const am_sourceID_t sourceID) ;
    am_Error_e registerCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) ;
    void confirmRoutingReady(uint16_t starupHandle);
    void confirmRoutingRundown(uint16_t rundownHandle);

private:

    CAmSocketHandler *mSocketHandler;
    IAmRoutingReceive *mRoutingReceiveInterface;
    CAmSerializer mSerializer;
};

} /* namespace am */
#endif /* ROUTINGRECEIVERASYNCSHADOW_H_ */
