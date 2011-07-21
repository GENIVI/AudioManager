/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file Router.h
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 *
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#include "audioManagerIncludes.h"

class DataBaseHandler;
class RoutingReceiver;
class AudioManagerCore;

/**calculates routes from sinks to sources
 *  navigation for audio
 */
class Router {
public:
	Router();
	virtual ~Router();

	/**Registers the database handler
	 *
	 * @param db_handler pointer to dabase handler
	 */
	void registerDatabasehandler(DataBaseHandler* db_handler);

	/**the routing algorithm. Returns a list of routes
	 *
	 * @param onlyfree true if only free gateways shall be used
	 * @param Source_ID the source ID from wich the route should start
	 * @param Sink_ID the sink ID where the route should end
	 * @param ReturnList buffer for the answer.
	 * @return returns true on success
	 */
	bool get_Route_from_Source_ID_to_Sink_ID(const bool onlyfree,
			const source_t Source_ID, const sink_t Sink_ID,
			std::list<genRoute_t>* ReturnList);

private:
	DataBaseHandler* m_dbHandler;
};

/**This represents one Item in the RoutingTree
 *
 */
class RoutingTreeItem {
public:
	RoutingTreeItem();
	virtual ~RoutingTreeItem();

	/**overloaded Constructor
	 *
	 * @param Domain_Id the domain ID where the gateway ends
	 * @param Gateway_Id the gateway ID that connects the domains
	 * @param parent pointer to the parent item in the tree
	 */
	RoutingTreeItem(const domain_t Domain_Id, const gateway_t Gateway_Id = 0,
			RoutingTreeItem *parent = 0);

	/**appends a child from the same type to the tree
	 *
	 * @param child
	 */
	void appendChild(RoutingTreeItem *child);

	/**returns a list of all child items
	 *
	 * @param ChildItems buffer for the child items
	 */
	void returnChildItems(std::list<RoutingTreeItem*> ChildItems);

	/**returns the domain ID
	 *
	 * @return the domain ID
	 */
	domain_t returnDomainID();

	/**returns the gateway ID
	 *
	 * @return gateway ID
	 */
	gateway_t returnGatewayID(void);

	/** is used to retrieve the parent item
	 *
	 * @return pointer to parent RoutingTreeItem
	 */
	RoutingTreeItem* return_Parent();

private:
	std::list<RoutingTreeItem*> childItems; //!< List of all child items
	domain_t m_domainID; //!< the domain ID of the item
	gateway_t m_gatewayID; //!< the gateway Id
	RoutingTreeItem *parentItem; //!< pointer to the parent item
};

/**The routing tree iself
 *
 */
class RoutingTree {
public:

	/**constructor must always be called with the root domain ID
	 *
	 * @param Root_Domain_ID
	 */
	RoutingTree(const domain_t Root_Domain_ID);
	~RoutingTree();

	/**Insert an item in the Tree with the parent parentItem
	 *
	 * @param Domain_ID the domain Id
	 * @param Gateway_ID the gateway ID
	 * @param parentItem pointer to the parent Item
	 * @return returns a pointer to the new item
	 */
	RoutingTreeItem* insertItem(const domain_t Domain_ID,
			const gateway_t Gateway_ID, RoutingTreeItem* parentItem);

	/**reverses the tree to get a route to the TargetItem
	 *
	 * @param TargetItem pointer to the Item from which should be reversed
	 * @param route pointer to a list of gateway IDs that need to be connected
	 * @return the length of the route.
	 */
	int getRoute(RoutingTreeItem* TargetItem, std::list<gateway_t>* route);

	/**returns the DomainId of the rootItem
	 *
	 * @return domain ID of the root Item
	 */
	domain_t returnRootDomainID(void);

	/**returns a pointer to the rootitem
	 *
	 * @return pointer to the rootitem
	 */
	RoutingTreeItem* returnRootItem(void);

private:
	RoutingTreeItem m_rootItem; //!< pointer to root item
	std::list<RoutingTreeItem*> m_allChildList; //!< list of all childs
};

/**This class is responsible for loading the RoutingInterface Plugins
 * In order to let a plugin be loaded by the BusManager, just add it in main.cpp  like this
 * @code Q_IMPORT_PLUGIN(RoutingPlugin) @code
 */
class Bushandler {

public:
	Bushandler() {
	}
	;
	virtual ~Bushandler() {
	}
	;

	/**by calling this, all bus plugins are loaded
	 *
	 */
	void load_Bus_plugins();

	/**needed to register the m_receiver Instance
	 * via this function the receiver instance is given to the plugins so that they can call methods to talk to the audiomanager
	 * @param m_receiver pointer to the receiver
	 */
	void registerReceiver(RoutingReceiver* Receiver);

	void registerCore (AudioManagerCore* core);

	/**By calling this function the plugins are called to startup.
	 * Init functions etc are done by the plugins in this phase
	 */
	void StartupInterfaces();

	/**This function returns the pointer to the actual interface for a Bus
	 *
	 * @param bus the name of the bus
	 * @return pointer to the interface
	 */
	RoutingSendInterface* getInterfaceforBus(std::string bus);

	/**This signal informs the plugins that the AudioManager is ready to register Domains Sinks and Sources
	 *
	 */
	void signal_system_ready(void);

private:
	/**Struct to save Name and Interface together
	 *
	 */
	struct Bus {
		RoutingSendInterface* sendInterface;
		std::string Name;
	};

	std::list<Bus> Busses; //!< list of all busses
	RoutingReceiver* m_receiver; //!< pointer to the routing receiver
	AudioManagerCore* m_core;
};

#endif /* ROUTER_H_ */
