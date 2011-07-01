/*
 * Bus_Interface.h
 *
 *  Created on: Feb 24, 2011
 *      Author: demo
 */

#ifndef BUS_INTERFACE_H_
#define BUS_INTERFACE_H_

#include <qplugin.h>
#include "routinginterface.h"
#include "TcpServer.h"
#include "tcpClient.h"

#define BUS_NAME "TCP"

class RoutingSendTcp: public RoutingSendInterface
{
Q_OBJECT
public:
	void startup_interface(RoutingReceiveInterface * audioman);
	void return_BusName(char * BusName);
	int Connect(int source, int sink);
public slots:
	void system_ready();

private:
	RoutingReceiveInterface *audiomanager;
	TcpServer* m_server;
	tcpClient* m_client;
};

//That is the actual implementation of the Factory Class returning the real Interface

class SampleRoutingInterfaceTcpFactory: public QObject, public RoutingInterfaceFactory
{
Q_OBJECT
Q_INTERFACES(RoutingInterfaceFactory)
public:
	RoutingSendInterface* returnInstance();
};

#endif /* BUS_INTERFACE_H_ */
