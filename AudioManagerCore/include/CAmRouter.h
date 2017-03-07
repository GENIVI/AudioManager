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
#include <iomanip>
#include <functional>
#include "audiomanagertypes.h"
#include "CAmGraph.h"
#include "CAmDatabaseHandlerMap.h"

namespace am
{

    /**
     * Optimal path search is implemented with graph which contains nodes - sinks, sources, gateways, converters.
     * The nodes are identified by sinkID, sourceID, gatewayID, converterID.
     * A possible connection between two nodes represents the facts that the nodes can be connected with one or more connectionFormats (Node[id=1] ---> Node[id=2]).
     * It is assumption that the two nodes can be connected. The controller itself decides later whether the connection is possible or not.
     *
     */

    /**
     * Trace on/off.
     */
#undef TRACE_GRAPH

    /**
     * Max paths count returned to the controller
     */
#ifndef MAX_ROUTING_PATHS
#define MAX_ROUTING_PATHS 5
#endif
    /**
     * How many times the routing algorithm should look back into domains.
     *
     * 0 - no cycles are allowed
     * 1 - default is one cycle
     * ...
     * UINT_MAX - set this define to UINT_MAX in order to allow cycles.
     *
     */
#ifndef MAX_ALLOWED_DOMAIN_CYCLES
#define MAX_ALLOWED_DOMAIN_CYCLES 1
#endif

    class CAmRouter;

    /**
     * A structure used as user data in the graph nodes.
     */
    struct am_RoutingNodeData_s
    {
        typedef enum
            :int
            {   SINK, SOURCE, GATEWAY, CONVERTER
        } am_NodeDataType_e;
        am_NodeDataType_e type;										//!< data type:sink, source, gateway or converter
        union
        {
            am_Source_s *source;
            am_Sink_s *sink;
            am_Gateway_s *gateway;
            am_Converter_s *converter;
        } data;													//!< union pointer to sink, source, gateway or converter

        am_RoutingNodeData_s() :
                type(SINK)
        {
        }

        bool operator==(const am_RoutingNodeData_s & anotherObject) const
        {
            bool result = false;
            if (type == anotherObject.type)
            {
                result = true;
                if (type == SINK)
                    result &= (data.sink->sinkID == anotherObject.data.sink->sinkID);
                else if (type == SOURCE)
                    result &= (data.source->sourceID == anotherObject.data.source->sourceID);
                else if (type == GATEWAY)
                    result &= (data.gateway->gatewayID == anotherObject.data.gateway->gatewayID);
                else if (type == CONVERTER)
                    result &= (data.converter->converterID == anotherObject.data.converter->converterID);
            }
            return result;
        }
        ;

#ifdef TRACE_GRAPH
#define COUT_NODE(HEAD, NAME, ID) \
	std::cout << HEAD << "(" << std::setfill('0') << std::setw(4) << ID << " " << NAME << ")";

        void trace() const
        {
            if(type==SINK)
            COUT_NODE("SI", data.sink->name, data.sink->sinkID )
            else if(type==SOURCE)
            COUT_NODE("SO", data.source->name, data.source->sourceID )
            else if(type==GATEWAY)
            COUT_NODE("GA", data.gateway->name, data.gateway->gatewayID )
            else if(type==CONVERTER)
            COUT_NODE("CO", data.converter->name, data.converter->converterID )
        };
#endif

        am_domainID_t domainID() const
        {
            if (type == SINK)
                return data.sink->domainID;
            else if (type == SOURCE)
                return data.source->domainID;
            else if (type == GATEWAY)
                return data.gateway->controlDomainID;
            else if (type == CONVERTER)
                return data.converter->domainID;
            return 0;
        }
        ;
    };

    typedef am_RoutingNodeData_s::am_NodeDataType_e CAmNodeDataType;
    typedef CAmNode<am_RoutingNodeData_s> CAmRoutingNode;
    typedef CAmGraph<am_RoutingNodeData_s, uint16_t> CAmRoutingGraph;
    typedef CAmVertex<am_RoutingNodeData_s, uint16_t> CAmRoutingVertex;
    typedef std::list<CAmRoutingVertex> CAmRoutingListVertices;
    typedef std::vector<CAmRoutingListVertices*> CAmRoutingVertexReferenceList;

    class CAmControlSender;

