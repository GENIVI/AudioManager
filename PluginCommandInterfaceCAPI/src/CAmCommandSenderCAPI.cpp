/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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

#include "CAmCommandSenderCAPI.h"
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <set>
#include "shared/CAmDltWrapper.h"
#include "CAmCommandSenderCommon.h"


DLT_DECLARE_CONTEXT(ctxCommandCAPI)

/**
 * factory for plugin loading
 */
extern "C" IAmCommandSend* PluginCommandInterfaceCAPIFactory()
{
    CAmDltWrapper::instance()->registerContext(ctxCommandCAPI, "CAPIP", "Common-API Plugin");
    return (new CAmCommandSenderCAPI(Am_CAPI));
}

/**
 * destroy instance of commandSendInterface
 */
extern "C" void destroyPluginCommandInterfaceCAPIFactory(IAmCommandSend* commandSendInterface)
{
    delete commandSendInterface;
}


const char * CAmCommandSenderCAPI::COMMAND_SENDER_SERVICE = "local:org.genivi.audiomanger.commandinterface:org.genivi.audiomanger";

#define RETURN_IF_NOT_READY() if(!mReady) return;

CAmCommandSenderCAPI::CAmCommandSenderCAPI() :
        mService(), //
        mpCAmCAPIWrapper(NULL), //
        mpIAmCommandReceive(NULL), //
        mReady(false),
        mIsServiceStarted(false)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CommandSenderCAPI constructor called");
}

CAmCommandSenderCAPI::CAmCommandSenderCAPI(CAmCommonAPIWrapper *aWrapper) :
        mService(), //
        mpCAmCAPIWrapper(aWrapper), //
        mpIAmCommandReceive(NULL), //
        mReady(false),
        mIsServiceStarted(false)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CommandSenderCAPI constructor called");
    assert(mpCAmCAPIWrapper!=NULL);
}

CAmCommandSenderCAPI::~CAmCommandSenderCAPI()
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "CAPICommandSender destructed");
    CAmDltWrapper::instance()->unregisterContext(ctxCommandCAPI);
    tearDownInterface(mpIAmCommandReceive);
}

/**
 * registers a service
 */
am_Error_e CAmCommandSenderCAPI::startService(IAmCommandReceive* commandreceiveinterface)
{
	if(!mIsServiceStarted)
	{
		assert(commandreceiveinterface);
		mService = std::make_shared<CAmCommandSenderService>(commandreceiveinterface);
		//Registers the service
		if( false == mpCAmCAPIWrapper->registerStub(mService, CAmCommandSenderCAPI::COMMAND_SENDER_SERVICE) )
		{
			return (E_NOT_POSSIBLE);
		}
		mIsServiceStarted = true;
	}
    return (E_OK);
}

/**
 * sets a command receiver object and registers a service
 */
am_Error_e CAmCommandSenderCAPI::startupInterface(IAmCommandReceive* commandreceiveinterface)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "startupInterface called");
    mpIAmCommandReceive = commandreceiveinterface;
    return startService(commandreceiveinterface);
}

/**
 * stops the service
 */
am_Error_e CAmCommandSenderCAPI::tearDownInterface(IAmCommandReceive*)
{
    if(mpCAmCAPIWrapper)
    {
    	if(mIsServiceStarted)
    	{
    		mIsServiceStarted = false;
			mpCAmCAPIWrapper->unregisterStub(CAmCommandSenderCAPI::COMMAND_SENDER_SERVICE);
			mService.reset();
    	}
   		return (E_OK);
    }
    return (E_NOT_POSSIBLE);
}

void CAmCommandSenderCAPI::setCommandReady(const uint16_t handle)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbCommunicationReady called");
    mReady = true;
    mpIAmCommandReceive->confirmCommandReady(handle,E_OK);
}

void CAmCommandSenderCAPI::setCommandRundown(const uint16_t handle)
{
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbCommunicationRundown called");
    mReady = false;
    mpIAmCommandReceive->confirmCommandRundown(handle,E_OK);
}

