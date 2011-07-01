/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DataBaseHandler.cpp
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

#include "DataBaseHandler.h"

DataBaseHandler::DataBaseHandler() {

	//knock down database
	QString path = (QDir::home().path());

	path.append(QDir::separator()).append(AUDIO_DATABASE);
	path = QDir::toNativeSeparators(path);

	QFile db_file(path);
	if (db_file.exists()) {
		db_file.remove();
	}
	if (!this->open_database()) {
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Problems with opening the database"));
	}
}

DataBaseHandler::~DataBaseHandler() {
	this->close_database();
}

bool DataBaseHandler::open_database() {
	m_database = QSqlDatabase::addDatabase("QSQLITE");
	QString path = (QDir::home().path());

	path.append(QDir::separator()).append(AUDIO_DATABASE);
	path = QDir::toNativeSeparators(path);
	m_database.setDatabaseName(path);
	return m_database.open();
}

void DataBaseHandler::close_database() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Closed Database"));
	m_database.close();
}

bool DataBaseHandler::delete_data(QString table) {
	QSqlQuery query;
	QString comand = "DELETE FROM " + table;
	return query.exec(comand);
}

bool DataBaseHandler::create_tables() {

	QSqlQuery query;
	QString
			command =
					"CREATE TABLE " + QString(DOMAIN_TABLE)
							+ " (ID INTEGER NOT NULL, DomainName VARCHAR(50), BusName VARCHAR(50), NodeName VARCHAR(50), EarlyMode BOOL, PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(DOMAIN_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(SOURCE_CLASS_TABLE)
					+ " (ID INTEGER NOT NULL, ClassName VARCHAR(50), VolumeOffset INTEGER, IsInterrupt BOOL, IsMixed BOOL, PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(SOURCE_CLASS_TABLE));
		return false;
	}

	command = "CREATE TABLE " + QString(SINK_CLASS_TABLE)
			+ " (ID INTEGER NOT NULL, ClassName VARCHAR(50), PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(SINK_CLASS_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(SOURCE_TABLE)
					+ " (ID INTEGER NOT NULL, Name VARCHAR(50), Class_ID INTEGER, Domain_ID INTEGER, IsGateway BOOL, PRIMARY KEY(ID), FOREIGN KEY (Domain_ID) REFERENCES "
					+ DOMAIN_TABLE + "(ID), FOREIGN KEY (Class_ID) REFERENCES "
					+ SOURCE_CLASS_TABLE + "(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(SOURCE_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(SINK_TABLE)
					+ " (ID INTEGER NOT NULL, Name VARCHAR(50), Class_ID INTEGER, Domain_ID INTEGER, IsGateway BOOL, PRIMARY KEY(ID), FOREIGN KEY (DOMAIN_ID) REFERENCES "
					+ DOMAIN_TABLE + "(ID), FOREIGN KEY (Class_ID) REFERENCES "
					+ SOURCE_CLASS_TABLE + "(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(SOURCE_CLASS_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(GATEWAY_TABLE)
					+ " (ID INTEGER NOT NULL, Name VARCHAR(50), Sink_ID INTEGER, Source_ID INTEGER, DomainSource_ID INTEGER, DomainSink_ID INTEGER, ControlDomain_ID Integer, IsBlocked BOOL, PRIMARY KEY(ID), FOREIGN KEY (Sink_ID) REFERENCES "
					+ SINK_TABLE + "(ID), FOREIGN KEY (Source_ID) REFERENCES "
					+ SOURCE_TABLE
					+ "(ID),FOREIGN KEY (DomainSource_ID) REFERENCES "
					+ DOMAIN_TABLE
					+ "(ID),FOREIGN KEY (DomainSink_ID) REFERENCES "
					+ DOMAIN_TABLE + "(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(GATEWAY_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(CONNECTION_TABLE)
					+ " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(CONNECTION_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(INTERRUPT_TABLE)
					+ " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, Connection_ID INTEGER, mixed BOOL, listInterrruptedSources INTEGER, PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(INTERRUPT_TABLE));
		return false;
	}

	command
			= "CREATE TABLE " + QString(MAIN_TABLE)
					+ " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, route INTEGER, PRIMARY KEY(ID));";
	if (query.exec(command) != true) {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Databasehandler: Could not create table"), DLT_STRING(MAIN_TABLE));
		return false;
	}

	return true;

}

