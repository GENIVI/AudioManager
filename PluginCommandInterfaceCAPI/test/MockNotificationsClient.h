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


#ifndef MOCKNOTIFICATIONSCLIENT_H_
#define MOCKNOTIFICATIONSCLIENT_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <org/genivi/audiomanager/CommandInterfaceStubDefault.h>


namespace am {
using namespace testing;
using namespace CommonAPI;
using namespace org::genivi::audiomanager;

class IAmNotificationsClient
{
public:
		IAmNotificationsClient()
		{}

		virtual ~IAmNotificationsClient()
		{}
	    virtual void onNumberOfMainConnectionsChangedEvent() = 0;
	    virtual void onNumberOfSourceClassesChangedEvent() = 0;
		virtual void onMainConnectionStateChangedEvent(CommandInterface::am_mainConnectionID_t, CommandInterface::am_ConnectionState_e) = 0;
		virtual void onSourceAddedEvent(const CommandInterface::am_SourceType_s &) = 0;
		virtual void onSourceRemovedEvent(CommandInterface::am_sourceID_t)  = 0;
		virtual void onMainSourceSoundPropertyChangedEvent(CommandInterface::am_sourceID_t, const CommandInterface::am_MainSoundProperty_s & )  = 0;
		virtual void onSourceAvailabilityChangedEvent(CommandInterface::am_sourceID_t, const CommandInterface::am_Availability_s &)  = 0;
		virtual void onNumberOfSinkClassesChangedEvent()  = 0;
		virtual void onSinkAddedEvent(const CommandInterface::am_SinkType_s &)  = 0;
		virtual void onSinkRemovedEvent(CommandInterface::am_sinkID_t)  = 0;
		virtual void onMainSinkSoundPropertyChangedEvent(CommandInterface::am_sinkID_t, const CommandInterface::am_MainSoundProperty_s &)  = 0;
		virtual void onSinkAvailabilityChangedEvent(CommandInterface::am_sinkID_t, const CommandInterface::am_Availability_s &)  = 0;
		virtual void onVolumeChangedEvent(CommandInterface::am_sinkID_t, CommandInterface::am_mainVolume_t)  = 0;
		virtual void onSinkMuteStateChangedEvent(CommandInterface::am_sinkID_t, CommandInterface::am_MuteState_e)  = 0;
		virtual void onSystemPropertyChangedEvent(const CommandInterface::am_SystemProperty_s &) = 0;
		virtual void onTimingInformationChangedEvent(CommandInterface::am_mainConnectionID_t, CommandInterface::am_timeSync_t)  = 0;
		virtual void onSinkUpdatedEvent(CommandInterface::am_sinkID_t, CommandInterface::am_sinkClass_t, const CommandInterface::am_MainSoundProperty_l &)  = 0;
		virtual void onSourceUpdatedEvent(CommandInterface::am_sourceID_t, CommandInterface::am_sourceClass_t, const CommandInterface::am_MainSoundProperty_l &)   = 0;
		virtual void onSinkNotificationEvent(CommandInterface::am_sinkID_t, const CommandInterface::am_NotificationPayload_s & ) = 0;
		virtual void onSourceNotificationEvent(CommandInterface::am_sourceID_t, const CommandInterface::am_NotificationPayload_s &)  = 0;
		virtual void onMainSinkNotificationConfigurationChangedEvent(CommandInterface::am_sinkID_t, const org::genivi::audiomanager::am::am_NotificationConfiguration_s &)  = 0;
		virtual void onMainSourceNotificationConfigurationChangedEvent(CommandInterface::am_sourceID_t, const org::genivi::audiomanager::am::am_NotificationConfiguration_s &) = 0;
};

class MockNotificationsClient : public IAmNotificationsClient {
 public:
	MOCK_METHOD0(onNumberOfMainConnectionsChangedEvent,
			void());
	MOCK_METHOD0(onNumberOfSourceClassesChangedEvent, void());
	MOCK_METHOD2(onMainConnectionStateChangedEvent,
			void(CommandInterface::am_mainConnectionID_t mcID, CommandInterface::am_ConnectionState_e cs));
	MOCK_METHOD1(onSourceAddedEvent, void(const CommandInterface::am_SourceType_s & st));
	MOCK_METHOD1(onSourceRemovedEvent, void(CommandInterface::am_sourceID_t sid));
	MOCK_METHOD2(onMainSourceSoundPropertyChangedEvent,
			void(CommandInterface::am_sourceID_t sid, const CommandInterface::am_MainSoundProperty_s & msp) );
	MOCK_METHOD2(onSourceAvailabilityChangedEvent,
			void(CommandInterface::am_sourceID_t st, const CommandInterface::am_Availability_s & a) );
	MOCK_METHOD0(onNumberOfSinkClassesChangedEvent,
			void());
	MOCK_METHOD1(onSinkAddedEvent,
			void(const CommandInterface::am_SinkType_s & st));
	MOCK_METHOD1(onSinkRemovedEvent,
			void(CommandInterface::am_sinkID_t sid));
	MOCK_METHOD2(onMainSinkSoundPropertyChangedEvent,
			void(CommandInterface::am_sinkID_t sid, const CommandInterface::am_MainSoundProperty_s & msp) );
	MOCK_METHOD2(onSinkAvailabilityChangedEvent,
			void(CommandInterface::am_sinkID_t sid, const CommandInterface::am_Availability_s & a) );
	MOCK_METHOD2(onVolumeChangedEvent,
			void(CommandInterface::am_sinkID_t sid, CommandInterface::am_mainVolume_t mv) );
	MOCK_METHOD2(onSinkMuteStateChangedEvent,
			void(CommandInterface::am_sinkID_t sid, CommandInterface::am_MuteState_e ms) );
	MOCK_METHOD1(onSystemPropertyChangedEvent,
			void(const CommandInterface::am_SystemProperty_s & sp));
	MOCK_METHOD2(onTimingInformationChangedEvent,
			void(CommandInterface::am_mainConnectionID_t cid, CommandInterface::am_timeSync_t ts) );
	MOCK_METHOD3(onSinkUpdatedEvent,
			void(CommandInterface::am_sinkID_t sid, CommandInterface::am_sinkClass_t sc, const CommandInterface::am_MainSoundProperty_l & msp) );
	MOCK_METHOD3(onSourceUpdatedEvent,
			void(CommandInterface::am_sourceID_t sid, CommandInterface::am_sourceClass_t sc, const CommandInterface::am_MainSoundProperty_l & msp)  );
	MOCK_METHOD2(onSinkNotificationEvent,
			void(CommandInterface::am_sinkID_t sid, const CommandInterface::am_NotificationPayload_s & np));
	MOCK_METHOD2(onSourceNotificationEvent,
			void(CommandInterface::am_sourceID_t sid, const CommandInterface::am_NotificationPayload_s & np) );
	MOCK_METHOD2(onMainSinkNotificationConfigurationChangedEvent,
			void(CommandInterface::am_sinkID_t sid, const org::genivi::audiomanager::am::am_NotificationConfiguration_s & nc) );
	MOCK_METHOD2(onMainSourceNotificationConfigurationChangedEvent,
			void(CommandInterface::am_sourceID_t sid, const org::genivi::audiomanager::am::am_NotificationConfiguration_s & nc));
};

}  // namespace am
#endif /* MOCKCOMMANDRECEIVENTERFACE_H_ */
