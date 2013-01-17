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

#include "CAmControlSenderBase.h"
#include <cassert>
#include <algorithm>
#include "shared/CAmDltWrapper.h"
#include "control/IAmControlReceive.h"

using namespace am;

extern "C" IAmControlSend* PluginControlInterfaceFactory()
{
    return (new CAmControlSenderBase());
}

extern "C" void destroyControlPluginInterface(IAmControlSend* controlSendInterface)
{
    delete controlSendInterface;
}

CAmControlSenderBase::CAmControlSenderBase() :
        mControlReceiveInterface(NULL), //
        mListOpenConnections(), //
        mListOpenDisconnections(), //
        mListOpenVolumeChanges(),//
        mConnectSf (SFC_RAMP_DOWN), //
        mNaviSf(NAVC_RAMP_DOWN), //
        mTrafficSf(TA_RAMP_DOWN),//
        mConnectData(), //
        mStateflow(SF_NONE)
{
}

CAmControlSenderBase::~CAmControlSenderBase()
{
}

am_Error_e CAmControlSenderBase::startupController(IAmControlReceive *controlreceiveinterface)
{
    assert(controlreceiveinterface);
    mControlReceiveInterface = controlreceiveinterface;
    am_sourceClass_t sourceClassID;
    am_SourceClass_s sourceClass;
    sourceClass.name="player";
    sourceClass.sourceClassID=1;
    mControlReceiveInterface->enterSourceClassDB(sourceClassID,sourceClass);
    sourceClass.name="navi";
    sourceClass.sourceClassID=2;
    mControlReceiveInterface->enterSourceClassDB(sourceClassID,sourceClass);
    sourceClass.name="ta";
    sourceClass.sourceClassID=3;
    mControlReceiveInterface->enterSourceClassDB(sourceClassID,sourceClass);
    //here is a good place to insert SystemProperties into the database...
    //and might be a good place to insert the Source and Sink CLasses as well
    return E_NOT_USED;
}

void CAmControlSenderBase::setControllerReady()
{
    //here is a good place to insert Source and SinkClasses into the database...
    mControlReceiveInterface->setRoutingReady();
    mControlReceiveInterface->setCommandReady();
}

am_Error_e CAmControlSenderBase::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    if (mStateflow!=SF_NONE)
        return (E_NOT_POSSIBLE);

    std::vector<am_Route_s> listRoutes;
    std::vector<am_connectionID_t> listConnectionIDs;
    am_SourceClass_s sourceClass, tempSourceClass;
    bool exististingConnection(false);
    mControlReceiveInterface->getRoute(false, sourceID, sinkID, listRoutes);
    if (listRoutes.empty())
        return (E_NOT_POSSIBLE);

    std::vector<am_MainConnection_s> listAllMainConnections;
    mControlReceiveInterface->getListMainConnections(listAllMainConnections);
    mControlReceiveInterface->getSourceClassInfoDB(sourceID, sourceClass);

    //go through all connections
    std::vector<am_MainConnection_s>::iterator itAll(listAllMainConnections.begin());
    for (; itAll != listAllMainConnections.end(); ++itAll)
    {
        if (itAll->sinkID == sinkID && itAll->sourceID == sourceID)
            return (E_ALREADY_EXISTS);

        //check if there is already an existing connection
        mControlReceiveInterface->getSourceClassInfoDB(itAll->sourceID, tempSourceClass);
        if (tempSourceClass.sourceClassID == 1)
        {
            mConnectData.currentMainConnection=itAll->mainConnectionID;
            exististingConnection=true;
            mConnectData.oldSourceID=itAll->sourceID;
        }
    }

    am_MainConnection_s mainConnectionData;
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID = sinkID;
    mainConnectionData.sourceID = sourceID;
    mainConnectionData.connectionState = CS_CONNECTING;
    mainConnectionData.delay = 0;

    mControlReceiveInterface->enterMainConnectionDB(mainConnectionData, mainConnectionID);
    mConnectData.newMainConnection=mainConnectionID;
    mConnectData.sourceID=sourceID;
    mConnectData.sinkID=sinkID;

    if(sourceClass.sourceClassID==1)
    {
        if(exististingConnection)
            mConnectSf=SFC_RAMP_DOWN;
        else
            mConnectSf=SFC_CONNECT;

        mStateflow=SF_CONNECT;
    }
    else if (sourceClass.sourceClassID==2)
    {
        if(exististingConnection)
            mConnectSf=SFC_RAMP_DOWN;
        else
            mConnectSf=SFC_CONNECT;

        mStateflow=SF_NAVI;
    }
    else if (sourceClass.sourceClassID==3)
    {
        if(exististingConnection)
            mConnectSf=SFC_RAMP_DOWN;
        else
            mConnectSf=SFC_CONNECT;

        mStateflow=SF_TA;
    }

    callStateFlowHandler();
    return (E_OK);
}

