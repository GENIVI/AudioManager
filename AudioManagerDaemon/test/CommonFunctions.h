/*
 * CommonHeaders.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef COMMONHEADERS_H_
#define COMMONHEADERS_H_

#include <gtest/gtest.h>
#include "DatabaseHandler.h"

using namespace am;

class CommonFunctions
{
public:

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
	void createSink(am_Sink_s& sink) const;
	void createSource(am_Source_s& source) const ;
	void createDomain(am_Domain_s& domain) const;
	void createGateway(am_Gateway_s& gateway);
	void createConnection(am_Connection_s& connection) const ;
	void createMainConnection(am_MainConnection_s& mainConnection, am_Route_s route) const;
	void connectionList2RoutingList(std::vector<am_RoutingElement_s>& routingList, const std::vector<am_Connection_s>& connectionList);

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




#endif /* COMMONHEADERS_H_ */
