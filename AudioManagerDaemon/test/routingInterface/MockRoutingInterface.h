/*
 * MockRoutingInterface.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef MOCKROUTINGINTERFACE_H_
#define MOCKROUTINGINTERFACE_H_

#include <routing/RoutingSendInterface.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace am {

class MockRoutingSendInterface : public RoutingSendInterface {
	 public:
	  MOCK_METHOD1(startupRoutingInterface,
	      void(RoutingReceiveInterface* routingreceiveinterface));
	  MOCK_METHOD0(routingInterfacesReady,
	      void());
	  MOCK_METHOD0(routingInterfacesRundown,
	      void());
	  MOCK_METHOD1(asyncAbort,
	      am_Error_e(const am_Handle_s handle));
	  MOCK_METHOD5(asyncConnect,
	      am_Error_e(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat));
	  MOCK_METHOD2(asyncDisconnect,
	      am_Error_e(const am_Handle_s handle, const am_connectionID_t connectionID));
	  MOCK_METHOD5(asyncSetSinkVolume,
	      am_Error_e(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time));
	  MOCK_METHOD5(asyncSetSourceVolume,
	      am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time));
	  MOCK_METHOD3(asyncSetSourceState,
	      am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state));
	  MOCK_METHOD3(asyncSetSinkSoundProperty,
	      am_Error_e(const am_Handle_s handle, const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID));
	  MOCK_METHOD3(asyncSetSourceSoundProperty,
	      am_Error_e(const am_Handle_s handle, const am_SoundProperty_s& soundProperty, const am_sourceID_t sourceID));
	  MOCK_METHOD5(asyncCrossFade,
	      am_Error_e(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time));
	  MOCK_METHOD2(setDomainState,
	      am_Error_e(const am_domainID_t domainID, const am_DomainState_e domainState));
	  MOCK_CONST_METHOD1(returnBusName,
	      am_Error_e(std::string& BusName));
	};

}  // namespace am


#endif /* MOCKROUTINGINTERFACE_H_ */