am_Error_e CAmControlSenderBase::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    if (mStateflow==SF_NAVI)
    {
        mNaviSf=NAVC_RAMP_DOWN_AGAIN;
        callStateFlowHandler();
        return (E_OK);
    }
    if (mStateflow==SF_TA)
    {
        mTrafficSf=TA_RAMP_DOWN_AGAIN;
        callStateFlowHandler();
        return (E_OK);
    }
    else  if (mStateflow!=SF_NONE)
    {
        return (E_NOT_POSSIBLE);
    }
    disconnect(connectionID);
    return (E_OK);
}

am_Error_e CAmControlSenderBase::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    if (mStateflow!=SF_NONE)
        return (E_NOT_POSSIBLE);

    if (sinkID == 0)
        return (E_NON_EXISTENT);

    mainSinkSoundPropertySet set;
    set.sinkID = sinkID;
    set.mainSoundProperty = soundProperty;
    am_SoundProperty_s sp;
    std::vector<mainSinkSoundPropertySet>::iterator it(mListMainSoundPropertyChanges.begin());
    for (; it != mListMainSoundPropertyChanges.end(); ++it)
    {
        if (it->sinkID == sinkID)
            return E_NOT_POSSIBLE;
    }

//I know this is bad - just for the reference, ok?
    sp.type = static_cast<am_SoundPropertyType_e>(soundProperty.type);
    sp.value = (soundProperty.value - 5) * 30;
    am_Error_e error;
    if ((error = mControlReceiveInterface->setSinkSoundProperty(set.handle, sinkID, sp)) != E_OK)
    {
        return error;
    }
    mListMainSoundPropertyChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    (void) sourceID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e CAmControlSenderBase::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    (void) property;
    return E_NOT_USED;
}