    /**
     * Implements autorouting algorithm for connecting sinks and sources via different audio domains.
     */
    class CAmRouter: public CAmDatabaseHandlerMap::AmDatabaseObserverCallbacks
    {
        IAmDatabaseHandler* mpDatabaseHandler; 									    //!< pointer to database handler
        CAmControlSender* mpControlSender;                                          //!< pointer the controlsender - is used to retrieve information for the optimal route
        bool mUpdateGraphNodesAction;                                               //!< Flag which marks whether the graph should be rebuild
        unsigned mMaxAllowedCycles;												    //!< max allowed cycles, default is 1
        unsigned mMaxPathCount;							                            //!< max paths count returned to the controller, default is 5
        CAmRoutingGraph mRoutingGraph;											    //!< graph object
        std::map<am_domainID_t, std::vector<CAmRoutingNode*>> mNodeListSources;	    //!< map with pointers to nodes with sources, used for quick access
        std::map<am_domainID_t, std::vector<CAmRoutingNode*>> mNodeListSinks;	    //!< map with pointers to nodes with sinks, used for quick access
        std::map<am_domainID_t, std::vector<CAmRoutingNode*>> mNodeListGateways;	//!< map with pointers to nodes with gateways, used for quick access
        std::map<am_domainID_t, std::vector<CAmRoutingNode*>> mNodeListConverters;	//!< map with pointers to nodes with converters, used for quick access

        /**
         * Check whether given converter or gateway has been connected.
         *
         * @param comp  converter or gateway .
         */
        template<class Component> bool isComponentConnected(const Component & comp)
        {
            return mpDatabaseHandler->isComponentConnected(comp);
        }

        /**
         * Connect all converters to its sink and sources if possible.
         *
         */
        void constructConverterConnections();

        /**
         * Connect all gateways to its sink and sources if possible.
         *
         */
        void constructGatewayConnections();

        /**
         * Connect all sources to the sinks if possible.
         *
         */
        void constructSourceSinkConnections();

        /**
         * Construct list with all vertices
         */
        void getVerticesForNode(const CAmRoutingNode & node, CAmRoutingListVertices & list);

        /**
         * Construct list with all vertices from given source.
         */
        void getVerticesForSource(const CAmRoutingNode & node, CAmRoutingListVertices & list);

        /**
         * Construct list with all vertices from given sink.
         */
        void getVerticesForSink(const CAmRoutingNode & node, CAmRoutingListVertices & list);

        /**
         * Construct list with all vertices from given converter.
         */
        void getVerticesForConverter(const CAmRoutingNode & node, CAmRoutingListVertices & list);

        /**
         * Construct list with all vertices from given gateway.
         */
        void getVerticesForGateway(const CAmRoutingNode & node, CAmRoutingListVertices & list);

