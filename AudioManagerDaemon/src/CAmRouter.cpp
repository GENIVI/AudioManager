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


void getSourceSinkPossibleConnectionFormats(std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator iteratorSource,
		 	 	 	 	 	 	 	 	 	 std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator iteratorSink,
		 	 	 	 	 	 	 	 	 	 std::vector<am_CustomConnectionFormat_t> & outConnectionFormats)
{
	CAmNode<am_RoutingNodeData_s> * nodeSink = *iteratorSink;
	assert(nodeSink->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::SINK);

	CAmNode<am_RoutingNodeData_s> * nodeSource = *iteratorSource;
	assert(nodeSource->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::SOURCE);

	am_Source_s *source = nodeSource->getData().data.source;
	am_Sink_s *sink = nodeSink->getData().data.sink;
	CAmRouter::listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, outConnectionFormats);
}

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
		mNodeListConverters(),
		mNodeListSourceStatus(),
		mNodeListSinkStatus(),
		mNodeListConverterStatus(),
		mNodeListGatewayStatus(),
		mpRootSource(0),
		mpRootSink(0)
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
    mpDatabaseHandler->getSourceInfoDB(sourceID, source);
    mpDatabaseHandler->getSinkInfoDB(sinkID, sink);
    return getRoute(onlyfree, source, sink, returnList);
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

#ifdef EXTENDED_ROUTING_GRAPH
void appendNodes(CAmGraph<am_RoutingNodeData_s, uint16_t> & routingGraph,
					const std::vector<am_CustomConnectionFormat_t> & listConnectionFormats,
					am_RoutingNodeData_s & nodeData,
					std::vector<CAmNode<am_RoutingNodeData_s>*> & nodeList)
{
	std::for_each(listConnectionFormats.begin(), listConnectionFormats.end(), [&](const am_CustomConnectionFormat_t cf){
		nodeData.inConnectionFormat = cf;
		nodeList.push_back(&routingGraph.addNode(nodeData));
	});
}

void appendNodes(CAmGraph<am_RoutingNodeData_s, uint16_t> & routingGraph,
					const std::vector<am_CustomConnectionFormat_t> & sourceConnectionFormats,
					const std::vector<am_CustomConnectionFormat_t> & sinkConnectionFormats,
					const std::vector<bool> & convertionMatrix,
					am_RoutingNodeData_s & nodeData,
					std::vector<CAmNode<am_RoutingNodeData_s>*> & nodeList)
{

	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats;
	if(CAmRouter::getAllowedFormatsFromConvMatrix(convertionMatrix, sourceConnectionFormats, sinkConnectionFormats, sourceFormats, sinkFormats))
	{
		for(size_t i = 0; i < sourceFormats.size(); i++)
		{
			nodeData.inConnectionFormat = sinkFormats[i];
			nodeData.outConnectionFormat = sourceFormats[i];
			nodeList.push_back(&routingGraph.addNode(nodeData));
		}
	}
}
#endif

am_Error_e CAmRouter::getRoute(const bool onlyfree, const am_Source_s & aSource, const am_Sink_s & aSink, std::vector<am_Route_s> & listRoutes)
{
	clear();
	mOnlyFreeConversionNodes = onlyfree;

	// We need to copy the sinks, sources, converters, gateways, because we don't know what store is being used (SQLite or Map).
	//todo: Don't make copy for map storage.
	std::deque<am_Source_s> listSources;
	std::deque<am_Sink_s> listSinks;
	std::deque<am_Gateway_s> listGateways;
	std::deque<am_Converter_s> listConverters;

	am_Error_e error = E_OK;

	am_RoutingNodeData_s nodeDataSrc;
	nodeDataSrc.type = NodeDataType::SOURCE;
	mpDatabaseHandler->enumerateSources([&](const am_Source_s & obj){
		listSources.push_back(obj);
		nodeDataSrc.data.source = &listSources.back();
		mNodeListSourceStatus[obj.sourceID]=false;
#ifdef EXTENDED_ROUTING_GRAPH
		appendNodes(mRoutingGraph, obj.listConnectionFormats, nodeDataSrc, mNodeListSources);
#else
		mNodeListSources.push_back(&mRoutingGraph.addNode(nodeDataSrc));
#endif
	});
	am_RoutingNodeData_s nodeDataSink;
	nodeDataSink.type = NodeDataType::SINK;
	mpDatabaseHandler->enumerateSinks([&](const am_Sink_s & obj){
		listSinks.push_back(obj);
		nodeDataSink.data.sink = &listSinks.back();
		mNodeListSinkStatus[obj.sinkID]=false;
#ifdef EXTENDED_ROUTING_GRAPH
		appendNodes(mRoutingGraph, obj.listConnectionFormats, nodeDataSink, mNodeListSinks);
#else
		mNodeListSinks.push_back(&mRoutingGraph.addNode(nodeDataSink));
#endif
	});
	am_RoutingNodeData_s nodeDataGateway;
	nodeDataGateway.type = NodeDataType::GATEWAY;
	mpDatabaseHandler->enumerateGateways([&](const am_Gateway_s & obj){
		listGateways.push_back(obj);
		nodeDataGateway.data.gateway = &listGateways.back();
		mNodeListGatewayStatus[obj.gatewayID]=false;
#ifdef EXTENDED_ROUTING_GRAPH
		appendNodes(mRoutingGraph, obj.listSourceFormats, obj.listSinkFormats, obj.convertionMatrix, nodeDataGateway, mNodeListGateways);
#else
		mNodeListGateways.push_back(&mRoutingGraph.addNode(nodeDataGateway));
#endif
	});
	am_RoutingNodeData_s nodeDataConverter;
	nodeDataConverter.type = NodeDataType::CONVERTER;
	mpDatabaseHandler->enumerateConverters([&](const am_Converter_s & obj){
		listConverters.push_back(obj);
		nodeDataConverter.data.converter = &listConverters.back();
		mNodeListConverterStatus[obj.converterID]=false;
#ifdef EXTENDED_ROUTING_GRAPH
		appendNodes(mRoutingGraph, obj.listSourceFormats, obj.listSinkFormats, obj.convertionMatrix, nodeDataConverter, mNodeListConverters);
#else
		mNodeListConverters.push_back(&mRoutingGraph.addNode(nodeDataConverter));
#endif
	});

	buildGraph(aSource, aSink);

#ifdef EXTENDED_ROUTING_GRAPH
	std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> pathNodes;
	getShortestPath(listRoutes, pathNodes);
#else
	std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> pathNodes;
	error = getAllPaths(listRoutes, pathNodes);
#endif
	return error;
}

