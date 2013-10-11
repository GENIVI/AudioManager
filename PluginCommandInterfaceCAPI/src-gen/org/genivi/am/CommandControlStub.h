/*
* This file was generated by the CommonAPI Generators. 
* Used org.genivi.commonapi.core 2.1.2.201309301424.
* Used org.franca.core 0.8.9.201308271211.
*
*  Copyright (c) 2012 BMW
*  
*   \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
*  
*   \copyright
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
*   including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
*   subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*   THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*    For further information see http://www.genivi.org/.
*  
*/
/**
 * The interface towards the Controlling Instance (e.g HMI). It handles the
 *  communication towards the HMI and other system components who need to interact
 *  with the audiomanagement.
There are two rules that have to be kept in mind
 *  when implementing against this interface:
 * @author Christian Mueller
 */
#ifndef ORG_GENIVI_AM_Command_Control_STUB_H_
#define ORG_GENIVI_AM_Command_Control_STUB_H_



#include <org/genivi/am.h>

#include "CommandControl.h"

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include <CommonAPI/InputStream.h>
#include <CommonAPI/OutputStream.h>
#include <CommonAPI/SerializableStruct.h>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include <CommonAPI/Stub.h>

#undef COMMONAPI_INTERNAL_COMPILATION

namespace org {
namespace genivi {
namespace am {

/**
 * Receives messages from remote and handles all dispatching of deserialized calls
 * to a stub for the service CommandControl. Also provides means to send broadcasts
 * and attribute-changed-notifications of observable attributes as defined by this service.
 * An application developer should not need to bother with this class.
 */
class CommandControlStubAdapter: virtual public CommonAPI::StubAdapter, public CommandControl {
 public:

    /**
     * Sends a broadcast event for newMainConnection. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireNewMainConnectionEvent(const am_MainConnectionType_s& mainConnection) = 0;
    /**
     * Sends a broadcast event for removedMainConnection. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireRemovedMainConnectionEvent(const am_mainConnectionID_t& mainConnection) = 0;
    /**
     * Sends a broadcast event for newSink. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireNewSinkEvent(const am_SinkType_s& sink) = 0;
    /**
     * Sends a broadcast event for removedSink. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireRemovedSinkEvent(const am_sinkID_t& sinkID) = 0;
    /**
     * Sends a broadcast event for newSource. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireNewSourceEvent(const am_SourceType_s& source) = 0;
    /**
     * Sends a broadcast event for removedSource. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireRemovedSourceEvent(const am_sourceID_t& source) = 0;
    /**
     * Sends a broadcast event for numberOfSinkClassesChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireNumberOfSinkClassesChangedEvent() = 0;
    /**
     * Sends a broadcast event for numberOfSourceClassesChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireNumberOfSourceClassesChangedEvent() = 0;
    /**
     * Sends a broadcast event for mainConnectionStateChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireMainConnectionStateChangedEvent(const am_mainConnectionID_t& connectionID, const am_ConnectionState_e& connectionState) = 0;
    /**
     * Sends a broadcast event for mainSinkSoundPropertyChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireMainSinkSoundPropertyChangedEvent(const am_sinkID_t& sinkID, const am_MainSoundProperty_s& soundProperty) = 0;
    /**
     * Sends a broadcast event for mainSourceSoundPropertyChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireMainSourceSoundPropertyChangedEvent(const am_sourceID_t& sourceID, const am_MainSoundProperty_s& soundProperty) = 0;
    /**
     * Sends a broadcast event for sinkAvailabilityChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSinkAvailabilityChangedEvent(const am_sinkID_t& sinkID, const am_Availability_s& availability) = 0;
    /**
     * Sends a broadcast event for sourceAvailabilityChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSourceAvailabilityChangedEvent(const am_sourceID_t& sourceID, const am_Availability_s& availability) = 0;
    /**
     * Sends a broadcast event for volumeChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireVolumeChangedEvent(const am_sinkID_t& sinkID, const am_mainVolume_t& volume) = 0;
    /**
     * Sends a broadcast event for sinkMuteStateChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSinkMuteStateChangedEvent(const am_sinkID_t& sinkID, const am_MuteState_e& muteState) = 0;
    /**
     * Sends a broadcast event for systemPropertyChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSystemPropertyChangedEvent(const am_SystemProperty_s& systemProperty) = 0;
    /**
     * Sends a broadcast event for timingInformationChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireTimingInformationChangedEvent(const am_mainConnectionID_t& mainConnectionID, const am_timeSync_t& time) = 0;
    /**
     * Sends a broadcast event for sinkUpdated. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSinkUpdatedEvent(const am_sinkID_t& sinkID, const am_sinkClass_t& sinkClassID, const am_MainSoundProperty_L& listMainSoundProperties) = 0;
    /**
     * Sends a broadcast event for sourceUpdated. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSourceUpdatedEvent(const am_sourceID_t& sourceID, const am_sourceClass_t& sourceClassID, const am_MainSoundProperty_L& listMainSoundProperties) = 0;
    /**
     * Sends a broadcast event for sinkNotification. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSinkNotificationEvent(const am_sinkID_t& sinkID, const am_NotificationPayload_s& notification) = 0;
    /**
     * Sends a broadcast event for sourceNotification. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireSourceNotificationEvent(const am_sourceID_t& sourceID, const am_NotificationPayload_s& notification) = 0;
    /**
     * Sends a broadcast event for mainSinkNotificationConfigurationChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireMainSinkNotificationConfigurationChangedEvent(const am_sinkID_t& sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration) = 0;
    /**
     * Sends a broadcast event for mainSourceNotificationConfigurationChanged. Should not be called directly.
     * Instead, the "fire<broadcastName>Event" methods of the stub should be used.
     */
    virtual void fireMainSourceNotificationConfigurationChangedEvent(const am_sourceID_t& sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration) = 0;
    
    
    virtual void deactivateManagedInstances() = 0;
    
protected:
    /**
     * Defines properties for storing the ClientIds of clients / proxies that have
     * subscribed to the selective broadcasts
     */
};


/**
 * Defines the necessary callbacks to handle remote set events related to the attributes
 * defined in the IDL description for CommandControl.
 * For each attribute two callbacks are defined:
 * - a verification callback that allows to verify the requested value and to prevent setting
 *   e.g. an invalid value ("onRemoteSet<AttributeName>").
 * - an action callback to do local work after the attribute value has been changed
 *   ("onRemote<AttributeName>Changed").
 *
 * This class and the one below are the ones an application developer needs to have
 * a look at if he wants to implement a service.
 */
class CommandControlStubRemoteEvent {
 public:
    virtual ~CommandControlStubRemoteEvent() { }

};


/**
 * Defines the interface that must be implemented by any class that should provide
 * the service CommandControl to remote clients.
 * This class and the one above are the ones an application developer needs to have
 * a look at if he wants to implement a service.
 */
class CommandControlStub : public CommonAPI::Stub<CommandControlStubAdapter , CommandControlStubRemoteEvent> {
 public:
    virtual ~CommandControlStub() { }