am_Error_e CAmControlSenderBase::hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume)
{
    if (mStateflow!=SF_NONE)
        return (E_NOT_POSSIBLE);

    assert(SinkID!=0);
    mainVolumeSet set;
    set.sinkID = SinkID;
    set.mainVolume = newVolume;
    am_Error_e error;

    std::vector<mainVolumeSet>::iterator it(mListOpenVolumeChanges.begin());
    for (; it != mListOpenVolumeChanges.end(); ++it)
    {
        if (it->sinkID == SinkID)
            return E_NOT_POSSIBLE;
    }

    am_Sink_s sinkData;
    mControlReceiveInterface->getSinkInfoDB(SinkID, sinkData);

    if (sinkData.mainVolume == newVolume)
        return E_NO_CHANGE;

    if ((error = mControlReceiveInterface->setSinkVolume(set.handle, SinkID, (newVolume-10)*6, RAMP_UNKNOWN, 20)) != E_OK)
    {
        return error;
    }
    mListOpenVolumeChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment)
{
    if (mStateflow!=SF_NONE)
        return (E_NOT_POSSIBLE);

    assert(SinkID!=0);
    mainVolumeSet set;
    set.sinkID = SinkID;
    am_Error_e error;
    am_Sink_s sink;
    std::vector<mainVolumeSet>::iterator it(mListOpenVolumeChanges.begin());
    for (; it != mListOpenVolumeChanges.end(); ++it)
    {
        if (it->sinkID == SinkID)
            return E_NOT_POSSIBLE;
    }
    mControlReceiveInterface->getSinkInfoDB(SinkID, sink);
    set.mainVolume = sink.mainVolume + increment;
    if ((error = mControlReceiveInterface->setSinkVolume(set.handle, SinkID, (set.mainVolume-10)*6, RAMP_UNKNOWN, 20)) != E_OK)
    {
        return error;
    }
    mListOpenVolumeChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    if (mStateflow!=SF_NONE)
        return (E_NOT_POSSIBLE);

    assert(sinkID!=0);

    mainVolumeSet set;
    set.sinkID = sinkID;
    am_Error_e error;
    am_Sink_s sink;
    mControlReceiveInterface->getSinkInfoDB(sinkID, sink);

    if (muteState == MS_MUTED)
    {
        set.mainVolume = sink.mainVolume;
        if ((error = mControlReceiveInterface->setSinkVolume(set.handle, sinkID, 0, RAMP_GENIVI_DIRECT, 20)) != E_OK)
        {
            return error;
        }
    }
    else
    {
        set.mainVolume = sink.mainVolume;
        if ((error = mControlReceiveInterface->setSinkVolume(set.handle, sinkID, set.mainVolume, RAMP_GENIVI_DIRECT, 20)) != E_OK)
        {
            return error;
        }
    }
    mListOpenVolumeChanges.push_back(set);
    mControlReceiveInterface->changeSinkMuteStateDB(muteState, sinkID);
    return (E_OK);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterDomainDB(domainData, domainID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeDomainDB(domainID);
}

void CAmControlSenderBase::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    (void) domainID;
}

am_Error_e CAmControlSenderBase::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterSinkDB(sinkData, sinkID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeSinkDB(sinkID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterSourceDB(sourceData, sourceID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeSourceDB(sourceID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterGatewayDB(gatewayData, gatewayID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeGatewayDB(gatewayID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterCrossfaderDB(crossfaderData, crossfaderID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
//this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeCrossfaderDB(crossfaderID);
}

void CAmControlSenderBase::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    (void) handle;
    (void) sinkID;
    (void) volume;
}

void CAmControlSenderBase::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    (void) handle;
    (void) sourceID;
    (void) volume;
}

void CAmControlSenderBase::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    (void) sourceID;
    (void) interruptState;
}

void CAmControlSenderBase::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    (void) sinkID;
    (void) availability;
}

void CAmControlSenderBase::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    (void) sourceID;
    (void) availability;
}

void CAmControlSenderBase::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    (void) domainID;
    (void) state;
}

void CAmControlSenderBase::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    (void) data;
}

void CAmControlSenderBase::hookSystemSpeedChange(const am_speed_t speed)
{
    (void) speed;
}

void CAmControlSenderBase::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    (void) mainConnectionID;
    (void) time;
}

void CAmControlSenderBase::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) errorID;
//\todo:error checking
    std::vector<mainConnectionSet>::iterator it(mListOpenConnections.begin());
    for (; it != mListOpenConnections.end(); ++it)
    {
        std::vector<handleStatus>::iterator hit;
        handleStatus status;
        status.status = true;
        status.handle = handle;
        hit = std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), findHandle(status));
        if (hit == it->listHandleStaus.end())
            continue;
        hit->status = true;
        if (it->listHandleStaus.end() == std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), checkHandle(status)))
        {
            mControlReceiveInterface->changeMainConnectionStateDB(it->connectionID, CS_CONNECTED);
            mListOpenConnections.erase(it);
            break;
        }
    }
    callStateFlowHandler();
}