void CAmCommandSenderCAPI::cbNewMainConnection(const am_MainConnectionType_s& )
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNumberOfMainConnectionsChanged called");
    mService->fireNumberOfMainConnectionsChangedEvent();
}

void CAmCommandSenderCAPI::cbRemovedMainConnection(const am_mainConnectionID_t mainConnection)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNumberOfMainConnectionsChanged called");
    mService->fireNumberOfMainConnectionsChangedEvent();
}

void CAmCommandSenderCAPI::cbNewSink(const am_SinkType_s& sink)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNewSink called");
    CommandInterface::am_Availability_s convAvailability;
    CAmConvertAvailablility(sink.availability, convAvailability);
    CommandInterface::am_SinkType_s ciSink(sink.sinkID, sink.name, convAvailability, sink.volume, CAmConvert2CAPIType(sink.muteState), sink.sinkClassID);
    mService->fireSinkAddedEvent(ciSink);
}

void CAmCommandSenderCAPI::cbRemovedSink(const am_sinkID_t sinkID)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbRemovedSink called");
    mService->fireSinkRemovedEvent(sinkID);
}

void CAmCommandSenderCAPI::cbNewSource(const am_SourceType_s& source)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNewSource called");
    CommandInterface::am_Availability_s convAvailability;
    CAmConvertAvailablility(source.availability, convAvailability);
    CommandInterface::am_SourceType_s ciSource(source.sourceID, source.name, convAvailability, source.sourceClassID);
    mService->fireSourceAddedEvent(ciSource);
}

void CAmCommandSenderCAPI::cbRemovedSource(const am_sourceID_t source)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbRemovedSource called");
    mService->fireSourceRemovedEvent(source);
}

void CAmCommandSenderCAPI::cbNumberOfSinkClassesChanged()
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNumberOfSinkClassesChanged called");
    mService->fireNumberOfSinkClassesChangedEvent();
}

void CAmCommandSenderCAPI::cbNumberOfSourceClassesChanged()
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbNumberOfSourceClassesChanged called");
    mService->fireNumberOfSourceClassesChangedEvent();
}

void CAmCommandSenderCAPI::cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbMainConnectionStateChanged called, connectionID=", connectionID, "connectionState=", connectionState);
    CommandInterface::am_mainConnectionID_t cID = connectionID;
    mService->fireMainConnectionStateChangedEvent(cID, CAmConvert2CAPIType(connectionState));
}

void CAmCommandSenderCAPI::cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbMainSinkSoundPropertyChanged called, sinkID", sinkID, "SoundProperty.type", soundProperty.type, "SoundProperty.value", soundProperty.value);
    CommandInterface::am_MainSoundProperty_s mainSoundProp(CAmConvert2CAPIType(soundProperty.type), soundProperty.value);
	mService->fireMainSinkSoundPropertyChangedEvent(sinkID, mainSoundProp);
}

void CAmCommandSenderCAPI::cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbMainSourceSoundPropertyChanged called, sourceID", sourceID, "SoundProperty.type", SoundProperty.type, "SoundProperty.value", SoundProperty.value);
    CommandInterface::am_MainSoundProperty_s convValue;
    CAmConvertMainSoundProperty(SoundProperty, convValue);
    mService->fireMainSourceSoundPropertyChangedEvent(sourceID, convValue);
}

void CAmCommandSenderCAPI::cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSinkAvailabilityChanged called, sinkID", sinkID, "availability.availability", availability.availability, "SoundProperty.reason", availability.availabilityReason);
    CommandInterface::am_Availability_s convAvailability;
    CAmConvertAvailablility(availability, convAvailability);
    mService->fireSinkAvailabilityChangedEvent(sinkID, convAvailability);
}

void CAmCommandSenderCAPI::cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSourceAvailabilityChanged called, sourceID", sourceID, "availability.availability", availability.availability, "SoundProperty.reason", availability.availabilityReason);
    CommandInterface::am_Availability_s convAvailability;
    CAmConvertAvailablility(availability, convAvailability);
    mService->fireSourceAvailabilityChangedEvent(sourceID, convAvailability);
}

void CAmCommandSenderCAPI::cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbVolumeChanged called, sinkID", sinkID, "volume", volume);
    mService->fireVolumeChangedEvent(sinkID, volume);
}

