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

#ifndef DBUSCOMMANDSENDER_H_
#define DBUSCOMMANDSENDER_H_

#include <dbus/dbus.h>
#include <map>
#include "shared/CAmDbusWrapper.h"
#include "CAmDbusMessageHandler.h"
#include "IAmCommandReceiverShadow.h"
#include "command/IAmCommandSend.h"

#ifdef UNIT_TEST
#include  "../test/CAmCommandSenderDbusBackdoor.h"
#endif



namespace am
{

const char MY_NODE[] = "CommandInterface";

/**
 * Dbus Implementation of CommandSendInterface
 */
class CAmCommandSenderDbus: public IAmCommandSend
{
public:
    CAmCommandSenderDbus();
    virtual ~CAmCommandSenderDbus();
    am_Error_e startupInterface(IAmCommandReceive* commandreceiveinterface);
    void setCommandReady(const uint16_t handle);
    void setCommandRundown(const uint16_t handle);
    void cbNewMainConnection(const am_MainConnectionType_s& mainConnection);
    void cbRemovedMainConnection(const am_mainConnectionID_t mainConnection);
    void cbNewSink(const am_SinkType_s& sink);
    void cbRemovedSink(const am_sinkID_t sinkID);
    void cbNewSource(const am_SourceType_s& source);
    void cbRemovedSource(const am_sourceID_t source);
    void cbNumberOfSinkClassesChanged();
    void cbNumberOfSourceClassesChanged();
    void cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState);
    void cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty);
    void cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty);
    void cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume);
    void cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    void cbSystemPropertyChanged(const am_SystemProperty_s& systemProperty);
    void cbTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time);
    void getInterfaceVersion(std::string& version) const;

#ifdef UNIT_TEST
    friend class CAmCommandSenderDbusBackdoor;
#endif
private:
    CAmDbusMessageHandler mCAmDbusMessageHandler; ///< ! instance of message handler
    IAmCommandReceiverShadow mIAmCommandReceiverShadow; ///< ! instance of shadow
    CAmDbusWrapper* mpCAmDbusWrapper; ///< ! pointer to dbus wrapper
    IAmCommandReceive* mpIAmCommandReceive; ///< ! pointer to commandReceive Interface
    bool mReady; ///< ! if false, calls shall be ignored.

    /**
     * list of sinks, needed to send out only deltas
     */
    std::vector<am_SinkType_s> mlistSinks;

    /**
     * list of sources, needed to send out only deltas
     */
    std::vector<am_SourceType_s> mlistSources;

    /**
     * used as comparison function for am_SinkType_s
     */
    struct sortBySinkID
    {
        bool operator()(const am_SinkType_s & a, const am_SinkType_s & b)
        {
            return (a.sinkID < b.sinkID);
        }
    };

    /**
     * used as comparison function for am_SourceType_s
     */
    struct sortBySourceID
    {
        bool operator()(const am_SourceType_s & a, const am_SourceType_s & b)
        {
            return (a.sourceID < b.sourceID);
        }
    };

};

}

#endif /* DBUSCOMMANDSENDER_H_ */