void CAmControlSenderBase::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) errorID;
//\todo:error checking
    std::vector<mainConnectionSet>::iterator it(mListOpenDisconnections.begin());
    for (; it != mListOpenDisconnections.end(); ++it)
    {
        std::vector<handleStatus>::iterator hit;
        handleStatus status;
        status.status = true;
        status.handle = handle;
        hit = std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), findHandle(status));
        if (hit == it->listHandleStaus.end())
            continue;
        hit->status = true;
        if (it->listHandleStaus.end() == std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), checkHandle(status)))
        {
            mControlReceiveInterface->removeMainConnectionDB(it->connectionID);
            mListOpenDisconnections.erase(it);
            break;
        }
    }
    callStateFlowHandler();
}

void CAmControlSenderBase::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    (void) handle;
    (void) hostsink;
    (void) error;
}

void CAmControlSenderBase::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    (void) error;
    (void) volume;
    //\todo:error checking
    std::vector<mainVolumeSet>::iterator it(mListOpenVolumeChanges.begin());
    for (; it != mListOpenVolumeChanges.end(); ++it)
    {
        if (handle.handle == it->handle.handle)
        {
            mControlReceiveInterface->changeSinkMainVolumeDB(it->mainVolume, it->sinkID);
            mListOpenVolumeChanges.erase(it);
            break;
        }
    }
    callStateFlowHandler();
}

void CAmControlSenderBase::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
    (void) error;
    (void) voulme;
    (void) handle;
    callStateFlowHandler();
}

void CAmControlSenderBase::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
    callStateFlowHandler();
}

void CAmControlSenderBase::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
//\todo:error checking
    std::vector<mainSinkSoundPropertySet>::iterator it(mListMainSoundPropertyChanges.begin());
    for (; it != mListMainSoundPropertyChanges.end(); ++it)
    {
        if (handle.handle == it->handle.handle)
        {
            mControlReceiveInterface->changeMainSinkSoundPropertyDB(it->mainSoundProperty, it->sinkID);
            mListMainSoundPropertyChanges.erase(it);
            break;
        }
    }
}

void CAmControlSenderBase::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::setControllerRundown(const int16_t signal)
{
    logInfo("CAmControlSenderBase::setControllerRundown() was called signal=",signal);
    if (signal==2)
        mControlReceiveInterface->confirmControllerRundown(E_UNKNOWN);

    mControlReceiveInterface->confirmControllerRundown(E_OK);
}

am_Error_e CAmControlSenderBase::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e> & listPrioConnectionFormats)
{
    (void) sourceID;
    (void) sinkID;
    (void) listRoute;
//ok, this is cheap. In a real product you have your preferences, right?
    listPrioConnectionFormats = listPossibleConnectionFormats;
    return (E_OK);
}

void CAmControlSenderBase::getInterfaceVersion(std::string & version) const
{
    version = ControlSendVersion;
}


void CAmControlSenderBase::disconnect(am_mainConnectionID_t connectionID)
{

    am_MainConnection_s mainConnection;
    am_Error_e error;
    if ((error = mControlReceiveInterface->getMainConnectionInfoDB(connectionID, mainConnection)) != E_OK)
    {
        logError("CAmControlSenderBase::disconnect Could not getInfor for mainconnection Error: ", error);
        return;
    }

    std::vector<am_connectionID_t>::iterator it(mainConnection.listConnectionID.begin());
    std::vector<handleStatus> listHandleStaus;
    for (; it != mainConnection.listConnectionID.end(); ++it)
    {
        handleStatus status;
        status.status = false;
        if ((error = mControlReceiveInterface->disconnect(status.handle, *it)))
        {
            logError("Could not disconnect, Error", error);
        }
        listHandleStaus.push_back(status);
    }
    mainConnectionSet set;
    set.connectionID = connectionID;
    set.listHandleStaus = listHandleStaus;
    mListOpenDisconnections.push_back(set);
}

