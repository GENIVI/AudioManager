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

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "routing/IAmRoutingSend.h"

using namespace am;

class CAmRoutingSenderDbus: public IAmRoutingSend
{
public:
    CAmRoutingSenderDbus();
    virtual ~CAmRoutingSenderDbus();
    am_Error_e startupInterface(IAmRoutingReceive* routingreceiveinterface) ;
    void setRoutingReady(const uint16_t handle) ;
    void setRoutingRundown(const uint16_t handle) ;
    am_Error_e asyncAbort(const am_Handle_s handle) ;
    am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) ;
    am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID) ;
    am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
    am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
    am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
    am_Error_e asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties) ;
    am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty) ;
    am_Error_e asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties) ;
    am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty) ;
    am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time) ;
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
    am_Error_e returnBusName(std::string& BusName) const ;
    void getInterfaceVersion(std::string& version) const ;
};

#endif /* ROUTINGSENDER_H_ */
