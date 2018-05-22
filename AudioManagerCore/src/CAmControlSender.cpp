/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \file CAmControlSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmControlSender.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "TAmPluginTemplate.h"
#include "CAmDltWrapper.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1  //!< major interface version. All versions smaller than this will be rejected
#define REQUIRED_INTERFACE_VERSION_MINOR 0 //!< minor interface version. All versions smaller than this will be rejected

CAmControlSender* CAmControlSender::mInstance=NULL;

CAmControlSender::CAmControlSender(std::string controlPluginFile,CAmSocketHandler* sockethandler) :
        receiverCallbackT(this, &CAmControlSender::receiverCallback),
        checkerCallbackT(this, &CAmControlSender::checkerCallback),
        dispatcherCallbackT(this, &CAmControlSender::dispatcherCallback),
        mPipe(),
        mlibHandle(NULL),
        mController(NULL),
        mSignal(0)
{
    assert(sockethandler);

    //Check if a folder is given, then select the first plugin
    struct stat buf;
    const char* conFile(controlPluginFile.c_str());
    stat(conFile, &buf);
    if (S_ISDIR(buf.st_mode))
    {
        std::string directoryName(controlPluginFile);
        logInfo("Searching for ControlPlugin in", directoryName);
        DIR *directory = opendir(directoryName.c_str());

        if (!directory)
        {
            logError("Error opening directory ", directoryName);
            throw std::runtime_error("Controller directory could not be openend");
        }

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
            unsigned char entryType = itemInDirectory->d_type;
            std::string entryName = itemInDirectory->d_name;
            std::string fullName = directoryName + "/" + entryName;

            bool regularFile = (entryType == DT_REG || entryType == DT_LNK);
            bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

            // Handle cases where readdir() could not determine the file type
	        if (entryType == DT_UNKNOWN) {
	            struct stat buf;

	            if (stat(fullName.c_str(), &buf)) {
	                logInfo(__PRETTY_FUNCTION__,"Failed to stat file: ", entryName, errno);
	                continue;
	            }

	            regularFile = S_ISREG(buf.st_mode);
	        }

            if (regularFile && sharedLibExtension)
            {
                controlPluginFile=directoryName + "/" + entryName;
				logInfo("Found ControlPlugin:", controlPluginFile);
				break;
            }
        }
        closedir(directory);
	
    }
    
    std::ifstream isfile(controlPluginFile.c_str());
    if (!isfile)
    {
        logError("ControlSender::ControlSender: Controller plugin not found:", controlPluginFile);
        throw std::runtime_error("Could not find controller plugin!");
    }
    else if (!controlPluginFile.empty())
    {
        mInstance=this;
        IAmControlSend* (*createFunc)();
        createFunc = getCreateFunction<IAmControlSend*()>(controlPluginFile, mlibHandle);
        assert(createFunc!=NULL);
        mController = createFunc();
        mControlPluginFile = controlPluginFile;
        //check libversion
        std::string version, cVersion(ControlVersion);
        mController->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion, cMinorVersion, cMajorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;
        std::istringstream(cVersion.substr(0, 1)) >> cMajorVersion;
        std::istringstream(cVersion.substr(2, 1)) >> cMinorVersion;
        
        

        if (majorVersion < cMajorVersion || ((majorVersion == cMajorVersion) && (minorVersion < cMinorVersion)))
        {
            logError("ControlSender::ControlSender: Interface Version of Controller too old, required version:",ControlVersion," Controller Version:",version,"exiting now");
            throw std::runtime_error("Interface Version of Controller too old");
        }
    }
    else
    {
        logError("ControlSender::ControlSender: No controller loaded !");
    }

    //here we need a pipe to be able to call the rundown function out of the mainloop
    if (pipe(mPipe) == -1)
    {
        logError("CAmControlSender could not create pipe!");
    }

    //add the pipe to the poll - nothing needs to be proccessed here we just need the pipe to trigger the ppoll
    short event = 0;
    sh_pollHandle_t handle;
    event |= POLLIN;
    sockethandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, &dispatcherCallbackT, NULL, handle);
}

CAmControlSender::~CAmControlSender()
{
    close(mPipe[0]);
    close(mPipe[1]);

    if (mlibHandle)
    {
        void (*destroyFunc)(IAmControlSend*);
        destroyFunc = getDestroyFunction<void(IAmControlSend*)>(mControlPluginFile, mlibHandle);
        if (destroyFunc)
        {
            destroyFunc(mController);
        }
        else
        {
            logError("CAmControlSender Dtor: destroyFunc is invalid or not found");
        }
        dlclose(mlibHandle);
    }
}

