/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DBusTypes.h
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 *
 */

#ifndef DBUSTYPES_H
#define DBUSTYPES_H

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusAbstractInterface>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QMetaType>

class QDBusArgument;

class SinkType {
public:
	QString name;
	int ID;
};

class SourceType {
public:
	QString name;
	int ID;
};

class ConnectionType {
public:
	int Source_ID;
	int Sink_ID;
};

#ifndef REGISTER_METATYPES
#define REGISTER_METATYPES							\
	qRegisterMetaType<SourceType>();				\
	qRegisterMetaType<QList <SourceType> >();		\
	qDBusRegisterMetaType<SourceType>();			\
	qDBusRegisterMetaType<QList <SourceType> >();	\
	qRegisterMetaType<SinkType>();					\
	qRegisterMetaType<QList <SinkType> >();			\
	qDBusRegisterMetaType<SinkType>();				\
	qDBusRegisterMetaType<QList <SinkType> >();		\
	qRegisterMetaType<ConnectionType>();			\
	qRegisterMetaType<QList <ConnectionType> >();	\
	qDBusRegisterMetaType<ConnectionType>();		\
	qDBusRegisterMetaType<QList <ConnectionType> >();
#endif


Q_DECLARE_METATYPE(SourceType);
Q_DECLARE_METATYPE(QList<SourceType>);
Q_DECLARE_METATYPE(SinkType);
Q_DECLARE_METATYPE(QList<SinkType>);
Q_DECLARE_METATYPE(ConnectionType);
Q_DECLARE_METATYPE(QList<ConnectionType>);


QDBusArgument &operator<<(QDBusArgument &argument, const SourceType &mySource);
const QDBusArgument &operator>>(const QDBusArgument &argument, SourceType &mySource);
QDBusArgument &operator<<(QDBusArgument &argument, const SinkType &mySource);
const QDBusArgument &operator>>(const QDBusArgument &argument, SinkType &mySource);
QDBusArgument &operator<<(QDBusArgument &argument, const ConnectionType &myConnection);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConnectionType &myConnection);


#endif // DBUSTYPES_H
