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

#ifndef CONTROLSENDER_H_
#define CONTROLSENDER_H_

#include "control/IAmControlSend.h"
#include <list>

using namespace am;

class CAmControlSenderBase: public IAmControlSend
{
public:
    CAmControlSenderBase();
    virtual ~CAmControlSenderBase();
    am_Error_e startupController(IAmControlReceive* controlreceiveinterface);
    void setControllerReady();
    void setControllerRundown();
    am_Error_e hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID);
    am_Error_e hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID);
    am_Error_e hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty);
    am_Error_e hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty);
    am_Error_e hookUserSetSystemProperty(const am_SystemProperty_s& property);
    am_Error_e hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume);
    am_Error_e hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment);
    am_Error_e hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    am_Error_e hookSystemRegisterDomain(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e hookSystemDeregisterDomain(const am_domainID_t domainID);
    void hookSystemDomainRegistrationComplete(const am_domainID_t domainID);
    am_Error_e hookSystemRegisterSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e hookSystemDeregisterSink(const am_sinkID_t sinkID);
    am_Error_e hookSystemRegisterSource(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e hookSystemDeregisterSource(const am_sourceID_t sourceID);
    am_Error_e hookSystemRegisterGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e hookSystemDeregisterGateway(const am_gatewayID_t gatewayID);
    am_Error_e hookSystemRegisterCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID);
    void hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume);
    void hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume);
    void hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState);
    void hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state);
    void hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s>& data);
    void hookSystemSpeedChange(const am_speed_t speed);
    void hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time);
    void cbAckConnect(const am_Handle_s handle, const am_Error_e errorID);
    void cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID);
    void cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error);
    void cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error);
    void cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error);
    am_Error_e getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e>& listPrioConnectionFormats);
    void getInterfaceVersion(std::string& version) const;
    void confirmCommandReady();
    void confirmRoutingReady();
    void confirmCommandRundown();
    void confirmRoutingRundown();

private:
    IAmControlReceive * mControlReceiveInterface;

    void disconnect(am_mainConnectionID_t connectionID);
    void connect(am_sourceID_t sourceID, am_sinkID_t sinkID, am_mainConnectionID_t mainConnectionID);

    struct handleStatus
    {
        bool status;
        am_Handle_s handle;
    };

    struct mainConnectionSet
    {
        am_mainConnectionID_t connectionID;
        std::vector<handleStatus> listHandleStaus;
    };

    struct mainVolumeSet
    {
        am_sinkID_t sinkID;
        am_Handle_s handle;
        am_mainVolume_t mainVolume;
    };

    struct mainSinkSoundPropertySet
    {
        am_sinkID_t sinkID;
        am_Handle_s handle;
        am_MainSoundProperty_s mainSoundProperty;
    };

    class findHandle
    {
        handleStatus mHandle;
    public:
        explicit findHandle(handleStatus handle) :
                mHandle(handle)
        {
        }
        bool operator()(const handleStatus& handle) const
        {
            return (handle.handle.handle == mHandle.handle.handle);
        }
    };

    struct checkHandle
    {

        handleStatus mHandleStatus;
        explicit checkHandle(const handleStatus& value) :
                mHandleStatus(value)
        {
        }

        bool operator()(const handleStatus &value)
        {
            return !value.status;
        }
    };

    struct checkMainConnectionID
    {
        am_MainConnection_s mMainConnection;
        explicit checkMainConnectionID(const am_MainConnection_s& mainConnection) :
                mMainConnection(mainConnection)
        {
        }
        bool operator()(const am_MainConnection_s& mainConnection)
        {
            if (mMainConnection.mainConnectionID == mainConnection.mainConnectionID)
                return true;
            return false;
        }
    };

    enum cs_stateflow_e
    {
        SF_NONE,
        SF_CONNECT,
        SF_NAVI,
        SF_TA
    };

    enum cs_connectSf_e
    {
        SFC_RAMP_DOWN,
        SFC_SOURCE_STATE_OFF,
        SFC_DISCONNECT,
        SFC_CONNECT,
        SFC_SOURCE_STATE_ON,
        SFC_RAMP_UP,
        SFC_FINISHED
    };

    enum cs_naviSf_e
    {
        NAVC_RAMP_DOWN,
        NAVC_CONNECT,
        NAVC_SOURCE_STATE_ON,
        NAVC_RAMP_UP,
        NAVC_WAIT_STATE,
        NAVC_RAMP_DOWN_AGAIN,
        NAVC_SOURCE_VOLUME_BACK,
        NAVC_SOURCE_ACTIVITY_BACK,
        NAVC_DISCONNECT,
        NAVC_FINISHED
    };

    enum cs_trafficSf_e
    {
        TA_RAMP_DOWN,
        TA_CONNECT,
        TA_SOURCE_STATE_ON,
        TA_RAMP_UP,
        TA_WAIT_STATE,
        TA_RAMP_DOWN_AGAIN,
        TA_SOURCE_STATE_OFF,
        TA_SOURCE_STATE_OLD_OFF,
        TA_DISCONNECT,
        TA_FINISHED
    };

    struct cs_connectData_s
    {
        am_mainConnectionID_t currentMainConnection;
        am_mainConnectionID_t newMainConnection;
        am_sourceID_t oldSourceID;
        am_sinkID_t sinkID;
        am_sourceID_t sourceID;
    };

    void callStateFlowHandler();
    void callConnectHandler();
    void callNaviHandler();
    void callTAHandler();

    std::vector<mainConnectionSet> mListOpenConnections;
    std::vector<mainConnectionSet> mListOpenDisconnections;
    std::vector<mainVolumeSet> mListOpenVolumeChanges;
    std::vector<mainSinkSoundPropertySet> mListMainSoundPropertyChanges;

    cs_connectSf_e mConnectSf;
    cs_naviSf_e mNaviSf;
    cs_trafficSf_e mTrafficSf;
    cs_connectData_s mConnectData;
    cs_stateflow_e mStateflow;

};

#endif /* CONTROLSENDER_H_ */
