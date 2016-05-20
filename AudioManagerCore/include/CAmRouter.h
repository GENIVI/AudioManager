/**
 * SPDX license identifier: MPL-2.0
 *
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
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Aleksandar Donchev, Aleksander.Donchev@partner.bmw.de BMW 2013,2014
 *
 * \file CAmRouter.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#include <assert.h>
#include <vector>
#include <functional>
#include "audiomanagertypes.h"
#include "CAmGraph.h"
#include "IAmDatabaseHandler.h"


namespace am
{
#define ROUTING_BUILD_CONNECTIONS 1

/**
 * Optimal path search between a source and a sink is implemented with a graph which contains nodes - sinks, sources, gateways, converters.
 * The nodes are identified by sinkID, sourceID, gatewayID, converterID.
 * A possible connection between two nodes represents the facts that the nodes can be connected with one or more connectionFormats (Node[id=1] ---> Node[id=2]).
 * It is assumption that the two nodes can be connected. The controller itself decides later whether the connection is possible or not.
 *
 */

/**
 * Trace on/off.
 */
#if !defined(ROUTING_BUILD_CONNECTIONS)
	#undef TRACE_GRAPH
#endif

/**
 * Default behavior is to do the search in one step without connections, which are identified during the search.
 * Alternatively the search can be done in two steps.
 */
#if !defined(ROUTING_BUILD_CONNECTIONS)
	#undef ROUTING_BUILD_CONNECTIONS
#endif

#if defined(TRACE_GRAPH)
#if !defined(ROUTING_BUILD_CONNECTIONS)
#warning "You should define ROUTING_BUILD_CONNECTIONS in order to be able to see the connections in the trace."
#endif
#endif

class CAmRouter;

/**
 * A structure used as user data in the graph nodes.
 */
struct am_RoutingNodeData_s
{
	typedef enum:uint8_t {SINK, SOURCE, GATEWAY, CONVERTER} am_NodeDataType_e;
	am_NodeDataType_e type;															//!< data type:sink, source, gateway or converter
	union
	{
		am_Source_s *source;
		am_Sink_s *sink;
		am_Gateway_s *gateway;
		am_Converter_s *converter;
	} data;																			//!< union pointer to sink, source, gateway or converter

	am_RoutingNodeData_s():type(SINK)
	{}

	bool operator==(const am_RoutingNodeData_s & anotherObject) const
					{
		bool result = false;
		if(type==anotherObject.type)
		{
			result = true;
			if(type==SINK)
				result &= (data.sink->sinkID==anotherObject.data.sink->sinkID);
			else if(type==SOURCE)
				result &= (data.source->sourceID==anotherObject.data.source->sourceID);
			else if(type==GATEWAY)
				result &= (data.gateway->gatewayID==anotherObject.data.gateway->gatewayID);
			else if(type==CONVERTER)
				result &= (data.converter->converterID==anotherObject.data.converter->converterID);
		}
		return result;
					};

#ifdef TRACE_GRAPH
	void trace() const
	{
		if(type==SINK)
			std::cout << "[SINK:" << data.sink->sinkID << ":" << data.sink->name << "(" << data.sink->domainID << ")"
			<< "]";
		else if(type==SOURCE)
			std::cout << "[SOUR:" << data.source->sourceID << ":" << data.source->name << "(" << data.source->domainID << ")"
			<< "]";
		else if(type==GATEWAY)
			std::cout << "[GATE:" << data.gateway->gatewayID << ":" << data.gateway->name << "(" << data.gateway->controlDomainID << ")"
			<< "]";
		else if(type==CONVERTER)
			std::cout << "[CONV:" << data.converter->converterID << ":" << data.converter->name << "(" << data.converter->domainID << ")"
			<< "]";
	};
#endif

	am_domainID_t domainID() const
	{
		if(type==SINK)
			return data.sink->domainID;
		else if(type==SOURCE)
			return data.source->domainID;
		else if(type==GATEWAY)
			return data.gateway->controlDomainID;
		else if(type==CONVERTER)
			return data.converter->domainID;
		return 0;
	};
};