void CAmRouter::buildGraph(	const am_Source_s & aSource, const am_Sink_s & aSink)
{
	//build up a topological sorted graph
#ifdef EXTENDED_ROUTING_GRAPH
	mpRootSource = (am_Source_s*)&aSource;
	mpRootSink = (am_Sink_s*)&aSink;
	for(auto it1=aSource.listConnectionFormats.begin(); it1!=aSource.listConnectionFormats.end(); it1++)
	{
		am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSource = sourceNodeWithID(aSource.sourceID, *it1);
		for(auto it2=aSink.listConnectionFormats.begin(); it2!=aSink.listConnectionFormats.end(); it2++)
		{
			am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSink = sinkNodeWithID(aSink.sinkID, *it2);
			if(pRootNodeSource && pRootNodeSink)
			{
				if(aSource.domainID==aSink.domainID)
					routeInSameDomain(*pRootNodeSource, *pRootNodeSink);
				else
					routeInAnotherDomain(*pRootNodeSource, *pRootNodeSink);
			}
		}
	}
#else
	mpRootSource = sourceNodeWithID(aSource.sourceID);
	mpRootSink = sinkNodeWithID(aSink.sinkID);

	assert(mpRootSource);
	assert(mpRootSink);

	if(aSource.domainID==aSink.domainID)
	{
		routeInSameDomain(*mpRootSource, *mpRootSink);
	}
	else
	{
		routeInAnotherDomain(*mpRootSource, *mpRootSink);
	}
#endif
#ifdef TRACE_GRAPH
	mRoutingGraph.trace([&](const CAmNode<am_RoutingNodeData_s> & node, const std::vector<CAmVertex<am_RoutingNodeData_s,uint16_t>*> & list) {
				std::cout << "Node " << node.getIndex() << " :";
				((CAmNode<am_RoutingNodeData_s> &)node).getData().trace();
				std::cout << "-->";
				std::for_each(list.begin(), list.end(), [&](const CAmVertex<am_RoutingNodeData_s,uint16_t>* refVertex){
					am::CAmNode<am::am_RoutingNodeData_s>* data = refVertex->getNode();
					std::cout << "Node " << data->getIndex() << " :";
					data->getData().trace();
				});
				std::cout << std::endl;
			});
#endif
}
#ifdef EXTENDED_ROUTING_GRAPH

CAmNode<am_RoutingNodeData_s>* CAmRouter::sinkNodeWithID(const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat)
{
	auto iter = std::find_if(mNodeListSinks.begin(), mNodeListSinks.end(), [sinkID, connectionFormat](CAmNode<am_RoutingNodeData_s>* node){
		return node->getData().data.sink->sinkID==sinkID && node->getData().inConnectionFormat==connectionFormat;
	});
	if(iter!=mNodeListSinks.end())
		return *iter;
	return NULL;
}

