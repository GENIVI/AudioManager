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
 * \file CAmRouter.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#include "audiomanagertypes.h"

namespace am
{

class CAmDatabaseHandler;
class CAmControlSender;

/**
 * Implements an autorouting algorithm for connecting sinks and sources via different audio domains.
 */
class CAmRouter
{
public:
    CAmRouter(CAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iSender);
    ~CAmRouter();
    am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);

private:
    am_Error_e findBestWay(am_sinkID_t sinkID, am_sourceID_t sourceID, std::vector<am_RoutingElement_s>& listRoute, std::vector<am_RoutingElement_s>::iterator routeIterator, std::vector<am_gatewayID_t>::iterator gatewayIterator);
    void listPossibleConnectionFormats(const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_ConnectionFormat_e>& listFormats) const;
    void listRestrictedOutputFormatsGateways(const am_gatewayID_t gatewayID, const am_ConnectionFormat_e sinkConnectionFormat, std::vector<am_ConnectionFormat_e>& listFormats) const;
    CAmDatabaseHandler* mpDatabaseHandler; //!< pointer to database handler
    CAmControlSender* mpControlSender; //!< pointer the controlsender - is used to retrieve information for the optimal route
};

/**
 * an item in the routing tree
 */
class CAmRoutingTreeItem
{
public:
    CAmRoutingTreeItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID = 0, CAmRoutingTreeItem *parent = 0);
    ~CAmRoutingTreeItem();
    void appendChild(CAmRoutingTreeItem *newChild); //!< appends a new child
    void returnChildItems(std::vector<CAmRoutingTreeItem*> listChildItems); //!< returns the list of childs
    am_domainID_t returnDomainID() const; //!< returns the domainID of the tree
    am_gatewayID_t returnGatewayID() const; //!< returns the gatewayID of the tree
    CAmRoutingTreeItem* returnParent() const; //!< returns the parent item of the tree
private:
    std::vector<CAmRoutingTreeItem*> mListChildItems; //!< List of all child items
    am_domainID_t mDomainID; //!< the domain ID of the item
    am_gatewayID_t mGatewayID; //!< the gateway Id
    CAmRoutingTreeItem *mpParentItem; //!< pointer to the parent item
};

/**
 * a routing tree
 */
class CAmRoutingTree
{
public:
    CAmRoutingTree(const am_domainID_t rootDomainID);
    ~CAmRoutingTree();
    CAmRoutingTreeItem* insertItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, CAmRoutingTreeItem *parent);
    void getRoute(CAmRoutingTreeItem* targetItem, std::vector<am_gatewayID_t>& listGateways);
    am_domainID_t returnRootDomainID() const;
    CAmRoutingTreeItem* returnRootItem();
private:
    CAmRoutingTreeItem mRootItem; //!< pointer to root item
    std::vector<CAmRoutingTreeItem*> mListChild; //!< list of all childs
};

} /* namespace am */
#endif /* ROUTER_H_ */

