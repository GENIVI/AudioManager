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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2014
 *
 * \file CAmGraph.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <algorithm>
#include <limits.h>
#include <iomanip>
#include <cstring>
#include <set>


namespace am
{
	/**
	 * Graph element status.
	 */
	typedef enum:uint8_t
	{
		GES_NOT_VISITED,
		GES_IN_PROGRESS,
		GES_VISITED
	}am_GraphElementStatus_e;

	/**
	 * Callback parameter telling on which position in the path we are.
	 */
	typedef enum:uint8_t
	{
		GRAPH_PATH_START,	//at the beginning of the path
		GRAPH_PATH_MIDDLE,	//in middle of the path
		GRAPH_PATH_END		//at the end of the path
	}am_GraphPathPosition_e;


	/**
	 * This class is base class for nodes and vertices.
	 */
	class CAmGraphElement
	{
		am_GraphElementStatus_e mStatus; //!< item status
	public:
		CAmGraphElement(): mStatus(GES_NOT_VISITED) { };
		~CAmGraphElement() { };
		/**
		 * Setter and getter.
		 */
		void setStatus(const am_GraphElementStatus_e s) { mStatus = s; };
		am_GraphElementStatus_e getStatus() const { return mStatus; };
	};

	template <class NodeData> class CAmNode : public CAmGraphElement
	{
		uint16_t mIndex;	//!< uint16_t index used for direct access
		NodeData mData;	//!< NodeData user data
	public:
		CAmNode(const NodeData & in):CAmGraphElement(),  mIndex(0), mData(in) { };
		CAmNode(const NodeData & in, const uint16_t index):CAmGraphElement(),  mIndex(index), mData(in) { };
		~CAmNode() { };
		/**
		 * Setters and getters.
		 */
		NodeData & getData() { return mData; }
		const NodeData & getData() const { return mData; }
		uint16_t getIndex() const { return mIndex; }
		void setIndex(uint16_t index) { mIndex = index; }
	};

	template <class NodeData, class VertexData> class CAmVertex : public CAmGraphElement
	{
		CAmNode<NodeData>* mpNode; //!< CAmNode<NodeData>* pointer to a node
		VertexData mVertexData;	//!< VertexData vertex user data
		uint16_t mWeight;			//!< uint16_t a positive value used in the shortest path algorithms
	public:
		CAmVertex(CAmNode<NodeData> *aNode, const VertexData & vertexData, const uint16_t weight):CAmGraphElement(),
				mpNode(aNode), mVertexData(vertexData), mWeight(weight) { };
		~CAmVertex() { };
		/**
		 * Setters and getters.
		 */
		CAmNode<NodeData>* getNode() const { return mpNode; }
		VertexData & getData() { return mVertexData; }
		uint16_t getWeight() const { return mWeight; }
		void setWeight(const uint16_t weight) { mWeight=weight; }
	};

