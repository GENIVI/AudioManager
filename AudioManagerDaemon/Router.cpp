/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file Router.cpp
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

#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <string>

#include "audioManagerIncludes.h"

const char* routingPluginDirectories[] = { "/home/christian/workspace/gitserver/build/plugins/routing"};
uint routingPluginDirectoriesCount = sizeof(routingPluginDirectories) / sizeof(routingPluginDirectories[0]);

Router::Router() {
}

Router::~Router() {
}

void Router::registerDatabasehandler(DataBaseHandler* db_handler) {
	m_dbHandler = db_handler;
}

bool Router::get_Route_from_Source_ID_to_Sink_ID(const bool onlyfree,
		const source_t Source_ID, const sink_t Sink_ID,
		std::list<genRoute_t>* ReturnList) {

	domain_t Source_Domain = m_dbHandler->get_Domain_ID_from_Source_ID(
			Source_ID); //first find out in which domains the source and sink are
	domain_t Sink_Domain = m_dbHandler->get_Domain_ID_from_Sink_ID(Sink_ID);

	if (Source_Domain == -1 || Sink_Domain == -1) {
		return false;
	} //if source or sink does not exists, exit here

	RoutingTree routingtree(Source_Domain); //Build up a Tree from the Source_Domain to every other domain.
	std::list<RoutingTreeItem*> flattree; //This list is the flat tree
	std::list<RoutingTreeItem*> matchtree; //This List holds all TreeItems which have the right Domain Sink IDs
	std::list<gateway_t> gwids; //holds all gateway ids of the route
	genRoutingElement_t element;
	std::list<genRoutingElement_t> actualRoutingElement;//intermediate list of current routing pairs
	genRoute_t actualRoute; //holds the actual Route
	source_t ReturnSource = 0;
	sink_t ReturnSink = 0;
	source_t LastSource = 0;
	domain_t ReturnDomain = 0;

	//TODO: kind of unclean. The separation between database and router could be better.
	m_dbHandler->get_Domain_ID_Tree(onlyfree, &routingtree, &flattree); //Build up the tree out of the database as

	//we go through the returned flattree and look for our sink, after that flattree holds only treeItems that match
	for(std::list<RoutingTreeItem*>::iterator rTree=flattree.begin();rTree!=flattree.end();rTree++) {
		RoutingTreeItem *p=*rTree;
		if (p->returnDomainID() == Sink_Domain) {
			matchtree.push_back(*rTree);
		}
	}

	//No we need to trace back the routes for each entry in matchtree
	for(std::list<RoutingTreeItem*>::iterator match=matchtree.begin(); match!=matchtree.end(); match++)
		{
			//getting the route for the actual item
			actualRoute.len = routingtree.getRoute(*match, &gwids); //This gives only the Gateway IDs we need more

			//go throught the gatewayids and get more information
			for (std::list<gateway_t>::iterator i=gwids.begin(); i!=gwids.end();i++) {
				m_dbHandler->get_Gateway_Source_Sink_Domain_ID_from_ID(*i, &ReturnSource, &ReturnSink,&ReturnDomain);
				//first routing pair is source to ReturnSink of course;
//				if (i == 0) {
//					element.source = Source_ID;
//					element.sink = ReturnSink;
//					element.Domain_ID = Source_Domain;
//				}
//
//				else {
//					element.source = LastSource;
//					element.sink = ReturnSink;
//					element.Domain_ID = ReturnDomain;
//				}
				actualRoutingElement.push_back(element);
				LastSource = ReturnSource;
			}
			element.source = LastSource;
			element.sink = Sink_ID;
			element.Domain_ID = Sink_Domain;
			actualRoutingElement.push_back(element);

			actualRoute.Source_ID = Source_ID;
			actualRoute.Sink_ID = Sink_ID;
			actualRoute.route = actualRoutingElement;
			ReturnList->push_back(actualRoute);
		}

	return true;
	//TODO: return actual status !
}

RoutingTreeItem::RoutingTreeItem(const domain_t Domain_Id,
		const gateway_t Gateway_Id, RoutingTreeItem *parent) {
	parentItem = parent;
	m_domainID = Domain_Id;
	m_gatewayID = Gateway_Id;
}

RoutingTreeItem::RoutingTreeItem() {

}

RoutingTreeItem::~RoutingTreeItem() {
	for (std::list<RoutingTreeItem*>::iterator i=childItems.begin();i!=childItems.end();i++) {
		delete *i;
	}
}

