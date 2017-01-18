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
 * \file CAmRouter.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>
#include "CAmRouter.h"
#include "IAmDatabaseHandler.h"
#include "CAmControlSender.h"
#include "CAmDltWrapper.h"



namespace am {


template <class X> void getMergeConnectionFormats(const X * element,
							 const am_CustomConnectionFormat_t connectionFormat,
							 const std::vector<am_CustomConnectionFormat_t> & listConnectionFormats,
							 std::vector<am_CustomConnectionFormat_t> & outListMergeConnectionFormats)
{
	std::vector<am_CustomConnectionFormat_t> listRestrictedConnectionFormats;
	CAmRouter::getRestrictedOutputFormats(element->convertionMatrix,
										  element->listSourceFormats,
										  element->listSinkFormats,
										  connectionFormat,
										  listRestrictedConnectionFormats);
	std::sort(listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end()); //todo: this might be not needed if we use strictly sorted input
	std::insert_iterator<std::vector<am_CustomConnectionFormat_t> > inserter(outListMergeConnectionFormats, outListMergeConnectionFormats.begin());
	set_intersection(listConnectionFormats.begin(), listConnectionFormats.end(), listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end(), inserter);
}


CAmRouter::CAmRouter(IAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iSender) :
        mpDatabaseHandler(iDatabaseHandler), //
        mpControlSender(iSender),
        mOnlyFreeConversionNodes(false),
		mRoutingGraph(),
		mNodeListSources(),
		mNodeListSinks(),
		mNodeListGateways(),
		mNodeListConverters()
{
    assert(mpDatabaseHandler);
    assert(mpControlSender);
}

CAmRouter::~CAmRouter()
{
}

/**
 * returns the best route between a source and a sink
 * @param onlyfree if true only free gateways are used
 * @param sourceID
 * @param sinkID
 * @param returnList this list contains a set of routes
 * @return E_OK in case of success
 */
am_Error_e CAmRouter::getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
    returnList.clear();
    am_Source_s source;
    am_Sink_s sink;
    am_Error_e error = mpDatabaseHandler->getSourceInfoDB(sourceID, source);
    if(error!=E_OK)
    	return error;
    error = mpDatabaseHandler->getSinkInfoDB(sinkID, sink);
    if(error!=E_OK)
        return error;
    error = getRoute(onlyfree, source, sink, returnList);
    return error;
}


am_Error_e CAmRouter::getRoute(const bool onlyfree, const am_Source_s & aSource, const am_Sink_s & aSink, std::vector<am_Route_s> & listRoutes)
{
	am_Error_e error;
	load(onlyfree);

	CAmRoutingNode* pRootSource = sourceNodeWithID(aSource.sourceID);
	CAmRoutingNode* pRootSink = sinkNodeWithID(aSink.sinkID);

	assert(pRootSource);
	assert(pRootSink);

#ifdef TRACE_GRAPH
	mRoutingGraph.trace([&](const CAmRoutingNode & node, const std::vector<CAmVertex<am_RoutingNodeData_s,uint16_t>*> & list) {
				std::cout << "Node " << node.getIndex() << ":";
				((CAmRoutingNode &)node).getData().trace();
				std::cout << "-->[";
				int count = 0;
				std::for_each(list.begin(), list.end(), [&](const CAmVertex<am_RoutingNodeData_s,uint16_t>* refVertex){
					am::CAmNode<am::am_RoutingNodeData_s>* data = refVertex->getNode();
					if(count>0)
						std::cout << ", ";
					std::cout << "Node " << data->getIndex() << ":";
					data->getData().trace();
					count++;
				});
				std::cout << "]" << std::endl;
			});
#endif

	std::vector<std::vector<CAmRoutingNode*>> pathNodes;
	error = getAllPaths(*pRootSource, *pRootSink, listRoutes, pathNodes);
	return error;
}

