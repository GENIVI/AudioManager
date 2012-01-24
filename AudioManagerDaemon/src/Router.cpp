/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file Router.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#include "Router.h"
#include "DatabaseHandler.h"
#include <assert.h>
#include <algorithm>
#include <vector>
#include <iterator>

using namespace am;

am_Error_e getConnectionFormatChoice(const am_sinkID_t, const am_sourceID_t, const std::vector<am_ConnectionFormat_e>& listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e>& listPriorityConnectionFormats)
{
    listPriorityConnectionFormats = listPossibleConnectionFormats;
    return (E_OK);
}

Router::Router(DatabaseHandler *iDatabaseHandler) :
        mDatabaseHandler(iDatabaseHandler)
{
    assert(mDatabaseHandler);
}

am_Error_e Router::getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & returnList)
{
    //first find out in which domains the source and sink are
    am_domainID_t sourceDomainID;
    am_domainID_t sinkDomainID;
    if (mDatabaseHandler->getDomainOfSource(sourceID, sourceDomainID) != E_OK)
        return (E_NON_EXISTENT);
    if (mDatabaseHandler->getDomainOfSink(sinkID, sinkDomainID) != E_OK)
        return (E_NON_EXISTENT);

    RoutingTree routingtree(sourceDomainID); //Build up a Tree from the Source_Domain to every other domain.
    std::vector<RoutingTreeItem*> flattree; //This list is the flat tree
    std::vector<RoutingTreeItem*> matchtree; //This List holds all TreeItems which have the right Domain Sink IDs
    std::vector<am_gatewayID_t> listGatewayID; //holds all gateway ids of the route
    am_RoutingElement_s routingElement;
    std::vector<am_RoutingElement_s> actualRoutingElement; //intermediate list of current routing pairs
    am_Route_s actualRoute; //holds the actual Route
    am_sourceID_t lastSource = 0;

    //TODO: kind of unclean. The separation between database and router could be better.
    mDatabaseHandler->getRoutingTree(onlyfree, &routingtree, &flattree); //Build up the tree out of the database as

    //we go through the returned flattree and look for our sink, after that flattree holds only treeItems that match
    std::vector<RoutingTreeItem*>::iterator flatIterator = flattree.begin();
    for (; flatIterator != flattree.end(); ++flatIterator)
    {
        if ((*flatIterator)->returnDomainID() != sinkDomainID)
        {
            flatIterator = flattree.erase(flatIterator);
        }
    }

    //No we need to trace back the routes for each entry in matchtree
    flatIterator = flattree.begin();
    for (; flatIterator != flattree.end(); ++flatIterator)
    {
        //getting the route for the actual item
        routingtree.getRoute(*flatIterator, &listGatewayID); //This gives only the Gateway IDs we need more

        //go throught the gatewayids and get more information
        std::vector<am_gatewayID_t>::iterator gatewayIterator = listGatewayID.begin();
        for (; gatewayIterator != listGatewayID.end(); ++gatewayIterator)
        {
            am_Gateway_s gatewayData;
            if (mDatabaseHandler->getGatewayInfoDB(*gatewayIterator, gatewayData) != E_OK)
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
        if (findBestWay(actualRoutingElement, routingInterator, gatewayIterator, 0) != E_OK)
            continue;

        //add the route to the list of routes...
        actualRoute.sourceID = sourceID;
        actualRoute.sinkID = sinkID;
        actualRoute.route = actualRoutingElement;
        returnList.push_back(actualRoute);
    }
    return (E_OK);
}

void Router::listPossibleConnectionFormats(const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_ConnectionFormat_e>& listFormats) const
{
    std::vector<am_ConnectionFormat_e> listSourceFormats;
    std::vector<am_ConnectionFormat_e> listSinkFormats;
    mDatabaseHandler->getListSinkConnectionFormats(sinkID, listSinkFormats);
    mDatabaseHandler->getListSourceConnectionFormats(sourceID, listSourceFormats);
    std::insert_iterator<std::vector<am_ConnectionFormat_e> > inserter(listFormats, listFormats.begin());
    set_intersection(listSourceFormats.begin(), listSourceFormats.end(), listSinkFormats.begin(), listSinkFormats.end(), inserter);
}

am_Error_e Router::findBestWay(std::vector<am_RoutingElement_s> & listRoute, std::vector<am_RoutingElement_s>::iterator routeIterator, std::vector<am_gatewayID_t>::iterator gatewayIterator, int choiceNumber)
{
    std::vector<am_ConnectionFormat_e> listConnectionFormats;
    std::vector<am_ConnectionFormat_e> listPriorityConnectionFormats;
    //get best connection format for the first connection, now
    listPossibleConnectionFormats(routeIterator->sourceID, routeIterator->sinkID, listConnectionFormats);

    //if we get to the point that no choice makes sense, return ...
    if (choiceNumber >= (int) listConnectionFormats.size())
        return (E_NOT_POSSIBLE);

    //if we have not just started, we need to take care about the gateways...
    if (routeIterator != listRoute.begin())
    {
        //since we have to deal with Gateways, there are restrictions what connectionFormat we can take. So we need to take the subset of connections that are restricted:
        std::vector<am_ConnectionFormat_e> listRestrictedConnectionFormats;
        std::insert_iterator<std::vector<am_ConnectionFormat_e> > inserter(listConnectionFormats, listConnectionFormats.begin());
        std::vector<am_RoutingElement_s>::iterator tempIterator(routeIterator);
        tempIterator--;
        listRestrictedOutputFormatsGateways(*gatewayIterator, (tempIterator)->connectionFormat, listRestrictedConnectionFormats);
        set_intersection(listConnectionFormats.begin(), listConnectionFormats.end(), listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end(), inserter);
        gatewayIterator++;
    }

    //let the controller decide:
    getConnectionFormatChoice(routeIterator->sinkID, routeIterator->sourceID, listConnectionFormats, listPriorityConnectionFormats);

    //go back one step, if we cannot find a format and take the next best!
    if (listPriorityConnectionFormats.empty())
    {
        findBestWay(listRoute, --routeIterator, --gatewayIterator, ++choiceNumber);
    }

    routeIterator->connectionFormat = listPriorityConnectionFormats.at(choiceNumber);

    //ok, we are through and found a way, if not, take the next part of the route and start with toplevel
    if (routeIterator < listRoute.end())
    {
        findBestWay(listRoute, ++routeIterator, gatewayIterator, 0);
    }
    return (E_OK);
}

void Router::listRestrictedOutputFormatsGateways(const am_gatewayID_t gatewayID, const am_ConnectionFormat_e sinkConnectionFormat, std::vector<am_ConnectionFormat_e> & listFormats) const
{
    listFormats.clear();
    am_Gateway_s gatewayData;
    mDatabaseHandler->getGatewayInfoDB(gatewayID, gatewayData);
    std::vector<am_ConnectionFormat_e>::const_iterator rowSinkIterator = gatewayData.listSinkFormats.begin();
    std::vector<bool>::const_iterator matrixIterator = gatewayData.convertionMatrix.begin();

    //find the row number of the sink
    rowSinkIterator = find(gatewayData.listSinkFormats.begin(), gatewayData.listSinkFormats.end(), sinkConnectionFormat);
    int rowNumberSink = rowSinkIterator - gatewayData.listSinkFormats.begin();

    //go through the convertionMatrix and find out if the conversion is possible, if yes, add connectionFormat ...
    matrixIterator + rowNumberSink;

    //iterate line-wise through the matrix and add more formats
    do
    {
        if (*matrixIterator)
        {
            listFormats.push_back(gatewayData.listSourceFormats.at(matrixIterator - gatewayData.convertionMatrix.begin()));
        }
        matrixIterator += gatewayData.listSinkFormats.size();
    } while (matrixIterator - gatewayData.convertionMatrix.begin() < (int) gatewayData.listSourceFormats.size());
}

Router::~Router()
{
}

RoutingTreeItem::RoutingTreeItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, RoutingTreeItem *parent) :
        mDomainID(domainID), //
        mGatewayID(gatewayID), //
        mParentItem(parent)
{
    assert(mDomainID!=0);
}

