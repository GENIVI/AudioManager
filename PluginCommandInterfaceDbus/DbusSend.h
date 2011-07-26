/*
 * DbusSend.h
 *
 *  Created on: Jul 20, 2011
 *      Author: christian
 */

#ifndef DBUSSEND_H_
#define DBUSSEND_H_

#include "headers.h"

class DbusSend {
public:
	DbusSend(DBusConnection* conn, const char* bus_name,const char* path, const char* interface, const char* method);
	virtual ~DbusSend();
	void appendString(char* string);
	void appendInteger(int integer);
	void sendReply(bool* reply);
	void sendReply(int* reply);

private:
	DBusMessage* m_msg;
	DBusMessageIter m_args;
	DBusConnection* m_conn;
};

#endif /* DBUSSEND_H_ */
