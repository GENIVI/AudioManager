/**
 * \mainpage
 *
 *
 * \image html genivilogo.png
 *
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \par Copyright
 * Copyright Copyright (C) 2011,2012 GENIVI Alliance\n
 * Copyright Copyright (C) 2011,2012 BMW AG
 *
 * \par License
 * Contributions are licensed to the GENIVI Alliance under one or more Contribution License Agreements.
 * This file is licensed under the terms of the Mozilla Public License 2.0.\n
 * A copy of the license text has been included in the “LICENSE” file in the root directory of the source distribution.
 * You can also obtain a copy of the license text at\n
 * http://mozilla.org/MPL/2.0/.
 *
 * \par More information
 * can be found at https://collab.genivi.org/wiki/display/genivi/GENIVI+Home \n
 *
 * \par About AudioManager
 * The AudioManager is a Deamon that manages all Audio Connections in a GENIVI headunit.
 * It is a managing instance that uses so called RoutingAdaptors to control AudioDomains that then do the "real" connections.
 * \n\n\n
 *
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
 * @created 25-Feb-2012 3:54:06 PM
 */

#ifndef MAINPAGE_H_
#define MAINPAGE_H_

#endif /* MAINPAGE_H_ */