    /**
     * connects a source to sink
    (at)return E_OK on success, E_NOT_POSSIBLE on
     *  failure, E_ALREADY_EXISTS if the connection does already exists
     */
    /// This is the method that will be called on remote calls on the method connect.
    virtual void connect(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sourceID_t sourceID, am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID, am_Error_e& error) = 0;
    /**
     * disconnects a mainConnection
    (at)return E_OK on successes, E_NON_EXISTENT if
     *  the connection does not exist, E_NOT_POSSIBLE on error.
     */
    /// This is the method that will be called on remote calls on the method disconnect.
    virtual void disconnect(const std::shared_ptr<CommonAPI::ClientId> clientId, am_mainConnectionID_t mainConnectionID, am_Error_e& error) = 0;
    /**
     * sets the volume for a sink
    (at)return E_OK on success, E_UNKOWN on error,
     *  E_OUT_OF_RANGE in case the value is out of range
     */
    /// This is the method that will be called on remote calls on the method setVolume.
    virtual void setVolume(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_mainVolume_t volume, am_Error_e& error) = 0;
    /**
     * This function is used to increment or decrement the current volume for a
     *  sink.
    (at)return E_OK on success, E_UNKNOWN on error and E_OUT_OF_RANGE if
     *  the value is not in the given volume range.
     */
    /// This is the method that will be called on remote calls on the method volumeStep.
    virtual void volumeStep(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, int16_t volumeStep_, am_Error_e& error) = 0;
    /**
     * sets the mute state of a sink
    (at)return E_OK on success, E_UNKNOWN on error.
     *  If the mute state is already the desired one, the Daemon will return E_OK.
     */
    /// This is the method that will be called on remote calls on the method setSinkMuteState.
    virtual void setSinkMuteState(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_MuteState_e muteState, am_Error_e& error) = 0;
    /**
     * This method is used to set sound properties, e.g. Equalizer Values. Since the
     *  capabilities of the system can differ, the exact key value pairs can be
     *  extended in each product
    (at)return E_OK on success, E_OUT_OF_RANGE if value
     *  exceeds range, E_UNKNOWN in case of an error
     */
    /// This is the method that will be called on remote calls on the method setMainSinkSoundProperty.
    virtual void setMainSinkSoundProperty(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_MainSoundProperty_s soundProperty, am_Error_e& error) = 0;
    /**
     * This method is used to set sound properties, e.g. Equalizer Values. Since the
     *  capabilities of the system can differ, the exact key value pairs can be
     *  extended in each product
    (at)return E_OK on success, E_OUT_OF_RANGE if value
     *  exceeds range, E_UNKNOWN in case of an error
     */
    /// This is the method that will be called on remote calls on the method setMainSourceSoundProperty.
    virtual void setMainSourceSoundProperty(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sourceID_t sourceID, am_MainSoundProperty_s soundProperty, am_Error_e& error) = 0;
    /**
     * is used to set a specific system property.
    (at)return E_OK on success,
     *  E_OUT_OF_RANGE if value exceeds range, E_UNKNOWN in case of an error
     */
    /// This is the method that will be called on remote calls on the method setSystemProperty.
    virtual void setSystemProperty(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SystemProperty_s property, am_Error_e& error) = 0;
    /**
     * returns the actual list of MainConnections
    (at)return E_OK on success,
     *  E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListMainConnections.
    virtual void getListMainConnections(const std::shared_ptr<CommonAPI::ClientId> clientId, am_MainConnection_L& listConnections, am_Error_e& error) = 0;
    /**
     * returns the actual list of Sinks
    (at)return E_OK on success, E_DATABASE_ERROR
     *  on error
     */
    /// This is the method that will be called on remote calls on the method getListMainSinks.
    virtual void getListMainSinks(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SinkType_L& listMainSinks, am_Error_e& error) = 0;
    /**
     * returns the actual list of Sources
    (at)return E_OK on success,
     *  E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListMainSources.
    virtual void getListMainSources(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SourceType_L& listMainSources, am_Error_e& error) = 0;
    /**
     * This is used to retrieve all source sound properties related to a source.
     *  Returns a vector of the sound properties and values as pair
    (at)return E_OK
     *  on success, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListMainSinkSoundProperties.
    virtual void getListMainSinkSoundProperties(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_MainSoundProperty_L& listSoundProperties, am_Error_e& error) = 0;
    /**
     * This is used to retrieve all source sound properties related to a
     *  source.
    (at)return E_OK on success, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListMainSourceSoundProperties.
    virtual void getListMainSourceSoundProperties(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sourceID_t sourceID, am_MainSoundProperty_L& listSourceProperties, am_Error_e& error) = 0;
    /**
     * This is used to retrieve SourceClass Information of all source classes
     *  
    (at)return E_OK on success, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListSourceClasses.
    virtual void getListSourceClasses(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SourceClass_L& listSourceClasses, am_Error_e& error) = 0;
    /**
     * This is used to retrieve SinkClass Information of all sink classes 
    (at)return
     *  E_OK on success, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListSinkClasses.
    virtual void getListSinkClasses(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SinkClass_L& listSinkClasses, am_Error_e& error) = 0;
    /**
     * Retrieves a complete list of all systemProperties.
    (at)return E_OK on success,
     *  E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method getListSystemProperties.
    virtual void getListSystemProperties(const std::shared_ptr<CommonAPI::ClientId> clientId, am_SystemProperty_L& listSystemProperties, am_Error_e& error) = 0;
    /**
     * returns the delay in ms that the audiopath for the given mainConnection
     *  has
    (at)return E_OK on success, E_NOT_POSSIBLE if timing information is not
     *  yet retrieved, E_DATABASE_ERROR on read error on the database
     */
    /// This is the method that will be called on remote calls on the method getTimingInformation.
    virtual void getTimingInformation(const std::shared_ptr<CommonAPI::ClientId> clientId, am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay, am_Error_e& error) = 0;
    /**
     * Retrieves the list of MainNotifications for a sink. Does not return the
     *  possible ones.
     */
    /// This is the method that will be called on remote calls on the method getListMainSinkNotificationConfigurations.
    virtual void getListMainSinkNotificationConfigurations(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_NotificationConfiguration_L& listMainNotificationConfigurations, am_Error_e& error) = 0;
    /**
     * Retrieves the list of MainNotifications for a source. Does not return the
     *  possible ones.
     */
    /// This is the method that will be called on remote calls on the method getListMainSourceNotificationConfigurations.
    virtual void getListMainSourceNotificationConfigurations(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sourceID_t sourceID, am_NotificationConfiguration_L& listMainNotificationConfigurations, am_Error_e& error) = 0;
    /**
     * sets a MainNotificationConfiuration. This can be used to turn on an off
     *  notifications an to change the mode of the configuration.
    (at)return E_OK on
     *  success, E_NON_EXISTENT if sinkID does not exists, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method setMainSinkNotificationConfiguration.
    virtual void setMainSinkNotificationConfiguration(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sinkID_t sinkID, am_NotificationConfiguration_s mainNotificationConfiguration, am_Error_e& error) = 0;
    /**
     * sets a MainNotificationConfiuration. This can be used to turn on an off
     *  notifications an to change the mode of the configuration.
    (at)return E_OK on
     *  success, E_NON_EXISTENT if sourceID does not exists, E_DATABASE_ERROR on error
     */
    /// This is the method that will be called on remote calls on the method setMainSourceNotificationConfiguration.
    virtual void setMainSourceNotificationConfiguration(const std::shared_ptr<CommonAPI::ClientId> clientId, am_sourceID_t sourceID, am_NotificationConfiguration_s mainNotificationConfiguration, am_Error_e& error) = 0;
    /**
     * Callback that is called when the number of connections change
     */
    /// Sends a broadcast event for newMainConnection.
    virtual void fireNewMainConnectionEvent(const am_MainConnectionType_s& mainConnection) = 0;
    /**
     * Callback that is called when the number of connections change
     */
    /// Sends a broadcast event for removedMainConnection.
    virtual void fireRemovedMainConnectionEvent(const am_mainConnectionID_t& mainConnection) = 0;
    /**
     * Callback that is called when the number of sinks change
     */
    /// Sends a broadcast event for newSink.
    virtual void fireNewSinkEvent(const am_SinkType_s& sink) = 0;
    /**
     * Callback that is called when the number of sinks change
     */
    /// Sends a broadcast event for removedSink.
    virtual void fireRemovedSinkEvent(const am_sinkID_t& sinkID) = 0;
    /**
     * Callback that is called when the number of sources change
     */
    /// Sends a broadcast event for newSource.
    virtual void fireNewSourceEvent(const am_SourceType_s& source) = 0;
    /**
     * Callback that is called when the number of sources change
     */
    /// Sends a broadcast event for removedSource.
    virtual void fireRemovedSourceEvent(const am_sourceID_t& source) = 0;
    /**
     * this callback is fired if the number of sink classes changed
     */
    /// Sends a broadcast event for numberOfSinkClassesChanged.
    virtual void fireNumberOfSinkClassesChangedEvent() = 0;
    /**
     * this callback is fired if the number of source classes changed
     */
    /// Sends a broadcast event for numberOfSourceClassesChanged.
    virtual void fireNumberOfSourceClassesChangedEvent() = 0;
    /**
     * This callback is called when the ConnectionState of a connection changed.
     */
    /// Sends a broadcast event for mainConnectionStateChanged.
    virtual void fireMainConnectionStateChangedEvent(const am_mainConnectionID_t& connectionID, const am_ConnectionState_e& connectionState) = 0;
    /**
     * this callback indicates that a sinkSoundProperty has changed.
     */
    /// Sends a broadcast event for mainSinkSoundPropertyChanged.
    virtual void fireMainSinkSoundPropertyChangedEvent(const am_sinkID_t& sinkID, const am_MainSoundProperty_s& soundProperty) = 0;
    /**
     * this callback indicates that a sourceSoundProperty has changed.
     */
    /// Sends a broadcast event for mainSourceSoundPropertyChanged.
    virtual void fireMainSourceSoundPropertyChangedEvent(const am_sourceID_t& sourceID, const am_MainSoundProperty_s& soundProperty) = 0;
    /**
     * this callback is called when the availability of a sink has changed
     */
    /// Sends a broadcast event for sinkAvailabilityChanged.
    virtual void fireSinkAvailabilityChangedEvent(const am_sinkID_t& sinkID, const am_Availability_s& availability) = 0;
    /**
     * this callback is called when the availability of source has changed.
     */
    /// Sends a broadcast event for sourceAvailabilityChanged.
    virtual void fireSourceAvailabilityChangedEvent(const am_sourceID_t& sourceID, const am_Availability_s& availability) = 0;
    /**
     * this callback indicates a volume change on the indicated sink
     */
    /// Sends a broadcast event for volumeChanged.
    virtual void fireVolumeChangedEvent(const am_sinkID_t& sinkID, const am_mainVolume_t& volume) = 0;
    /**
     * this callback indicates a mute state change on a sink.
     */
    /// Sends a broadcast event for sinkMuteStateChanged.
    virtual void fireSinkMuteStateChangedEvent(const am_sinkID_t& sinkID, const am_MuteState_e& muteState) = 0;
    /**
     * is fired if a systemProperty changed
     */
    /// Sends a broadcast event for systemPropertyChanged.
    virtual void fireSystemPropertyChangedEvent(const am_SystemProperty_s& systemProperty) = 0;
    /**
     * This callback is fired if the timinginformation for a mainConnectionID changed
     */
    /// Sends a broadcast event for timingInformationChanged.
    virtual void fireTimingInformationChangedEvent(const am_mainConnectionID_t& mainConnectionID, const am_timeSync_t& time) = 0;
    /**
     * This callback is called when a sink is updated.
     */
    /// Sends a broadcast event for sinkUpdated.
    virtual void fireSinkUpdatedEvent(const am_sinkID_t& sinkID, const am_sinkClass_t& sinkClassID, const am_MainSoundProperty_L& listMainSoundProperties) = 0;
    /**
     * This callback is called when a source is updated.
     */
    /// Sends a broadcast event for sourceUpdated.
    virtual void fireSourceUpdatedEvent(const am_sourceID_t& sourceID, const am_sourceClass_t& sourceClassID, const am_MainSoundProperty_L& listMainSoundProperties) = 0;
    /**
     * This callback is called when a notificated value of a sink changes.
     */
    /// Sends a broadcast event for sinkNotification.
    virtual void fireSinkNotificationEvent(const am_sinkID_t& sinkID, const am_NotificationPayload_s& notification) = 0;
    /**
     * This callback is called when a notifcated value of a source changes.
     */
    /// Sends a broadcast event for sourceNotification.
    virtual void fireSourceNotificationEvent(const am_sourceID_t& sourceID, const am_NotificationPayload_s& notification) = 0;
    /**
     * This callback is triggered when a mainNotificationConfiguration is changed.
     */
    /// Sends a broadcast event for mainSinkNotificationConfigurationChanged.
    virtual void fireMainSinkNotificationConfigurationChangedEvent(const am_sinkID_t& sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration) = 0;
    /**
     * This callback is triggered when a mainNotificationConfiguration is changed.
     */
    /// Sends a broadcast event for mainSourceNotificationConfigurationChanged.
    virtual void fireMainSourceNotificationConfigurationChangedEvent(const am_sourceID_t& sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration) = 0;
    
};

} // namespace am
} // namespace genivi
} // namespace org

#endif // ORG_GENIVI_AM_Command_Control_STUB_H_