am_Error_e CAmControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    assert(mController);
    return (mController->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

am_Error_e CAmControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    assert(mController);
    return (mController->hookUserDisconnectionRequest(connectionID));
}

am_Error_e CAmControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    assert(mController);
    return (mController->hookUserSetMainSinkSoundProperty(sinkID, soundProperty));
}

am_Error_e CAmControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    assert(mController);
    return (mController->hookUserSetMainSourceSoundProperty(sourceID, soundProperty));
}

am_Error_e CAmControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    assert(mController);
    return (mController->hookUserSetSystemProperty(property));
}

am_Error_e CAmControlSender::hookUserVolumeChange(const am_sinkID_t sinkID, const am_mainVolume_t newVolume)
{
    assert(mController);
    return (mController->hookUserVolumeChange(sinkID, newVolume));
}

am_Error_e CAmControlSender::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
    assert(mController);
    return (mController->hookUserVolumeStep(sinkID, increment));
}

am_Error_e CAmControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    assert(mController);
    return (mController->hookUserSetSinkMuteState(sinkID, muteState));
}

am_Error_e CAmControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    assert(mController);
    return (mController->hookSystemRegisterDomain(domainData, domainID));
}

am_Error_e CAmControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    assert(mController);
    return (mController->hookSystemDeregisterDomain(domainID));
}

void CAmControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    assert(mController);
    return (mController->hookSystemDomainRegistrationComplete(domainID));
}

am_Error_e CAmControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    assert(mController);
    return (mController->hookSystemRegisterSink(sinkData, sinkID));
}

am_Error_e CAmControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    assert(mController);
    return (mController->hookSystemDeregisterSink(sinkID));
}

am_Error_e CAmControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    assert(mController);
    return (mController->hookSystemRegisterSource(sourceData, sourceID));
}

am_Error_e CAmControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    assert(mController);
    return (mController->hookSystemDeregisterSource(sourceID));
}

am_Error_e CAmControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    assert(mController);
    return (mController->hookSystemRegisterGateway(gatewayData, gatewayID));
}

am_Error_e CAmControlSender::hookSystemRegisterConverter(const am_Converter_s& converterData, am_converterID_t& converterID)
{
    assert(mController);
    return (mController->hookSystemRegisterConverter(converterData, converterID));
}

am_Error_e CAmControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    assert(mController);
    return (mController->hookSystemDeregisterGateway(gatewayID));
}

am_Error_e CAmControlSender::hookSystemDeregisterConverter(const am_converterID_t converterID)
{
    assert(mController);
    return (mController->hookSystemDeregisterConverter(converterID));
}

am_Error_e CAmControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    assert(mController);
    return (mController->hookSystemRegisterCrossfader(crossfaderData, crossfaderID));
}

am_Error_e CAmControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    assert(mController);
    return (mController->hookSystemDeregisterCrossfader(crossfaderID));
}

void CAmControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    assert(mController);
    mController->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

void CAmControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    assert(mController);
    mController->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void CAmControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    assert(mController);
    mController->hookSystemInterruptStateChange(sourceID, interruptState);
}

void CAmControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    assert(mController);
    mController->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void CAmControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    assert(mController);
    mController->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void CAmControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    assert(mController);
    mController->hookSystemDomainStateChange(domainID, state);
}

void CAmControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    assert(mController);
    mController->hookSystemReceiveEarlyData(data);
}

void CAmControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
    assert(mController);
    mController->hookSystemSpeedChange(speed);
}

void CAmControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    assert(mController);
    mController->hookSystemTimingInformationChanged(mainConnectionID, time);
}

void CAmControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    assert(mController);
    mController->cbAckConnect(handle, errorID);
}

void CAmControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    assert(mController);
    mController->cbAckDisconnect(handle, errorID);
}

void CAmControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    assert(mController);
    mController->cbAckCrossFade(handle, hostsink, error);
}

void CAmControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceState(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceSoundProperty(handle, error);
}

am_Error_e CAmControlSender::startupController(IAmControlReceive *controlreceiveinterface)
{
    if (!mController)
    {
        logError("ControlSender::startupController: no Controller to startup!");
        throw std::runtime_error("ControlSender::startupController: no Controller to startup! Exiting now ...");
        return (E_NON_EXISTENT);
    }
    return (mController->startupController(controlreceiveinterface));
}

void CAmControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkSoundProperty(handle, error);
}

void CAmControlSender::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkSoundProperties(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceSoundProperties(handle, error);
}

void CAmControlSender::setControllerReady()
{
    assert(mController);
    mController->setControllerReady();
}

void CAmControlSender::setControllerRundown(const int16_t signal)
{
    assert(mController);
    logInfo("CAmControlSender::setControllerRundown received, signal=",signal);
    mController->setControllerRundown(signal);
}