CAmNode<am_RoutingNodeData_s>* CAmRouter::sourceNodeWithID(const am_sourceID_t sourceID, const am_CustomConnectionFormat_t connectionFormat)
{
	auto iter = std::find_if(mNodeListSources.begin(), mNodeListSources.end(), [sourceID, connectionFormat](CAmNode<am_RoutingNodeData_s>* node){
		return node->getData().data.source->sourceID==sourceID && node->getData().inConnectionFormat==connectionFormat;
	});
	if(iter!=mNodeListSources.end())
		return *iter;
	return NULL;
}
#else
CAmNode<am_RoutingNodeData_s>* CAmRouter::sinkNodeWithID(const am_sinkID_t sinkID)
{
	auto iter = std::find_if(mNodeListSinks.begin(), mNodeListSinks.end(), [sinkID](CAmNode<am_RoutingNodeData_s>* node){
		return node->getData().data.sink->sinkID==sinkID;
	});
	if(iter!=mNodeListSinks.end())
		return *iter;
	return NULL;
}

CAmNode<am_RoutingNodeData_s>* CAmRouter::sourceNodeWithID(const am_sourceID_t sourceID)
{
	auto iter = std::find_if(mNodeListSources.begin(), mNodeListSources.end(), [sourceID](CAmNode<am_RoutingNodeData_s>* node){
		return node->getData().data.source->sourceID==sourceID;
	});
	if(iter!=mNodeListSources.end())
		return *iter;
	return NULL;
}
#endif

void CAmRouter::connectNodes(const CAmNode<am_RoutingNodeData_s> & node1,
											const CAmNode<am_RoutingNodeData_s> & node2,
											const am_CustomConnectionFormat_t vertexData,
											const int16_t weight)
{
	if( !mRoutingGraph.isAnyVertex(node1, node2) )
		mRoutingGraph.connectNodes(node1, node2, vertexData, weight);
}

