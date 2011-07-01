/*
 * udpClient.cpp
 *
 *  Created on: Apr 6, 2011
 *      Author: blacky
 */

#include "tcpClient.h"
#include <iostream>

using namespace std;

tcpClient::tcpClient(QString serverIP_, int serverPort_) {
	serverIP=serverIP_;
	serverPort=serverPort_;
	QObject::connect(this, SIGNAL(connected()), this, SLOT(sendOut()));
}

tcpClient::~tcpClient() {
	// TODO Auto-generated destructor stub
}

int tcpClient::connect(int source, int sink) {

	sendBuffer.clear();
	sendBuffer.append(QByteArray::number(MSG_CONNECT));
	sendBuffer.append('#');
	sendBuffer.append(QByteArray::number(source));
	sendBuffer.append('#');
	sendBuffer.append(QByteArray::number(sink));
	if (send()) {
		this->waitForReadyRead(CONNECT_TIMEOUT);
		return this->readAll().toInt();
	} else {
		return -1;
	}

}

void tcpClient::system_ready() {

	sendBuffer.clear();
	sendBuffer.append(QByteArray::number(SIG_system_ready));
	send();
}

bool tcpClient::send() {
	QHostAddress adr;
	adr.setAddress(serverIP);
	this->connectToHost(adr,serverPort);
    if (!this->waitForConnected(CONNECT_TIMEOUT)) {
        cout<<"No connection"<<endl;
        return false;
    } else {
    	cout<<"got connection"<<endl;
    	return true;
    }
}

void tcpClient::sendOut() {
	this->write(sendBuffer);
}