am_Error_e am::CAmControlSender::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_CustomConnectionFormat_t> listPossibleConnectionFormats, std::vector<am_CustomConnectionFormat_t> & listPrioConnectionFormats)
{
    assert(mController);
    return (mController->getConnectionFormatChoice(sourceID, sinkID, listRoute, listPossibleConnectionFormats, listPrioConnectionFormats));
}

void CAmControlSender::getInterfaceVersion(std::string & version) const
{
    version = ControlVersion;
}

void CAmControlSender::confirmCommandReady(const am_Error_e error)
{
    assert(mController);
    mController->confirmCommandReady(error);
}

void CAmControlSender::confirmRoutingReady(const am_Error_e error)
{
    assert(mController);
    mController->confirmRoutingReady(error);
}

void CAmControlSender::confirmCommandRundown(const am_Error_e error)
{
    assert(mController);
    mController->confirmCommandRundown(error);
}

void CAmControlSender::confirmRoutingRundown(const am_Error_e error)
{
    assert(mController);
    mController->confirmRoutingRundown(error);
}

am_Error_e CAmControlSender::hookSystemUpdateSink(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(mController);
    return (mController->hookSystemUpdateSink(sinkID,sinkClassID,listSoundProperties,listConnectionFormats,listMainSoundProperties));
}

am_Error_e CAmControlSender::hookSystemUpdateSource(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(mController);
    return (mController->hookSystemUpdateSource(sourceID,sourceClassID,listSoundProperties,listConnectionFormats,listMainSoundProperties));
}

am_Error_e CAmControlSender::hookSystemUpdateGateway(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFromats, const std::vector<bool>& convertionMatrix)
{
    assert(mController);
    return (mController->hookSystemUpdateGateway(gatewayID,listSourceConnectionFormats,listSinkConnectionFromats,convertionMatrix));
}

am_Error_e CAmControlSender::hookSystemUpdateConverter(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFromats, const std::vector<bool>& convertionMatrix)
{
    assert(mController);
    return (mController->hookSystemUpdateConverter(converterID,listSourceConnectionFormats,listSinkConnectionFromats,convertionMatrix));
}

void CAmControlSender::cbAckSetVolume(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetVolumes(handle,listVolumes,error);
}

void CAmControlSender::cbAckSetSinkNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkNotificationConfiguration(handle,error);
}

void CAmControlSender::cbAckSetSourceNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceNotificationConfiguration(handle,error);
}

void CAmControlSender::hookSinkNotificationDataChanged(const am_sinkID_t sinkID, const am_NotificationPayload_s& payload)
{
    assert(mController);
    mController->hookSinkNotificationDataChanged(sinkID,payload);
}

void CAmControlSender::hookSourceNotificationDataChanged(const am_sourceID_t sourceID, const am_NotificationPayload_s& payload)
{
    assert(mController);
    mController->hookSourceNotificationDataChanged(sourceID,payload);
}

am_Error_e CAmControlSender::hookUserSetMainSinkNotificationConfiguration(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    assert(mController);
    return (mController->hookUserSetMainSinkNotificationConfiguration(sinkID,notificationConfiguration));
}

am_Error_e CAmControlSender::hookUserSetMainSourceNotificationConfiguration(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    assert(mController);
    return (mController->hookUserSetMainSourceNotificationConfiguration(sourceID,notificationConfiguration));
}

void CAmControlSender::receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
{
   (void) handle;
   (void) userData;
   //get the signal number from the socket
   ssize_t result = read(pollfd.fd, &mSignal, sizeof(mSignal));
}

bool CAmControlSender::checkerCallback(const sh_pollHandle_t handle, void* userData)
{
   (void) handle;
   (void) userData;
   return (true);
}

void CAmControlSender::hookSystemSingleTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t time)
{
    assert(mController);
    mController->hookSystemSingleTimingInformationChanged(connectionID,time);
}

/**for testing only contructor - do not use !
 *
 */
CAmControlSender::CAmControlSender() :
    receiverCallbackT(this, &CAmControlSender::receiverCallback),
    checkerCallbackT(this, &CAmControlSender::checkerCallback),
    dispatcherCallbackT(this, &CAmControlSender::dispatcherCallback),
    mlibHandle(NULL),
    mController(NULL),
    mSignal(0)
{
    logInfo("CAmControlSender was loaded in test mode!");
    std::memset(mPipe, -1, sizeof(mPipe));
}

bool CAmControlSender::dispatcherCallback(const sh_pollHandle_t handle, void* userData)
{
   (void)handle;
   (void)userData;
   setControllerRundown(mSignal);
   return (false);
}

}