bool CAmRouter::routeInSameDomain(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink)
{
	am_RoutingNodeData_s & sourceData = aSource.getData();
	am_RoutingNodeData_s & sinkData = aSink.getData();
	am_Source_s * source = sourceData.data.source;
	//If exists connection return
	if( mRoutingGraph.isAnyVertex(aSource, aSink))
		return true;
	if( mNodeListSourceStatus[source->sourceID] )
		return false;
	bool result = false;
	mNodeListSourceStatus[source->sourceID]=true;
#ifdef EXTENDED_ROUTING_GRAPH
	if(sourceData.inConnectionFormat!=sinkData.inConnectionFormat) // it is not possible to connect them directly
	{
#else
	am_Sink_s * sink = sinkData.data.sink;
	std::vector<am_CustomConnectionFormat_t> listFormats;
	CAmRouter::listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, listFormats);
	if(listFormats.size()==0) // it is not possible to connect them directly
	{
#endif
		bool availableConverter = false;
		for(auto iter = mNodeListConverters.begin(); iter!=mNodeListConverters.end(); iter++)
		{
			CAmNode<am_RoutingNodeData_s>* converterNode = *iter;
			am_RoutingNodeData_s & converterNodeData = converterNode->getData();
			am_Converter_s * converter = converterNodeData.data.converter;
			//Get only converters with end point in current source domain
			if( !mNodeListConverterStatus[converter->converterID] && converter->domainID==source->domainID && (!mOnlyFreeConversionNodes || !isComponentConnected(*converter)))
			{
				//Get the sink connected to the converter...
				mNodeListConverterStatus[converter->converterID]=true;
#ifdef EXTENDED_ROUTING_GRAPH
				CAmNode<am_RoutingNodeData_s> *converterSinkNode = this->sinkNodeWithID(converter->sinkID, converterNodeData.inConnectionFormat);
#else
				CAmNode<am_RoutingNodeData_s> *converterSinkNode = this->sinkNodeWithID(converter->sinkID);
#endif
				if(converterSinkNode)
				{
					am_RoutingNodeData_s & converterSinkData = converterSinkNode->getData();
					//Check whether the hidden sink formats match the source formats...
#ifdef EXTENDED_ROUTING_GRAPH
					if(sourceData.inConnectionFormat==converterSinkData.inConnectionFormat)
					{
						CAmNode<am_RoutingNodeData_s> *converterSourceNode = this->sourceNodeWithID(converter->sourceID, converterNodeData.outConnectionFormat);
						if(converterSourceNode)
						{
							am_RoutingNodeData_s & converterSourceData = converterSourceNode->getData();
							if(converterSourceData.inConnectionFormat==converterSinkData.outConnectionFormat)
							{
								availableConverter=true;
								// Connection source->conv_sink->converter->conv_source
								this->connectNodes(aSource, *converterSinkNode, converterNodeData.inConnectionFormat, 1);
								this->connectNodes(*converterSinkNode, *converterNode, converterNodeData.inConnectionFormat, 1);
								this->connectNodes(*converterNode, *converterSourceNode, converterNodeData.outConnectionFormat, 1);
								result|=this->routeInSameDomain(*converterSourceNode, aSink);
							}
						}
					}
					else
					{
							//the converter is not suitable, lets try to find paths through another domains
							bool alternResult = this->routeInAnotherDomain(aSource, *converterSinkNode);
							if(alternResult)
							{
								CAmNode<am_RoutingNodeData_s> *converterSourceNode = this->sourceNodeWithID(converter->sourceID, converterNodeData.outConnectionFormat);
								if(converterSourceNode)
								{
									am_RoutingNodeData_s & converterSourceData = converterSourceNode->getData();
									if(converterSourceData.inConnectionFormat==converterSinkData.outConnectionFormat)
									{
										// Connection source->conv_sink->converter->conv_source
										this->connectNodes(*converterSinkNode, *converterNode, converterNodeData.inConnectionFormat, 1);
										this->connectNodes(*converterNode, *converterSourceNode, converterNodeData.outConnectionFormat, 1);
										result|=this->routeInSameDomain(*converterSourceNode, aSink);
									}
								}
							}
					}
#else
					am_Sink_s * nextSink = converterSinkData.data.sink;
					std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats, intersection;
					//Check whether the hidden sink formats match the source formats...
					CAmRouter::listPossibleConnectionFormats(source->listConnectionFormats, nextSink->listConnectionFormats, intersection);
					if(intersection.size()>0)//OK  match source -> conv_sink
					{
						//Are there convertible formats or not
						if(CAmRouter::getAllowedFormatsFromConvMatrix(converter->convertionMatrix, converter->listSourceFormats, converter->listSinkFormats, sourceFormats, sinkFormats))
						{
							availableConverter=true;
							CAmNode<am_RoutingNodeData_s> *nextSourceNode = this->sourceNodeWithID(converter->sourceID);
							assert(nextSourceNode);
							//Connections source->hidden_sink->converter->hidden_source
							this->connectNodes(aSource, *converterSinkNode, CF_UNKNOWN, 1);
							this->connectNodes(*converterSinkNode, *converterNode, CF_UNKNOWN, 1);
							this->connectNodes(*converterNode, *nextSourceNode, CF_UNKNOWN, 1);
							//Go ahead with hidden_source and sink
							result|=this->routeInSameDomain(*nextSourceNode, aSink);
						}
					}
					else
					{
						//the converter is not suitable, lets try to find paths through another domains
						bool alternResult = this->routeInAnotherDomain(aSource, *converterSinkNode);
						if(alternResult)
						{
							if(CAmRouter::getAllowedFormatsFromConvMatrix(converter->convertionMatrix, converter->listSourceFormats, converter->listSinkFormats, sourceFormats, sinkFormats))
							{
								CAmNode<am_RoutingNodeData_s> *nextSourceNode = this->sourceNodeWithID(converter->sourceID);
								assert(nextSourceNode);
								//Connections source->hidden_sink->converter->hidden_source
								this->connectNodes(*converterSinkNode, *converterNode, CF_UNKNOWN, 1);
								this->connectNodes(*converterNode, *nextSourceNode, CF_UNKNOWN, 1);
								//Go ahead with hidden_source and sink
								result|=this->routeInSameDomain(*nextSourceNode, aSink);
							}
						}
					}
#endif
				}
				mNodeListConverterStatus[converter->converterID]=false;
			}
		}
		if(!availableConverter)
		{
			mNodeListSourceStatus[source->sourceID]=false;
			result|=this->routeInAnotherDomain(aSource, aSink);
		}
	}
	else
	{
		//success only if the source can be connected with the sink
#ifdef EXTENDED_ROUTING_GRAPH
		this->connectNodes(aSource, aSink, sourceData.inConnectionFormat, 1);
#else
		this->connectNodes(aSource, aSink, CF_UNKNOWN, 1);
#endif
		result=true;
	}
	mNodeListSourceStatus[source->sourceID]=false;
	return result;
}