domain_t DataBaseHandler::insert_into_Domains_table(QString DomainName,
		QString BusName, QString NodeName, bool EarlyMode) {
	QSqlQuery query;
	QString _EarlyMode = "false";
	if (EarlyMode) {
		_EarlyMode = "true";
	}

	QString command = "SELECT BusName,ID FROM " + QString(DOMAIN_TABLE)
			+ " WHERE DomainName=\"" + DomainName + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			if (!query.value(0).toString().isEmpty()) {
				return query.value(1).toInt();
			} else {
				command = "UPDATE " + QString(DOMAIN_TABLE) + "SET Busname="
						+ BusName + " Nodename=" + NodeName + " EarlyMode="
						+ _EarlyMode + " WHERE DomainName=" + DomainName;
			}
		} else {
			command = "INSERT INTO " + QString(DOMAIN_TABLE)
					+ " (DomainName, BusName, NodeName, EarlyMode) VALUES (\""
					+ DomainName + "\",\"" + BusName + "\",\"" + NodeName
					+ "\",\"" + _EarlyMode + "\")";
		}
	}

	if (query.exec(command) != true) {
		return -1;
	} else {
		return get_Domain_ID_from_Name(DomainName);
	}
}
sourceClass_t DataBaseHandler::insert_into_Source_Class_table(
		QString ClassName, volume_t VolumeOffset, bool IsInterrupt,
		bool IsMixed) {
	QSqlQuery query;
	QString _IsInterrupt = "false";
	QString _IsMixed = "false";

	if (IsInterrupt) {
		_IsInterrupt = "true";
	}
	if (IsMixed) {
		_IsMixed = "true";
	}

	QString command = "SELECT ID FROM " + QString(SOURCE_CLASS_TABLE)
			+ " WHERE ClassName=\"" + ClassName + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}

	command = "INSERT INTO " + QString(SOURCE_CLASS_TABLE)
			+ " (ClassName, VolumeOffset, IsInterrupt, IsMixed) VALUES (\""
			+ ClassName + "\"," + QString::number(VolumeOffset) + ",\""
			+ _IsInterrupt + "\",\"" + _IsMixed + "\")";

	if (query.exec(command) != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

sink_t DataBaseHandler::insert_into_Sink_Class_table(QString ClassName) {
	QSqlQuery query;

	QString command = "SELECT ID FROM " + QString(SINK_CLASS_TABLE)
			+ " WHERE ClassName=\"" + ClassName + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}

	command = "INSERT INTO " + QString(SINK_CLASS_TABLE)
			+ " (ClassName) VALUES (\"" + ClassName + "\")";

	if (query.exec(command) != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

source_t DataBaseHandler::insert_into_Source_table(QString Name,
		sourceClass_t Class_ID, domain_t Domain_ID, bool IsGateway) {
	QSqlQuery query;
	QString _IsGateway = "false";

	if (IsGateway) {
		_IsGateway = "true";
	}

	QString command = "SELECT ID FROM " + QString(SOURCE_TABLE)
			+ " WHERE Name=\"" + Name + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}

	command = "INSERT INTO " + QString(SOURCE_TABLE)
			+ " (Name, Class_ID, Domain_ID, IsGateway) VALUES (\"" + Name
			+ "\"," + QString::number(Class_ID) + ",\"" + QString::number(
			Domain_ID) + "\",\"" + _IsGateway + "\")";

	if (query.exec(command) != true) {
		return -1;
	} else {
		emit signal_numberOfSourcesChanged();
		return query.lastInsertId().toInt();
	}
}

sink_t DataBaseHandler::insert_into_Sink_table(QString Name,
		sinkClass_t Class_ID, domain_t Domain_ID, bool IsGateway) {
	QSqlQuery query;
	QString _IsGateway = "false";

	if (IsGateway) {
		_IsGateway = "true";
	}

	QString command = "SELECT ID FROM " + QString(SINK_TABLE)
			+ " WHERE Name=\"" + Name + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}

	command = "INSERT INTO " + QString(SINK_TABLE)
			+ " (Name, Class_ID, Domain_ID, IsGateway) VALUES (\"" + Name
			+ "\"," + QString::number(Class_ID) + ",\"" + QString::number(
			Domain_ID) + "\",\"" + _IsGateway + "\")";

	if (query.exec(command) != true) {
		return -1;
	} else {
		emit signal_numberOfSinksChanged();
		return query.lastInsertId().toInt();
	}
}

