/**
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
#include "audiomanagertypes.h"
#include "CAmGraph.h"
#include "IAmDatabaseHandler.h"


namespace am
{
/**
 * Optimal path search between a source and a sink is implemented with a graph which contains nodes - sinks, sources, gateways, converters.
 *
 * If EXTENDED_ROUTING_GRAPH is defined the graph will contain nodes, which are identified by sinkID, sourceID, gatewayID, converterID and connectionFormat.
 * All possible connections between all nodes (1 connection is 1 connection format) are represented in the graph (Node[id=1, connectionFormat=1] ---> Node[id=2, connectionFormat=1]).
 *
 * If EXTENDED_ROUTING_GRAPH is NOT defined the graph will contain nodes, which are identified by sinkID, sourceID, gatewayID, converterID.
 * A possible connection between two nodes represents the facts that the nodes can be connected with one or more connectionFormats (Node[id=1] ---> Node[id=2]).
 * It is assumption that the two nodes can be connected. The controller itself decides later whether the connection is possible or not. This is default.
 *
 */
#undef EXTENDED_ROUTING_GRAPH

/**
 * Trace on/off.
 */
#undef TRACE_GRAPH


class CAmRouter;

/**
 * A structure used as user data in the graph nodes.
 */
struct am_RoutingNodeData_s
{
	typedef enum:uint8_t {SINK, SOURCE, GATEWAY, CONVERTER} am_NodeDataType_e;
	am_NodeDataType_e type;															//!< data type:sink, source, gateway or converter
#ifdef EXTENDED_ROUTING_GRAPH
	am_CustomConnectionFormat_t inConnectionFormat;									//!< input connection format for sink, source, gateway or converter
	am_CustomConnectionFormat_t outConnectionFormat;								//!< output connection format usually for gateways and converters
#endif
	union
	{
		am_Source_s *source;
		am_Sink_s *sink;
		am_Gateway_s *gateway;
		am_Converter_s *converter;
	} data;																			//!< union pointer to sink, source, gateway or converter

	am_RoutingNodeData_s():type(SINK)
#ifdef EXTENDED_ROUTING_GRAPH
			,inConnectionFormat(CF_UNKNOWN)
			,outConnectionFormat(CF_UNKNOWN)
#endif
	{}

	bool operator==(const am_RoutingNodeData_s & anotherObject) const
	{
		bool result = false;
		if(type==anotherObject.type)
		{
#ifdef EXTENDED_ROUTING_GRAPH
			result = (inConnectionFormat==anotherObject.inConnectionFormat && outConnectionFormat==anotherObject.outConnectionFormat);
#else
			result = true;
#endif
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
			std::cout << "[SINK:" << data.sink->sinkID << ":" << data.sink->name
#ifdef EXTENDED_ROUTING_GRAPH
					<< "(" << inConnectionFormat << "," << outConnectionFormat << ")"
#endif
					<< "]";
		else if(type==SOURCE)
			std::cout << "[SOUR:" << data.source->sourceID << ":" << data.source->name
#ifdef EXTENDED_ROUTING_GRAPH
					 << "(" << inConnectionFormat << "," << outConnectionFormat << ")"
#endif
					<< "]";
		else if(type==GATEWAY)
			std::cout << "[GATE:" << data.gateway->gatewayID << ":" << data.gateway->name
#ifdef EXTENDED_ROUTING_GRAPH
					<< "(" << inConnectionFormat << "," << outConnectionFormat << ")"
#endif
					<< "]";
		else if(type==CONVERTER)
			std::cout << "[CONV:" << data.converter->converterID << ":" << data.converter->name
#ifdef EXTENDED_ROUTING_GRAPH
					<< "(" << inConnectionFormat << "," << outConnectionFormat << ")"
#endif
					<< "]";
	};
#endif
};

#define NodeDataType am_RoutingNodeData_s::am_NodeDataType_e


class CAmControlSender;


/**
 * Implements an autorouting algorithm for connecting sinks and sources via different audio domains.
 */
class CAmRouter
{
	IAmDatabaseHandler* mpDatabaseHandler; 							//!< pointer to database handler
	CAmControlSender* mpControlSender; 								//!< pointer the controlsender - is used to retrieve information for the optimal route
	bool mOnlyFreeConversionNodes;									//!< bool flag whether only disconnected elements should be considered or not
	CAmGraph<am_RoutingNodeData_s, uint16_t> mRoutingGraph;			//!< graph object
	std::vector<CAmNode<am_RoutingNodeData_s>*> mNodeListSources;	//!< vector with pointers to nodes with sources, used for quick access
	std::vector<CAmNode<am_RoutingNodeData_s>*> mNodeListSinks;		//!< vector with pointers to nodes with sinks, used for quick access
	std::vector<CAmNode<am_RoutingNodeData_s>*> mNodeListGateways;	//!< vector with pointers to nodes with gateways, used for quick access
	std::vector<CAmNode<am_RoutingNodeData_s>*> mNodeListConverters;//!< vector with pointers to nodes with converters, used for quick access
	std::map<am_sourceID_t, bool> mNodeListSourceStatus;			//!< vector with flags preventing going through group of nodes during the path search
	std::map<am_sinkID_t, bool> mNodeListSinkStatus;				//!< vector with flags preventing going through group of nodes during the path search
	std::map<am_converterID_t, bool> mNodeListConverterStatus;		//!< vector with flags preventing going through group of nodes during the path search
	std::map<am_gatewayID_t, bool> mNodeListGatewayStatus;			//!< vector with flags preventing going through group of nodes during the path search
#ifdef EXTENDED_ROUTING_GRAPH
	am_Source_s *mpRootSource;										//!< pointer to source
	am_Sink_s *mpRootSink;											//!< pointer to sink