bool CAmRouter::routeInAnotherDomain(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink)
{
	am_RoutingNodeData_s & sourceData = aSource.getData();
	am_RoutingNodeData_s & sinkData = aSink.getData();
	am_Source_s * source = sourceData.data.source;
	am_Sink_s * sink = sinkData.data.sink;

	if(mRoutingGraph.isAnyVertex(aSource, aSink))
		return true;
	if( mNodeListSourceStatus[source->sourceID] )
		return false;
	mNodeListSourceStatus[source->sourceID]=true;
#ifndef EXTENDED_ROUTING_GRAPH
	std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats, intersection;
#endif
	bool result = false;
	for(auto iter = mNodeListGateways.begin(); iter!=mNodeListGateways.end(); iter++)
	{
		CAmNode<am_RoutingNodeData_s>* gatewayNode = *iter;
		am_RoutingNodeData_s & gatewayNodeData = gatewayNode->getData();
		am_Gateway_s * gateway = gatewayNodeData.data.gateway;
		//Get only gateways with end point in current source domain
		if(
			!mNodeListGatewayStatus[gateway->gatewayID] &&
			gateway->domainSinkID==source->domainID &&
			(!mOnlyFreeConversionNodes || !isComponentConnected(*gateway)) )
		{
			//Get the sink connected to the gateway...
			mNodeListGatewayStatus[gateway->gatewayID]=true;
#ifdef EXTENDED_ROUTING_GRAPH
			CAmNode<am_RoutingNodeData_s> *gatewaySinkNode = this->sinkNodeWithID(gateway->sinkID, gatewayNodeData.inConnectionFormat);
#else
			CAmNode<am_RoutingNodeData_s> *gatewaySinkNode = this->sinkNodeWithID(gateway->sinkID);
#endif

			if(gatewaySinkNode)
			{
				am_RoutingNodeData_s & gatewaySinkData = gatewaySinkNode->getData();
					//Check whether the hidden sink formats match the source formats...

#ifdef EXTENDED_ROUTING_GRAPH
				if(sourceData.inConnectionFormat==gatewaySinkData.inConnectionFormat)
				{
					//Check if the gateway is able to convert any formats...
					CAmNode<am_RoutingNodeData_s> *gatewaySourceNode = this->sourceNodeWithID(gateway->sourceID, gatewayNodeData.outConnectionFormat);
					if(gatewaySourceNode)
					{
						am_RoutingNodeData_s & gatewaySourceData = gatewaySourceNode->getData();
						if(gatewaySourceData.inConnectionFormat==gatewayNodeData.outConnectionFormat)
						{
							am_Source_s * gatewaySource = gatewaySourceData.data.source;
							//Connections source->hidden_sink->gateway->hidden_source
							this->connectNodes(aSource, *gatewaySinkNode, gatewayNodeData.inConnectionFormat, 1);
							this->connectNodes(*gatewaySinkNode, *gatewayNode, gatewayNodeData.inConnectionFormat, 1);
							this->connectNodes(*gatewayNode, *gatewaySourceNode, gatewayNodeData.outConnectionFormat, 1);
							//Go ahead with hidden_source and sink
							if(gatewaySource->domainID==sink->domainID)
								result|=this->routeInSameDomain(*gatewaySourceNode, aSink);
							else
								result|=this->routeInAnotherDomain(*gatewaySourceNode, aSink);
						}
					}
				}
				else
				{
					//the gateway is not suitable, lets try to find paths within this domains
					bool alternResult = this->routeInSameDomain(aSource, *gatewaySinkNode);
					if(alternResult)
					{
						CAmNode<am_RoutingNodeData_s> *gatewaySourceNode = this->sourceNodeWithID(gateway->sourceID, gatewayNodeData.outConnectionFormat);
						if(gatewaySourceNode)
						{
							am_RoutingNodeData_s & gatewaySourceData = gatewaySourceNode->getData();
							if(gatewaySourceData.inConnectionFormat==gatewayNodeData.outConnectionFormat)
							{
								am_Source_s * gatewaySource = gatewaySourceData.data.source;
								//Connections source->hidden_sink->gateway->hidden_source
								this->connectNodes(aSource, *gatewaySinkNode, gatewayNodeData.inConnectionFormat, 1);
								this->connectNodes(*gatewaySinkNode, *gatewayNode, gatewayNodeData.inConnectionFormat, 1);
								this->connectNodes(*gatewayNode, *gatewaySourceNode, gatewayNodeData.outConnectionFormat, 1);
								//Go ahead with hidden_source and sink
								if(gatewaySource->domainID==sink->domainID)
									result|=this->routeInSameDomain(*gatewaySourceNode, aSink);
								else
									result|=this->routeInAnotherDomain(*gatewaySourceNode, aSink);
							}
						}
					}
				}
#else
				am_Sink_s * nextSink = gatewaySinkData.data.sink;
				std::vector<am_CustomConnectionFormat_t> sourceFormats, sinkFormats, intersection;
				CAmRouter::listPossibleConnectionFormats(source->listConnectionFormats, nextSink->listConnectionFormats, intersection);
				if(intersection.size()>0)//OK  match source -> conv_sink
				{
					//Check if the gateway is able to convert any formats...
					if(CAmRouter::getAllowedFormatsFromConvMatrix(gateway->convertionMatrix, gateway->listSourceFormats, gateway->listSinkFormats, sourceFormats, sinkFormats))
					{
						CAmNode<am_RoutingNodeData_s> *nextSourceNode = this->sourceNodeWithID(gateway->sourceID);
						assert(nextSourceNode);
						am_Source_s * nextSource = nextSourceNode->getData().data.source;
						//Connections source->hidden_sink->gateway->hidden_source
						this->connectNodes(aSource, *gatewaySinkNode, CF_UNKNOWN, 1);
						this->connectNodes(*gatewaySinkNode, *gatewayNode, CF_UNKNOWN, 1);
						this->connectNodes(*gatewayNode, *nextSourceNode, CF_UNKNOWN, 1);
						//Go ahead with hidden_source and sink
						if(nextSource->domainID==sink->domainID)
							result|=this->routeInSameDomain(*nextSourceNode, aSink);
						else
							result|=this->routeInAnotherDomain(*nextSourceNode, aSink);
					}
				}
				else
				{
					//the gateway is not suitable, lets try to find paths within this domains
					mNodeListSourceStatus[source->sourceID]=false;
					bool alternResult = this->routeInSameDomain(aSource, *gatewaySinkNode);
					if(alternResult)
					{
						if(CAmRouter::getAllowedFormatsFromConvMatrix(gateway->convertionMatrix, gateway->listSourceFormats, gateway->listSinkFormats, sourceFormats, sinkFormats))
						{
							CAmNode<am_RoutingNodeData_s> *nextSourceNode = this->sourceNodeWithID(gateway->sourceID);
							assert(nextSourceNode);
							am_Source_s * nextSource = nextSourceNode->getData().data.source;
							//Connections source->hidden_sink->gateway->hidden_source
							this->connectNodes(*gatewaySinkNode, *gatewayNode, CF_UNKNOWN, 1);
							this->connectNodes(*gatewayNode, *nextSourceNode, CF_UNKNOWN, 1);
							//Go ahead with hidden_source and sink
							if(nextSource->domainID==sink->domainID)
								result|=this->routeInSameDomain(*nextSourceNode, aSink);
							else
								result|=this->routeInAnotherDomain(*nextSourceNode, aSink);
						}
					}
				}
#endif
			}
			mNodeListGatewayStatus[gateway->gatewayID]=false;
		}
	}
	mNodeListSourceStatus[source->sourceID]=false;
	return result;
}

