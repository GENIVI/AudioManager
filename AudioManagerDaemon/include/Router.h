/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file Router.h
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

#ifndef ROUTER_H_
#define ROUTER_H_

#include <audiomanagertypes.h>

namespace am
{

class DatabaseHandler;
class ControlSender;

class Router
{
public:
    Router(DatabaseHandler* iDatabaseHandler, ControlSender* iSender);
    am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);
    ~Router();

private:
    am_Error_e findBestWay(am_sinkID_t sinkID, am_sourceID_t sourceID, std::vector<am_RoutingElement_s>& listRoute, std::vector<am_RoutingElement_s>::iterator routeIterator, std::vector<am_gatewayID_t>::iterator gatewayIterator);
    void listPossibleConnectionFormats(const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_ConnectionFormat_e>& listFormats) const;
    void listRestrictedOutputFormatsGateways(const am_gatewayID_t gatewayID, const am_ConnectionFormat_e sinkConnectionFormat, std::vector<am_ConnectionFormat_e>& listFormats) const;
    DatabaseHandler* mDatabaseHandler;
    ControlSender* mControlSender;
};

class RoutingTreeItem
{
public:
    RoutingTreeItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID = 0, RoutingTreeItem *parent = 0);
    void appendChild(RoutingTreeItem *newChild);
    void returnChildItems(std::vector<RoutingTreeItem*> listChildItems);
    am_domainID_t returnDomainID() const;
    am_gatewayID_t returnGatewayID() const;
    virtual ~RoutingTreeItem();
    RoutingTreeItem* returnParent() const;
private:
    std::vector<RoutingTreeItem*> mListChildItems; //!< List of all child items
    am_domainID_t mDomainID; //!< the domain ID of the item
    am_gatewayID_t mGatewayID; //!< the gateway Id
    RoutingTreeItem *mParentItem; //!< pointer to the parent item
};

class RoutingTree
{
public:
    RoutingTree(const am_domainID_t rootDomainID);
    RoutingTreeItem* insertItem(const am_domainID_t domainID, const am_gatewayID_t gatewayID, RoutingTreeItem *parent);
    void getRoute(RoutingTreeItem* targetItem, std::vector<am_gatewayID_t>& listGateways);
    am_domainID_t returnRootDomainID() const;
    RoutingTreeItem* returnRootItem();
    virtual ~RoutingTree();
private:
    RoutingTreeItem mRootItem; //!< pointer to root item
    std::vector<RoutingTreeItem*> mListChild; //!< list of all childs
};

} /* namespace am */
#endif /* ROUTER_H_ */

