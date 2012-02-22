/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger
*
* \file  
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
* THIS CODE HAS BEEN GENERATED BY ENTERPRISE ARCHITECT GENIVI MODEL. PLEASE CHANGE ONLY IN ENTERPRISE ARCHITECT AND GENERATE AGAIN
*/
#if !defined(EA_6D2D8AED_B7CC_424e_8C3F_EB10C5EBDC21__INCLUDED_)
#define EA_6D2D8AED_B7CC_424e_8C3F_EB10C5EBDC21__INCLUDED_

#include <stdint.h>
#include "projecttypes.h"
#include <string>
#include <vector>

#define AM_MUTE -3000

namespace am {
	/**
	 * This is the domain type. Each domain has a unique identifier.
	 * 
	 * \mainpage
	 * Copyright Copyright (C) 2011,2012 BMW AG
	 * 
	 * \date 21.2.2012 
	 * 
	 * \author Christian Mueller (christian.ei.mueller@bmw.de)
	 * 
	 * \par About AudioManagerInterfaces
	 * The AudioManager is a Deamon that manages all Audio Connections in a GENIVI headunit.
	 * It is a managing instance that uses so called RoutingAdaptors to control AudioDomains that then do the "real" connections.
	 * 
	 * \par More information
	 * can be found at https://collab.genivi.org/wiki/display/genivi/GENIVI+Home \n \n \n \n
	 * 
	 * \section architecture Architecture Overview
	 * 
	 * The architecture concept bases on the partition of management (logic) and routing (action). Sinks and sources are clustered into independent parts which are capable of exchanging audio with each other (AudioDomains). Between these AudioDomains, Audio can be interchanged via Gateways. \n
	 * Since the routing and the management shall be independent from the actual used system, it is realized as an OwnedComponent, the AudioManager. Each AudioDomain has a Routing Adapter which implements some necessary logic and is the interface between the AudioManager and the AudioDomains.
	 * 
	 * \section domains Audio Domains
	 * 
	 * An Audio Domain consists of sinks and sources that can exchange audio with each other. To make the most out of the concept, AudioDomains shall be chosen in such a way that they are implemented by already existing audio routing engines.
	 * 
	 * The AudioManager assumes that there are no restrictions in interconnection of sinks and sources. One or more sources can be connected to one sink and one or more sinks can be connected to one source. Since real hardware or software might end up in having restrictions, the knowledge of this must exist in the AudioManager and handled by him accordingly. This shall be accomplished via a plug-in mechanism. An AudioDomain is not tied to a hardware or software implementation. It can be software or hardware or even a combination of both. \n
	 * 
	 * Examples for possible audio domains:\n
	 * PulseAudio, Alsa, Jack, DSP, FPGA, MOST, In-chip switching matrix\n
	 * 
	 * The clustering and usage of the AudioDomains will vary from each product. Care must be taken while choosing the right AudioDomains in regards to system load (due to resampling), latency and of course flexibility.\n
	 * In special implementations of the AudioDomain, it is capable of operation a certain time without interaction to the AudioManager. This is needed to fulfill the requirements for Early & Late Audio, more information can be found below.
	 * 
	 * \section routing_adaptor Routing Adapter
	 * 
	 * Via this adapter, the interconnection from the AudioManager to the AudioDomains is accomplished. An AudioDomain shall have exactly one RoutingAdapter. In the terms of GENIVI, a RoutingAdapter is an AbstractComponent, this means that we define an API and a certain behavior in UML models but do not maintain components itself. Existing implementations from Proof of Concepts are shipped as example Adapters "as is" but cannot be seen as maintained components.\n
	 * The implementation of a routing adapter can and will vary from each project to another since the combination of sinks and sources, the used hardware etc has influence on the adapters. Besides interchanging and abstracting information between the AudioManager and the sinks and sources, the Adapters also need to implement some business logic in order to interact with the AudioManager. This includes for example the registering of components, managing the current state, error handling etc.\n
	 * In the special case of an EarlyDomain, the routing adapter also has to manage start-up and rundown including persistence for his domain while the AudioManager is not started or already stopped. During this periods of time, these special adapters have to be able to fulfill basic tasks like changing volumes, for example (this implies that the Adapter is implemented on a different piece of hardware, e.g. vehicle processor).
	 * 
	 * \section Gateway
	 * 
	 * Gateways are used to let audio flow between two domains. They always have a direction and can only transport one stream at a time. Several gateways connecting the same domains together can exist in parallel so that more than one source can be connected to more than one sink from the same domains at the same time.\n
	 * The representation of a Gateway in the domain which originates the audio is a sink. In the receiving domain, the gateway appears as a source. The AudioManager knows about the Gateways, in terms of connection, it handles it as simple sources and sinks.
	 * 
	 * \section AudioManagerDaemon
	 * 
	 * The AudioManager is the central managing instance of the Audio architecture. It is designed as an OwnedComponent, this means that the software is maintained within GENIVI as open source component. The AudioManager consists of 4 central components.\n
	 * 
	 * GOwnedComponent: AudioManager Daemon\n
	 * 
	 * This component is owned and maintained by Genivi. It is the central audio framework component. There can be only one daemon in a system (singleton).
	 * 
	 * \subsection controlinterface Control Interface Plugin
	 * 
	 * This describes the interface towards the Controlling Instances of the AudioManagerDaemon. This is the HMI and interrupt sources that use this interface to start their interrupt and stop it again. The interface shall be asynchronous. Via this interface all user interactions are handled.
	 * 
	 * \subsection routinginterface Routing Interface Plugin
	 * 
	 * This interface is used by the AudioManager to control the RoutingAdapters and communicate with them. The communication is based on two interfaces, one is provided by the AudioManager for communication from the adapters towards the AudioManager and one for the opposite direction. The design of the AudioManager shall be done in such a way that several Interfaces are supported at the same time via a plug-in mechanism. The plug-ins are (either statically - due to performance reasons or dynamically) loaded at start-up. Due to this architecture, the number of buses and routing adapters that are supported are as low as possible for each system and as high as needed without the need of changing the AudioManager itself. The AudioManager expects a bus-like structure behind each plug-in, so that a plug-in can implement a bus interface and proxy the messages to the routing adapters - the AudioManager will be capable of addressing more than one adapter one each plug-in. The interface shall is asynchronous for all timely critical commands.
	 * 
	 * \section interfaces Interfaces
	 * the calls to the interfaces of the AudioManagerDaemon are generally not threadsafe !
	 * Nevertheless if such calls from a different thread-context are needed, you may use the defered-call pattern that utilizes the mainloop (Sockethandler) to get self called in the next loop of the mainloop. For more infomation please check the audiomanger wiki page.
	 * 
	 * \section deferred The deferred call pattern
	 * Create a unix pipe or socket and add the file descriptor to the Sockethandler. Whenever a call needs to be deferred you can store the necessary information protected by a mutex in a queue and write to the socket or pipe. This will lead to a callback in the next loop of the mainloop - when getting called by the callback that was registered at the Sockethandler execute your call with the information stored away.
	 * 
	 * 
	 * \section sources_sinks Sources & Sinks
	 * \subsection Visibility
	 * Sources and sinks can either be visible or not. If they are visible, the HMI is informed about their existence and can use them. \n
	 * Invisible Sources and Sinks either are system only relevant (e.g. an audio processing that has a source and a sink) or belong to a gateway.
	 * 
	 * \subsection Availability
	 * It can be the case, that sources and sinks are present in the system but cannot be used at the moment. This is indicated via the availability. A sample use-case for this feature is CD drive that shall only be available if a CD is inserted.
	 * 
	 * \section Interrupts
	 * \subsection llinterrupts Low level interrupts
	 * \todo write low level Interrupts description
	 * 
	 * \subsection Interrupts
	 * \todo write Interrupts description
	 * 
	 * \section Persistency
	 * It is the job of the AudioManagerController to handle the persistency. It is planned to expose an interface via the ControlInterface to accomplish this but the GENIVI persistance is not ready yet. \n
	 * 
	 * 
	 * \section speed Speed dependent volume
	 * The adjustments for the speed are done product specific in the controller. The speed information itself is retrieved by the AudioManagerDaemon, sampled and quantified and forwarded to the controller.\n
	 * Turning speed controlled volume on/off and possible settings are achieved via SinkSoundProperty settings.
	 * 
	 * \section Lipsync
	 * It is the job of the AudioManager to retrieve all latency timing information from each connection, to aggregate this information and provide a latency information on a per MainConnection Basis. It is not the task of the AudioManager to actually delay or speed up video or audio signals to achieve a lipsync. The actual correction shall be done in the videoplayer with the information provided by the AudioManager.
	 * The time information is always reported by the routingadaptors for each connection. Delays that are introduced in a sink or a gateway are counting for the connection that connects to this sink or gateway.\n
	 * After the buildup of a connection the first timing information needs to be sent within 5 seconds, the timing information from the routing adaptors need to be sent via 4 seconds. If the latency for a connection is variable and changes over lifetime of the connection, the routing adaptors shall resend the value and the audiomanger will correct the over all latency.\n
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_domainID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_sourceID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_sinkID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_gatewayID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_crossfaderID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_connectionID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_mainConnectionID_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_speed_t;

	/**
	 * The unit is 0.1 db steps,The smallest value -3000 (=AM_MUTE). The minimum and maximum can be limited by actual project.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef int16_t am_volume_t;

	/**
	 * This is the volume presented on the command interface. It is in the duty of the Controller to change the volumes given here into meaningful values on the routing interface.
	 * The range of this type is customer specific.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef int16_t am_mainVolume_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_sourceClass_t;

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_sinkClass_t;

	/**
	 * time in ms!
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef uint16_t am_time_t;

	/**
	 * offset time that is introduced in milli seconds.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	typedef int16_t am_timeSync_t;

	/**
	 * with the help of this enum, sinks and sources can report their availability state
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_Availablility_e
	{
		/**
		 * default
		 */
		A_UNKNOWN = 0,
		/**
		 * The source / sink is available
		 */
		A_AVAILABLE = 1,
		/**
		 * the source / sink is not available
		 */
		A_UNAVAILABLE = 2,
		A_MAX
	};

	/**
	 * represents the connection state
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_ConnectionState_e
	{
		CS_UNKNOWN = 0,
		/**
		 * This means the connection is just building up
		 */
		CS_CONNECTING = 1,
		/**
		 * the connection is ready to be used
		 */
		CS_CONNECTED = 2,
		/**
		 * the connection is in the course to be knocked down
		 */
		CS_DISCONNECTING = 3,
		/**
		 * only relevant for connectionStatechanged. Is send after the connection was removed
		 */
		CS_DISCONNECTED = 4,
		/**
		 * this means the connection is still build up but unused at the moment
		 */
		CS_SUSPENDED = 5,
		CS_MAX
	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_DomainState_e
	{
		/**
		 * default
		 */
		DS_UNKNOWN = 0,
		/**
		 * the domain is controlled by the daemon
		 */
		DS_CONTROLLED = 1,
		/**
		 * the domain is independent starting up
		 */
		DS_INDEPENDENT_STARTUP = 1,
		/**
		 * the domain is independent running down
		 */
		DS_INDEPENDENT_RUNDOWN = 2,
		DS_MAX
	};

	/**
	 * This enum characterizes the data of the EarlyData_t
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_EarlyDataType_e
	{
		/**
		 * default
		 */
		ES_UNKNOWN = 0,
		/**
		 * the source volume
		 */
		ED_SOURCE_VOLUME = 1,
		/**
		 * the sink volume
		 */
		ED_SINK_VOLUME = 2,
		/**
		 * a source property
		 */
		ED_SOURCE_PROPERTY = 3,
		/**
		 * a sink property
		 */
		ED_SINK_PROPERTY = 4,
		ED_MAX
	};

	/**
	 * the errors of the audiomanager. All possible errors are in here. This enum is used widely as return parameter.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_Error_e
	{
		/**
		 * default
		 */
		E_UNKNOWN = 0,
		/**
		 * no error - positive reply
		 */
		E_OK = 1,
		/**
		 * value out of range
		 */
		E_OUT_OF_RANGE = 2,
		/**
		 * not used
		 */
		E_NOT_USED = 3,
		/**
		 * a database error occurred 
		 */
		E_DATABASE_ERROR = 4,
		/**
		 * the desired object already exists
		 */
		E_ALREADY_EXISTS = 5,
		/**
		 * there is no change
		 */
		E_NO_CHANGE = 6,
		/**
		 * the desired action is not possible
		 */
		E_NOT_POSSIBLE = 7,
		/**
		 * the desired object is non existent
		 */
		E_NON_EXISTENT = 8,
		/**
		 * the asynchronous action was aborted
		 */
		E_ABORTED = 9,
		/**
		 * This error is returned in case a connect is issued with a connectionFormat that cannot be selected for the connection. This could be either due to the capabilities of a source or a sink or gateway compatibilities for example
		 */
		E_WRONG_FORMAT = 10,
		E_MAX
	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_MuteState_e
	{
		/**
		 * default
		 */
		MS_UNKNOWN = 0,
		/**
		 * the source / sink is muted
		 */
		MS_MUTED = 1,
		/**
		 * the source / sink is unmuted
		 */
		MS_UNMUTED = 2,
		MS_MAX
	};

	/**
	 * The source state reflects the state of the source
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_SourceState_e
	{
		SS_UNKNNOWN = 0,
		/**
		 * The source can be activly heared
		 */
		SS_ON = 1,
		/**
		 * The source cannot be heared
		 */
		SS_OFF = 2,
		/**
		 * The source is paused. Meaning it cannot be heared but should be prepared to play again soon.
		 */
		SS_PAUSED = 3,
		SS_MAX
	};

	/**
	 * This enumeration is used to define the type of the action that is correlated to a handle.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_Handle_e
	{
		H_UNKNOWN = 0,
		H_CONNECT = 1,
		H_DISCONNECT = 2,
		H_SETSOURCESTATE = 3,
		H_SETSINKVOLUME = 4,
		H_SETSOURCEVOLUME = 5,
		H_SETSINKSOUNDPROPERTY = 6,
		H_SETSOURCESOUNDPROPERTY = 7,
		H_SETSINKSOUNDPROPERTIES = 8,
		H_SETSOURCESOUNDPROPERTIES = 9,
		H_CROSSFADE = 10,
		H_MAX
	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:31 PM
	 */
	enum am_InterruptState_e
	{
		/**
		 * default
		 */
		IS_UNKNOWN = 0,
		/**
		 * the interrupt state is off - no interrupt 
		 */
		IS_OFF = 1,
		/**
		 * the interrupt state is interrupted - the interrupt is active
		 */
		IS_INTERRUPTED = 2,
		IS_MAX
	};

	/**
	 * describes the active sink of a crossfader.
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	enum am_HotSink_e
	{
		/**
		 * default
		 */
		HS_UNKNOWN = 0,
		/**
		 * sinkA is active
		 */
		HS_SINKA = 1,
		/**
		 * sinkB is active
		 */
		HS_SINKB = 2,
		/**
		 * the crossfader is in the transition state
		 */
		HS_INTERMEDIATE = 3,
		HS_MAX
	};

	/**
	 * this describes the availability of a sink or a source together with the latest change
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_Availability_s
	{

	public:
		/**
		 * the current availability state
		 */
		am_Availablility_e availability;
		/**
		 * the reason for the last change. This can be used to trigger events that deal with state changes.
		 */
		am_AvailabilityReason_e availabilityReason;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_ClassProperty_s
	{

	public:
		am_ClassProperty_e classProperty;
		int16_t value;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_Crossfader_s
	{

	public:
		am_crossfaderID_t crossfaderID;
		std::string name;
		am_sinkID_t sinkID_A;
		am_sinkID_t sinkID_B;
		am_sourceID_t sourceID;
		am_HotSink_e hotSink;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_Gateway_s
	{

	public:
		am_gatewayID_t gatewayID;
		std::string name;
		am_sinkID_t sinkID;
		am_sourceID_t sourceID;
		am_domainID_t domainSinkID;
		am_domainID_t domainSourceID;
		am_domainID_t controlDomainID;
		std::vector<am_ConnectionFormat_e> listSourceFormats;
		std::vector<am_ConnectionFormat_e> listSinkFormats;
		/**
		 * This is matrix holding information about the conversion capability of the gateway, it's length is defined by the length(listSinkFormats) x length(listSourceFormats).
		 * If a SinkFormat can be converted into a SourceFormat, the vector will hold a 1, if no conversion is possible, a 0.
		 * The data is stored row orientated, where the rows are related to the sinksFormats and the columns to the sourceFormats. The first value will hold the conversion information from the first sourceFormat to the first sinkFormat for example and the seventh value the information about the 3rd sinkFormat to the 1st sourceFormat in case we would have 3 sourceFormats.
		 * 
		 * This matrix 
		 * 110 011 000 111 001
		 * 
		 * reads as this:
		 *           Source
		 * 	**  1  2  3 
		 * *********************
		 * S	1*  1  1  0 
		 * i	2*  0  1  1
		 * n	3*  0  0  0
		 * k	4*  1  1  1
		 * 	5*  0  0  1
		 */
		std::vector<bool> convertionMatrix;

	};

	/**
	 * This represents one "hopp" in a route
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_RoutingElement_s
	{

	public:
		am_sourceID_t sourceID;
		am_sinkID_t sinkID;
		am_domainID_t domainID;
		am_ConnectionFormat_e connectionFormat;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_Route_s
	{

	public:
		am_sourceID_t sourceID;
		am_sinkID_t sinkID;
		std::vector<am_RoutingElement_s> route;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:32 PM
	 */
	struct am_SoundProperty_s
	{

	public:
		am_SoundPropertyType_e type;
		int16_t value;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_SystemProperty_s
	{

	public:
		/**
		 * the type that is set
		 */
		am_SystemPropertyType_e type;
		/**
		 * the value
		 */
		int16_t value;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_SinkClass_s
	{

	public:
		am_sinkClass_t sinkClassID;
		std::string name;
		std::vector<am_ClassProperty_s> listClassProperties;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_SourceClass_s
	{

	public:
		/**
		 * the source ID
		 */
		am_sourceClass_t sourceClassID;
		std::string name;
		std::vector<am_ClassProperty_s> listClassProperties;

	};

	/**
	 * this type holds all information of sources relevant to the HMI
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_SourceType_s
	{

	public:
		am_sourceID_t sourceID;
		std::string name;
		am_Availability_s availability;
		am_sourceClass_t sourceClassID;

	};

	/**
	 * this type holds all information of sinks relevant to the HMI
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_SinkType_s
	{

	public:
		am_sinkID_t sinkID;
		std::string name;
		am_Availability_s availability;
		am_mainVolume_t volume;
		am_MuteState_e muteState;
		am_sinkClass_t sinkClassID;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:33 PM
	 */
	struct am_Handle_s
	{

	public:
		am_Handle_e handleType:4;
		uint16_t handle:12;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_MainSoundProperty_s
	{

	public:
		am_MainSoundPropertyType_e type;
		int16_t value;

	};

	/**
	 * this type holds all information of connections relevant to the HMI
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_MainConnectionType_s
	{

	public:
		am_mainConnectionID_t mainConnectionID;
		am_sourceID_t sourceID;
		am_sinkID_t sinkID;
		am_timeSync_t delay;
		am_ConnectionState_e connectionState;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_MainConnection_s
	{

	public:
		am_mainConnectionID_t mainConnectionID;
		am_ConnectionState_e connectionState;
		/**
		 * the sinkID
		 */
		am_sinkID_t sinkID;
		/**
		 * the sourceID
		 */
		am_sourceID_t sourceID;
		am_timeSync_t delay;
		std::vector<am_connectionID_t> listConnectionID;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_Sink_s
	{

	public:
		am_sinkID_t sinkID;
		std::string name;
		am_domainID_t domainID;
		am_sinkClass_t sinkClassID;
		am_volume_t volume;
		bool visible;
		am_Availability_s available;
		am_MuteState_e muteState;
		am_mainVolume_t mainVolume;
		std::vector<am_SoundProperty_s> listSoundProperties;
		std::vector<am_ConnectionFormat_e> listConnectionFormats;
		std::vector<am_MainSoundProperty_s> listMainSoundProperties;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_Source_s
	{

	public:
		am_sourceID_t sourceID;
		am_domainID_t domainID;
		std::string name;
		am_sourceClass_t sourceClassID;
		am_SourceState_e sourceState;
		am_volume_t volume;
		bool visible;
		am_Availability_s available;
		am_InterruptState_e interruptState;
		/**
		 * This list holds all soundProperties of the source
		 */
		std::vector<am_SoundProperty_s> listSoundProperties;
		/**
		 * list of the supported ConnectionFormats
		 */
		std::vector<am_ConnectionFormat_e> listConnectionFormats;
		/**
		 * This list holds all MainSoundProperties of the source (all the ones that can be set via the HMI)
		 */
		std::vector<am_MainSoundProperty_s> listMainSoundProperties;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_Domain_s
	{

	public:
		am_domainID_t domainID;
		std::string name;
		std::string busname;
		std::string nodename;
		bool early;
		bool complete;
		am_DomainState_e state;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:34 PM
	 */
	struct am_Connection_s
	{

	public:
		am_connectionID_t connectionID;
		am_sourceID_t sourceID;
		am_sinkID_t sinkID;
		am_timeSync_t delay;
		am_ConnectionFormat_e connectionFormat;

	};

	/**
	 * data type depends of am_EarlyDataType_e:
	 * volume_t in case of ED_SOURCE_VOLUME, ED_SINK_VOLUME 
	 * soundProperty_t in case of ED_SOURCE_PROPERTY, ED_SINK_PROPERTY 
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:35 PM
	 */
	union am_EarlyData_u
	{

	public:
		am_volume_t volume;
		am_SoundProperty_s soundProperty;

	};

	/**
	 * data type depends of am_EarlyDataType_e:
	 * sourceID in case of ED_SOURCE_VOLUME, ED_SOURCE_PROPERTY
	 * sinkID in case of ED_SINK_VOLUME, ED_SINK_PROPERTY 
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:35 PM
	 */
	union am_DataType_u
	{

	public:
		am_sinkID_t sink;
		am_sourceID_t source;

	};

	/**
	 * @author Christian Mueller
	 * @created 21-Feb-2012 4:58:35 PM
	 */
	struct am_EarlyData_s
	{

	public:
		am_EarlyDataType_e type;
		am_DataType_u sinksource;
		am_EarlyData_u data;

	};
}
#endif // !defined(EA_6D2D8AED_B7CC_424e_8C3F_EB10C5EBDC21__INCLUDED_)