#ifdef EXTENDED_ROUTING_GRAPH
void CAmRouter::getShortestPath(const am_Source_s & aSource,
										   const am_Sink_s & aSink,
										   std::vector<am_Route_s> & resultPath,
										   std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath)
{
	std::vector<CAmNode<am_RoutingNodeData_s>*> listTargets;
	std::vector<CAmNode<am_RoutingNodeData_s>*> * pathNodes;
	am_Route_s *path;
	am_RoutingElement_s * element;
	std::function<void(const am_GraphPathPosition_e, CAmNode<am_RoutingNodeData_s> &)> cb = [&](const am_GraphPathPosition_e pos, CAmNode<am_RoutingNodeData_s> & object)
	{
		if(pos==GRAPH_PATH_START)
		{
			resultNodesPath.emplace_back();
			pathNodes=&resultNodesPath.back();
			resultPath.emplace_back();
			path= &resultPath.back();
			path->sinkID = aSink.sinkID;
			path->sourceID = aSource.sourceID;
		}
		pathNodes->insert(pathNodes->begin(), (CAmNode<am_RoutingNodeData_s>*)&object);

		am_RoutingNodeData_s & routingData =  object.getData();

		if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SINK)
		{
			auto iter = path->route.emplace(path->route.begin());
			element = &(*iter);

			element->domainID = routingData.data.sink->domainID;
			element->sinkID = routingData.data.sink->sinkID;
			element->connectionFormat = routingData.inConnectionFormat;

		}
		else if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SOURCE)
		{
			element->domainID = routingData.data.source->domainID;
			element->sourceID = routingData.data.source->sourceID;
			element->connectionFormat = routingData.inConnectionFormat;
		}
	};


	for(auto it2=aSink.listConnectionFormats.begin(); it2!=aSink.listConnectionFormats.end(); it2++)
	{
		am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSink = sinkNodeWithID(aSink.sinkID, *it2);
		if(pRootNodeSink)
			listTargets.push_back(pRootNodeSink);
	}
	if(listTargets.size())
	{
		for(auto it1=aSource.listConnectionFormats.begin(); it1!=aSource.listConnectionFormats.end(); it1++)
		{
			am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSource = sourceNodeWithID(aSource.sourceID, *it1);
			if(pRootNodeSource)
			{
				mRoutingGraph.getShortestPath(*pRootNodeSource, listTargets, cb);
			}
		}
	}
}

void CAmRouter::getShortestPath(std::vector<am_Route_s> & routes, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & nodes)
{
	getShortestPath(*mpRootSource, *mpRootSink, routes, nodes);
}

void CAmRouter::getShortestPath(const am_Source_s & aSource, const am_Sink_s & aSink,
												std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultPath)
{
	std::vector<CAmNode<am_RoutingNodeData_s>*> listTargets;
	for(auto it2=aSink.listConnectionFormats.begin(); it2!=aSink.listConnectionFormats.end(); it2++)
	{
		am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSink = sinkNodeWithID(aSink.sinkID, *it2);
		if(pRootNodeSink)
			listTargets.push_back(pRootNodeSink);
	}
	if(listTargets.size())
	{
		for(auto it1=aSource.listConnectionFormats.begin(); it1!=aSource.listConnectionFormats.end(); it1++)
		{
			am::CAmNode<am::am_RoutingNodeData_s>* pRootNodeSource = sourceNodeWithID(aSource.sourceID, *it1);
			if(pRootNodeSource)
			{
				mRoutingGraph.getShortestPath(*pRootNodeSource, listTargets, resultPath);
			}
		}
	}
}
#else