void CAmControlSenderBase::connect(am_sourceID_t sourceID, am_sinkID_t sinkID, am_mainConnectionID_t mainConnectionID)
{
    std::vector<am_Route_s> listRoutes;
    std::vector<am_connectionID_t> listConnectionIDs;
    am_SourceClass_s sourceClass, tempSourceClass;
    am_Handle_s handle;
    mControlReceiveInterface->getRoute(false, sourceID, sinkID, listRoutes);
    if (listRoutes.empty())
        logError("CAmControlSenderBase::connect not possible");

    std::vector<handleStatus> listHandleStaus;
    std::vector<am_RoutingElement_s>::iterator it(listRoutes[0].route.begin());
    for (; it != listRoutes[0].route.end(); ++it)
    {
        am_connectionID_t connectionID;
        mControlReceiveInterface->connect(handle, connectionID, it->connectionFormat, it->sourceID, it->sinkID);
        handleStatus status;
        status.handle = handle;
        status.status = false;
        listHandleStaus.push_back(status);
        listConnectionIDs.push_back(connectionID);
    }
    mainConnectionSet set;
    set.connectionID = mainConnectionID;
    set.listHandleStaus = listHandleStaus;
    mControlReceiveInterface->changeMainConnectionRouteDB(mainConnectionID,listConnectionIDs);
    mListOpenConnections.push_back(set);
}

void CAmControlSenderBase::callStateFlowHandler()
{
    logInfo("CAmControlSenderBase::callStateFlowHandler() called, state ",mStateflow);

    switch(mStateflow)
    {
    case SF_CONNECT:
        callConnectHandler();
        break;
    case SF_TA:
        callTAHandler();
        break;
    case SF_NAVI:
        callNaviHandler();
        break;
    default:
        break;

    }
}

void CAmControlSenderBase::callConnectHandler()
{
    logInfo("CAmControlSenderBase::callConnectHandler() called, state ",mConnectSf);

    am_Handle_s handle;
    am_Sink_s sinkData;
    switch (mConnectSf)
    {
    case SFC_RAMP_DOWN:
        mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, -60, RAMP_GENIVI_EXP_INV, 4000);
        mConnectSf = SFC_SOURCE_STATE_OFF;
        break;
    case SFC_SOURCE_STATE_OFF:
        mControlReceiveInterface->setSourceState(handle, mConnectData.oldSourceID, SS_OFF);
        mConnectSf = SFC_DISCONNECT;
        break;
    case SFC_DISCONNECT:
        disconnect(mConnectData.currentMainConnection);
        mConnectSf = SFC_CONNECT;
        break;
    case SFC_CONNECT:
        connect(mConnectData.sourceID, mConnectData.sinkID, mConnectData.newMainConnection);
        mConnectSf = SFC_SOURCE_STATE_ON;
        break;
    case SFC_SOURCE_STATE_ON:
        mControlReceiveInterface->setSourceState(handle, mConnectData.sourceID, SS_ON);
        mConnectSf = SFC_RAMP_UP;
        break;
    case SFC_RAMP_UP:
        mControlReceiveInterface->getSinkInfoDB(mConnectData.sinkID, sinkData);
        if (mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, (sinkData.mainVolume-10)*6, RAMP_GENIVI_EXP_INV, 4000)==E_NO_CHANGE)
            mStateflow=SF_NONE;
        mConnectSf=SFC_FINISHED;
        break;
    case SFC_FINISHED:
        mStateflow=SF_NONE;
        break;
    default:
        break;
    }
}

