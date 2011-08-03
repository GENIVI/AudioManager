/*
 * dbusRoothandler.h
 *
 *  Created on: Aug 1, 2011
 *      Author: christian
 */

#ifndef DBUSROOTHANDLER_H_
#define DBUSROOTHANDLER_H_

#include "audioManagerIncludes.h"
#ifdef WITH_DBUS
#include <dbus/dbus.h>
#include <string.h>
#include <list>
#include <sstream>

const char DBUS_SERVICE_SERVICE[]="org.genivi.audiomanager\0";
const char DBUS_SERVICE_ROOT[]="/org/genivi/audiomanager\0";

class dbusRoothandler {
public:
	dbusRoothandler();
	virtual ~dbusRoothandler();
	DBusConnection* returnConnection();
	static DBusHandlerResult cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference);
	void registerNode(std::string node);
private:
	static dbusRoothandler* m_reference;
    DBusConnection* m_pConnection;
    DBusError m_err;
    std::list<std::string> m_nodesList;
};

#endif
#endif /* DBUSROOTHANDLER_H_ */