void CAmRouter::load(const bool onlyFree)
{
	clear();
	mOnlyFreeConversionNodes = onlyFree;

	am_RoutingNodeData_s nodeDataSrc;
	nodeDataSrc.type = CAmNodeDataType::SOURCE;
	mpDatabaseHandler->enumerateSources([&](const am_Source_s & obj){
		nodeDataSrc.data.source = (am_Source_s*)&obj;
		mNodeListSources[nodeDataSrc.data.source->domainID].push_back(&mRoutingGraph.addNode(nodeDataSrc));
		});
	am_RoutingNodeData_s nodeDataSink;
	nodeDataSink.type = CAmNodeDataType::SINK;
	mpDatabaseHandler->enumerateSinks([&](const am_Sink_s & obj){
		nodeDataSink.data.sink = (am_Sink_s*)&obj;
		mNodeListSinks[nodeDataSink.data.sink->domainID].push_back(&mRoutingGraph.addNode(nodeDataSink));
		});
	am_RoutingNodeData_s nodeDataGateway;
	nodeDataGateway.type = CAmNodeDataType::GATEWAY;
	mpDatabaseHandler->enumerateGateways([&](const am_Gateway_s & obj){
		nodeDataGateway.data.gateway = (am_Gateway_s*)&obj;
		mNodeListGateways[nodeDataGateway.data.gateway->controlDomainID].push_back(&mRoutingGraph.addNode(nodeDataGateway));
	});
	am_RoutingNodeData_s nodeDataConverter;
	nodeDataConverter.type = CAmNodeDataType::CONVERTER;
	mpDatabaseHandler->enumerateConverters([&](const am_Converter_s & obj){
		nodeDataConverter.data.converter = (am_Converter_s*)&obj;
		mNodeListConverters[nodeDataConverter.data.converter->domainID].push_back(&mRoutingGraph.addNode(nodeDataConverter));
	});

#ifdef ROUTING_BUILD_CONNECTIONS
	constructConverterConnections();
	constructGatewayConnections();
	constructSourceSinkConnections();
#endif
}

void CAmRouter::clear()
{
	mRoutingGraph.clear();
	mNodeListSources.clear();
	mNodeListSinks.clear();
	mNodeListGateways.clear();
	mNodeListConverters.clear();
}

CAmRoutingNode* CAmRouter::sinkNodeWithID(const am_sinkID_t sinkID)
{
	CAmRoutingNode* result = NULL;
	for(auto it = mNodeListSinks.begin(); it!=mNodeListSinks.end(); it++)
	{
		result = sinkNodeWithID(sinkID, it->first);
		if(result)
			return result;
	}
	return result;
}

CAmRoutingNode* CAmRouter::sinkNodeWithID(const am_sinkID_t sinkID, const am_domainID_t domainID)
{
	CAmRoutingNode* result = NULL;
	std::vector<CAmRoutingNode*> & value = mNodeListSinks[domainID];
	auto iter = std::find_if(value.begin(), value.end(), [sinkID](CAmRoutingNode* node){
		return node->getData().data.sink->sinkID==sinkID;
	});
	if(iter!=value.end())
		result = *iter;
	return result;
}

CAmRoutingNode* CAmRouter::sourceNodeWithID(const am_sourceID_t sourceID)
{
	CAmRoutingNode* result = NULL;
	for(auto it = mNodeListSources.begin(); it!=mNodeListSources.end(); it++)
	{
		result = sourceNodeWithID(sourceID, it->first);
		if(result)
			return result;
	}
	return result;
}

CAmRoutingNode* CAmRouter::sourceNodeWithID(const am_sourceID_t sourceID, const am_domainID_t domainID)
{
	CAmRoutingNode* result = NULL;
	std::vector<CAmRoutingNode*> & value = mNodeListSources[domainID];
	auto iter = std::find_if(value.begin(), value.end(), [sourceID](CAmRoutingNode* node){
		return node->getData().data.source->sourceID==sourceID;
	});
	if(iter!=value.end())
		result = *iter;
	return result;
}

CAmRoutingNode* CAmRouter::converterNodeWithSinkID(const am_sinkID_t sinkID, const am_domainID_t domainID)
{
	CAmRoutingNode* result = NULL;
	std::vector<CAmRoutingNode*> & value = mNodeListConverters[domainID];
	auto iter = std::find_if(value.begin(), value.end(), [sinkID](CAmRoutingNode* node){
		return node->getData().data.converter->sinkID==sinkID;
	});
	if(iter!=value.end())
		result = *iter;
	return result;
}

CAmRoutingNode* CAmRouter::gatewayNodeWithSinkID(const am_sinkID_t sinkID)
{
	for(auto it = mNodeListGateways.begin(); it!=mNodeListGateways.end(); it++)
	{
		std::vector<CAmRoutingNode*> & value = it->second;
		auto iter = std::find_if(value.begin(), value.end(), [sinkID](CAmRoutingNode* node){
			return node->getData().data.gateway->sinkID==sinkID;
		});
		if(iter!=value.end())
			return *iter;
	}
	return NULL;
}

#ifdef ROUTING_BUILD_CONNECTIONS