void CAmControlSenderBase::callNaviHandler()
{
    logInfo("CAmControlSenderBase::callNaviHandler() called, state ",mConnectSf);

       am_Handle_s handle;
       am_Source_s sourceData;
       am_Sink_s sinkData;
       switch (mNaviSf)
       {
       case NAVC_RAMP_DOWN:
           mControlReceiveInterface->setSourceVolume(handle, mConnectData.oldSourceID, -13, RAMP_UNKNOWN, 2000);
           mNaviSf = NAVC_CONNECT;
           break;
       case NAVC_CONNECT:
           connect(mConnectData.sourceID, mConnectData.sinkID, mConnectData.newMainConnection);
           mNaviSf = NAVC_SOURCE_STATE_ON;
           break;
       case NAVC_SOURCE_STATE_ON:
           mControlReceiveInterface->setSourceState(handle, mConnectData.sourceID, SS_ON);
           mNaviSf = NAVC_RAMP_UP;
           break;
       case NAVC_RAMP_UP:
           mControlReceiveInterface->getSinkInfoDB(mConnectData.sinkID, sinkData);
           mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, (sinkData.mainVolume-10)*60+2, RAMP_UNKNOWN, 2000);
           mNaviSf=NAVC_WAIT_STATE;
           break;
       case NAVC_WAIT_STATE:
           mNaviSf=NAVC_RAMP_DOWN_AGAIN;
           break;
       case NAVC_RAMP_DOWN_AGAIN:
           mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, (sinkData.mainVolume-10)*60, RAMP_UNKNOWN, 2000);
           mNaviSf=NAVC_SOURCE_VOLUME_BACK;
           break;
       case NAVC_SOURCE_VOLUME_BACK:
           mControlReceiveInterface->setSourceVolume(handle, mConnectData.oldSourceID, 0, RAMP_UNKNOWN, 2000);
           mNaviSf=NAVC_SOURCE_ACTIVITY_BACK;
           break;
       case NAVC_SOURCE_ACTIVITY_BACK:
           mControlReceiveInterface->setSourceState(handle, mConnectData.sourceID, SS_OFF);
           mNaviSf=NAVC_DISCONNECT;
           break;
       case NAVC_DISCONNECT:
           disconnect(mConnectData.newMainConnection);
           mNaviSf=NAVC_FINISHED;
           break;
       case NAVC_FINISHED:
           mStateflow=SF_NONE;
           break;
       default:
           break;
       }
}

void CAmControlSenderBase::confirmCommandReady(const am_Error_e error)
{
    (void) error;
    logInfo("ControlSenderPlugin got Command Ready confirmed");
}

void CAmControlSenderBase::confirmRoutingReady(const am_Error_e error)
{
    (void)error;
    logInfo("ControlSenderPlugin got Routing Ready confirmed");
}

void CAmControlSenderBase::confirmCommandRundown(const am_Error_e error)
{
    (void)error;
	logInfo("ControlSenderPlugin got Command Rundown confirmed");
}

void CAmControlSenderBase::confirmRoutingRundown(const am_Error_e error)
{
    (void)error;
	logInfo("ControlSenderPlugin got Routing Rundown confirmed");
}

void CAmControlSenderBase::hookSystemNodeStateChanged(const NsmNodeState_e NodeStateId)
{
    (void) NodeStateId;
    //here you can process informations about the notestate
}

void CAmControlSenderBase::hookSystemNodeApplicationModeChanged(const NsmApplicationMode_e ApplicationModeId)
{
    (void) ApplicationModeId;
}


void CAmControlSenderBase::cbAckSetSinkNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    (void) handle;
    (void) error;
}

void CAmControlSenderBase::cbAckSetSourceNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    (void) handle;
    (void) error;
}


NsmErrorStatus_e CAmControlSenderBase::hookSystemLifecycleRequest(const uint32_t Request, const uint32_t RequestId)
{
    (void) Request;
    (void) RequestId;
    logInfo("CAmControlSenderBase::hookSystemLifecycleRequest request=",Request," requestID=",RequestId);
    return (NsmErrorStatus_Error);
}

