/*
 * TcpServer.h
 *
 *  Created on: Apr 6, 2011
 *      Author: blacky
 */

#ifndef I2CSERVER_H_
#define I2CSERVER_H_

#include <QtNetwork>
#include <QObject>

#include "routinginterface.h"
#include "tcpMessages.h"

class TcpServer : public QTcpServer{
Q_OBJECT
public:
	TcpServer(int ListenPort);
	virtual ~TcpServer();

	void registerAudioManager(RoutingReceiveInterface *audiomanager_);

public slots:
	void receivedCommand();
	void startRead();

private:
	QTcpSocket* socket;
	RoutingReceiveInterface *audiomanager;
};

#endif /* I2CSERVER_H_ */