am_Error_e CAmRouter::determineConnectionFormatsForPath(am_Route_s & routeObjects, std::vector<CAmNode<am_RoutingNodeData_s>*> & nodes)
{
	std::vector<am_RoutingElement_s>::iterator routingElementIterator = routeObjects.route.begin();
	std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator nodeIterator = nodes.begin();
	if( routingElementIterator!= routeObjects.route.end() && nodeIterator!=nodes.end() )
		return doConnectionFormatsForPath(routeObjects, nodes, routingElementIterator, nodeIterator);
	return E_OK;
}

am_Error_e CAmRouter::doConnectionFormatsForPath(am_Route_s & routeObjects,
													 std::vector<CAmNode<am_RoutingNodeData_s>*> & nodes,
													 std::vector<am_RoutingElement_s>::iterator routingElementIterator,
													 std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator nodeIterator)
{
    am_Error_e returnError = E_NOT_POSSIBLE;
    std::vector<am_CustomConnectionFormat_t> listConnectionFormats;
    std::vector<am_CustomConnectionFormat_t> listMergeConnectionFormats;

    std::vector<CAmNode<am_RoutingNodeData_s>*>::iterator  currentNodeIterator = nodeIterator;
    std::vector<am_RoutingElement_s>::iterator  currentRoutingElementIterator = routingElementIterator;

    if (currentRoutingElementIterator!=routeObjects.route.begin())
    {
    	std::vector<am_CustomConnectionFormat_t> listConnectionFormats;
    	std::vector<am_RoutingElement_s>::iterator tempIterator = (currentRoutingElementIterator-1);
      	CAmNode<am_RoutingNodeData_s> * currentNode = *currentNodeIterator;
      	getSourceSinkPossibleConnectionFormats(currentNodeIterator+1, currentNodeIterator+2, listConnectionFormats);

		if(currentNode->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::GATEWAY)
		{
			am_Gateway_s *gateway = currentNode->getData().data.gateway;
			getMergeConnectionFormats(gateway, tempIterator->connectionFormat, listConnectionFormats, listMergeConnectionFormats);
		}
		else if(currentNode->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::CONVERTER)
		{
			am_Converter_s *converter = currentNode->getData().data.converter;
			getMergeConnectionFormats(converter, tempIterator->connectionFormat, listConnectionFormats, listMergeConnectionFormats);
		}
		currentNodeIterator+=3;
    }
    else
    {
    	CAmNode<am_RoutingNodeData_s> * currentNode = *currentNodeIterator;
    	assert(currentNode->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::SOURCE);

    	currentNodeIterator++;
		assert(currentNodeIterator!=nodes.end());

		CAmNode<am_RoutingNodeData_s> * nodeSink = *currentNodeIterator;
		assert(nodeSink->getData().type==am_RoutingNodeData_s::am_NodeDataType_e::SINK);

		am_Source_s *source = currentNode->getData().data.source;
		am_Sink_s *sink = nodeSink->getData().data.sink;
		CAmRouter::listPossibleConnectionFormats(source->listConnectionFormats, sink->listConnectionFormats, listMergeConnectionFormats);
		currentNodeIterator+=1; //now we are on the next converter/gateway
    }

    //let the controller decide:
    std::vector<am_CustomConnectionFormat_t> listPriorityConnectionFormats;
    mpControlSender->getConnectionFormatChoice(currentRoutingElementIterator->sourceID, currentRoutingElementIterator->sinkID, routeObjects,
    										  listMergeConnectionFormats, listPriorityConnectionFormats);

    //we have the list sorted after priors - now we try one after the other with the next part of the route
	std::vector<am_CustomConnectionFormat_t>::iterator connectionFormatIterator = listPriorityConnectionFormats.begin();
	//here we need to check if we are at the end and stop
	 std::vector<am_RoutingElement_s>::iterator nextIterator = currentRoutingElementIterator + 1;//next pair source and sink
	if (nextIterator == routeObjects.route.end())
	{
		if (!listPriorityConnectionFormats.empty())
		{
			currentRoutingElementIterator->connectionFormat = listPriorityConnectionFormats.front();
			return (E_OK);
		}
		else
			return (E_NOT_POSSIBLE);
	}

	for (; connectionFormatIterator != listPriorityConnectionFormats.end(); ++connectionFormatIterator)
	{
		currentRoutingElementIterator->connectionFormat = *connectionFormatIterator;
		if ((returnError = doConnectionFormatsForPath(routeObjects, nodes, nextIterator, currentNodeIterator)) == E_OK)
		{
			break;
		}
	}
    return (returnError);
}

void CAmRouter::getShortestPath(const CAmNode<am_RoutingNodeData_s> & source,
											   const CAmNode<am_RoutingNodeData_s> & destination,
											   std::vector<CAmNode<am_RoutingNodeData_s>*> & resultPath)
{
	mRoutingGraph.getShortestPath(source, destination, resultPath);
}