void CAmCommandSenderCAPI::cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSinkMuteStateChanged called, sinkID", sinkID, "muteState", muteState);
    CommandInterface::am_MuteState_e ciMuteState = CAmConvert2CAPIType(muteState);
    mService->fireSinkMuteStateChangedEvent(sinkID, ciMuteState);
}

void CAmCommandSenderCAPI::cbSystemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSystemPropertyChanged called, SystemProperty.type", SystemProperty.type, "SystemProperty.value", SystemProperty.value);
    CommandInterface::am_SystemProperty_s convValue;
    CAmConvertSystemProperty(SystemProperty, convValue);
    mService->fireSystemPropertyChangedEvent(convValue);
}

void CAmCommandSenderCAPI::cbTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbTimingInformationChanged called, mainConnectionID=", mainConnectionID, "time=", time);
    CommandInterface::am_mainConnectionID_t ciMainConnection = mainConnectionID;
    mService->fireTimingInformationChangedEvent(ciMainConnection, time);
}

void CAmCommandSenderCAPI::getInterfaceVersion(std::string & version) const
{
    version = CommandSendVersion;
}

void CAmCommandSenderCAPI::cbSinkUpdated(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSinkUpdated called, sinkID", sinkID);
    CommandInterface::am_MainSoundProperty_l list;
    std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				CommandInterface::am_MainSoundProperty_s prop(CAmConvert2CAPIType(ref.type), ref.value);
				list.push_back(prop);
    });
    mService->fireSinkUpdatedEvent(sinkID, sinkClassID, list);
}

void CAmCommandSenderCAPI::cbSourceUpdated(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSourceUpdated called, sourceID", sourceID);
    CommandInterface::am_MainSoundProperty_l list;
    std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				CommandInterface::am_MainSoundProperty_s prop(CAmConvert2CAPIType(ref.type), ref.value);
				list.push_back(prop);
    });
    mService->fireSourceUpdatedEvent(sourceID, sourceClassID, list);
}

void CAmCommandSenderCAPI::cbSinkNotification(const am_sinkID_t sinkID, const am_NotificationPayload_s& notification)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSinkNotification called, sinkID", sinkID);
    CommandInterface::am_NotificationPayload_s ciNnotif(CAmConvert2CAPIType(notification.type), notification.value);
    mService->fireSinkNotificationEvent(sinkID, ciNnotif);
}

void CAmCommandSenderCAPI::cbSourceNotification(const am_sourceID_t sourceID, const am_NotificationPayload_s& notification)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSourceNotification called, sourceID", sourceID);
    CommandInterface::am_NotificationPayload_s ciNnotif(CAmConvert2CAPIType(notification.type), notification.value);
    mService->fireSourceNotificationEvent(sourceID, ciNnotif);
}

void CAmCommandSenderCAPI::cbMainSinkNotificationConfigurationChanged(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSinkMainNotificationConfigurationChanged called, sinkID", sinkID);
    org::genivi::audiomanager::am::am_NotificationConfiguration_s ciNotifConfig(CAmConvert2CAPIType(mainNotificationConfiguration.type),
    																			CAmConvert2CAPIType(mainNotificationConfiguration.status),
																				mainNotificationConfiguration.parameter);
    mService->fireMainSinkNotificationConfigurationChangedEvent(sinkID, ciNotifConfig);
}

void CAmCommandSenderCAPI::cbMainSourceNotificationConfigurationChanged(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
	RETURN_IF_NOT_READY()
	assert((bool)mService);
    log(&ctxCommandCAPI, DLT_LOG_INFO, "cbSourceMainNotificationConfigurationChanged called, sourceID", sourceID);
    org::genivi::audiomanager::am::am_NotificationConfiguration_s ciNotifConfig(CAmConvert2CAPIType(mainNotificationConfiguration.type),
    																			CAmConvert2CAPIType(mainNotificationConfiguration.status),
																				mainNotificationConfiguration.parameter);
    mService->fireMainSourceNotificationConfigurationChangedEvent(sourceID, ciNotifConfig);
}
