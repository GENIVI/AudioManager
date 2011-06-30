#include "DBusTypes.h"

class QDBusArgument;

QDBusArgument &operator<<(QDBusArgument &argument, const SourceType &mySource) {
	argument.beginStructure();
  	argument << mySource.name << mySource.ID;
 	argument.endStructure();
  	return argument;
}

const QDBusArgument &operator>>(QDBusArgument const &argument, SourceType &mySource) {
	argument.beginStructure();
    argument >> mySource.name >> mySource.ID;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const SinkType &mySink) {
	argument.beginStructure();
  	argument << mySink.name << mySink.ID;
 	argument.endStructure();
  	return argument;
}

const QDBusArgument &operator>>(QDBusArgument const &argument, SinkType &mySink) {
	argument.beginStructure();
    argument >> mySink.name >> mySink.ID;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConnectionType &myConnection) {
	argument.beginStructure();
  	argument << myConnection.Source_ID << myConnection.Sink_ID;
 	argument.endStructure();
  	return argument;
}

const QDBusArgument &operator>>(QDBusArgument const &argument, ConnectionType &myConnection) {
	argument.beginStructure();
    argument >> myConnection.Source_ID >> myConnection.Sink_ID;
    argument.endStructure();
    return argument;
}
