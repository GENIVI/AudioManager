/**
 * \mainpage
 *
 * \image html genivilogo.png
 *
 * Copyright (C) 2012, GENIVI Alliance, Inc.
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
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \par More information
 * can be found at https://collab.genivi.org/wiki/display/genivi/GENIVI+Home \n
 *
 * \par About AudioManager
 * The AudioManager is a Deamon that manages all Audio Connections in a GENIVI headunit.
 * It is a managing instance that uses so called RoutingAdaptors to control AudioDomains that then do the "real" connections.
 *
 */

/**
 * \page lic License
 * \section contr Code Contribution License
 * The contribution is done under GENIVI CLA, please see here:
 * https://collab.genivi.org/wiki/display/genivi/Code+Contribution+Team
 * \section split License Split
 * The licenses of this project are split into two parts:\n
 * 1. the AudioManagerDaemon, licensed under MPL 2.0\n
 * 2. all other parts that serve as example code that can be taken to build up an own project with it these parts are licensed
 * \section mpl Mozilla Public License, v. 2.0
 * http://mozilla.org/MPL/2.0/
 * \section mit MIT license
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \page architecturepage Architecture Overview
 *
 * The architecture concept bases on the partition of management (logic) and routing (action). Sinks and sources are clustered
 * into independent parts which are capable of exchanging audio with each other (AudioDomains). Between these AudioDomains,
 * Audio can be interchanged via Gateways. \n
 * Since the routing and the management shall be independent from the actual used system, it is realized as an OwnedComponent,
 * the AudioManager. Each AudioDomain has a Routing Adapter which implements some necessary logic and is the interface between
 * the AudioManager and the AudioDomains.
 *
 * \section domains Audio Domains
 *
 * \image html AudioDomains.gif
 * An Audio Domain consists of sinks and sources that can exchange audio with each other. To make the most out of the concept,
 * AudioDomains shall be chosen in such a way that they are implemented by already existing audio routing engines.
 *
 * The AudioManager assumes that there are no restrictions in interconnection of sinks and sources. One or more sources can be
 * connected to one sink and one or more sinks can be connected to one source. Since real hardware or software might end up in
 * having restrictions, the knowledge of this must exist in the AudioManager and handled by him accordingly. This shall be
 * accomplished via a plug-in mechanism. An AudioDomain is not tied to a hardware or software implementation. It can be software
 * or hardware or even a combination of both. \n
 *
 * Examples for possible audio domains:\n
 * PulseAudio, Alsa, Jack, DSP, FPGA, MOST, In-chip switching matrix\n
 *
 * The clustering and usage of the AudioDomains will vary from each product. Care must be taken while choosing the right AudioDomains
 * in regards to system load (due to resampling), latency and of course flexibility.\n
 * In special implementations of the AudioDomain, it is capable of operation a certain time without interaction to the AudioManager.
 * This is needed to fulfill the requirements for Early & Late Audio, more information can be found below.
 * am::am_Domain_s describe the attribiutes of a domain.
 *
 * \section routing_adaptor Routing Adapter
 *
 * Via this adapter, the interconnection from the AudioManager to the AudioDomains is accomplished. An AudioDomain shall have exactly
 * one RoutingAdapter. In the terms of GENIVI, a RoutingAdapter is an AbstractComponent, this means that we define an API and a certain
 * behavior in UML models but do not maintain components itself. Existing implementations from Proof of Concepts are shipped as example
 * Adapters "as is" but cannot be seen as maintained components.\n
 * The implementation of a routing adapter can and will vary from each project to another since the combination of sinks and sources,
 * the used hardware etc has influence on the adapters. Besides interchanging and abstracting information between the AudioManager and
 * the sinks and sources, the Adapters also need to implement some business logic in order to interact with the AudioManager.
 * This include for example the registering of components, managing the current state, error handling etc.\n
 * In the special case of an EarlyDomain, the routing adapter also has to manage start-up and rundown including persistence for his
 * domain while the AudioManager is not started or already stopped. During this periods of time, these special adapters have to be able
 * to fulfill basic tasks like changing volumes, for example (this implies that the Adapter is implemented on a different piece of
 * hardware, e.g. vehicle processor).
 *
 * \section gateway Gateway
 *
 * \image html Gateway.gif
 *
 * Gateways are used to let audio flow between two domains. They always have a direction and can only transport one stream at a time.
 * Several gateways connecting the same domains together can exist in parallel so that more than one source can be connected to more
 * than one sink from the same domains at the same time.\n
 * In principle, gateways have the ability to convert the connectionFormat of an audiostream, for example the sink could receive audio
 * in a digital form and output it as analog (sound card). In order to express the conversion capabilities of a gateway, a matrix of
 * all source/sink connectionFormats is given (details below). The sources and sinks of a gateway are registered like ordinary sources
 * and sinks where the domains have the responsibility to register "their" sinks and sources.\n
 * For every gateway, a controlDomain is defined, this is the domain that registered the gateway. At the time of registering, the ID of
 * the "other end" of the gateway might be unknown. To handle this situation, a domain can "peek" Domains, Sources and Sinks. When
 * something is peeked, it means that an ID is reserved for a unique name without registering it.\n
 * If a gateway is deregistered, the source or sink of the controlling domain is deregistered as well - not the one in the "other" domain.
 */

