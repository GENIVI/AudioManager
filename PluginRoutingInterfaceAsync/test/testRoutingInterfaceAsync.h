/*
 * testRoutingInterfaceAsync.h
 *
 *  Created on: Dec 27, 2011
 *      Author: christian
 */

#ifndef TESTROUTINGINTERFACEASYNC_H_
#define TESTROUTINGINTERFACEASYNC_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlt/dlt.h>
#include "mocklnterfaces.h"
#include "SocketHandler.h"
#include "../../AudioManagerDaemon/include/RoutingSender.h"

#define UNIT_TEST 1

namespace am {

class testRoutingInterfaceAsync :public ::testing::Test
{
public:
	static std::vector<std::string> pListRoutingPluginDirs;
	SocketHandler pSocketHandler;
	MockRoutingReceiveInterface pReceiveInterface;
	static RoutingSender pRoutingSender;
	static std::vector<std::string> returnListPlugins();
	static am_Error_e handleDomainRegister (const am_Domain_s& domainData, am_domainID_t& domainID);
	static am_Error_e handleSourceRegister (const am_Source_s& sourceData, am_sourceID_t& sourceID);
	static am_Error_e handleSinkRegister(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
	void timerCallback(sh_timerHandle_t handle, void* userData);
	shTimerCallBack_T<testRoutingInterfaceAsync> ptimerCallback;
	testRoutingInterfaceAsync();
	virtual ~testRoutingInterfaceAsync();

	void SetUp();
	void TearDown();
private:
	static am_domainID_t mDomainIDCount;
};

} /* namespace am */
#endif /* TESTROUTINGINTERFACEASYNC_H_ */
