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
#include <org/genivi/am/CommandControlProxy.h>


namespace am {
using namespace testing;
using namespace CommonAPI;

class IAmNotificationsClient
{
public:
		IAmNotificationsClient() {}
		virtual ~IAmNotificationsClient() {}
	    virtual void onNewMainConnection(const org::genivi::am::am_MainConnectionType_s &) = 0 ;
	    virtual void removedMainConnection(org::genivi::am::am_mainConnectionID_t) = 0 ;
	    virtual void onNumberOfSourceClassesChangedEvent() = 0 ;
		virtual void onMainConnectionStateChangedEvent(org::genivi::am::am_mainConnectionID_t, org::genivi::am::am_ConnectionState_e) = 0 ;
		virtual void onSourceAddedEvent(const org::genivi::am::am_SourceType_s &) = 0 ;
		virtual void onSourceRemovedEvent(org::genivi::am::am_sourceID_t) = 0  ;
		virtual void onMainSourceSoundPropertyChangedEvent(org::genivi::am::am_sourceID_t, const org::genivi::am::am_MainSoundProperty_s & ) = 0 ;
		virtual void onSourceAvailabilityChangedEvent(org::genivi::am::am_sourceID_t, const org::genivi::am::am_Availability_s &) = 0 ;
		virtual void onNumberOfSinkClassesChangedEvent() = 0 ;
		virtual void onSinkAddedEvent(const org::genivi::am::am_SinkType_s &)= 0  ;
		virtual void onSinkRemovedEvent(org::genivi::am::am_sinkID_t) = 0  ;
		virtual void onMainSinkSoundPropertyChangedEvent(org::genivi::am::am_sinkID_t, const org::genivi::am::am_MainSoundProperty_s &) = 0  ;
		virtual void onSinkAvailabilityChangedEvent(org::genivi::am::am_sinkID_t, const org::genivi::am::am_Availability_s &) = 0 ;
		virtual void onVolumeChangedEvent(org::genivi::am::am_sinkID_t, org::genivi::am::am_mainVolume_t)  = 0 ;
		virtual void onSinkMuteStateChangedEvent(org::genivi::am::am_sinkID_t, org::genivi::am::am_MuteState_e)  = 0 ;
		virtual void onSystemPropertyChangedEvent(const org::genivi::am::am_SystemProperty_s &) = 0 ;
		virtual void onTimingInformationChangedEvent(org::genivi::am::am_mainConnectionID_t, org::genivi::am::am_timeSync_t)  = 0 ;
		virtual void onSinkUpdatedEvent(org::genivi::am::am_sinkID_t, org::genivi::am::am_sinkClass_t, const org::genivi::am::am_MainSoundProperty_L &)  = 0 ;
		virtual void onSourceUpdatedEvent(org::genivi::am::am_sourceID_t, org::genivi::am::am_sourceClass_t, const org::genivi::am::am_MainSoundProperty_L &)   = 0 ;
		virtual void onSinkNotificationEvent(org::genivi::am::am_sinkID_t, const org::genivi::am::am_NotificationPayload_s & ) = 0 ;
		virtual void onSourceNotificationEvent(org::genivi::am::am_sourceID_t, const org::genivi::am::am_NotificationPayload_s &)  = 0 ;
		virtual void onMainSinkNotificationConfigurationChangedEvent(org::genivi::am::am_sinkID_t, const org::genivi::am::am_NotificationConfiguration_s &)  = 0 ;
		virtual void onMainSourceNotificationConfigurationChangedEvent(org::genivi::am::am_sourceID_t, const org::genivi::am::am_NotificationConfiguration_s &) = 0 ;
};

class MockNotificationsClient : public IAmNotificationsClient {
public:
	 MOCK_METHOD1(onNewMainConnection,
	      void(const org::genivi::am::am_MainConnectionType_s&));
	  MOCK_METHOD1(removedMainConnection,
	      void(org::genivi::am::am_mainConnectionID_t));
	  MOCK_METHOD0(onNumberOfSourceClassesChangedEvent,
	      void());
	  MOCK_METHOD2(onMainConnectionStateChangedEvent,
	      void(org::genivi::am::am_mainConnectionID_t, org::genivi::am::am_ConnectionState_e));
	  MOCK_METHOD1(onSourceAddedEvent,
	      void(const org::genivi::am::am_SourceType_s &));
	  MOCK_METHOD1(onSourceRemovedEvent,
	      void(org::genivi::am::am_sourceID_t));
	  MOCK_METHOD2(onMainSourceSoundPropertyChangedEvent,
	      void(org::genivi::am::am_sourceID_t, const org::genivi::am::am_MainSoundProperty_s&));
	  MOCK_METHOD2(onSourceAvailabilityChangedEvent,
	      void(org::genivi::am::am_sourceID_t, const org::genivi::am::am_Availability_s&));
	  MOCK_METHOD0(onNumberOfSinkClassesChangedEvent,
	      void());
	  MOCK_METHOD1(onSinkAddedEvent,
	      void(const org::genivi::am::am_SinkType_s&));
	  MOCK_METHOD1(onSinkRemovedEvent,
	      void(org::genivi::am::am_sinkID_t));
	  MOCK_METHOD2(onMainSinkSoundPropertyChangedEvent,
	      void(org::genivi::am::am_sinkID_t, const org::genivi::am::am_MainSoundProperty_s&));
	  MOCK_METHOD2(onSinkAvailabilityChangedEvent,
	      void(org::genivi::am::am_sinkID_t, const org::genivi::am::am_Availability_s&));
	  MOCK_METHOD2(onVolumeChangedEvent,
	      void(org::genivi::am::am_sinkID_t, org::genivi::am::am_mainVolume_t));
	  MOCK_METHOD2(onSinkMuteStateChangedEvent,
	      void(org::genivi::am::am_sinkID_t, org::genivi::am::am_MuteState_e));
	  MOCK_METHOD1(onSystemPropertyChangedEvent,
	      void(const org::genivi::am::am_SystemProperty_s&));
	  MOCK_METHOD2(onTimingInformationChangedEvent,
	      void(org::genivi::am::am_mainConnectionID_t, org::genivi::am::am_timeSync_t));
	  MOCK_METHOD3(onSinkUpdatedEvent,
	      void(org::genivi::am::am_sinkID_t, org::genivi::am::am_sinkClass_t, const org::genivi::am::am_MainSoundProperty_L&));
	  MOCK_METHOD3(onSourceUpdatedEvent,
	      void(org::genivi::am::am_sourceID_t, org::genivi::am::am_sourceClass_t, const org::genivi::am::am_MainSoundProperty_L&));
	  MOCK_METHOD2(onSinkNotificationEvent,
	      void(org::genivi::am::am_sinkID_t, const org::genivi::am::am_NotificationPayload_s&));
	  MOCK_METHOD2(onSourceNotificationEvent,
	      void(org::genivi::am::am_sourceID_t, const org::genivi::am::am_NotificationPayload_s&));
	  MOCK_METHOD2(onMainSinkNotificationConfigurationChangedEvent,
	      void(org::genivi::am::am_sinkID_t, const org::genivi::am::am_NotificationConfiguration_s&));
	  MOCK_METHOD2(onMainSourceNotificationConfigurationChangedEvent,
	      void(org::genivi::am::am_sourceID_t, const org::genivi::am::am_NotificationConfiguration_s&));
	};



}  // namespace am
#endif /* MOCKCOMMANDRECEIVENTERFACE_H_ */