	/*
	 * Methods for getting shortest path from the graph.
	 */
	void getShortestPath(const am_Source_s & aSource, const am_Sink_s & aSink, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultPath);
	void getShortestPath(std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultPath);
	void getShortestPath(const am_Source_s & aSource, const am_Sink_s & aSink,
						  std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath);
	void getShortestPath(std::vector<am_Route_s> & routes, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & nodes);

	/**
	 * Returns a sink node with given sinkID and connection format.
	 *
	 * @param sinkID sink id.
	 * @param connectionFormat connection format.
	 * @return pointer to node or NULL.
	 */

	CAmNode<am_RoutingNodeData_s>* sinkNodeWithID(const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat);
	/**
	 * Returns a source node with given sourceID and connection format.
	 *
	 * @param sourceID source id.
	 * @param connectionFormat connection format.
	 * @return pointer to node or NULL.
	 */
	CAmNode<am_RoutingNodeData_s>* sourceNodeWithID(const am_sourceID_t sourceID, const am_CustomConnectionFormat_t connectionFormat);
#else
	CAmNode<am_RoutingNodeData_s> *mpRootSource;					//!< pointer to source node
	CAmNode<am_RoutingNodeData_s> *mpRootSink;						//!< pointer to sink node

	am_Error_e determineConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmNode<am_RoutingNodeData_s>*> & nodes);
	am_Error_e doConnectionFormatsForPath(am_Route_s & routeObjects,
											 std::vector<CAmNode<am_RoutingNodeData_s>*> & route,
											 std::vector<am_RoutingElement_s>::iterator routingElementIterator,
											 std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator routeIterator);
	void getShortestPath(const CAmNode<am_RoutingNodeData_s> & source,
							const CAmNode<am_RoutingNodeData_s> & destination,
							std::vector<CAmNode<am_RoutingNodeData_s>*> & resultPath);
	void getShortestPath(std::vector<CAmNode<am_RoutingNodeData_s>*> & resultPath);
	void getShortestPath(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink,
							am_Route_s & resultPath, std::vector<CAmNode<am_RoutingNodeData_s>*> & resultNodesPath);
	void getShortestPath(am_Route_s & resultPath, std::vector<CAmNode<am_RoutingNodeData_s>*> & resultNodesPath);
	am_Error_e getAllPaths(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink,
							std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath);
	am_Error_e getAllPaths(std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath);

	/**
	 * Returns a sink node with given sinkID.
	 *
	 * @param sinkID sink id.
	 * @return pointer to node or NULL.
	 */
	CAmNode<am_RoutingNodeData_s>* sinkNodeWithID(const am_sinkID_t sinkID);

	/**
	 * Returns a source node with given sourceID.
	 *
	 * @param sourceID source id.
	 * @return pointer to node or NULL.
	 */
	CAmNode<am_RoutingNodeData_s>* sourceNodeWithID(const am_sourceID_t sourceID);
#endif

	/**
	 * Makes connection between two nodes.
	 *
	 * @param node1.
	 * @param node2.
	 * @param vertexData associated data.
	 * @param weight connection weight used for finding optimal path.
	 */
	void connectNodes(const CAmNode<am_RoutingNodeData_s> & node1,
						const CAmNode<am_RoutingNodeData_s> & node2,
						const am_CustomConnectionFormat_t vertexData,
						const int16_t weight = 1);
	/**
	 * Builds path in a domain from source to sink.
	 *
	 * @param aSource starting point.
	 * @param aSink ending point.
	 */
	bool routeInSameDomain(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink);

	/**
	 * Builds path from source to sink when the source and the sink belongs to different domains.
	 *
	 * @param aSource starting point.
	 * @param aSink ending point.
	 */
	bool routeInAnotherDomain(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink);
	void clear();

	/**
	 * Fills the graph with nodes and connections.
	 *
	 * @param aSource starting point.
	 * @param aSink ending point.
	 */
	void buildGraph(const am_Source_s & aSource, const am_Sink_s & aSink);
    template <class Component> bool isComponentConnected(const Component & comp)
	{
		return mpDatabaseHandler->isComponentConnected(comp);
	}

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

    /**
   	 * Helper methods.
   	 */
    static bool getAllowedFormatsFromConvMatrix(const std::vector<bool> & convertionMatrix,
													  const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
													  const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
													  std::vector<am_CustomConnectionFormat_t> & sourceFormats,
													  std::vector<am_CustomConnectionFormat_t> & sinkFormats);
    static bool getRestrictedOutputFormats(const std::vector<bool> & convertionMatrix,
											  	const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
											  	const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
											  	const am_CustomConnectionFormat_t connectionFormat,
											  	std::vector<am_CustomConnectionFormat_t> & listFormats);

    static void listPossibleConnectionFormats(std::vector<am_CustomConnectionFormat_t> & inListSourceFormats,
												   std::vector<am_CustomConnectionFormat_t> & inListSinkFormats,
												   std::vector<am_CustomConnectionFormat_t> & outConnectionFormats);
};


} /* namespace am */
#endif /* ROUTER_H_ */