/**
 * \page audiomanagercomponentspage AudioManager Components
 * \image html AudioManagement.png
 *
 * The AudioManager is the central managing instance of the Audio architecture. It is designed as an OwnedComponent, this means that the
 * software is maintained within GENIVI as open source component. The AudioManager consists of 4 central components.\n
 *
 *
 * \section audiomanagercomponents AudioManagerDaemon
 *
 * This component is owned and maintained by Genivi. It is the central audio framework component. There can be only one daemon in a system (singleton).\n
 * The AudioManagerDaemon is subject to this documentation
 *
 * \section commander AudioManagerCommandPlugin
 *
 * This describes the interface towards the Commanding Instances of the AudioManagerDaemon. This is the HMI and interrupt sources that use this
 * interface to start their interrupt and stop it again. The interface shall be asynchronous. Via this interface all user interactions are handled.
 * This component is designed to be a dynamic linked library that will be loaded on the startup of the AudioManager. There can be more than one
 * CommandPlugin at a time.
 * Since the implementation of this component is project specific, only examples are included.\n
 * An example Dbus Implementation can be found in the folder PluginCommandInterfaceDbus.
 * \n
 * All commands that must be fulfilled by an AudioManagerCommandPlugin are described in am::IAmCommandSend.\n
 * All commands that are presented to AudioManagerCommandPlugin by the AudioManagerDaemon are described in am::IAmCommandReceive.\n
 *
 * \section controller AudioManagerController
 * The controller is the intelligent "heart" of the AudioManager and it is project specific. In principle, the controller gets commands from the command
 * interface or events from outside and reacts to them. For this purpose, some basic commands are in the "toolbox" of the Controller that he can use
 * to interact with the rest of the system.\n
 * Among this there are commands to read/write the database and to perform actions on the Audiodomains like connect or disconnect. There must be only one
 * Controller in the system at a time, like the AudioManagerCommandPlugins, the Controller is loaded at startup by the daemon\n
 * A simple example Implementation can be found in the folder PluginControlInterface.
 * \n
 * All commands that must be fulfilled by an AudioManagerController are described in am::IAmControlSend.\n
 * All commands that are presented to AudioManagerController by the AudioManagerDaemon are described in am::IAmControlReceive.\n
 *
 * \section router Routing AudioManagerRoutingPlugin
 *
 * The AudioManagerRoutingPlugins are used to abstract the actual Hard- and Software that does the routing. There can be more than one plugins at a
 * time, they are loaded at startup time like the commandplugins. \n
 * The AudioManager expects a bus-like structure behind each plug-in, so that a plug-in can implement a bus interface and proxy the messages to the
 * routing adapters - the AudioManager is capable of addressing more than one adapter one each plug-in. The AudioManagerController does not have to
 * know anything about the real system plugins - he sends his commands to sources and sinks. The daemon does the dispatching of these commands.
 * The interface is mainly asynchronous.\
 * Sample plugins can be found in the directory, for example PluginRoutingInterfaceAsync.\n
 * \n
 * All commands that must be fulfilled by an AudioManagerRoutingPlugin are described in am::IAmRoutingSend.\n
 * All commands that are presented to AudioManagerRoutingPlugins by the AudioManagerDaemon are described in am::IAmRoutingReceive.\n
 *
 * \page elementspage Elements of the AudioManagement
 *
 * The audiomanagement in principle consists of the following elements:
 *
 * \section source Sources
 * This is where audio comes from, for examples tuner, mediaplayer. But sources can also be part of a building block that processes audio, examples
 * are here crossfaders or gateways. Several Sinks can be connected to one source.\n
 * \subsection sourceattributes Attributes
 * - am::am_SourceType_s describes the attributes that are accessible from the AudioManagerCommandPlugins.\n
 * - am::am_Source_s describes the general attributes.\n
 *
 * \section sinks Sinks
 * This is where audio flows to, for examples amplifier, headphones. But sources can also be part of a building block that processes audio,
 * examples are here crossfaders or gateways. Several Sources can be connected to one sink.\n
 * \subsection sinkattributes Attributes
 * - am::am_SinkType_s describes the attribiutes that are accessible form the AudioManagerCommandPlugins.\n
 * - am::am_Sink_s describes the general attributes.\n
 *
 * \section gw Gateways
 * Gateways are described here: \ref gateway
 * A specialitry of a gateways is the convertionmatrix. It indicates which sinksoundformats can be transferred in which sourcesoundformats. A convertion
 * matrix looks like this:
 * \image html GatewayMatrix.png
 * \subsection gwattributes Attributes
 * - am::am_Gateway_s describe the attribiutes of a gateway\n
 *
 * \section crossfaders Crossfaders
 * Cross-faders are special elements that can perform cross-fading between two sources connected to the sinks of the crossfader. The audio of either source
 * or both (mixed, during the fade) is put out at the source of the fader. Cross-fading within a source (for example from one song to another) is out of
 * scope audio management and must be performed in the source.\n
 * A crossfader has two sinks and one source, where one sink is the "hot" one. It is in the duty of the AudioManagerController to connect the correct
 * sources to the sinks in order to perform a cross-fade. When fading is started, the hotSink changes from either HS_SINKA or HS_SINKB to HS_INTERMEDIATE,
 * when the fading is finished, it changes to HS_SINKA or HS_SINKB (the sink that was "cold" before).Fading itself is done in the RoutingAdapters, the
 * implementation has to ensure the smooth and synchronous change of volumes. With different rampTypes, different kinds of cross-fade ramps can be supported.
 * The actual status of the "hot" sink is reported by the routingAdapter. Care has to be taken that the correct "hot" end of the crossfader is given
 * at registration time.\n
 * \subsection cfattributes Attributes
 * - am::am_Crossfader_s describes the attribiutes of a Crossfader
 *
 */

