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

#include "CAmRoutingSenderDbus.h"

extern "C" IAmRoutingSend* PluginRoutingInterfaceDbusFactory()
{
    return (new CAmRoutingSenderDbus());
}

extern "C" void destroyRoutingPluginInterfaceDbus(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

CAmRoutingSenderDbus::CAmRoutingSenderDbus()
{
}

CAmRoutingSenderDbus::~CAmRoutingSenderDbus()
{
}

am_Error_e CAmRoutingSenderDbus::asyncAbort(const am_Handle_s handle)
{
    (void) handle;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
    (void) handle;
    (void) connectionID;
    (void) sourceID;
    (void) sinkID;
    (void) connectionFormat;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    (void) handle;
    (void) connectionID;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    (void) handle;
    (void) sinkID;
    (void) volume;
    (void) ramp;
    (void) time;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    (void) handle;
    (void) sourceID;
    (void) volume;
    (void) ramp;
    (void) time;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    (void) handle;
    (void) sourceID;
    (void) state;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    (void) handle;
    (void) crossfaderID;
    (void) hotSink;
    (void) rampType;
    (void) time;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    (void) domainID;
    (void) domainState;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::startupInterface(IAmRoutingReceive *routingreceiveinterface)
{
    (void)routingreceiveinterface;
    return (E_NOT_USED);
}

void CAmRoutingSenderDbus::setRoutingReady(const uint16_t handle)
{
    (void) handle;
}

void CAmRoutingSenderDbus::setRoutingRundown(const uint16_t handle)
{
    (void) handle;
}

am_Error_e CAmRoutingSenderDbus::returnBusName(std::string & BusName) const
{
    BusName = "DbusPlugin";
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    (void) handle;
    (void) sinkID;
    (void) soundProperty;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    (void) handle;
    (void) sourceID;
    (void) soundProperty;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

void CAmRoutingSenderDbus::getInterfaceVersion(std::string & version) const
{
    version=RoutingSendVersion;
}