void CAmRouter::constructSourceSinkConnections()
{
	std::vector<am_CustomConnectionFormat_t> intersection;
	for(auto itSrc = mNodeListSources.begin(); itSrc!=mNodeListSources.end(); itSrc++)
	{
		for(auto it = itSrc->second.begin(); it!=itSrc->second.end(); it++)
		{
			CAmRoutingNode* srcNode = *it;
			am_RoutingNodeData_s & srcNodeData = srcNode->getData();
			am_Source_s * source = srcNodeData.data.source;
			for(auto itSink = mNodeListSinks[itSrc->first].begin(); itSink!=mNodeListSinks[itSrc->first].end(); itSink++)
			{
				CAmRoutingNode* sinkNode = *itSink;
				am_RoutingNodeData_s & sinkNodeData = sinkNode->getData();
				am_Sink_s * sink = sinkNodeData.data.sink;

				intersection.clear();
				//Check whether the hidden sink formats match the source formats...
				listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, intersection);
				if(intersection.size()>0)//OK  match source -> sink
				{
					mRoutingGraph.connectNodes(*srcNode, *sinkNode, CF_UNKNOWN, 1);
				}
			}
		}
	}
}

void CAmRouter::constructGatewayConnections()
{
	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
	for(auto iter = mNodeListGateways.begin(); iter!=mNodeListGateways.end(); iter++)
	{
		for(auto it = iter->second.begin(); it!=iter->second.end(); it++)
		{
			CAmRoutingNode* gatewayNode = *it;
			am_RoutingNodeData_s & gatewayNodeData = gatewayNode->getData();
			am_Gateway_s * gateway = gatewayNodeData.data.gateway;
			//Get only gateways with end point in current source domain
			if(!mOnlyFreeConversionNodes || !isComponentConnected(*gateway))
			{
				//Get the sink connected to the gateway...
				CAmRoutingNode *gatewaySinkNode = this->sinkNodeWithID(gateway->sinkID, gateway->domainSinkID);
				if(gatewaySinkNode)
				{
					am_RoutingNodeData_s & gatewaySinkData = gatewaySinkNode->getData();
					//Check whether the hidden sink formats match the source formats...
					sourceFormats.clear();
					sinkFormats.clear();
					if(getAllowedFormatsFromConvMatrix(gateway->convertionMatrix, gateway->listSourceFormats, gateway->listSinkFormats, sourceFormats, sinkFormats))
					{
						CAmRoutingNode *gatewaySourceNode = this->sourceNodeWithID(gateway->sourceID, gateway->domainSourceID);
						if(gatewaySourceNode)
						{
							//Connections hidden_sink->gateway->hidden_source
							mRoutingGraph.connectNodes(*gatewaySinkNode, *gatewayNode, CF_UNKNOWN, 1);
							mRoutingGraph.connectNodes(*gatewayNode, *gatewaySourceNode, CF_UNKNOWN, 1);
						}
					}
				}
			}
		}
	}
}

void CAmRouter::constructConverterConnections()
{
	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;

	for(auto iter = mNodeListConverters.begin(); iter!=mNodeListConverters.end(); iter++)
	{
		for(auto it = iter->second.begin(); it!=iter->second.end(); it++)
		{
			CAmRoutingNode* converterNode = *it;
			am_RoutingNodeData_s & converterNodeData = converterNode->getData();
			am_Converter_s * converter = converterNodeData.data.converter;
			//Get only converters with end point in current source domain
			if(!mOnlyFreeConversionNodes || !isComponentConnected(*converter))
			{
				//Get the sink connected to the converter...
				CAmRoutingNode *converterSinkNode = this->sinkNodeWithID(converter->sinkID, converter->domainID);
				if(converterSinkNode)
				{
					am_RoutingNodeData_s & converterSinkData = converterSinkNode->getData();
					//Check whether the hidden sink formats match the source formats...
					sourceFormats.clear();
					sinkFormats.clear();
					if(getAllowedFormatsFromConvMatrix(converter->convertionMatrix, converter->listSourceFormats, converter->listSinkFormats, sourceFormats, sinkFormats))
					{
						CAmRoutingNode *converterSourceNode = this->sourceNodeWithID(converter->sourceID, converter->domainID);
						if(converterSourceNode)
						{
							//Connections hidden_sink->converter->hidden_source
							mRoutingGraph.connectNodes(*converterSinkNode, *converterNode, CF_UNKNOWN, 1);
							mRoutingGraph.connectNodes(*converterNode, *converterSourceNode, CF_UNKNOWN, 1);
						}
					}
				}
			}
		}
	}
}
#else

