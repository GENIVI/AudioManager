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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmRouter.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmRouter.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>
#include "CAmDatabaseHandler.h"
#include "CAmControlSender.h"

namespace am {

CAmRouter::CAmRouter(CAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iSender) :
        mpDatabaseHandler(iDatabaseHandler), //
        mpControlSender(iSender)
{
    assert(mpDatabaseHandler);
    assert(mpControlSender);
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
    //first find out in which domains the source and sink are
    am_domainID_t sourceDomainID;
    am_domainID_t sinkDomainID;
    if (mpDatabaseHandler->getDomainOfSource(sourceID, sourceDomainID) != E_OK)
        return (E_NON_EXISTENT);
    if (mpDatabaseHandler->getDomainOfSink(sinkID, sinkDomainID) != E_OK)
        return (E_NON_EXISTENT);

    if (sourceDomainID == sinkDomainID) //shortcut if the domains are the same...
    {
        //first get the list of possible connection formats
        std::vector<am_ConnectionFormat_e> listFormats, listPriorityConnectionFormats;
        listPossibleConnectionFormats(sourceID, sinkID, listFormats);

        //dummy route
        am_Route_s route;
        route.sinkID = sinkID;
        route.sourceID = sourceID;
        route.route.clear();

        //get the prio of the Controller:
        mpControlSender->getConnectionFormatChoice(sourceID, sinkID, route, listFormats, listPriorityConnectionFormats);

        //no possible connection, so no route ! But we report OK since there is no fault ...
        if (listPriorityConnectionFormats.empty())
            return (E_OK);

        //return the first item as route:
        am_RoutingElement_s routingElement;
        routingElement.sourceID = sourceID;
        routingElement.sinkID = sinkID;
        routingElement.connectionFormat = listPriorityConnectionFormats[0];
        routingElement.domainID = sourceDomainID;

        //Now get a route:
        am_Route_s actualRoute;
        actualRoute.route.push_back(routingElement);
        actualRoute.sourceID = sourceID;
        actualRoute.sinkID = sinkID;

        //push it to the return list - we are done here ...
        returnList.push_back(actualRoute);
        return (E_OK);

    }
    CAmRoutingTree routingtree(sourceDomainID); //Build up a Tree from the Source_Domain to every other domain.
    std::vector<CAmRoutingTreeItem*> flattree; //This list is the flat tree
    std::vector<CAmRoutingTreeItem*> matchtree;
    std::vector<am_gatewayID_t> listGatewayID; //holds all gateway ids of the route
    am_RoutingElement_s routingElement;
    am_Route_s actualRoute; //holds the actual Route
    am_sourceID_t lastSource = 0;

    mpDatabaseHandler->getRoutingTree(onlyfree, routingtree, flattree); //Build up the tree out of the database as

    //we go through the returned flattree and look for our sink, after that flattree holds only treeItems that match
    std::vector<CAmRoutingTreeItem*>::iterator iterator = flattree.begin();
    for (; iterator != flattree.end(); ++iterator)
    {
        if ((*iterator)->returnDomainID() == sinkDomainID)
        {
            matchtree.push_back(*iterator);
        }
    }

    //No we need to trace back the routes for each entry in matchtree
    iterator = matchtree.begin();
    for (; iterator != matchtree.end(); ++iterator)
    {
        std::vector<am_RoutingElement_s> actualRoutingElement; //intermediate list of current routing pairs
        //getting the route for the actual item
        routingtree.getRoute(*iterator, listGatewayID); //This gives only the Gateway IDs we need more

        //go throught the gatewayids and get more information
        std::vector<am_gatewayID_t>::iterator gatewayIterator = listGatewayID.begin();
        for (; gatewayIterator != listGatewayID.end(); ++gatewayIterator)
        {
            am_Gateway_s gatewayData;
            if (mpDatabaseHandler->getGatewayInfoDB(*gatewayIterator, gatewayData) != E_OK)
                return (E_UNKNOWN);

            //at the beginning of the route, we connect first the source to the first gateway
            if (gatewayIterator == listGatewayID.begin())
            {
                routingElement.sourceID = sourceID;
                routingElement.domainID = sourceDomainID;
            }
            else
            {
                routingElement.sourceID = lastSource;
                routingElement.domainID = gatewayData.domainSinkID;
            }
            routingElement.sinkID = gatewayData.sinkID;
            actualRoutingElement.push_back(routingElement);
            lastSource = gatewayData.sourceID;
        }
        //at the end of the route, connect to the sink !
        routingElement.sourceID = lastSource;
        routingElement.sinkID = sinkID;
        routingElement.domainID = sinkDomainID;
        actualRoutingElement.push_back(routingElement);

        //So now we got the route, what is missing are the connectionFormats...

        //Step through the routes and try to use always the best connectionFormat
        std::vector<am_RoutingElement_s>::iterator routingInterator = actualRoutingElement.begin();
        gatewayIterator = listGatewayID.begin();
        if (findBestWay(sinkID, sourceID, actualRoutingElement, routingInterator, gatewayIterator) != E_OK)
        {
            continue;
        }

        //add the route to the list of routes...
        actualRoute.sourceID = sourceID;
        actualRoute.sinkID = sinkID;
        actualRoute.route = actualRoutingElement;
        returnList.push_back(actualRoute);
    }
    return (E_OK);
}

void CAmRouter::listPossibleConnectionFormats(const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_ConnectionFormat_e>& listFormats) const
{
    std::vector<am_ConnectionFormat_e> listSourceFormats;
    std::vector<am_ConnectionFormat_e> listSinkFormats;
    mpDatabaseHandler->getListSinkConnectionFormats(sinkID, listSinkFormats);
    mpDatabaseHandler->getListSourceConnectionFormats(sourceID, listSourceFormats);
    std::sort(listSinkFormats.begin(), listSinkFormats.end()); //todo: this might be not needed if we use strictly sorted input
    std::sort(listSourceFormats.begin(), listSourceFormats.end()); //todo: this might be not needed if we use strictly sorted input
    std::insert_iterator<std::vector<am_ConnectionFormat_e> > inserter(listFormats, listFormats.begin());
    set_intersection(listSourceFormats.begin(), listSourceFormats.end(), listSinkFormats.begin(), listSinkFormats.end(), inserter);
}

am_Error_e CAmRouter::findBestWay(am_sinkID_t sinkID, am_sourceID_t sourceID, std::vector<am_RoutingElement_s> & listRoute, std::vector<am_RoutingElement_s>::iterator routeIterator, std::vector<am_gatewayID_t>::iterator gatewayIterator)
{
    am_Error_e returnError = E_NOT_POSSIBLE;
    std::vector<am_ConnectionFormat_e> listConnectionFormats;
    std::vector<am_ConnectionFormat_e> listMergeConnectionFormats;
    std::vector<am_ConnectionFormat_e> listPriorityConnectionFormats;
    std::vector<am_RoutingElement_s>::iterator nextIterator = routeIterator + 1;
    //get best connection format
    listPossibleConnectionFormats(routeIterator->sourceID, routeIterator->sinkID, listConnectionFormats);

    //if we have not just started, we need to take care about the gateways...
    if (routeIterator != listRoute.begin())
    {
        //since we have to deal with Gateways, there are restrictions what connectionFormat we can take. So we need to take the subset of connections that are restricted:
        std::vector<am_ConnectionFormat_e> listRestrictedConnectionFormats;
        std::insert_iterator<std::vector<am_ConnectionFormat_e> > inserter(listMergeConnectionFormats, listMergeConnectionFormats.begin());
        std::vector<am_RoutingElement_s>::iterator tempIterator(routeIterator);
        tempIterator--;
        listRestrictedOutputFormatsGateways(*gatewayIterator, (tempIterator)->connectionFormat, listRestrictedConnectionFormats);
        std::sort(listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end()); //todo: this might be not needed if we use strictly sorted input
        set_intersection(listConnectionFormats.begin(), listConnectionFormats.end(), listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end(), inserter);
        gatewayIterator++;
    }
    else
    {
        listMergeConnectionFormats = listConnectionFormats;
    }

    am_Route_s route;
    route.sinkID = sinkID;
    route.sourceID = sourceID;
    route.route = listRoute;

    //let the controller decide:
    mpControlSender->getConnectionFormatChoice(routeIterator->sourceID, routeIterator->sinkID, route, listMergeConnectionFormats, listPriorityConnectionFormats);

    //we have the list sorted after prios - now we try one after the other with the next part of the route
    std::vector<am_ConnectionFormat_e>::iterator connectionFormatIterator = listPriorityConnectionFormats.begin();

    //here we need to check if we are at the end and stop
    if (nextIterator == listRoute.end())
    {
        if (!listPriorityConnectionFormats.empty())
        {
            routeIterator->connectionFormat = listPriorityConnectionFormats.front();
            return (E_OK);
        }
        else
            return (E_NOT_POSSIBLE);
    }

    for (; connectionFormatIterator != listPriorityConnectionFormats.end(); ++connectionFormatIterator)
    {
        routeIterator->connectionFormat = *connectionFormatIterator;
        if ((returnError = findBestWay(sinkID, sourceID, listRoute, nextIterator, gatewayIterator)) == E_OK)
        {
            break;
        }
    }

    return (returnError);
}

void CAmRouter::listRestrictedOutputFormatsGateways(const am_gatewayID_t gatewayID, const am_ConnectionFormat_e sinkConnectionFormat, std::vector<am_ConnectionFormat_e> & listFormats) const
{
    listFormats.clear();
    am_Gateway_s gatewayData;
    mpDatabaseHandler->getGatewayInfoDB(gatewayID, gatewayData);
    std::vector<am_ConnectionFormat_e>::const_iterator rowSinkIterator = gatewayData.listSinkFormats.begin();
    std::vector<bool>::const_iterator matrixIterator = gatewayData.convertionMatrix.begin();

    //find the row number of the sink
    rowSinkIterator = find(gatewayData.listSinkFormats.begin(), gatewayData.listSinkFormats.end(), sinkConnectionFormat);
    int rowNumberSink = rowSinkIterator - gatewayData.listSinkFormats.begin();

    //go through the convertionMatrix and find out if the conversion is possible, if yes, add connectionFormat ...
    std::advance(matrixIterator, rowNumberSink);

    //iterate line-wise through the matrix and add more formats
    do
    {
        if (*matrixIterator)
        {
            listFormats.push_back(gatewayData.listSourceFormats.at((matrixIterator - gatewayData.convertionMatrix.begin()) / gatewayData.listSinkFormats.size()));
        }
        std::advance(matrixIterator, gatewayData.listSinkFormats.size());
    } while (gatewayData.convertionMatrix.end() - matrixIterator > 0);
}

CAmRouter::~CAmRouter()
{
}

CAmRoutingTreeItem::CAmRoutingTreeItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, CAmRoutingTreeItem *parent) :
        mDomainID(domainID), //
        mGatewayID(gatewayID), //
        mpParentItem(parent)
{
    assert(mDomainID!=0);
}

