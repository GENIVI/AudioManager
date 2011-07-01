/*
 * TcpServer.cpp
 *
 *  Created on: Apr 6, 2011
 *      Author: blacky
 */

#include "TcpServer.h"

#include <iostream>
#include <QList>
#include <QByteArray>

using namespace std;

TcpServer::TcpServer(int ListenPort) {
	 connect(this, SIGNAL(newConnection()), this, SLOT(receivedCommand()));
	 if (! this->listen(QHostAddress::Any,ListenPort)) {
		 cout<<" Problem: "<<this->errorString().toStdString()<<endl;
	 }
}

TcpServer::~TcpServer() {
	// TODO Auto-generated destructor stub
}

void TcpServer::registerAudioManager(RoutingReceiveInterface *audiomanager_) {
	audiomanager=audiomanager_;
}

void TcpServer::receivedCommand() {
	socket=this->nextPendingConnection ();
	connect(socket, SIGNAL(readyRead()), this, SLOT(startRead()));
	cout<<"got new command"<<endl;
}

void TcpServer::startRead() {
	QByteArray buffer;
	int answer;
	bool earlyMode=false;
	buffer=socket->readAll();
	QList<QByteArray> bList=buffer.split('#');
	switch (bList.at(0).toInt()) {
		case MSG_registerDomain:
			if (QString(bList.at(4))=="true") {
				earlyMode=true;
			}
			answer=audiomanager->registerDomain((char*)bList.at(1).data(),(char*)bList.at(2).data(),(char*)bList.at(3).data(),earlyMode);
			socket->write(QByteArray::number(answer));
			break;
		case MSG_registerGateway:
			answer=audiomanager->registerGateway((char*)bList.at(1).data(),(char*)bList.at(2).data(),(char*)bList.at(3).data(),(char*)bList.at(4).data(),(char*)bList.at(5).data(),(char*)bList.at(6).data());
			socket->write(QByteArray::number(answer));
			break;
		case MSG_registerSink:
			answer=audiomanager->registerSink((char*)bList.at(1).data(),(char*)bList.at(2).data(),(char*)bList.at(3).data());
			socket->write(QByteArray::number(answer));
			break;
		case MSG_registerSource:
			answer=audiomanager->registerSource((char*)bList.at(1).data(),(char*)bList.at(2).data(),(char*)bList.at(3).data());
			socket->write(QByteArray::number(answer));
			break;
		case MSG_peekDomain:
			answer=audiomanager->peekDomain((char*)bList.at(1).data());
			socket->write(QByteArray::number(answer));
			break;
	}
}
