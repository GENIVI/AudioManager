/*
 * udpClient.h
 *
 *  Created on: Apr 6, 2011
 *      Author: blacky
 */
#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include <QtNetwork>
#include "tcpMessages.h"

class tcpClient : public QTcpSocket
{
Q_OBJECT
public:
	tcpClient(QString serverIP, int serverPort);
	virtual ~tcpClient();

	int connect(int source, int sink);
	void system_ready();

	bool send();

public slots:
	void sendOut();


private:
	QString serverIP;
	int serverPort;
	QByteArray sendBuffer;

};

#endif /* UDPCLIENT_H_ */