void CAmRouter::getVerticesForSource(const CAmRoutingNode & node, CAmRoutingListVertices & list)
{
	am_RoutingNodeData_s & srcNodeData = ((CAmRoutingNode*)&node)->getData();
	std::vector<am_CustomConnectionFormat_t> intersection;
	am_Source_s * source = srcNodeData.data.source;
	std::vector<CAmRoutingNode*> & sinks = mNodeListSinks[source->domainID];
	for(auto itSink = sinks.begin(); itSink!=sinks.end(); itSink++)
	{
		CAmRoutingNode* sinkNode = *itSink;
		am_RoutingNodeData_s & sinkNodeData = sinkNode->getData();
		am_Sink_s * sink = sinkNodeData.data.sink;

		intersection.clear();
		//Check whether the hidden sink formats match the source formats...
		listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, intersection);
		if(intersection.size()>0)//OK  match source -> sink
		{
			list.emplace_back(sinkNode, CF_UNKNOWN, 1);
		}
	}
}

void CAmRouter::getVerticesForSink(const CAmRoutingNode & node, CAmRoutingListVertices & list)
{
	am_RoutingNodeData_s & sinkNodeData = ((CAmRoutingNode*)&node)->getData();
	std::vector<am_CustomConnectionFormat_t> intersection;
	am_Sink_s * sink = sinkNodeData.data.sink;

	CAmRoutingNode *converterNode = converterNodeWithSinkID(sink->sinkID, sink->domainID);
	if(converterNode)
	{
		std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
		am_RoutingNodeData_s & converterData = converterNode->getData();
		am_Converter_s * converter = converterData.data.converter;
		if(!mOnlyFreeConversionNodes || !isComponentConnected(*converter))
		{
			if(getAllowedFormatsFromConvMatrix(converter->convertionMatrix, converter->listSourceFormats, converter->listSinkFormats, sourceFormats, sinkFormats))
				list.emplace_back(converterNode, CF_UNKNOWN, 1);
		}
	}
	else
	{
		std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
		CAmRoutingNode *gatewayNode = gatewayNodeWithSinkID(sink->sinkID);
		if(gatewayNode)
		{
			std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
			am_RoutingNodeData_s & gatewayData = gatewayNode->getData();
			am_Gateway_s * gateway = gatewayData.data.gateway;
			if(!mOnlyFreeConversionNodes || !isComponentConnected(*gateway))
			{
				if(getAllowedFormatsFromConvMatrix(gateway->convertionMatrix, gateway->listSourceFormats, gateway->listSinkFormats, sourceFormats, sinkFormats))
					list.emplace_back(gatewayNode, CF_UNKNOWN, 1);
			}
		}
	}

}

void CAmRouter::getVerticesForConverter(const CAmRoutingNode & node, CAmRoutingListVertices & list)
{
	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
	am_RoutingNodeData_s & converterNodeData = ((CAmRoutingNode*)&node)->getData();
	am_Converter_s * converter = converterNodeData.data.converter;
	//Get only converters with end point in current source domain
	if(getAllowedFormatsFromConvMatrix(converter->convertionMatrix, converter->listSourceFormats, converter->listSinkFormats, sourceFormats, sinkFormats))
	{
		CAmRoutingNode *converterSourceNode = this->sourceNodeWithID(converter->sourceID, converter->domainID);
		if(converterSourceNode)
		{
			list.emplace_back(converterSourceNode, CF_UNKNOWN, 1);
		}
	}
}

void CAmRouter::getVerticesForGateway(const CAmRoutingNode & node, CAmRoutingListVertices & list)
{
	am_RoutingNodeData_s & gatewayNodeData = ((CAmRoutingNode*)&node)->getData();
	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
	am_Gateway_s * gateway = gatewayNodeData.data.gateway;
	if(getAllowedFormatsFromConvMatrix(gateway->convertionMatrix, gateway->listSourceFormats, gateway->listSinkFormats, sourceFormats, sinkFormats))
	{
		CAmRoutingNode *gatewaySourceNode = this->sourceNodeWithID(gateway->sourceID, gateway->domainSourceID);
		if(gatewaySourceNode)
		{
			//Connections hidden_sink->gateway->hidden_source
			list.emplace_back(gatewaySourceNode, CF_UNKNOWN, 1);
		}
	}
}

void CAmRouter::getVerticesForNode(
		const CAmRoutingNode & node,
		CAmRoutingListVertices & list
		)
{
	am_RoutingNodeData_s & nodeData = ((CAmRoutingNode*)&node)->getData();
	if(nodeData.type==CAmNodeDataType::SOURCE)
	{
		getVerticesForSource(node, list);
	}
	else if(nodeData.type==CAmNodeDataType::SINK)
	{
		getVerticesForSink(node, list);
	}
	else if(nodeData.type==CAmNodeDataType::CONVERTER)
	{
		getVerticesForConverter(node, list);
	}
	else if(nodeData.type==CAmNodeDataType::GATEWAY)
	{
		getVerticesForGateway(node, list);
	}
}

