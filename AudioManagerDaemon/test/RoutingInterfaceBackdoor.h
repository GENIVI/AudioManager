/*
 * RoutingInterfaceBackdoor.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef ROUTINGINTERFACEBACKDOOR_H_
#define ROUTINGINTERFACEBACKDOOR_H_

#include "RoutingSender.h"

using namespace am;

class RoutingSender;

class RoutingInterfaceBackdoor {
public:
	RoutingInterfaceBackdoor();
	virtual ~RoutingInterfaceBackdoor();
	bool unloadPlugins(RoutingSender *RoutingSender);
	bool injectInterface(RoutingSender *RoutingSender, RoutingSendInterface *newInterface, const std::string& busname);
};

//definitions are in CommonFunctions.cpp!

#endif /* ROUTINGINTERFACEBACKDOOR_H_ */