gateway_t DataBaseHandler::insert_into_Gatway_table(QString Name,
		sink_t Sink_ID, source_t Source_ID, domain_t DomainSource_ID,
		domain_t DomainSink_ID, domain_t ControlDomain_ID) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(GATEWAY_TABLE)
			+ " WHERE Name=\"" + Name + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}

	command
			= "INSERT INTO " + QString(GATEWAY_TABLE)
					+ " (Name, Sink_ID, Source_ID, DomainSource_ID, DomainSink_ID, ControlDomain_ID, IsBlocked) VALUES (\""
					+ Name + "\"," + QString::number(Sink_ID) + ","
					+ QString::number(Source_ID) + "," + QString::number(
					DomainSource_ID) + "," + QString::number(DomainSink_ID)
					+ "," + QString::number(ControlDomain_ID) + ",\"false\")";
	if (query.exec(command) != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

genInt_t DataBaseHandler::reserveInterrupt(sink_t Sink_ID, source_t Source_ID) {
	QSqlQuery query;
	query.prepare(
			"INSERT INTO " + QString(INTERRUPT_TABLE) + "(Source_ID, Sink_ID)"
				" VALUES(:Source_ID, :Sink_ID)");
	query.bindValue(":Source_ID", Source_ID);
	query.bindValue(":Sink_ID", Sink_ID);
	if (query.exec() != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

genError_t DataBaseHandler::updateInterrupt(const genInt_t intID,
		connection_t connID, bool mixed,
		QList<source_t> listInterrruptedSources) {
	QSqlQuery query;
	QString _mixed = "false";

	if (mixed) {
		_mixed = "true";
	}

	//This information is not handy to be stored directly in the database. So we put it on the heap and store the pointer to it.
	QList<source_t>* pointer = new QList<source_t> (listInterrruptedSources);

	query.prepare(
			"UPDATE " + QString(INTERRUPT_TABLE)
					+ " SET Connection_ID=:Connection_ID, mixed=:mixed ,listInterrruptedSources=:listInterrruptedSources WHERE ID=:id");
	query.bindValue(":Connection_ID", connID);
	query.bindValue(":mixed", _mixed);
	query.bindValue(":listInterrruptedSources", int(pointer));
	query.bindValue(":id", intID);
	if (query.exec() != true) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

genError_t DataBaseHandler::getInterruptDatafromID(const genInt_t intID,
		connection_t* return_connID, sink_t* return_Sink_ID,
		source_t* return_Source_ID, bool* return_mixed,
		QList<source_t>** return_listInterrruptedSources) {
	QSqlQuery query;
	QString
			command =
					"SELECT Connection_ID, Sink_ID, Source_ID, mixed, listInterrruptedSources FROM "
							+ QString(INTERRUPT_TABLE) + " WHERE ID="
							+ QString::number(intID) + ";";

	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (query.next()) {
			*return_connID = query.value(0).toInt();
			*return_Sink_ID = query.value(1).toInt();
			*return_Source_ID = query.value(2).toInt();
			*return_mixed = query.value(3).toBool();
			*return_listInterrruptedSources
					= reinterpret_cast<QList<source_t>*> (query.value(4).toInt());
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

genError_t DataBaseHandler::removeInterrupt(const genInt_t intID) {
	QSqlQuery query;
	QString command = "SELECT listInterrruptedSources FROM " + QString(
			INTERRUPT_TABLE) + " WHERE ID=" + QString::number(intID) + ";";
	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (query.next()) {
			delete reinterpret_cast<QList<source_t>*> (query.value(0).toInt());
			command = "DELETE FROM " + QString(INTERRUPT_TABLE)
					+ " WHERE ID=\"" + QString::number(intID) + "\";";
			if (query.exec(command) != true) {
				return GEN_DATABASE_ERROR;
			} else {
				return GEN_OK;
			}
		}
	}
	return GEN_UNKNOWN;
}

domain_t DataBaseHandler::peek_Domain_ID(QString DomainName) {
	QSqlQuery query;

	QString command = "SELECT ID FROM " + QString(DOMAIN_TABLE)
			+ " WHERE DomainName=\"" + DomainName + "\";";

	if (query.next()) {
		return query.value(0).toInt();
	} else {
		command = "INSERT INTO " + QString(DOMAIN_TABLE)
				+ " (DomainName) VALUES (\"" + DomainName + "\")";
	}

	if (query.exec(command) != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Source_ID(source_t Source_ID) {
	QSqlQuery query;
	QString command = "SELECT Domain_ID FROM " + QString(SOURCE_TABLE)
			+ " WHERE ID=" + QString::number(Source_ID) + ";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		query.next();
		return query.value(0).toInt();
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Sink_ID(sink_t Sink_ID) {
	QSqlQuery query;
	QString command = "SELECT Domain_ID FROM " + QString(SINK_TABLE)
			+ " WHERE ID=" + QString::number(Sink_ID) + ";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		query.next();
		return query.value(0).toInt();
	}
}

source_t DataBaseHandler::get_Source_ID_from_Name(QString name) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(SOURCE_TABLE)
			+ " WHERE Name=\"" + name + "\";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return 0;
		}
	}
}

sourceClass_t DataBaseHandler::get_Source_Class_ID_from_Name(QString name) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(SOURCE_CLASS_TABLE)
			+ " WHERE ClassName=\"" + name + "\";";
	if (query.exec(command) != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return 0;
		}
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Name(QString name) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(DOMAIN_TABLE)
			+ " WHERE DomainName=\"" + name + "\";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return 0;
		}
	}
}

gateway_t DataBaseHandler::get_Gateway_ID_with_Domain_ID(
		domain_t startDomain_ID, domain_t targetDomain_ID) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(GATEWAY_TABLE)
			+ " WHERE DomainSource_ID=" + QString::number(startDomain_ID)
			+ " AND DomainSink_ID=" + QString::number(targetDomain_ID) + ";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return 0;
		}
	}
}