#endif

am_Error_e CAmRouter::determineConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmRoutingNode*> & nodes, std::vector<am_Route_s> & result)
{
	std::vector<am_RoutingElement_s>::iterator routingElementIterator = routeObjects.route.begin();
	std::vector<CAmRoutingNode*>::iterator nodeIterator = nodes.begin();
	if( routingElementIterator!= routeObjects.route.end() && nodeIterator!=nodes.end() )
		return doConnectionFormatsForPath(routeObjects, nodes, routingElementIterator, nodeIterator, result);
	return E_OK;
}

am_Error_e CAmRouter::doConnectionFormatsForPath(am_Route_s & routeObjects,
													 std::vector<CAmRoutingNode*> & nodes,
													 std::vector<am_RoutingElement_s>::iterator routingElementIterator,
													 std::vector<CAmRoutingNode*>::iterator nodeIterator,
													 std::vector<am_Route_s> & result)
{
    am_Error_e returnError = E_NOT_POSSIBLE;
    std::vector<am_CustomConnectionFormat_t> listConnectionFormats;
    std::vector<am_CustomConnectionFormat_t> listMergeConnectionFormats;

    std::vector<CAmRoutingNode*>::iterator  currentNodeIterator = nodeIterator;
    std::vector<am_RoutingElement_s>::iterator  currentRoutingElementIterator = routingElementIterator;

    if (currentRoutingElementIterator!=routeObjects.route.begin())
    {
    	std::vector<am_CustomConnectionFormat_t> listConnectionFormats;
    	std::vector<am_RoutingElement_s>::iterator tempIterator = (currentRoutingElementIterator-1);
      	CAmRoutingNode * currentNode = *currentNodeIterator;
      	if((returnError = getSourceSinkPossibleConnectionFormats(currentNodeIterator+1, currentNodeIterator+2, listConnectionFormats))!=E_OK)
      		return returnError;

		if(currentNode->getData().type==CAmNodeDataType::GATEWAY)
		{
			am_Gateway_s *gateway = currentNode->getData().data.gateway;
			getMergeConnectionFormats(gateway, tempIterator->connectionFormat, listConnectionFormats, listMergeConnectionFormats);
		}
		else if(currentNode->getData().type==CAmNodeDataType::CONVERTER)
		{
			am_Converter_s *converter = currentNode->getData().data.converter;
			getMergeConnectionFormats(converter, tempIterator->connectionFormat, listConnectionFormats, listMergeConnectionFormats);
		}
		else
			return (E_UNKNOWN);
		currentNodeIterator+=3;
    }
    else
    {
    	CAmRoutingNode * currentNode = *currentNodeIterator;
    	if(currentNode->getData().type!=CAmNodeDataType::SOURCE)
    		return (E_UNKNOWN);
    	currentNodeIterator++;

    	if(currentNodeIterator==nodes.end())
			return (E_UNKNOWN);

		CAmRoutingNode * nodeSink = *currentNodeIterator;
		if(nodeSink->getData().type!=CAmNodeDataType::SINK)
			return (E_UNKNOWN);

		am_Source_s *source = currentNode->getData().data.source;
		am_Sink_s *sink = nodeSink->getData().data.sink;
		listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, listMergeConnectionFormats);
		currentNodeIterator+=1; //now we are on the next converter/gateway
    }
    //let the controller decide:
    std::vector<am_CustomConnectionFormat_t> listPriorityConnectionFormats;
    if((returnError = mpControlSender->getConnectionFormatChoice(currentRoutingElementIterator->sourceID, currentRoutingElementIterator->sinkID, routeObjects,
    										  listMergeConnectionFormats, listPriorityConnectionFormats))!= E_OK)
    	return (returnError);

    //we have the list sorted after priors - now we try one after the other with the next part of the route
	std::vector<am_CustomConnectionFormat_t>::iterator connectionFormatIterator = listPriorityConnectionFormats.begin();
	//here we need to check if we are at the end and stop
	 std::vector<am_RoutingElement_s>::iterator nextIterator = currentRoutingElementIterator + 1;//next pair source and sink
	if (nextIterator == routeObjects.route.end())
	{
		if (!listPriorityConnectionFormats.empty())
		{
			for (; connectionFormatIterator != listPriorityConnectionFormats.end(); ++connectionFormatIterator)
			{
				currentRoutingElementIterator->connectionFormat = *connectionFormatIterator;
				result.push_back(routeObjects);
			}
			return (E_OK);
		}
		else
			return (E_NOT_POSSIBLE);
	}
	else
	{
		for (; connectionFormatIterator != listPriorityConnectionFormats.end(); ++connectionFormatIterator)
		{
			currentRoutingElementIterator->connectionFormat = *connectionFormatIterator;
			doConnectionFormatsForPath(routeObjects, nodes, nextIterator, currentNodeIterator, result);
		}
		return (E_OK);
	}
}