/**
 * \page uniquepage About unique IDs : Static vs Dynamic IDs
 *
 * \section why Why having two different kinds of ids?\n
 * The complexity of up-to-date IVI-systems demand to support sources and sinks dynamically added and removed in order to support the variety of CE products,
 * but parts of the system are never going to change - to start a dynamic registration here is a waste of system capacity.\n
 * \section setup The setup
 * The AudioManagement is capable of handling static, dynamic or mixed setups. In case of a dynamic setup, all elements of the system like domains, sinks,
 * sources, gateways etc are registered at system start-up. In a static setup, the IDs of the elements are known and fixed - no further registration is needed.
 * The start-up for static elements works as follows:\n
 * when a domain known as static (this is knowledge of the AudioManagerController, recognized by the unique name of the domain) registers, the
 * AudioManagerController enters all elements of this domain in the database. Still, this domain can register additional elements during runtime.
 * In case of static setups, the RoutingAdapter needs to ensure that all static elements are ready to be used when the domain registers.\n
 * In order to ensure the uniqueness of IDs, there exist two separate ID areas (for each of sources, sinks, gateways and crossfaders):\n\n
 *         Fixed area (from 1..100)\n
 *         Variable area (starting from 101)\n\n
 * In case of dynamic added elements, the audiomanagerdaemon ensures the uniqueness of the ID's, in case of the static setup, the project has to ensure the
 * uniqueness by assigning the IDs wisely. The knowledge of the static IDs need to be in the AudioManagerController, the RoutingAdapters and in the HMI
 * (optional because IDs will be reported anyway).\n
 * Domains cannot be static because registering them is the trigger for the AudioManagerController to enter the static values into the database.
 *
 */

/**
 * \page classficationpage Classification of Sinks and Sources
 * \section classification Classification
 * The AudioManagement offers classification for sources and sinks. It can be used to group sources and sinks together and link certain behaviors to these groups.
 * An example for such a use is to group all interrupt sources in a class together and let the AudioManagerController react different for this class.\n
 * Elements can only have one class at a time. The AudioManagerDaemon will take care of registration of sourceClasses, sinkClasses and will assign unique IDs,
 * but the parameters of the Classes itself are product specific and have to be interpreted by the AudioManagerController. This concept allows for very individual
 * implementations of system behaviors.\n
 * Since Classes are held in the database, Classes can be registered and deregistered during runtime, it is recommended that the AudioManagerController enters
 * all source & sink classes at start-up into the database.A ClassProperty exists out of an enumeration (am_ClassProperty_e, project specific) and a corresponding
 * value (integer) that is interpreted by the AudioManagerController according to am_ClassProperty_e.\n
 * There is no restriction to the number of properties a class can have and how many classes can exist in a system.\n
 * \section attributes Attributes
 * - am::am_SourceClass_s describes the attributes of a source class\n
 * - am::am_SinkClass_s describes the attributes of a sink class\n
 *
 */

