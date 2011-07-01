/*
 * Bus_Interface.cpp
 *
 *  Created on: Jan 19, 2011
 *      Author: demo
 */
#include <iostream>


#include "RoutingSend.h"
#include "routinginterface.h"
#include "tcpMessages.h"


using namespace std;


void RoutingSendTcp::startup_interface(RoutingReceiveInterface* audioman){
	audiomanager=audioman;
	m_server=new TcpServer(AUDIOMAN_PORT);
	m_client=new tcpClient(BEAGLE_IP,BEAGLE_PORT);
	m_server->registerAudioManager(audiomanager);
	cout<<"server started"<<endl;
}

void RoutingSendTcp::return_BusName(char* BusName) {
	strcpy(BusName,BUS_NAME);
}

int RoutingSendTcp::Connect(int source, int sink){
	return m_client->connect(source,sink);
}

void RoutingSendTcp::system_ready() {

	m_client->system_ready();
	cout<<"Tcp ready"<<endl;

}


RoutingSendInterface* SampleRoutingInterfaceTcpFactory::returnInstance(){
	return new RoutingSendTcp();
}

Q_EXPORT_PLUGIN2(RoutingTcpPlugin, SampleRoutingInterfaceTcpFactory);