	/**
	 * Class representing a directed or undirected graph. It contains nodes and connections.
	 * T, V are types for custom user data.
	 */
	template <class T, class V> class CAmGraph
	{
        typedef typename std::vector<CAmNode<T>*>                 			CAmListNodePtrs;
        typedef typename std::list<CAmVertex<T,V>>                 			CAmListVertices;
		typedef typename std::list<CAmVertex<T,V>>::iterator       			CAmListVerticesItr;
		typedef typename std::list<CAmVertex<T,V>>::const_iterator  		CAmListVerticesItrConst;
        typedef typename std::list<CAmListVertices>         				CAmNodesAdjList;
		typedef typename std::list<CAmListVertices>::iterator       		CAmNodesAdjListItr;
		typedef typename std::list<CAmListVertices>::const_iterator 		CAmNodesAdjListItrConst;
		typedef typename std::list<CAmNode<T>>                              CAmListNodes;
		typedef typename std::list<CAmNode<T>>::iterator                    CAmListNodesItr;
		typedef typename std::list<CAmNode<T>>::const_iterator  			CAmListNodesItrConst;
        typedef typename std::vector<CAmNode<T>*>                 			CAmNodeReferenceList;
        typedef typename std::vector<CAmListVertices*>                 		CAmVertexReferenceList;

        CAmListNodes         	mStoreNodes;		//!< CAmListNodes list with all nodes
        CAmNodesAdjList      	mStoreAdjList;		//!< CAmNodesAdjList adjacency list
		CAmNodeReferenceList 	mPointersNodes;		//!< CAmNodeReferenceList vector with pointers to nodes for direct access
        CAmVertexReferenceList 	mPointersAdjList;	//!< CAmVertexReferenceList vector with pointers to vertices for direct access
		bool          		 	mIsCyclic;			//!< bool the graph has cycles or not

		struct IterateThroughAllNodesDelegate
		{
			CAmNode<T> * source;
			CAmNode<T> * destination;
			CAmNodeReferenceList visited;
			std::function<bool(const CAmNode<T> * )> shouldVisitNode;
			std::function<void(const CAmNode<T> *)> willVisitNode;
			std::function<void(const CAmNode<T> *)> didVisitNode;
			std::function<void(const CAmNodeReferenceList & path)> didFindPath;
		};

		struct VisitNodeDelegate
		{
			CAmNode<T> * source;
			CAmNode<T> * destination;
			std::function<void(const am_GraphPathPosition_e, CAmNode<T> &)> visitedNode;
		};

		/**
		 * Updates the node indexes after adding or removing nodes.
		 *
		 * @param fromIndex updates all nodes from given index.
		 */
		void updateIndexes(const int16_t fromIndex)
		{
			if( fromIndex<mPointersNodes.size() )
			{
				for(auto iter = mPointersNodes.begin()+fromIndex; iter!=mPointersNodes.end(); iter++)
					(*iter)->setIndex(iter-mPointersNodes.begin());
			}
		}


		/**
		 * Finds the shortest path and the minimal weights from given node.
		 *
		 * @param node start node.
		 * @param minDistance vector with all result distances.
		 * @param previous vector with previous nodes.
		 */

		typedef uint16_t vertex_t;
		typedef uint16_t weight_t;

		void findShortestPathsFromNode(const CAmNode<T> & node, std::vector<weight_t> &minDistance, std::vector<CAmNode<T> *> &previous)
		{
			typename CAmListVertices::const_iterator nIter;
			CAmListVertices * neighbors;
			weight_t dist, weight, v, distanceThroughU;
			CAmNode<T>* pU;
			CAmVertex<T,V> * pVertex;
			CAmNode<T> *pDstNode;

			size_t n = mPointersAdjList.size();
			std::set<std::pair<weight_t, CAmNode<T>*> > vertexQueue;

			minDistance.clear();
			minDistance.resize(n, std::numeric_limits<weight_t>::max());
			minDistance[node.getIndex()] = 0;
			previous.clear();
			previous.resize(n, NULL);

			vertexQueue.insert(std::make_pair(minDistance[node.getIndex()], (CAmNode<T>*)&node));

			while (!vertexQueue.empty())
			{
				dist = vertexQueue.begin()->first;
				pU = vertexQueue.begin()->second;
				vertexQueue.erase(vertexQueue.begin());
				//todo: terminate the search at this position if you want the path to a target node ( if(pU==target)break; )

				// Visit each edge exiting u
				neighbors = mPointersAdjList[pU->getIndex()];
				nIter = neighbors->begin();
				for (; nIter != neighbors->end(); nIter++)
				{
					pVertex = (CAmVertex<T,V> *)&(*nIter);
					pDstNode = pVertex->getNode();

					v = pDstNode->getIndex();
					weight = pVertex->getWeight();
					distanceThroughU = dist + weight;
					if (distanceThroughU < minDistance[pDstNode->getIndex()])
					{
						vertexQueue.erase(std::make_pair(minDistance[v], pDstNode));
						minDistance[v] = distanceThroughU;
						previous[v] = pU;
						vertexQueue.insert(std::make_pair(minDistance[v], pDstNode));
					}
				}
			}
		}

		/**
		 * Constructs a path to given node after findShortestsPathsFromNode has been called.
		 *
		 * @param node end node.
		 * @param previous vector with previous nodes.
		 * @param result result path.
		 */
		void constructShortestPathTo(const CAmNode<T> & node, const std::vector<CAmNode<T> *> &previous, CAmListNodePtrs & result)
		{
			CAmNode<T> * vertex = (CAmNode<T> *)&node;

			int i=0;
			while ( (vertex = previous[vertex->getIndex()])!=NULL )
			{
				result.insert(result.begin(), vertex);
				i++;
			}
			if(i)
				result.push_back((CAmNode<T> *)&node);
		}

		/**
		 * Calls a function with every node from this path after findShortestsPathsFromNode has been called.
		 * The construction of the path is delegated to the caller.
		 *
		 * @param node end node.
		 * @param previous vector with previous nodes.
		 * @param cb callback which is mostly used for constructing.
		 */
		void constructShortestPathTo(const CAmNode<T> & node, const std::vector<CAmNode<T> *> &previous, std::function<void(const am_GraphPathPosition_e pos, CAmNode<T> &)> cb)
		{
			CAmNode<T> * vertex = (CAmNode<T> *)&node;
			CAmNode<T> * prev = vertex;
			int i=0;
			while ( (vertex = previous[vertex->getIndex()])!=NULL )
			{
				cb(i==0?GRAPH_PATH_START:GRAPH_PATH_MIDDLE, *prev);
				prev = vertex;
				i++;
			}
			if(i)
				cb(GRAPH_PATH_END, *prev);
		}

		/**
		 * Iterate through the nodes and generate all paths to given node.
		 *
		 * @param dst end node.
		 * @param visited vector with current path.
		 * @param delegate enumeration delegate.
		 */
		void findAllPaths(IterateThroughAllNodesDelegate & delegate)
		{
			CAmListVertices * nodes = mPointersAdjList[delegate.visited.back()->getIndex()];
			CAmListVerticesItrConst vItr(nodes->begin());

			CAmVertex<T,V> * pNextVertex;
			CAmNode<T> * pNextNode;
			for (; vItr != nodes->end(); ++vItr)
			{
				pNextVertex = (CAmVertex<T,V> *)&(*vItr);
				pNextNode = pNextVertex->getNode();
				if(
					pNextNode->getStatus()!=GES_NOT_VISITED ||
					!delegate.shouldVisitNode(pNextNode)
				  )
					continue;
				if (pNextNode==delegate.destination)
				{
					delegate.willVisitNode(pNextNode);
					pNextNode->setStatus(GES_IN_PROGRESS);
					delegate.visited.push_back(pNextNode);
					//notify observer
					delegate.didFindPath(delegate.visited);
					//remove last node from the list
					auto last = delegate.visited.end()-1;
					delegate.visited.erase(last);
					pNextNode->setStatus(GES_NOT_VISITED);
					delegate.didVisitNode(pNextNode);
					break;
				}
			}
			vItr = nodes->begin();
			//bfs like loop
			for (; vItr != nodes->end(); ++vItr)
			{
				pNextVertex = (CAmVertex<T,V> *)&(*vItr);
				pNextNode = pNextVertex->getNode();

				if(pNextNode->getStatus()!=GES_NOT_VISITED ||
						pNextNode==delegate.destination ||
				   !delegate.shouldVisitNode(pNextNode)
					)
					continue;
				delegate.willVisitNode(pNextNode);
				pNextNode->setStatus(GES_IN_PROGRESS);
				delegate.visited.push_back(pNextNode);
				findAllPaths(delegate);
				//remove last node from the list
				auto last = delegate.visited.end()-1;
				delegate.visited.erase(last);
				pNextNode->setStatus(GES_NOT_VISITED);
				delegate.didVisitNode(pNextNode);
			}
		}

	public:

		explicit CAmGraph(const std::vector<T> &v):mStoreNodes(), mStoreAdjList(), mPointersNodes(), mPointersAdjList()
		{
			typedef typename std::vector<T>::const_iterator inItr;
			inItr itr(v.begin());

			for (; itr != v.end(); ++itr)
			{
				addNode(*itr);
			}

			mIsCyclic = false;
		};
		CAmGraph():mStoreNodes(), mStoreAdjList(), mPointersNodes(), mPointersAdjList(), mIsCyclic(false){};
		~CAmGraph(){}

		const CAmListNodes & getNodes() const
		{
			return mStoreNodes;
		}

		const CAmVertexReferenceList & getVertexList() const
		{
			return mPointersAdjList;
		}

		/**
		 * Returns pointer to a node which data is equal to the given.
		 * @return pointer to a node or NULL.
		 */
        const CAmNode<T>* findNode(const T & in)
		{
        	typename CAmNodeReferenceList::const_iterator itr (mPointersNodes.begin());

			for (; itr != mPointersNodes.end(); ++itr)
			{
				if ((*itr)->getData() == in) {
					return (*itr);
				}
			}
			return NULL;
		}

        /**
		 * Returns pointer to a vertex which two ends are equal to the given nodes.
		 * @return pointer to a vertex or NULL.
		 */
		const CAmVertex<T,V>* findVertex(const CAmNode<T> & edge1, const CAmNode<T> & edge2) const
		{
			const CAmNode<T> * pEdge2 = (CAmNode<T> *)&edge2;
			const CAmListVertices * list = mPointersAdjList[edge1.getIndex()];
			CAmListVerticesItrConst result = std::find_if(list->begin(), list->end(), [&](const CAmVertex<T,V> & refObject){
				return refObject.getNode()==pEdge2;
			});
			if(result!=list->end())
				return (CAmVertex<T,V>*)&(*result);

			return NULL;
		}

		bool hasCycles() const
		{
			return mIsCyclic;
		}


		/**
		 * Adds a new node to the graph with given user data.
		 * @return reference to the newly inserted node.
		 */
		CAmNode<T> & addNode(const T & in)
		{
                    size_t index = mStoreNodes.size();
                    mStoreNodes.emplace_back(in, index);
                    mStoreAdjList.emplace_back();
                    mPointersNodes.push_back(&mStoreNodes.back());
                    mPointersAdjList.push_back(&mStoreAdjList.back());
                    return mStoreNodes.back();
		}

		/**
		 * Removes a vertex with two ends equal to the given nodes .
		 */
        void removeVertex(const CAmNode<T> & edge1, const CAmNode<T> & edge2)
		{
			const CAmListVertices * list = mPointersAdjList[edge1.getIndex()];
			CAmListVerticesItr iter = std::find_if(list->begin(), list->end(), [&edge2](const CAmVertex<T,V> & refVertex){
				return (refVertex.getNode()==&edge2);
			});
			if(iter!=list->end())
				list->erase(iter);
		}

        /**
		 * Removes all vertices to given node .
		 */
        void removeAllVerticesToNode(const CAmNode<T> & node)
		{
        	auto comparator = [&node](const CAmVertex<T,V> & refVertex){
				return (refVertex.getNode()==&node);
			};
			auto itr = mPointersAdjList.begin();
			for(;itr!=mPointersAdjList.end();itr++)
			{
				CAmListVertices * vertices = *itr;
				auto iterVert = std::find_if(vertices->begin(), vertices->end(), comparator);
				if(iterVert!=vertices->end())
					vertices->erase(iterVert);
			}
		}

        /**
		 * Removes a node with given user data .
		 */
		void removeNode(const T & in)
		{
			CAmNode<T> * node = findNode(in);
            if(node!=NULL)
                removeNode(*node);
		}

		 /**
		 * Removes the given node from the graph .
		 */
		void removeNode(const CAmNode<T> & node)
		{
                    uint16_t index = node.getIndex();
                    removeAllVerticesToNode(node);
                    mPointersAdjList.erase(mPointersAdjList.begin()+index);
                    mPointersNodes.erase(mPointersNodes.begin()+index);
                    auto iter = std::find_if(mStoreNodes.begin(), mStoreNodes.end(), [&node](const CAmNode<T> & otherNode){
                        return &otherNode==&node;
                    });
                    if(iter!=mStoreNodes.end())
                        mStoreNodes.erase(iter);
                    updateIndexes(index);
		}

		/**
		 * Connect first with last node and set user data and weight to the vertex.
		 */
		void connectNodes(const CAmNode<T> & first, const CAmNode<T> & last, const V & vertexData, const int16_t weight = 1)
		{
			CAmListVertices * list = mPointersAdjList[first.getIndex()];
			CAmNode<T> * node = mPointersNodes[last.getIndex()];
			list->emplace_back(node, vertexData, weight);
		}

		/**
		 * Exists any vertex with two given ends.
		 * @return TRUE on successfully changed ID.
		 */
		bool isAnyVertex(const CAmNode<T> & edge1, const CAmNode<T> & edge2) const
		{
			return findVertex(edge1, edge2)!=NULL;
		}

		/**
		 * Sets the status of all nodes and vertices to GES_NOT_VISITED.
		 */
		void reset()
		{
			// set all nodes to GES_NOT_VISITED
			std::for_each(mPointersNodes.begin(), mPointersNodes.end(), [](CAmNode<T> * refNode){
				if(refNode->getStatus()!= GES_NOT_VISITED)
					refNode->setStatus(GES_NOT_VISITED);
			});
			// set all vertices to GES_NOT_VISITED
			auto action = [](CAmVertex<T,V> & refVertex){
				if(refVertex.getStatus()!= GES_NOT_VISITED)
					refVertex.setStatus(GES_NOT_VISITED);
			};
			auto itr1(mPointersAdjList.begin());
			for (; itr1 != mPointersAdjList.end(); ++itr1)
			{
				CAmListVertices * vertices = *itr1;
				std::for_each(vertices->begin(), vertices->end(), action);
			}
		}

		/**
		 * Clears all nodes and vertices.
		 */
		void clear()
		{
			mStoreNodes.clear();
			mStoreAdjList.clear();
			mPointersAdjList.clear();
			mPointersNodes.clear();
			mPointersAdjList.clear();
		}

		/**
		 * Goes through all nodes and vertices and calls the callback.
		 */
		void trace(std::function<void(const CAmNode<T> &, const std::vector<CAmVertex<T,V>*> &)> cb)
		{
			std::for_each(mPointersNodes.begin(), mPointersNodes.end(), [&](CAmNode<T> * refNode){
				CAmListVertices * vertices = this->mPointersAdjList[refNode->getIndex()];
				std::vector<CAmVertex<T,V>*> list;
				std::for_each(vertices->begin(), vertices->end(), [&list](CAmVertex<T,V> & refVertex){
						list.push_back(&refVertex);
				});
				cb(*refNode, list);
			});
		}

		/**
		 * Finds the shortest path from given node to all nodes in listTargets.
		 *
		 * @param source start node.
		 * @param listTargets destination nodes.
		 * @param resultPath list with all shortest paths.
		 */
		void getShortestPath(const CAmNode<T> & source, const CAmListNodePtrs & listTargets, std::vector<CAmListNodePtrs> & resultPath )
		{
			const size_t numberOfNodes = mPointersNodes.size();
			if(numberOfNodes==0)
				return;

			std::vector<weight_t> min_distance;
			std::vector<CAmNode<T>*> previous;
			findShortestPathsFromNode(source, min_distance, previous);

			for(auto it=listTargets.begin(); it!=listTargets.end(); it++)
			{
				CAmNode<T> *node = *it;
				resultPath.emplace_back();
				CAmListNodePtrs & path = resultPath.back();
				constructShortestPathTo(*node, previous, path);
				if(path.empty())
				{
					typename std::vector<CAmListNodePtrs>::iterator iter = resultPath.end();
					resultPath.erase(--iter);
				}
			}
		}

		/**
		 * Finds the shortest path between two nodes.
		 *
		 * @param source start node.
		 * @param destination destination node.
		 * @param resultPath list with the found shortest paths.
		 */
		void getShortestPath(const CAmNode<T> & source, const CAmNode<T> & destination, CAmListNodePtrs & resultPath )
		{
			const size_t numberOfNodes = mPointersNodes.size();
			if(numberOfNodes==0)
				return;
			std::vector<weight_t> min_distance;
			std::vector<CAmNode<T>*> previous;
			findShortestPathsFromNode(source, min_distance, previous);
			constructShortestPathTo(destination, previous, resultPath);
		}

		/**
		 * Finds the shortest path from given node to all nodes in listTargets.
		 * Delegates the construction of the path to the caller.
		 *
		 * @param source start node.
		 * @param listTargets destination nodes.
		 * @param cb callabck.
		 */
		void getShortestPath(const CAmNode<T> & source,
							    const CAmListNodePtrs & listTargets,
							   std::function<void(const am_GraphPathPosition_e, CAmNode<T> &)> cb )
		{
			const size_t numberOfNodes = mPointersNodes.size();
			if(numberOfNodes==0)
				return;

			std::vector<weight_t> min_distance;
			std::vector<CAmNode<T>*> previous;
			findShortestPathsFromNode(source, min_distance, previous);

			for(auto it=listTargets.begin(); it!=listTargets.end(); it++)
			{
				CAmNode<T>* node = *it;
				constructShortestPathTo(*node, previous, cb);
			}
		}

		/**
		 * Finds the shortest path between two given nodes.
		 * Delegates the construction of the path to the caller.
		 *
		 * @param source start node.
		 * @param destination destination node.
		 * @param cb callabck.
		 */
		void getShortestPath(const CAmNode<T> & source,
								const CAmNode<T> & destination,
							    std::function<void(const am_GraphPathPosition_e, CAmNode<T> &)> cb )
		{
			const size_t numberOfNodes = mPointersNodes.size();
			if(numberOfNodes==0)
				return;

			std::vector<weight_t> min_distance;
			std::vector<CAmNode<T>*> previous;
			findShortestPathsFromNode(source, min_distance, previous);
			constructShortestPathTo(destination, previous, cb);
		}

		/**
		 * Finds all possible paths between two given nodes.
		 * Delegates the construction of the path to the caller.
		 *
		 * @param src start node.
		 * @param dst destination node.
		 * @param cbShouldVisitNode ask the delegate if we should proceed with the current node.
		 * @param cbWillVisitNode tell the delegate the current node will be visited.
		 * @param cbDidVisitNode tell the delegate the current node was visited.
		 * @param cbDidFindPath return the path to the delegate.
		 */
		void getAllPaths(CAmNode<T> & src,
						 CAmNode<T> & dst,
						 std::function<bool(const CAmNode<T> * )> cbShouldVisitNode,
						 std::function<void(const CAmNode<T> *)> cbWillVisitNode,
						 std::function<void(const CAmNode<T> *)> cbDidVisitNode,
						 std::function<void(const CAmNodeReferenceList & path)> cbDidFindPath)
		{
			IterateThroughAllNodesDelegate delegate;
			delegate.source = &src;
			delegate.destination = &dst;
			delegate.shouldVisitNode = cbShouldVisitNode;
			delegate.willVisitNode = cbWillVisitNode;
			delegate.didVisitNode = cbDidVisitNode;
			delegate.didFindPath = cbDidFindPath;
			delegate.visited.push_back((CAmNode<T>*)&src);
			((CAmNode<T>*)&src)->setStatus(GES_VISITED);
			findAllPaths(delegate);
			((CAmNode<T>*)&src)->setStatus(GES_NOT_VISITED);
		}
	};

}

#endif