/**
 * \page interrupts Interrups & Low Level Interrupts
 * \section diff Differences
 * The only difference between a "normal" interrupt and a source for the audioManagement may lie in it's classification, so playing a "normal" interrupt is not
 * different to changing a source. An exception here are the so called "low level interrupts".\n
 * \section crit Criterias
 * These are special sources that fulfill the following criteria:
 *   - direct connection to a sink that is "always on", so no extra actions and communications need to be done in order to play audio
 *   - no dependencies to the current system state need to be checked before playing. This means that the source knows if it is allowed to play at the moment it
 *   wants to play
 *   - all information for the source that is needed to judge if it is allowed to play or not is either directly retrieved by the source or set static via a property.
 *
 *   This becomes very handy for implementing such things like park distance control. When the source is informed that it needs to output signals
 *   (due to an emerging wall for example) it outputs the beeps directly to the amplifier that then overlays the sound to the current active source.\n
 *   Settings that influence the behavior of low level interrupts like for example volume offset for park distance control or sound on/off for it need to be done via
 *   sourceProperties on the source level, so that the judgment and the adoptions that need to be taken can be taken by the source without system interaction.
 *   In order to give the AudioManagerController the chance to react on a low level interrupt (by lowering the main volume for example), a feedback path is provided
 *   and the AudioManagerController is informed about the current state of the low level interrupt (via hooklInterruptStatusChange).\n
 *
 */

/**
 * \page connpage Connections & MainConnections
 * \section con Connections
 * A connection connects a source to a sink to let audio data flow. The direction of the flow is always source to sink. For each connection the connectionFormat
 * must be defined when the connection is demanded, if source or sink is not capable of supporting this format, the connection cannot be established.
 * ConnectionFormats are product specific, also are some standard formats defined within GENIVI that can be enhanced in the product area. Examples of formats:
 * analog, autodefined, stereo 48khz 16bit PCM, ....
 *
 * \section maincon Mainconnections
 * is visible to the HMI and consists out of one or more connections. A MainConnection shall always connect a Source and a Sink visible to the HMI.
 * In contradiction to connections, the MainConnection does not know about connectionFormat. MainConnections are demanded by the commandInterface.
 * This picture demonstrates the relation between MainConnections and connections:
 * \image html Levels.png
 *
 * \section att Attributes
 * - am::am_Connection_s describes the attributes of a conenction
 * - am::am_MainConnection_s describes the attributes of a MainConnection
 */

 /** \page lip Lipsync
 * \section t The Task of the Audiomanager
 * It is the job of the AudioManager to retrieve all latency timing information from each connection, to aggregate this information and provide a latency #
 * information on a per MainConnection Basis. It is not the task of the AudioManager to actually delay or speed up video or audio signals to achieve a lipsync.
 * The actual correction shall be done in the with the information provided by the AudioManager.
 * The time information is always reported by the routingadapters for each connection. Delays that are introduced in a sink or a gateway are counting for the
 * connection that connects to this sink or gateway.
 * \section ex Example
 * \image html delay.jpg
 *
 * This graph shows how the delay is calculated:
 * - Connection 1 has a delay 2ms + 60ms that is added due to the gateway
 * - Connection 2 has a delay 0ms + 10ms due to sink
 * So the routing adapters report 62ms and 10ms, the audiomanager will add this to 72ms and report this value for the main connection from sink to source.
 * The videoplayer getting this information can now delay his video against the audio for 72ms and be in perfect lipsync.\n
 * After the buildup of a connection the first timing information needs to be sent within 5 seconds, the timing information from the routing adapters need to
 * be sent via 4 seconds. If the latency for a connection is variable and changes over lifetime of the connection, the routing adapters shall resend the value
 * and the audiomanager will correct the over all latency.
 */

/**
 * \page early Early Audio
 * \section req The Requirement
 * The requirement reviews showed that one very important aspect of managing audio within GENIVI is the early/late phase where the Linux part of the system is
 * not available. This feature is addressed via special domains: EarlyDomains. These domains are acting "unmanaged" - meaning without the interaction with the
 * AudioManager - providing a simpler set of features until the AudioManager is up and running. When the Linux system is then fully operable a handover is done
 * from the EarlyDomains towards the AudioManager. In order to be able to operate without the Linux up and running, the EarlyDomains must be implemented on a
 * second fast-boot or always-on controller, e.g. the so called vehicle processor.\n
 * \section earlys Early Startup
 * This picture shows the principle of the early startup:
 * \image html early.png
 * \section late Late Rundown
 * This picture shows the principle of the late rundown:
 * \image html late.png
 *
 */