void RoutingTreeItem::appendChild(RoutingTreeItem *item) {
	childItems.push_back(item);
}

RoutingTreeItem *RoutingTreeItem::return_Parent() {
	return parentItem;
}

domain_t RoutingTreeItem::returnDomainID() {
	return m_domainID;
}

gateway_t RoutingTreeItem::returnGatewayID() {
	return m_gatewayID;
}

RoutingTree::RoutingTree(const domain_t Root_ID) :
	m_rootItem(RoutingTreeItem(Root_ID)) {
}

RoutingTree::~RoutingTree() {
}

RoutingTreeItem* RoutingTree::insertItem(const domain_t Domain_ID,
		const gateway_t Gateway_ID, RoutingTreeItem *parentItem) {
	RoutingTreeItem *newTree = new RoutingTreeItem(Domain_ID, Gateway_ID,
			parentItem);
	parentItem->appendChild(newTree);
	m_allChildList.push_back(newTree);
	return newTree;
}

int RoutingTree::getRoute(RoutingTreeItem* Targetitem, std::list<gateway_t>* route) {
	int hopps = 0;
	RoutingTreeItem *parentItem = Targetitem;
	while (parentItem != &m_rootItem) {
		route->push_front(parentItem->returnGatewayID());
		hopps++;
		parentItem = parentItem->return_Parent();
	}
	return hopps;
}

int RoutingTree::returnRootDomainID() {
	return m_rootItem.returnDomainID();
}

RoutingTreeItem* RoutingTree::returnRootItem() {
	return &m_rootItem;
}


void Bushandler::load_Bus_plugins() {
	std::list<std::string> sharedLibraryNameList;

	for (uint dirIndex = 0; dirIndex < routingPluginDirectoriesCount; ++dirIndex) {
        const char* directoryName = routingPluginDirectories[dirIndex];
        DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Searching for Routing in"),DLT_STRING(directoryName));
        std::list<std::string> newList=m_core->getSharedLibrariesFromDirectory(directoryName);
        sharedLibraryNameList.insert(sharedLibraryNameList.end(),newList.begin(),newList.end());
    }


    // iterate all communicator plugins and start them
    std::list<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::list<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Loading Routing plugin"),DLT_STRING(iter->c_str()));

    	RoutingSendInterface* (*createFunc)();
        createFunc = getCreateFunction<RoutingSendInterface*()>(*iter);

        if (!createFunc) {
            DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
            continue;
        }

        RoutingSendInterface* RoutingPlugin = createFunc();


        if (!RoutingPlugin) {
        	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("RoutingPlugin initialization failed. Entry Function not callable"));
            continue;
        }

        Bus newBus;
    	char BusName[40];
        RoutingPlugin->return_BusName(BusName);
        newBus.Name=std::string(BusName);
        newBus.sendInterface=RoutingPlugin;
        RoutingPlugin->startup_interface(m_receiver,m_core->returnDbusHandler());
		DLT_LOG( AudioManager, DLT_LOG_INFO, DLT_STRING("Registered Routing Plugin:"), DLT_STRING(BusName));
        Busses.push_back(newBus);
    }
}

void Bushandler::StartupInterfaces() {
	std::list<Bus>::iterator busIter;
	std::list<Bus>::iterator busStart=Busses.begin();
	std::list<Bus>::iterator busEnd=Busses.end();

	for (busIter=busStart;busIter!=busEnd;busIter++) {
		busIter->sendInterface->system_ready();
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Bushandler:Started Interface"), DLT_STRING(busIter->Name.c_str()));
	}
}

void Bushandler::registerReceiver(RoutingReceiver * receiver) {
	m_receiver = receiver;
}

void Bushandler::registerCore (AudioManagerCore* core) {
	m_core=core;
}

RoutingSendInterface* Bushandler::getInterfaceforBus(std::string bus) {
	/**
	 * \todo this could be done more nicer and faster with a hash lookup or so.. worht it??
	 */
	std::list<Bus>::iterator busIter;
	std::list<Bus>::iterator busStart=Busses.begin();
	std::list<Bus>::iterator busEnd=Busses.end();

	for (busIter=busStart;busIter!=busEnd;busIter++) {
		{
			if (busIter->Name.compare(bus) == 0) {
				return busIter->sendInterface;
			}
		}
	}
	return NULL;
}