#ifdef ROUTING_BUILD_CONNECTIONS

void CAmRouter::getShortestPath(const CAmRoutingNode & source,
											   const CAmRoutingNode & destination,
											   std::vector<CAmRoutingNode*> & resultPath)
{
	mRoutingGraph.getShortestPath(source, destination, resultPath);
}

void CAmRouter::getShortestPath(CAmRoutingNode & aSource, CAmRoutingNode & aSink,
									am_Route_s & resultPath, std::vector<CAmRoutingNode*> & resultNodesPath)
{
	am_RoutingElement_s * element;
	am_RoutingNodeData_s & sinkNodeData = aSink.getData();
	am_RoutingNodeData_s & sourceNodeData = aSource.getData();
	resultPath.sinkID = sinkNodeData.data.sink->sinkID;
	resultPath.sourceID = sourceNodeData.data.source->sourceID;

	std::function<void(const am_GraphPathPosition_e, CAmRoutingNode &)> cb = [&](const am_GraphPathPosition_e, CAmRoutingNode & object)
	{
		resultNodesPath.insert(resultNodesPath.begin(), (CAmRoutingNode*)&object);
		am_RoutingNodeData_s & routingData =  object.getData();
		if(routingData.type==CAmNodeDataType::SINK)
		{
			auto iter = resultPath.route.emplace(resultPath.route.begin());
			element = &(*iter);
			element->domainID = routingData.data.sink->domainID;
			element->sinkID = routingData.data.sink->sinkID;
			element->connectionFormat = CF_UNKNOWN;
		}
		else if(routingData.type==CAmNodeDataType::SOURCE)
		{
			element->domainID = routingData.data.source->domainID;
			element->sourceID = routingData.data.source->sourceID;
			element->connectionFormat = CF_UNKNOWN;
		}
	};
	mRoutingGraph.getShortestPath(aSource, aSink, cb);
}

#endif

am_Error_e CAmRouter::getAllPaths(CAmRoutingNode & aSource,
									 CAmRoutingNode & aSink,
									 std::vector<am_Route_s> & resultPath,
									 std::vector<std::vector<CAmRoutingNode*>> & resultNodesPath,
									 const bool includeCycles)
{
#ifndef ROUTING_BUILD_CONNECTIONS
	bool cycles = false;
#else
	bool cycles = includeCycles;
#endif
	if(((CAmRoutingNode*)&aSource)->getData().type!=CAmNodeDataType::SOURCE ||
		((CAmRoutingNode*)&aSink)->getData().type!=CAmNodeDataType::SINK)
		return E_NOT_POSSIBLE;

	uint8_t errorsCount = 0, successCount = 0;
	generateAllPaths(aSource, aSink, cycles, [&](const std::vector<CAmRoutingNode*> & path) {
		resultNodesPath.push_back(path);
		am_Route_s nextRoute;
		nextRoute.sinkID = aSink.getData().data.sink->sinkID;
		nextRoute.sourceID = aSource.getData().data.source->sourceID;
		am_RoutingElement_s * element;
		for(auto it = path.begin(); it!=path.end(); it++)
		{
			am_RoutingNodeData_s & routingData =  (*it)->getData();
			if(routingData.type==CAmNodeDataType::SOURCE)
			{
				auto iter = nextRoute.route.emplace(nextRoute.route.end());
				element = &(*iter);
				element->domainID = routingData.data.source->domainID;
				element->sourceID = routingData.data.source->sourceID;
				element->connectionFormat = CF_UNKNOWN;
			}
			else if(routingData.type==CAmNodeDataType::SINK)
			{
				element->domainID = routingData.data.sink->domainID;
				element->sinkID = routingData.data.sink->sinkID;
				element->connectionFormat = CF_UNKNOWN;
			}
		}
		std::vector<am_Route_s> result;
		am_Error_e err = determineConnectionFormatsForPath(nextRoute, (std::vector<CAmRoutingNode*> &)path, result);
		if(err==E_UNKNOWN)
		{
			errorsCount++;
#ifdef TRACE_GRAPH
			std::cout<<"Error by determining connection formats for path from source:"<<nextRoute.sourceID<<" to sink:"<<nextRoute.sinkID<<"\n";
#endif
		}
		else
		{
			resultPath.insert(resultPath.end(), result.begin(), result.end());
#ifdef TRACE_GRAPH
			std::cout<<"Successfully determined connection formats for path from source:"<<nextRoute.sourceID<<" to sink:"<<nextRoute.sinkID<<"\n";
			for(auto routeConnectionFormats: result)
			{
				for(auto it = routeConnectionFormats.route.begin(); it!=routeConnectionFormats.route.end(); it++)
				{
					am_RoutingElement_s & routingElement =  *it;
					std::cout<<"["
							 <<routingElement.sourceID
							 <<"->"
							 <<routingElement.sinkID
							 <<" CF:"
							 <<routingElement.connectionFormat
							 <<" D:"
							 <<routingElement.domainID
							 <<"]";
				}
				std::cout<<"\n";
			}
#endif
			successCount++;
		}
	});
	if(successCount)
		return E_OK;
	if(errorsCount)
		return E_NOT_POSSIBLE;
	return E_OK;
}

