/*
 * databasetest.h
 *
 *  Created on: Dec 6, 2011
 *      Author: christian
 */

#ifndef DATABASETEST_H_
#define DATABASETEST_H_


#include <gtest/gtest.h>
#include <dlt/dlt.h>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "DatabaseHandler.h"

DLT_DECLARE_CONTEXT(AudioManager);

using namespace testing;

class databaseTest : public Test {
public:

	DatabaseHandler pDatabaseHandler;
	void SetUp();
	void TearDown();

	static std::vector<am_ConnectionFormat_e> getStandardConnectionFormatList();
	static std::vector<am_SoundProperty_s> getStandardSoundPropertyList();
	static std::vector<am_MainSoundProperty_s> getStandardMainSoundPropertyList();
	static std::vector<bool> getStandardConvertionMatrix();
	bool compareSource(std::vector<am_Source_s>::iterator listIterator, const am_Source_s& sourceData);
	bool compareSink(std::vector<am_Sink_s>::iterator listIterator, const am_Sink_s& sinkData);
	bool compareGateway(std::vector<am_Gateway_s>::iterator listIterator, const am_Gateway_s& gatewayData);
	bool compareGateway1(const am_Gateway_s gateway1, const am_Gateway_s gatewayData);
	bool compareSinkMainSink(std::vector<am_SinkType_s>::iterator listIterator, const std::vector<am_Sink_s>& sinkList);
	bool compareSinkMainSource(std::vector<am_SourceType_s>::iterator listIterator, const std::vector<am_Source_s>& sourceList);
	std::string int2string(int i);

	void createSink(am_Sink_s& sink,am_sinkID_t sinkID,std::string name,am_domainID_t domainID,	am_Availablility_e availability,
					am_AvailabilityReason_e availabilityReason,	am_sinkClass_t sinkClassID,
					std::vector<am_ConnectionFormat_e> connectionFormatList,
					std::vector<am_SoundProperty_s> soundPropertyList,
					std::vector<am_MainSoundProperty_s> mainSoundPropertyList,
					am_mainVolume_t mainVolume,	am_MuteState_e muteState,
					bool visible, am_volume_t volume);

	void createSource(am_Source_s& source, am_sourceID_t sourceID, std::string name, am_domainID_t domainID,
					am_Availablility_e availability, am_AvailabilityReason_e availabilityReason, am_sourceClass_t sourceClassID,
					std::vector<am_ConnectionFormat_e> listConnectionFormats,
					std::vector<am_SoundProperty_s> listSoundProperties,
					std::vector<am_MainSoundProperty_s> listMainSoundProperties,
					am_InterruptState_e interruptState, bool visible, am_volume_t volume, am_SourceState_e sourceState);

	void createDomain(am_Domain_s& domain, am_domainID_t domainID, std::string name, std::string nodename,
					std::string busname, bool complete, bool early, am_DomainState_e state);

	void createGateway(am_Gateway_s& gateway, am_gatewayID_t gatewayID, std::string name, am_sinkID_t sinkID, am_sourceID_t sourceID, am_domainID_t controlDomainID,
					am_domainID_t domainSinkID, am_domainID_t domainSourceID,std::vector<bool> convertionMatrix,
					std::vector<am_ConnectionFormat_e> listSourceFormats,std::vector<am_ConnectionFormat_e> listSinkFormats);

	void createConnection(am_Connection_s& connection, am_connectionID_t connectionID, am_sinkID_t sinkID,
					am_sourceID_t sourceID, am_timeSync_t delay, am_ConnectionFormat_e connectionFormat);

	void createMainConnection(am_MainConnection_s& mainConnection, am_Route_s route, am_timeSync_t delay,
					am_mainConnectionID_t connectionID, am_ConnectionState_e state);


	void connectionList2RoutingList(std::vector<am_RoutingElement_s>& routingList, const std::vector<am_Connection_s>& connectionList);

	void createMainConnectionSetup();

	struct sortBySinkID
	{
		bool operator()(const am_RoutingElement_s & a, const am_RoutingElement_s & b) const
		{
			return (a.sinkID < b.sinkID);
		}
	};

	struct sortByConnectionFormat
	{
		bool operator()(const am_ConnectionFormat_e & a, const am_ConnectionFormat_e & b) const
		{
			return (a < b);
		}
	};

	struct sortByMainSoundProperty
	{
		bool operator()(const am_MainSoundProperty_s & a, const am_MainSoundProperty_s & b) const
		{
			return (a.type > b.type);
		}
	};

	struct sortBySoundProperty
	{
		bool operator()(const am_SoundProperty_s & a, const am_SoundProperty_s & b) const
		{
			return (a.type < b.type);
		}
	};
};

#endif /* DATABASETEST_H_ */
