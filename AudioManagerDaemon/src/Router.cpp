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
#include "ControlSender.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>

using namespace am;

Router::Router(DatabaseHandler* iDatabaseHandler, ControlSender* iSender) :
        mDatabaseHandler(iDatabaseHandler), //
        mControlSender(iSender)
{
    assert(mDatabaseHandler);
    assert(mControlSender);
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
    std::vector<RoutingTreeItem*> matchtree;
    std::vector<am_gatewayID_t> listGatewayID; //holds all gateway ids of the route
    am_RoutingElement_s routingElement;
    am_Route_s actualRoute; //holds the actual Route
    am_sourceID_t lastSource = 0;

    mDatabaseHandler->getRoutingTree(onlyfree, routingtree, flattree); //Build up the tree out of the database as

    //we go through the returned flattree and look for our sink, after that flattree holds only treeItems that match
    std::vector<RoutingTreeItem*>::iterator iterator = flattree.begin();
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
        if (findBestWay(actualRoutingElement, routingInterator, gatewayIterator) != E_OK)
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

void Router::listPossibleConnectionFormats(const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_ConnectionFormat_e>& listFormats) const
{
    std::vector<am_ConnectionFormat_e> listSourceFormats;
    std::vector<am_ConnectionFormat_e> listSinkFormats;
    mDatabaseHandler->getListSinkConnectionFormats(sinkID, listSinkFormats);
    mDatabaseHandler->getListSourceConnectionFormats(sourceID, listSourceFormats);
    std::sort(listSinkFormats.begin(),listSinkFormats.end()); //todo: this might be not needed if we use strictly sorted input
    std::sort(listSourceFormats.begin(),listSourceFormats.end()); //todo: this might be not needed if we use strictly sorted input
    std::insert_iterator<std::vector<am_ConnectionFormat_e> > inserter(listFormats, listFormats.begin());
    set_intersection(listSourceFormats.begin(), listSourceFormats.end(), listSinkFormats.begin(), listSinkFormats.end(), inserter);
}

am_Error_e Router::findBestWay(std::vector<am_RoutingElement_s> & listRoute, std::vector<am_RoutingElement_s>::iterator routeIterator, std::vector<am_gatewayID_t>::iterator gatewayIterator)
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
        std::sort(listRestrictedConnectionFormats.begin(),listRestrictedConnectionFormats.end());           //todo: this might be not needed if we use strictly sorted input
        set_intersection(listConnectionFormats.begin(), listConnectionFormats.end(), listRestrictedConnectionFormats.begin(), listRestrictedConnectionFormats.end(), inserter);
        gatewayIterator++;
    }
    else
    {
        listMergeConnectionFormats = listConnectionFormats;
    }

    //let the controller decide:
    mControlSender->getConnectionFormatChoice(routeIterator->sourceID, routeIterator->sinkID, listMergeConnectionFormats, listPriorityConnectionFormats);

    //we have the list sorted after prios - now we try one after the other with the next part of the route
    std::vector<am_ConnectionFormat_e>::iterator connectionFormatIterator = listPriorityConnectionFormats.begin();

    //here we need to check if we are at the end and stop
    if (nextIterator == listRoute.end())
    {
        if (!listPriorityConnectionFormats.empty())
        {
            routeIterator->connectionFormat = listPriorityConnectionFormats.front();
            return E_OK;
        }
        else
            return E_NOT_POSSIBLE;
    }

    for (; connectionFormatIterator != listPriorityConnectionFormats.end(); ++connectionFormatIterator)
    {
        routeIterator->connectionFormat = *connectionFormatIterator;
        if ((returnError = findBestWay(listRoute, nextIterator, gatewayIterator)) == E_OK)
        {
            break;
        }
    }

    return (returnError);
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

void RoutingTree::getRoute(RoutingTreeItem *targetItem, std::vector<am_gatewayID_t>& listGateways)
{
    listGateways.clear();
    RoutingTreeItem *parentItem = targetItem;
    while (parentItem != &mRootItem)
    {
        listGateways.push_back(parentItem->returnGatewayID());
        parentItem = parentItem->returnParent();
    }
    std::reverse(listGateways.begin(), listGateways.end());
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