bool CAmRouter::shouldGoInDomain(const std::vector<am_domainID_t> & visitedDomains, const am_domainID_t nodeDomainID)
{
	if(visitedDomains.size())
	{
		if(visitedDomains.back()==nodeDomainID)
			return true;

		for(auto it=visitedDomains.begin();it!=visitedDomains.end()-1; it++)
		{
			if(nodeDomainID==*it)
				return false;
		}
	}
	return true;
}

void CAmRouter::generateAllPaths(const CAmRoutingNode & src,
									  const CAmRoutingNode & dst,
									  const bool includeCycles,
									  std::function<void(const std::vector<CAmRoutingNode*> & path)> cb)
{
	if(!includeCycles)
	{
		std::vector<CAmRoutingNode*> visited;
		std::vector<am_domainID_t> visitedDomains;
		visited.push_back((CAmRoutingNode*)&src);
		visitedDomains.push_back(((CAmRoutingNode*)&src)->getData().domainID());
		((CAmRoutingNode*)&src)->setStatus(GES_VISITED);
		goThroughAllPaths(dst, visited, visitedDomains, cb);
	}
	else
		mRoutingGraph.getAllPaths(src, dst, cb);
}

void CAmRouter::goThroughAllPaths(const CAmRoutingNode & dst,
									  std::vector<CAmRoutingNode*> & visited,
									  std::vector<am_domainID_t> & visitedDomains,
									  std::function<void(const std::vector<CAmRoutingNode*> & path)> cb)
{
#ifndef ROUTING_BUILD_CONNECTIONS
	CAmRoutingListVertices vertices;
	getVerticesForNode(*visited.back(), vertices);
	const CAmRoutingListVertices * nodes = &vertices;
#else
	const CAmRoutingListVertices * nodes = mRoutingGraph.getVertexList()[visited.back()->getIndex()];
#endif
	CAmRoutingListVertices::const_iterator vItr(nodes->begin());
	for (; vItr != nodes->end(); ++vItr)
	{
		const CAmRoutingVertex & vertex = (*vItr);
		if(vertex.getNode()->getStatus()!=GES_NOT_VISITED || !shouldGoInDomain(visitedDomains, vertex.getNode()->getData().domainID()))
			continue;
		if (vertex.getNode()==&dst)
		{
			vertex.getNode()->setStatus(GES_IN_PROGRESS);
			visited.push_back(vertex.getNode());
			visitedDomains.push_back(vertex.getNode()->getData().domainID());
			//notify observer
			cb(visited);
			//remove last node from the list
			auto last = visited.end()-1;
			visited.erase(last);
			visitedDomains.erase(visitedDomains.end()-1);
			vertex.getNode()->setStatus(GES_NOT_VISITED);
			break;
		}
	}
	vItr = nodes->begin();
	//bfs like loop
	for (; vItr != nodes->end(); ++vItr)
	{
		const CAmRoutingVertex & vertex = (*vItr);
		if(vertex.getNode()->getStatus()!=GES_NOT_VISITED
			||vertex.getNode()==&dst ||
			!shouldGoInDomain(visitedDomains, vertex.getNode()->getData().domainID()))
			continue;
		vertex.getNode()->setStatus(GES_IN_PROGRESS);
		visited.push_back(vertex.getNode());
		visitedDomains.push_back(vertex.getNode()->getData().domainID());
		goThroughAllPaths(dst, visited, visitedDomains, cb);
		//remove last node from the list
		auto last = visited.end()-1;
		visited.erase(last);
		visitedDomains.erase(visitedDomains.end()-1);
		vertex.getNode()->setStatus(GES_NOT_VISITED);
	}
}