am_Error_e CAmControlSenderBase::hookSystemUpdateSink(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_ConnectionFormat_e>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    (void) sinkID;
    (void) sinkClassID;
    (void) listMainSoundProperties;
    (void) listConnectionFormats;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e CAmControlSenderBase::hookSystemUpdateSource(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_ConnectionFormat_e>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    (void) sourceID;
    (void) sourceClassID;
    (void) listSoundProperties;
    (void) listMainSoundProperties;
    (void) listConnectionFormats;
    return (E_NOT_USED);
}

am_Error_e CAmControlSenderBase::hookSystemUpdateGateway(const am_gatewayID_t gatewayID, const std::vector<am_ConnectionFormat_e>& listSourceConnectionFormats, const std::vector<am_ConnectionFormat_e>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    (void) gatewayID;
    (void) listSourceConnectionFormats;
    (void) listSinkConnectionFormats;
    (void) convertionMatrix;
    return (E_NOT_USED);
}

void CAmControlSenderBase::cbAckSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes, const am_Error_e error)
{
    (void) handle;
    (void) listVolumes;
    (void) error;
}

void CAmControlSenderBase::hookSinkNotificationDataChanged(const am_sinkID_t sinkID, const am_NotificationPayload_s& payload)
{
    (void) sinkID;
    (void) payload;
}

void CAmControlSenderBase::hookSourceNotificationDataChanged(const am_sourceID_t sourceID, const am_NotificationPayload_s& payload)
{
    (void) sourceID;
    (void) payload;
}

am_Error_e CAmControlSenderBase::hookUserSetMainSinkNotificationConfiguration(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) sinkID;
    (void) notificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e CAmControlSenderBase::hookUserSetMainSourceNotificationConfiguration(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) sourceID;
    (void) notificationConfiguration;
    return (E_NOT_USED);
}

void CAmControlSenderBase::hookSystemSessionStateChanged(const std::string& sessionName, const NsmSeat_e seatID, const NsmSessionState_e sessionStateID)
{
    (void) sessionName;
    (void) seatID;
    (void) sessionStateID;
}

void CAmControlSenderBase::callTAHandler()
{
    logInfo("CAmControlSenderBase::callTAHandler() called, state ",mConnectSf);

       am_Handle_s handle;
       am_Source_s sourceData;
       am_Sink_s sinkData;
       switch (mTrafficSf)
       {
       case TA_RAMP_DOWN:
           mControlReceiveInterface->setSourceState(handle, mConnectData.oldSourceID, SS_PAUSED);
           mTrafficSf = TA_CONNECT;
           break;
       case TA_CONNECT:
           connect(mConnectData.sourceID, mConnectData.sinkID, mConnectData.newMainConnection);
           mTrafficSf = TA_SOURCE_STATE_ON;
           break;
       case TA_SOURCE_STATE_ON:
           mControlReceiveInterface->setSourceState(handle, mConnectData.sourceID, SS_ON);
           mTrafficSf = TA_RAMP_UP;
           break;
       case TA_RAMP_UP:
           mControlReceiveInterface->getSinkInfoDB(mConnectData.sinkID, sinkData);
           mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, (sinkData.mainVolume-10)*60+2, RAMP_UNKNOWN, 2000);
           mTrafficSf=TA_WAIT_STATE;
           break;
       case TA_WAIT_STATE:
           mTrafficSf=TA_RAMP_DOWN_AGAIN;
           break;
       case TA_RAMP_DOWN_AGAIN:
           mControlReceiveInterface->setSinkVolume(handle, mConnectData.sinkID, (sinkData.mainVolume-10)*60, RAMP_UNKNOWN, 2000);
           mTrafficSf=TA_SOURCE_STATE_OFF;
           break;
       case TA_SOURCE_STATE_OFF:
           mControlReceiveInterface->setSourceState(handle, mConnectData.oldSourceID, SS_ON);
           mTrafficSf=TA_SOURCE_STATE_OLD_OFF;
           break;
       case TA_SOURCE_STATE_OLD_OFF:
           mControlReceiveInterface->setSourceState(handle, mConnectData.sourceID, SS_OFF);
           mTrafficSf=TA_DISCONNECT;
           break;
       case TA_DISCONNECT:
           disconnect(mConnectData.newMainConnection);
           mTrafficSf=TA_FINISHED;
           break;
       case TA_FINISHED:
           mStateflow=SF_NONE;
           break;
       default:
           break;
       }
}