void CAmRoutingTreeItem::appendChild(CAmRoutingTreeItem *newChild)
{
    assert(newChild);
    mListChildItems.push_back(newChild);
}

void CAmRoutingTreeItem::returnChildItems(std::vector<CAmRoutingTreeItem*> listChildItems)
{
    listChildItems = mListChildItems;
}

am_domainID_t CAmRoutingTreeItem::returnDomainID() const
{
    return (mDomainID);
}

am_gatewayID_t CAmRoutingTreeItem::returnGatewayID() const
{
    return (mGatewayID);
}

CAmRoutingTreeItem* CAmRoutingTreeItem::returnParent() const
{
    return (mpParentItem);
}

CAmRoutingTreeItem::~CAmRoutingTreeItem()
{
}

CAmRoutingTree::CAmRoutingTree(const am_domainID_t rootDomainID) :
        mRootItem(CAmRoutingTreeItem(rootDomainID))
{
    assert(rootDomainID!=0);
}

CAmRoutingTreeItem *CAmRoutingTree::insertItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, CAmRoutingTreeItem *parent)
{
    CAmRoutingTreeItem *newTree = new CAmRoutingTreeItem(domainID, gatewayID, parent);
    parent->appendChild(newTree);
    mListChild.push_back(newTree);
    return (newTree);
}

void CAmRoutingTree::getRoute(CAmRoutingTreeItem *targetItem, std::vector<am_gatewayID_t>& listGateways)
{
    listGateways.clear();
    CAmRoutingTreeItem *parentItem = targetItem;
    while (parentItem != &mRootItem)
    {
        listGateways.push_back(parentItem->returnGatewayID());
        parentItem = parentItem->returnParent();
    }
    std::reverse(listGateways.begin(), listGateways.end());
}

am_domainID_t CAmRoutingTree::returnRootDomainID() const
{
    return (mRootItem.returnDomainID());
}

CAmRoutingTreeItem *CAmRoutingTree::returnRootItem()
{
    return (&mRootItem);
}

CAmRoutingTree::~CAmRoutingTree()
{
    std::vector<CAmRoutingTreeItem*>::iterator it = mListChild.begin();
    for (; it != mListChild.end(); ++it)
    {
        delete *it;
    }
}
}