bool  CAmRouter::getAllowedFormatsFromConvMatrix(	const std::vector<bool> & convertionMatrix,
										   	const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
										    const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
										    std::vector<am_CustomConnectionFormat_t> & sourceFormats,
										    std::vector<am_CustomConnectionFormat_t> & sinkFormats)
{
	const size_t sizeSourceFormats = listSourceFormats.size();
	const size_t sizeSinkFormats = listSinkFormats.size();
	const size_t sizeConvertionMatrix = convertionMatrix.size();

	if(sizeSourceFormats==0||sizeSinkFormats==0||sizeConvertionMatrix==0||sizeConvertionMatrix!=sizeSinkFormats*sizeSourceFormats)
	{
		return false;
	}

	std::vector<bool>::const_iterator iterator = convertionMatrix.begin();
	for (; iterator != convertionMatrix.end(); ++iterator)
	{
		if( true == *iterator )
		{
			const size_t index = iterator-convertionMatrix.begin();
			size_t idx = index%sizeSourceFormats;
			sourceFormats.push_back(listSourceFormats.at(idx));
			idx = index/sizeSourceFormats;
			sinkFormats.push_back(listSinkFormats.at(idx));
		}
	}
	return sourceFormats.size()>0;
}

void CAmRouter::listPossibleConnectionFormats(std::vector<am_CustomConnectionFormat_t> & inListSourceFormats,
													std::vector<am_CustomConnectionFormat_t> & inListSinkFormats,
													std::vector<am_CustomConnectionFormat_t> & outListFormats)
{
    std::sort(inListSourceFormats.begin(), inListSourceFormats.end());
    std::sort(inListSinkFormats.begin(), inListSinkFormats.end());
    std::insert_iterator<std::vector<am_CustomConnectionFormat_t> > inserter(outListFormats, outListFormats.begin());
    set_intersection(inListSourceFormats.begin(), inListSourceFormats.end(), inListSinkFormats.begin(), inListSinkFormats.end(), inserter);
}


bool CAmRouter::getRestrictedOutputFormats(const std::vector<bool> & convertionMatrix,
												  const std::vector<am_CustomConnectionFormat_t> & listSourceFormats,
												  const std::vector<am_CustomConnectionFormat_t> & listSinkFormats,
												  const am_CustomConnectionFormat_t connectionFormat,
												  std::vector<am_CustomConnectionFormat_t> & listFormats)
{
    listFormats.clear();
    std::vector<am_CustomConnectionFormat_t>::const_iterator rowSinkIterator = listSinkFormats.begin();
    std::vector<bool>::const_iterator matrixIterator = convertionMatrix.begin();

    //find the row number of the sink
    rowSinkIterator = find(listSinkFormats.begin(), listSinkFormats.end(), connectionFormat);
    int rowNumberSink = rowSinkIterator - listSinkFormats.begin();

    //go through the convertionMatrix and find out if the conversion is possible, if yes, add connectionFormat ...
    std::advance(matrixIterator, rowNumberSink);

    //iterate line-wise through the matrix and add more formats
    do
    {
        if (*matrixIterator)
        {
            listFormats.push_back(listSourceFormats.at((matrixIterator - convertionMatrix.begin()) / listSinkFormats.size()));
        }
        std::advance(matrixIterator, listSinkFormats.size());
    } while (convertionMatrix.end() - matrixIterator > 0);

    return listFormats.size();
}


am_Error_e CAmRouter::getSourceSinkPossibleConnectionFormats(std::vector<CAmRoutingNode*>::iterator iteratorSource,
		 	 	 	 	 	 	 	 	 	 std::vector<CAmRoutingNode*>::iterator iteratorSink,
		 	 	 	 	 	 	 	 	 	 std::vector<am_CustomConnectionFormat_t> & outConnectionFormats)
{
	CAmRoutingNode * nodeSink = *iteratorSink;
	if(nodeSink->getData().type!=CAmNodeDataType::SINK)
		return (E_UNKNOWN);

	CAmRoutingNode * nodeSource = *iteratorSource;
	if(nodeSource->getData().type!=CAmNodeDataType::SOURCE)
		return (E_UNKNOWN);

	am_Source_s *source = nodeSource->getData().data.source;
	am_Sink_s *sink = nodeSink->getData().data.sink;
	listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, outConnectionFormats);
	return (E_OK);
}

}