void RoutingTreeItem::appendChild(RoutingTreeItem *newChild)
{
    assert(newChild);
    mListChildItems.push_back(newChild);
}

void RoutingTreeItem::returnChildItems(std::vector<RoutingTreeItem*> listChildItems)
{
    listChildItems = mListChildItems;
}

am_domainID_t RoutingTreeItem::returnDomainID() const
{
    return (mDomainID);
}

am_gatewayID_t RoutingTreeItem::returnGatewayID() const
{
    return (mGatewayID);
}

RoutingTreeItem* RoutingTreeItem::returnParent() const
{
    return (mParentItem);
}

RoutingTreeItem::~RoutingTreeItem()
{
}

RoutingTree::RoutingTree(const am_domainID_t rootDomainID) :
        mRootItem(RoutingTreeItem(rootDomainID))
{
    assert(rootDomainID!=0);
}

RoutingTreeItem *RoutingTree::insertItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, RoutingTreeItem *parent)
{
    RoutingTreeItem *newTree = new RoutingTreeItem(domainID, gatewayID, parent);
    parent->appendChild(newTree);
    mListChild.push_back(newTree);
    return newTree;
}

void RoutingTree::getRoute(RoutingTreeItem *targetItem, std::vector<am_gatewayID_t> *listGateways)
{
    RoutingTreeItem *parentItem = targetItem;
    while (parentItem != &mRootItem)
    {
        listGateways->push_back(parentItem->returnGatewayID());
        parentItem = parentItem->returnParent();
    }
}

am_domainID_t RoutingTree::returnRootDomainID() const
{
    return (mRootItem.returnDomainID());
}

RoutingTreeItem *RoutingTree::returnRootItem()
{
    return (&mRootItem);
}

RoutingTree::~RoutingTree()
{
    std::vector<RoutingTreeItem*>::iterator it = mListChild.begin();
    for (; it != mListChild.end(); ++it)
    {
        delete *it;
    }
}