typedef am_RoutingNodeData_s::am_NodeDataType_e CAmNodeDataType;
typedef CAmNode<am_RoutingNodeData_s> CAmRoutingNode;
typedef CAmGraph<am_RoutingNodeData_s, uint16_t> CAmRoutingGraph;
typedef CAmVertex<am_RoutingNodeData_s, uint16_t> CAmRoutingVertex;
typedef std::list<CAmRoutingVertex> CAmRoutingListVertices;
typedef std::vector<CAmRoutingListVertices*> CAmRoutingVertexReferenceList;

class CAmControlSender;


/**
 * Implements an autorouting algorithm for connecting sinks and sources via different audio domains.
 */
class CAmRouter
{
	IAmDatabaseHandler* mpDatabaseHandler; 							//!< pointer to database handler
	CAmControlSender* mpControlSender; 								//!< pointer the controlsender - is used to retrieve information for the optimal route
	bool mOnlyFreeConversionNodes;									//!< bool flag whether only disconnected elements should be considered or not
	CAmRoutingGraph mRoutingGraph;			//!< graph object
	std::map<am_domainID_t,std::vector<CAmRoutingNode*>> mNodeListSources;	//!< map with pointers to nodes with sources, used for quick access
	std::map<am_domainID_t,std::vector<CAmRoutingNode*>> mNodeListSinks;		//!< map with pointers to nodes with sinks, used for quick access
	std::map<am_domainID_t,std::vector<CAmRoutingNode*>> mNodeListGateways;	//!< map with pointers to nodes with gateways, used for quick access
	std::map<am_domainID_t,std::vector<CAmRoutingNode*>> mNodeListConverters;//!< map with pointers to nodes with converters, used for quick access

	am_Error_e determineConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmRoutingNode*> & nodes);
	am_Error_e doConnectionFormatsForPath(am_Route_s & routeObjects,
			std::vector<CAmRoutingNode*> & route,
			std::vector<am_RoutingElement_s>::iterator routingElementIterator,
			std::vector<CAmRoutingNode*>::iterator routeIterator);


	/**
	 * Check whether given converter or gateway has been connected.
	 *
	 * @param comp  converter or gateway .
	 */
	template <class Component> bool isComponentConnected(const Component & comp)
	{
		return mpDatabaseHandler->isComponentConnected(comp);
	}
	void generateAllPaths(const CAmRoutingNode & src,
			const CAmRoutingNode & dst,
			const bool includeCycles,
			std::function<void(const std::vector<CAmRoutingNode*> & path)> cb);
	void goThroughAllPaths(const CAmRoutingNode & dst,
			std::vector<CAmRoutingNode*> & visited,
			std::vector<am_domainID_t> & visitedDomains,
			std::function<void(const std::vector<CAmRoutingNode*> & path)> cb);

#ifdef ROUTING_BUILD_CONNECTIONS
	/**
	 * Connects all converters to its sink and sources if possible.
	 *
	 */
	void constructConverterConnections();

	/**
	 * Connects all gateways to its sink and sources if possible.
	 *
	 */
	void constructGatewayConnections();

	/**
	 * Connects all sources to the sinks if possible.
	 *
	 */
	void constructSourceSinkConnections();
#else
	/**
	 * Construct a list with all vertices
	 */
	void getVerticesForNode(const CAmRoutingNode & node, CAmRoutingListVertices & list);

	/**
	 * Construct a list with all vertices from given source.
	 */
	void getVerticesForSource(const CAmRoutingNode & node, CAmRoutingListVertices & list);

	/**
	 * Construct a list with all vertices from given sink.
	 */
	void getVerticesForSink(const CAmRoutingNode & node, CAmRoutingListVertices & list);

	/**
	 * Construct a list with all vertices from given converter.
	 */
	void getVerticesForConverter(const CAmRoutingNode & node, CAmRoutingListVertices & list);

	/**
	 * Construct a list with all vertices from given gateway.
	 */
	void getVerticesForGateway(const CAmRoutingNode & node, CAmRoutingListVertices & list);