/**
 * \page views The two views of the AudioManager
 * In general, there are two views of the system:\n
 * \section command The CommandInterface View View
 * This is an abstracted view that the HMI and other controlling Instances have of the system. Every Information (with some little exceptions) here is maintained
 * by the AudioManagerController, so that he can "fake" situations for the HMI.
 * So why is that? Depending on the actual project it might be - for example - that not the volume at the sink must be changed, but instead of the source.
 * The HMI does not know about sourceVolumes (and does not need to!) so the HMI would change the sink volume and the AudioManagerController can translate it to a
 * sourceVolumeChange. The metrics of the volumes are different as well.
 * It is the duty of the AudioManagementController to keep the commandInterface information consistent with the "real" situation.
 * \section route RoutingInterface View
 * Here are the "real" system states. All changes that are done on this interface are maintained by the AudioMangerDaemon and here is the actual situation always
 * consistent with the reality. All actions on this interface are either triggered by the AudioManagerController or by the domains itself, like registration for
 * example.
 * \section over Overview
 * \image html views.png
 */

/**
 * \page vol Volumes & MainVolumes
 * \section mainVol MainVolumes
 * This is the volume at the CommanInterface level. The exact definition is project specific and can be adopted for each project since the AudioManagerController
 * and the HMI are the only ones who need to interfere with this volume. The actual value is an integer.
 * - am::am_mainVolume_t defines the mainvolume
 *
 * \section volv Volumes
 * These are the actual volumes that are set for sources and sinks. The unit is 0.1 db steps,The smallest value -3000 (=AM_MUTE). The minimum and maximum can be
 * limited by actual project.
 * - am::am_volume_t describes the volume
 */

/**
 * \page prop Properties
 *  \section soundprop SoundProperties & MainSoundProperties
 * SoundProperties are properties that are related to either a source or a sink. They are product specific and are used to set source or sink specific properties.
 * This could be for example: equalizer settings, relative volume offsets but also bool information "surround sound on/off".
 * A soundProperty is defined by its type (am_SoundProperty_e, a product specific enum) and a value (integer) that is interpreted according to the type.
 * There are not limits how many properties a sink or a source can have. SoundProperties in contradiction to MainSoundProperties are not visible to the
 * CommandInterface.\n
 * - am::am_SoundProperty_s describe the of the SoundProperties
 * - am::am_MainSoundProperty_s describe the attributes of MainSoundProperties
 *
 * \section sys SystemProperties
 * are properties that apply to the whole system or parts of it but cannot be tight to a specific sink or source. They are product specific and could be used
 * for example to set general behavior like all interrupts on/off for example. The information is only exchanged between the commandInterface and the
 * AudioManagerController. The AudioManagerController has to interpret the value and react accordingly.
 * - am::am_SystemProperty_s describes the attributes of Systemproperties
 */

/**
 * \page misc Miscellaneous
 *
 * \section connfor Connection Formats
 * Every flow of audio is using a format to exchange data. The format that the source and the sink uses must match together in order to have an undisturbed
 * experience. It is common that sources and sinks are capable of supporting more than one audioformat.\n
 * So all sources and sinks register with a list of connectionFormats that they support and for each connection a format must be chosen that is then used
 * to transport the audio data. Gateways (and Soundconverters) have the information which connectionFormat can be transformed into another one.
 * - am::am_ConnectionFormat_e has all formats listed.
 *
 * \section pers Persistence & Lifecycle
 * Details of persistence can be done when the si-team have defined the components and interfaces in the Enterprise Architect model.
 * So much is clear: the persistence will be based on POSIX interfaces that can be used to read and write data.\n
 * Persistence is a very system specific topic: what needs to be remembered over lifecycles, what will be reset to default? So this needs to be done via the
 * AudioMangerController. The Controller will then enter the values read from the persistence and write them to the daemon.
 * The lifecycle itself will be handles by the daemon which will then fire hooks in the controller to make sure appropriate actions are taken.
 *
 * \section speed Speed dependent volume
 * The adjustments for the speed are done product specific in the controller. The speed information itself is retrieved by the AudioManagerDaemon, sampled and
 * quantified and forwarded to the controller. The interface in not yet defined !\n
 * Turning speed controlled volume on/off and possible settings are achieved via SinkSoundProperty settings.
 */

/**
 * \page comp Compiling & Co
 * \section readme Readme
 * \include ../README
 */

#ifndef MAINPAGE_H_
#define MAINPAGE_H_

#endif /* MAINPAGE_H_ */