genError_t DataBaseHandler::get_Gateway_Source_Sink_Domain_ID_from_ID(
		gateway_t Gateway_ID, source_t* return_Source_ID,
		sink_t* return_Sink_ID, domain_t* return_ControlDomain_ID) {
	QSqlQuery query;
	QString command = "SELECT Source_ID, Sink_ID, ControlDomain_ID FROM "
			+ QString(GATEWAY_TABLE) + " WHERE ID=" + QString::number(
			Gateway_ID) + ";";

	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (query.next()) {
			*return_Source_ID = query.value(0).toInt();
			*return_Sink_ID = query.value(1).toInt();
			*return_ControlDomain_ID = query.value(2).toInt();
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

void DataBaseHandler::get_Domain_ID_Tree(bool onlyfree, RoutingTree* Tree,
		QList<RoutingTreeItem*>* allItems) {
	QSqlQuery query;
	int RootID = Tree->returnRootDomainID();
	RoutingTreeItem *parent = Tree->returnRootItem();
	QString _onlyfree = "false";
	int i = 0;

	if (onlyfree) {
		_onlyfree = "true";
	}

	query.prepare(
			"SELECT ID,DomainSource_ID FROM " + QString(GATEWAY_TABLE)
					+ " WHERE DomainSink_ID=:id AND IsBlocked=:flag;");

	do {
		query.bindValue(":id", RootID);
		query.bindValue(":flag", _onlyfree);
		query.exec();
		while (query.next()) {
			allItems->append(
					Tree->insertItem(query.value(1).toInt(),
							query.value(0).toInt(), parent));
		}
		parent = allItems->value(i);
		RootID = parent->returnDomainID();
		i++;
	} while (allItems->length() > i);
}

QString DataBaseHandler::get_Bus_from_Domain_ID(domain_t Domain_ID) {
	QSqlQuery query;
	QString command = "SELECT BusName FROM " + QString(DOMAIN_TABLE)
			+ " WHERE ID=" + QString::number(Domain_ID) + ";";

	if (query.exec(command) != true) {
		return NULL;
	} else {
		query.next();
		return query.value(0).toString();
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Connection_ID(connection_t ID) {
	QSqlQuery query;
	QString command = "SELECT Source_ID FROM " + QString(CONNECTION_TABLE)
			+ " WHERE ID=" + QString::number(ID) + ";";

	if (query.exec(command) != true) {
		return -1;
	}
	query.next();
	int SourceID = query.value(0).toInt();
	command = "SELECT Domain_ID FROM " + QString(SOURCE_TABLE) + " WHERE ID="
			+ QString::number(SourceID) + ";";
	if (query.exec(command) != true) {
		return -1;
	} else {
		query.next();
		return query.value(0).toInt();
	}
}

void DataBaseHandler::getListofSources(QList<SourceType>* SourceList) {
	QSqlQuery query;
	SourceType sType;
	QString command = "SELECT ID,NAME FROM " + QString(SOURCE_TABLE)
			+ " WHERE isGateway=\"false\";";
	if (query.exec(command) != true) {

	} else {
		while (query.next()) {
			sType.ID = query.value(0).toInt();
			sType.name = query.value(1).toString();
			SourceList->append(sType);
		}
	}
}

void DataBaseHandler::getListofSinks(QList<SinkType>* SinkList) {
	QSqlQuery query;
	SinkType sType;
	QString command = "SELECT ID,NAME FROM " + QString(SINK_TABLE) + ";";
	if (query.exec(command) != true) {

	} else {
		while (query.next()) {
			sType.ID = query.value(0).toInt();
			sType.name = query.value(1).toString();
			SinkList->append(sType);
		}
	}
}

void DataBaseHandler::getListofConnections(
		QList<ConnectionType>* ConnectionList) {
	QSqlQuery query;
	ConnectionType sType;
	QString command = "SELECT Source_ID,Sink_ID FROM " + QString(
			CONNECTION_TABLE) + ";";
	if (query.exec(command) != true) {

	} else {
		while (query.next()) {
			sType.Source_ID = query.value(0).toInt();
			sType.Sink_ID = query.value(1).toInt();
			ConnectionList->append(sType);
		}
	}
}

bool DataBaseHandler::is_source_Mixed(source_t source) {
	QSqlQuery query;
	int classID = 0;

	QString command = "SELECT Class_ID FROM " + QString(SOURCE_TABLE)
			+ " WHERE ID=\"" + QString::number(source) + "\";";
	if (query.exec(command) == true) {
		if (query.next()) {
			classID = query.value(0).toInt();
		}
	}
	command = "SELECT isMixed FROM " + QString(SOURCE_CLASS_TABLE)
			+ " WHERE ID=\"" + QString::number(classID) + "\";";

	if (query.exec(command) == true) {
		if (query.next()) {
			if (query.value(0).toString().compare("true") == 0) {
				return true;
			}
		}
	}
	return false;
}

sink_t DataBaseHandler::get_Sink_ID_from_Name(QString name) {
	QSqlQuery query;
	QString command = "SELECT ID FROM " + QString(SINK_TABLE)
			+ " WHERE Name=\"" + name + "\";";

	if (query.exec(command) != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return 0;
		}
	}
}

connection_t DataBaseHandler::getConnectionID(source_t SourceID, sink_t SinkID) {
	QSqlQuery query;
	query.prepare(
			"SELECT ID FROM " + QString(MAIN_TABLE)
					+ " WHERE Source_ID=:sourceID AND Sink_ID=:sinkID");
	query.bindValue(":sourceID", SourceID);
	query.bindValue(":sinkID", SinkID);
	if (query.exec() != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		} else {
			return -1;
		}
	}
}

connection_t DataBaseHandler::insertConnection(source_t SourceID, sink_t SinkID) {
	QSqlQuery query;
	QString command = "INSERT INTO " + QString(CONNECTION_TABLE)
			+ " (Source_ID, Sink_ID) VALUES (" + QString::number(SourceID)
			+ "," + QString::number(SinkID) + ");";
	if (query.exec(command) != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

genError_t DataBaseHandler::removeConnection(connection_t ConnectionID) {
	QSqlQuery query;
	QString command = "DELETE FROM " + QString(CONNECTION_TABLE)
			+ " WHERE ID=\"" + QString::number(ConnectionID) + "\";";
	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

connection_t DataBaseHandler::reserveMainConnection(source_t source,
		sink_t sink) {
	QSqlQuery query;
	query.prepare("INSERT INTO " + QString(MAIN_TABLE) + "(Source_ID, Sink_ID)"
		" VALUES(:Source_ID, :Sink_ID)");
	query.bindValue(":Source_ID", source);
	query.bindValue(":Sink_ID", sink);
	if (query.exec() != true) {
		return -1;
	} else {
		return query.lastInsertId().toInt();
	}
}

genError_t DataBaseHandler::updateMainConnection(connection_t connID,
		genRoute_t route) {
	QSqlQuery query;

	//This information is not handy to be stored directly in the database. So we put it on the heap and store the pointer to it.
	genRoute_t* routeheap = new genRoute_t(route);

	query.prepare(
			"UPDATE " + QString(MAIN_TABLE)
					+ " SET route=:route WHERE ID=:connID");
	query.bindValue(":connID", connID);
	query.bindValue(":route", int(routeheap));
	if (query.exec() != true) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

genError_t DataBaseHandler::getMainConnectionDatafromID(
		const connection_t connID, sink_t* return_sinkID,
		source_t* return_sourceID, genRoute_t** return_route) {
	QSqlQuery query;
	QString command = "SELECT Sink_ID, Source_ID, route FROM " + QString(
			MAIN_TABLE) + " WHERE ID=" + QString::number(connID) + ";";

	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (query.next()) {
			*return_sinkID = query.value(0).toInt();
			*return_sourceID = query.value(1).toInt();
			*return_route
					= reinterpret_cast<genRoute_t*> (query.value(2).toInt());
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

connection_t DataBaseHandler::returnMainconnectionIDforSinkSourceID(
		sink_t sink, source_t source) {
	QSqlQuery query;
	query.prepare(
			"SELECT ID FROM " + QString(MAIN_TABLE)
					+ " WHERE Sink_ID=:sinkID AND Source_ID=:SourceID");
	query.bindValue(":SinkID", sink);
	query.bindValue(":SourceID", source);

	if (query.exec() != true) {
		return -1;
	} else {
		if (query.next()) {
			return query.value(0).toInt();
		}
	}
	return -1;
}

QList<source_t> DataBaseHandler::getSourceIDsForSinkID(sink_t sink) {
	QList<source_t> list;
	QSqlQuery query;
	query.prepare(
			"SELECT Source_ID FROM " + QString(MAIN_TABLE)
					+ " WHERE Sink_ID=:sinkID");
	query.bindValue(":sinkID", sink);
	if (query.exec() == true) {
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("query good"));
		while (query.next()) {
			int p = query.value(0).toInt();
			DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("SourceID"), DLT_INT(p));
			list.append(query.value(0).toInt());
		}
	}
	return list;
}

QList<ConnectionType> DataBaseHandler::getListAllMainConnections() {
	QList<ConnectionType> connectionList;
	QSqlQuery query;
	QString command = "SELECT Sink_ID, Source_ID, route FROM " + QString(
			MAIN_TABLE) + ";";

	if (query.exec(command) != true) {

	} else {
		if (query.next()) {
			ConnectionType temp;
			temp.Sink_ID = query.value(0).toInt();
			temp.Source_ID = query.value(1).toInt();
			connectionList.append(temp);
		}
	}
	return connectionList;
}

genError_t DataBaseHandler::removeMainConnection(connection_t connID) {
	QSqlQuery query;
	QString command = "SELECT route FROM " + QString(MAIN_TABLE) + " WHERE ID="
			+ QString::number(connID) + ";";
	if (query.exec(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (query.next()) {
			delete reinterpret_cast<genRoute_t*> (query.value(0).toInt());
			command = "DELETE FROM " + QString(MAIN_TABLE) + " WHERE ID=\""
					+ QString::number(connID) + "\";";
			if (query.exec(command) != true) {
				return GEN_DATABASE_ERROR;
			} else {
				return GEN_OK;
			}
		}
	}
	return GEN_UNKNOWN;
}