#endif

public:
	CAmRouter(IAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iSender);
	~CAmRouter();

	/**
	 * Finds all possible paths between given source and sink.
	 *
	 * @param onlyfree only disconnected elements should be included or not.
	 * @param sourceID starting point.
	 * @param sinkID ending point.
	 * @param returnList list with all possible paths
	 * @return E_OK on success(0 or more paths) or E_NOT_POSSIBLE on failure.
	 */
	am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);
	am_Error_e getRoute(const bool onlyfree, const am_Source_s & aSource, const am_Sink_s & aSink, std::vector<am_Route_s> & listRoutes);

	am_Error_e getAllPaths(CAmRoutingNode & aSource, CAmRoutingNode & aSink,
			std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmRoutingNode*>> & resultNodesPath,
#if !defined(ROUTING_BUILD_CONNECTIONS)
			__attribute__((unused))
#endif
			const bool includeCycles = false);
#ifdef ROUTING_BUILD_CONNECTIONS
	void getShortestPath(const CAmRoutingNode & source, const CAmRoutingNode & destination, std::vector<CAmRoutingNode*> & resultPath);
	void getShortestPath(CAmRoutingNode & aSource, CAmRoutingNode & aSink, am_Route_s & resultPath, std::vector<CAmRoutingNode*> & resultNodesPath);
#endif

	static bool  getAllowedFormatsFromConvMatrix(	const std::vector<bool> & convertionMatrix,
			const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
			const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
			std::vector<am_CustomConnectionFormat_t> & sourceFormats,
			std::vector<am_CustomConnectionFormat_t> & sinkFormats);
	static void listPossibleConnectionFormats(std::vector<am_CustomConnectionFormat_t> & inListSourceFormats,
			std::vector<am_CustomConnectionFormat_t> & inListSinkFormats,
			std::vector<am_CustomConnectionFormat_t> & outListFormats);
	static bool getRestrictedOutputFormats(const std::vector<bool> & convertionMatrix,
			const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
			const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
			const am_CustomConnectionFormat_t connectionFormat,
			std::vector<am_CustomConnectionFormat_t> & listFormats);
	static void getSourceSinkPossibleConnectionFormats(std::vector<CAmRoutingNode*>::iterator iteratorSource,
			std::vector<CAmRoutingNode*>::iterator iteratorSink,
			std::vector<am_CustomConnectionFormat_t> & outConnectionFormats);

	static bool shouldGoInDomain(const std::vector<am_domainID_t> & visitedDomains, const am_domainID_t nodeDomainID);

	/**
	 * Returns a sink node with given sinkID.
	 *
	 * @param sinkID sink id.
	 * @return pointer to node or NULL.
	 */
	CAmRoutingNode* sinkNodeWithID(const am_sinkID_t sinkID);
	CAmRoutingNode* sinkNodeWithID(const am_sinkID_t sinkID, const am_domainID_t domainID);

	/**
	 * Returns a source node with given sourceID.
	 *
	 * @param sourceID source id.
	 * @return pointer to node or NULL.
	 */
	CAmRoutingNode* sourceNodeWithID(const am_sourceID_t sourceID);
	CAmRoutingNode* sourceNodeWithID(const am_sourceID_t sourceID, const am_domainID_t domainID);

	/**
	 * Returns a converter node for given sinkID.
	 *
	 * @param sinkID sink id.
	 * @param domainID domain id.
	 * @return pointer to node or NULL.
	 */
	CAmRoutingNode* converterNodeWithSinkID(const am_sinkID_t sinkID, const am_domainID_t domainID);

	/**
	 * Returns a gateway node for given sinkID.
	 *
	 * @param sinkID sink id.
	 * @return pointer to node or NULL.
	 */
	CAmRoutingNode* gatewayNodeWithSinkID(const am_sinkID_t sinkID);

	void load(const bool onlyFree);
	void clear();
};
} /* namespace am */
#endif /* ROUTER_H_ */