        /**
         * Connection format permutations.
         *
         * @return E_OK on success(1 or more paths),  E_NOT_POSSIBLE if the CF couldn't be matached or E_UNKNOWN in any other error case.
         */
        am_Error_e determineConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmRoutingNode*> & nodes, std::vector<am_Route_s> & result);
        am_Error_e doConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmRoutingNode*> & route,
                std::vector<am_RoutingElement_s>::iterator routingElementIterator, std::vector<CAmRoutingNode*>::iterator routeIterator,
                std::vector<am_Route_s> & result);
        am_Error_e cfPermutationsForPath(am_Route_s shortestRoute, std::vector<CAmRoutingNode*> resultNodesPath, std::vector<am_Route_s>& resultPath);

        /**
         * Helper method.
         */
        static int insertPostion(const std::vector<CAmRoutingNode*>& path, const std::vector<std::vector<CAmRoutingNode*> >& nodes);

    public:
        CAmRouter(IAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iSender);
        ~CAmRouter();

        unsigned getMaxAllowedCycles()
        {
            return mMaxAllowedCycles;
        }
        void setMaxAllowedCycles(unsigned count)
        {
            mMaxAllowedCycles = count;
        }

        unsigned getMaxPathCount()
        {
            return mMaxPathCount;
        }
        void setMaxPathCount(unsigned count)
        {
            mMaxPathCount = count;
        }

        bool getUpdateGraphNodesAction()
        {
            return mUpdateGraphNodesAction;
        }

        /**
         * Find first mMaxPathCount paths between given source and sink. This method will call the method load() if the parameter mUpdateGraphNodesAction is set which will rebuild the graph.
         *
         * @param onlyfree only disconnected elements should be included or not.
         * @param sourceID start point.
         * @param sinkID end point.
         * @param returnList list with all possible paths
         * @return E_OK on success(1 or more paths),  E_NOT_POSSIBLE if the CF couldn't be matached or E_UNKNOWN in any other error case.
         */
        am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);
        am_Error_e getRoute(const bool onlyfree, const am_Source_s & source, const am_Sink_s & sink, std::vector<am_Route_s> & listRoutes);

        /**
         * Find first mMaxPathCount paths between given source and sink after the nodes have been loaded. This method doesn't call load().
         *
         * @param onlyfree only disconnected elements should be included or not.
         * @param sourceID start point.
         * @param sinkID end point.
         * @param returnList list with all possible paths
         * @return E_OK on success(1 or more paths),  E_NOT_POSSIBLE if the CF couldn't be matached or E_UNKNOWN in any other error case.
         */
        am_Error_e getRouteFromLoadedNodes(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList);
        am_Error_e getRouteFromLoadedNodes(const bool onlyfree, const am_Source_s & aSource, const am_Sink_s & aSink, std::vector<am_Route_s> & listRoutes);

        /**
         * Find first mMaxPathCount paths between given source and sink. This method doesn't call load().
         *
         * @param onlyfree only disconnected elements should be included or not.
         * @param cycles allowed domain cycles.
         * @param maxPathCount max count of returned paths.
         * @param source start point.
         * @param sink end point.
         * @param returnList list with all possible paths.
         * @return E_OK on success(1 or more paths),  E_NOT_POSSIBLE if the CF couldn't be matached or E_UNKNOWN in any other error case.
         */
        am_Error_e getFirstNShortestPaths(const bool onlyfree, const unsigned cycles, const unsigned maxPathCount, CAmRoutingNode & source,
                CAmRoutingNode & sink, std::vector<am_Route_s> & resultPath);

        /**
         * Find the shortest path between given source and sink. This method doesn't call load().
         * It goes through all possible paths and returns the shortest of them.
         *
         * @param source start point.
         * @param sink end point.
         * @param returnList list with the connection format permutations of the shortest path.
         * @return E_OK on success(1 or more paths),  E_NOT_POSSIBLE if the CF couldn't be matached or E_UNKNOWN in any other error case.
         */
        am_Error_e getShortestPath(CAmRoutingNode & source, CAmRoutingNode & sink, std::vector<am_Route_s> & resultPath);

        static bool getAllowedFormatsFromConvMatrix(const std::vector<bool> & convertionMatrix,
                const std::vector<am_CustomConnectionFormat_t> & listSourceFormats, const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
                std::vector<am_CustomConnectionFormat_t> & sourceFormats, std::vector<am_CustomConnectionFormat_t> & sinkFormats);
        static void listPossibleConnectionFormats(std::vector<am_CustomConnectionFormat_t> & inListSourceFormats,
                std::vector<am_CustomConnectionFormat_t> & inListSinkFormats, std::vector<am_CustomConnectionFormat_t> & outListFormats);
        static bool getRestrictedOutputFormats(const std::vector<bool> & convertionMatrix, const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
                const std::vector<am_CustomConnectionFormat_t> & listSinkFormats, const am_CustomConnectionFormat_t connectionFormat,
                std::vector<am_CustomConnectionFormat_t> & listFormats);
        static am_Error_e getSourceSinkPossibleConnectionFormats(std::vector<CAmRoutingNode*>::iterator iteratorSource,
                std::vector<CAmRoutingNode*>::iterator iteratorSink, std::vector<am_CustomConnectionFormat_t> & outConnectionFormats);

        static bool shouldGoInDomain(const std::vector<am_domainID_t> & visitedDomains, const am_domainID_t nodeDomainID, const unsigned maxCyclesNumber);
        bool shouldGoInDomain(const std::vector<am_domainID_t> & visitedDomains, const am_domainID_t nodeDomainID);
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

        void load();
        void clear();

        /**
         * DEPRECATED!
         */
    public:
        am_Error_e getAllPaths(CAmRoutingNode & aSource, CAmRoutingNode & aSink, std::vector<am_Route_s> & resultPath,
                std::vector<std::vector<CAmRoutingNode*>> & resultNodesPath, const bool includeCycles = false,
                const bool onlyFree = false)
                        __attribute__((deprecated("You should use am_Error_e getFirstNShortestPaths(const bool onlyFree, CAmRoutingNode &, CAmRoutingNode &, std::vector<am_Route_s> &) instead!")));
    };
} /* namespace am */
#endif /* ROUTER_H_ */