void CAmRouter::getShortestPath(std::vector<CAmNode<am_RoutingNodeData_s>*> & resultPath)
{
	getShortestPath(*mpRootSource, *mpRootSink, resultPath);
}

void CAmRouter::getShortestPath(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink,
												am_Route_s & resultPath, std::vector<CAmNode<am_RoutingNodeData_s>*> & resultNodesPath)
{
	am_RoutingElement_s * element;
	am_RoutingNodeData_s & sinkNodeData = aSink.getData();
	am_RoutingNodeData_s & sourceNodeData = aSource.getData();
	resultPath.sinkID = sinkNodeData.data.sink->sinkID;
	resultPath.sourceID = sourceNodeData.data.source->sourceID;

	std::function<void(const am_GraphPathPosition_e, CAmNode<am_RoutingNodeData_s> &)> cb = [&](const am_GraphPathPosition_e, CAmNode<am_RoutingNodeData_s> & object)
	{
		resultNodesPath.insert(resultNodesPath.begin(), (CAmNode<am_RoutingNodeData_s>*)&object);
		am_RoutingNodeData_s & routingData =  object.getData();
		if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SINK)
		{
			auto iter = resultPath.route.emplace(resultPath.route.begin());
			element = &(*iter);
			element->domainID = routingData.data.sink->domainID;
			element->sinkID = routingData.data.sink->sinkID;
			element->connectionFormat = CF_UNKNOWN;
		}
		else if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SOURCE)
		{
			element->domainID = routingData.data.source->domainID;
			element->sourceID = routingData.data.source->sourceID;
			element->connectionFormat = CF_UNKNOWN;
		}
	};
	mRoutingGraph.getShortestPath(aSource, aSink, cb);
}

void CAmRouter::getShortestPath(am_Route_s & resultPath, std::vector<CAmNode<am_RoutingNodeData_s>*> & resultNodesPath)
{
	getShortestPath(*mpRootSource, *mpRootSink, resultPath, resultNodesPath);
}

am_Error_e CAmRouter::getAllPaths(CAmNode<am_RoutingNodeData_s> & aSource, CAmNode<am_RoutingNodeData_s> & aSink,
											   std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath)
{
	uint8_t errorsCount = 0, successCount = 0;
	mRoutingGraph.getAllPaths(aSource, aSink, [&](const std::vector<CAmNode<am_RoutingNodeData_s>*> & path) {
		resultNodesPath.push_back(path);
		resultPath.emplace_back();
		am_Route_s & nextRoute = resultPath.back();
		nextRoute.sinkID = aSink.getData().data.sink->sinkID;
		nextRoute.sourceID = aSource.getData().data.source->sourceID;
		am_RoutingElement_s * element;
		for(auto it = path.begin(); it!=path.end(); it++)
		{
			am_RoutingNodeData_s & routingData =  (*it)->getData();
			if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SOURCE)
			{
				auto iter = nextRoute.route.emplace(nextRoute.route.end());
				element = &(*iter);
				element->domainID = routingData.data.source->domainID;
				element->sourceID = routingData.data.source->sourceID;
				element->connectionFormat = CF_UNKNOWN;
			}
			else if(routingData.type==am_RoutingNodeData_s::am_NodeDataType_e::SINK)
			{
				element->domainID = routingData.data.sink->domainID;
				element->sinkID = routingData.data.sink->sinkID;
				element->connectionFormat = CF_UNKNOWN;
			}
		}
		am_Error_e err = determineConnectionFormatsForPath(nextRoute, (std::vector<CAmNode<am_RoutingNodeData_s>*> &)path);
		if(err!=E_OK)
		{
			errorsCount++;
			auto last = resultPath.end()-1;
			resultPath.erase(last);
#ifdef TRACE_GRAPH
			std::cout<<"Error by determining connection formats for path from source:"<<nextRoute.sourceID<<" to sink:"<<nextRoute.sinkID<<"\n";
#endif
		}
		else
		{
#ifdef TRACE_GRAPH
			std::cout<<"Successfully determined connection formats for path from source:"<<nextRoute.sourceID<<" to sink:"<<nextRoute.sinkID<<"\n";
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

am_Error_e CAmRouter::getAllPaths(std::vector<am_Route_s> & resultPath, std::vector<std::vector<CAmNode<am_RoutingNodeData_s>*>> & resultNodesPath)
{
	return getAllPaths(*mpRootSource, *mpRootSink, resultPath, resultNodesPath);
}

#endif

void CAmRouter::clear()
{
	mNodeListSourceStatus.clear();
	mNodeListSinkStatus.clear();
	mNodeListConverterStatus.clear();
	mNodeListGatewayStatus.clear();

	mRoutingGraph.clear();
	mNodeListSources.clear();
	mNodeListSinks.clear();
	mNodeListGateways.clear();
	mNodeListConverters.clear();
	mpRootSource=NULL;
	mpRootSink=NULL;
}


}
