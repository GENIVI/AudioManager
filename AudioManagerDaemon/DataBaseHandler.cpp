/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DataBaseHandler.cpp
 *	if (query.exec(command) != true) {

	}
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
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <sstream>
#include <sqlite3.h>

DataBaseHandler::DataBaseHandler() {

	//knock down database
	m_path = "/home/blacky";

	m_path.append("/");
	m_path.append(AUDIO_DATABASE);

	std::ifstream infile(m_path.c_str());

	if (infile) {
		remove(m_path.c_str());
		DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Knocked down database"));
	}
	if (!this->open_database()) {
		DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Problems with opening the database"));
	}
}

DataBaseHandler::~DataBaseHandler() {
	this->close_database();
}

bool DataBaseHandler::pQuery(std::string command) {
	sqlite3_stmt* query;
	if (sqlite3_exec(m_database,command.c_str(),NULL,&query,NULL)!= SQLITE_OK) {
		DLT_LOG( AudioManager, DLT_LOG_ERROR, DLT_STRING("SQL Query failed:"), DLT_STRING(command.c_str()));
		return false;
	}
	return true;
}

std::string DataBaseHandler::int2string(int i) {
	std::stringstream out;
	out << i;
	return out.str();
}

bool DataBaseHandler::open_database() {
	if (sqlite3_open_v2(m_path.c_str(), &m_database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK) {
		return true;
	} else {
		return false;
	}
}

void DataBaseHandler::close_database() {
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Closed Database"));
	sqlite3_close(m_database);
}

bool DataBaseHandler::delete_data(std::string table) {
	std::string command = "DELETE FROM" + table;
	if (!this->pQuery(command)) return false;
	return true;
}

bool DataBaseHandler::create_tables() {
	std::string command;
	command="CREATE TABLE " + std::string(DOMAIN_TABLE) + " (ID INTEGER NOT NULL, DomainName VARCHAR(50), BusName VARCHAR(50), NodeName VARCHAR(50), EarlyMode BOOL, PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(SOURCE_CLASS_TABLE) + " (ID INTEGER NOT NULL, ClassName VARCHAR(50), VolumeOffset INTEGER, IsInterrupt BOOL, IsMixed BOOL, PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(SINK_CLASS_TABLE) + " (ID INTEGER NOT NULL, ClassName VARCHAR(50), PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(SOURCE_TABLE) + " (ID INTEGER NOT NULL, Name VARCHAR(50), Class_ID INTEGER, Domain_ID INTEGER, IsGateway BOOL, PRIMARY KEY(ID), FOREIGN KEY (Domain_ID) REFERENCES " + DOMAIN_TABLE + "(ID), FOREIGN KEY (Class_ID) REFERENCES " + SOURCE_CLASS_TABLE + "(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(SINK_TABLE) + " (ID INTEGER NOT NULL, Name VARCHAR(50), Class_ID INTEGER, Domain_ID INTEGER, IsGateway BOOL, PRIMARY KEY(ID), FOREIGN KEY (DOMAIN_ID) REFERENCES " + DOMAIN_TABLE + "(ID), FOREIGN KEY (Class_ID) REFERENCES " + SOURCE_CLASS_TABLE + "(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(GATEWAY_TABLE) + " (ID INTEGER NOT NULL, Name VARCHAR(50), Sink_ID INTEGER, Source_ID INTEGER, DomainSource_ID INTEGER, DomainSink_ID INTEGER, ControlDomain_ID Integer, IsBlocked BOOL, PRIMARY KEY(ID), FOREIGN KEY (Sink_ID) REFERENCES " + SINK_TABLE + "(ID), FOREIGN KEY (Source_ID) REFERENCES " + SOURCE_TABLE + "(ID),FOREIGN KEY (DomainSource_ID) REFERENCES " + DOMAIN_TABLE + "(ID),FOREIGN KEY (DomainSink_ID) REFERENCES " + DOMAIN_TABLE + "(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(CONNECTION_TABLE) + " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(INTERRUPT_TABLE) + " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, Connection_ID INTEGER, mixed BOOL, listInterruptedSources INTEGER, PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	command = "CREATE TABLE " + std::string(MAIN_TABLE) + " (ID INTEGER NOT NULL, Source_ID INTEGER, Sink_ID INTEGER, route INTEGER, PRIMARY KEY(ID));";
	if (!this->pQuery(command)) return false;

	return true;
}

domain_t DataBaseHandler::insert_into_Domains_table(std::string DomainName, std::string BusName, std::string NodeName, bool EarlyMode) {
	sqlite3_stmt* query;
	std::string _EarlyMode = "false";
	if (EarlyMode) {
		_EarlyMode = "true";
	}

	std::string command = "SELECT BusName,ID FROM " + std::string(DOMAIN_TABLE) + " WHERE DomainName='" + DomainName + "'";

	if (sqlite3_exec(m_database,command.c_str(),NULL,&query,NULL)!= SQLITE_OK) {
		if (sqlite3_step(query)==SQLITE_ROW) {
			std::string name((const char*) sqlite3_column_text(query,0));
			if (!name.empty()) {
				return sqlite3_column_int(query,1);
			} else {
				command = "UPDATE " + std::string(DOMAIN_TABLE) + "SET Busname=" + BusName + " Nodename=" + NodeName + " EarlyMode=" + _EarlyMode + " WHERE DomainName=" + DomainName;
			}
		} else {
			command = "INSERT INTO " + std::string(DOMAIN_TABLE) + " (DomainName, BusName, NodeName, EarlyMode) VALUES ('" + DomainName + "','" + BusName + "'','" + NodeName + "','" + _EarlyMode + "')";
		}
	}

	sqlite3_finalize(query);

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return get_Domain_ID_from_Name(DomainName);
	}
}

sourceClass_t DataBaseHandler::insert_into_Source_Class_table(std::string ClassName, volume_t VolumeOffset, bool IsInterrupt, bool IsMixed) {
	sqlite3_stmt* query;
	std::string _IsInterrupt = "false";
	std::string _IsMixed = "false";

	if (IsInterrupt) {
		_IsInterrupt = "true";
	}
	if (IsMixed) {
		_IsMixed = "true";
	}

	std::string command = "SELECT ID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE ClassName='" + ClassName + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		}
	}

	command = "INSERT INTO " + std::string(SOURCE_CLASS_TABLE) + " (ClassName, VolumeOffset, IsInterrupt, IsMixed) VALUES ('" + ClassName + "'," + int2string(VolumeOffset) + ",'" + _IsInterrupt + "','" + _IsMixed + "')";

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

sink_t DataBaseHandler::insert_into_Sink_Class_table(std::string ClassName) {
	sqlite3_stmt* query;

	std::string command = "SELECT ID FROM " + std::string(SINK_CLASS_TABLE) + " WHERE ClassName='" + ClassName + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		}
	}

	command = "INSERT INTO " + std::string(SINK_CLASS_TABLE) + " (ClassName) VALUES ('" + ClassName + "')";

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

source_t DataBaseHandler::insert_into_Source_table(std::string Name, sourceClass_t Class_ID, domain_t Domain_ID, bool IsGateway) {
	sqlite3_stmt* query;
	std::string _IsGateway = "false";

	if (IsGateway) {
		_IsGateway = "true";
	}

	std::string command = "SELECT ID FROM " + std::string(SOURCE_TABLE) + " WHERE Name='" + Name + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		}
	}

	command = "INSERT INTO " + std::string(SOURCE_TABLE) + " (Name, Class_ID, Domain_ID, IsGateway) VALUES ('" + Name + "'," + int2string(Class_ID) + ",'" + int2string(Domain_ID) + "','" + _IsGateway + "')";

	if (sqlite3_step(query)) {
		return -1;
	} else {
		//emit signal_numberOfSourcesChanged();
		return sqlite3_last_insert_rowid(m_database);
	}
}

sink_t DataBaseHandler::insert_into_Sink_table(std::string Name, sinkClass_t Class_ID, domain_t Domain_ID, bool IsGateway) {
	sqlite3_stmt* query;
	std::string _IsGateway = "false";

	if (IsGateway) {
		_IsGateway = "true";
	}

	std::string command = "SELECT ID FROM " + std::string(SINK_TABLE) + " WHERE Name='" + Name + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		}
	}

	command = "INSERT INTO " + std::string(SINK_TABLE) + " (Name, Class_ID, Domain_ID, IsGateway) VALUES ('" + Name + "'," + int2string(Class_ID) + ",'" + int2string(Domain_ID) + "','" + _IsGateway + "')";

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

gateway_t DataBaseHandler::insert_into_Gatway_table(std::string Name, sink_t Sink_ID, source_t Source_ID, domain_t DomainSource_ID, domain_t DomainSink_ID, domain_t ControlDomain_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(GATEWAY_TABLE) + " WHERE Name='" + Name + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		}
	}

	command = "INSERT INTO " + std::string(GATEWAY_TABLE) + " (Name, Sink_ID, Source_ID, DomainSource_ID, DomainSink_ID, ControlDomain_ID, IsBlocked) VALUES ('" + Name + "'," + int2string(Sink_ID) + "," + int2string(Source_ID) + "," + int2string(DomainSource_ID) + "," + int2string(DomainSink_ID) + "," + int2string(ControlDomain_ID) + ",'false')";

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

genInt_t DataBaseHandler::reserveInterrupt(sink_t Sink_ID, source_t Source_ID) {
	sqlite3_stmt* query;
	std::string command= "INSERT INTO " + std::string(INTERRUPT_TABLE) + "(Source_ID, Sink_ID) VALUES(:Source_ID, :Sink_ID)";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0, Source_ID);
	sqlite3_bind_int(query,1, Sink_ID);

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

genError_t DataBaseHandler::updateInterrupt(const genInt_t intID, connection_t connID, bool mixed, std::list<source_t> listInterruptedSources) {
	sqlite3_stmt* query;
	std::string _mixed = "false";

	if (mixed) {
		_mixed = "true";
	}

	//This information is not handy to be stored directly in the database. So we put it on the heap and store the pointer to it.
	std::list<source_t>* pointer = new std::list<source_t>;
	pointer=&listInterruptedSources;

	std::string command="UPDATE " + std::string(INTERRUPT_TABLE) + " SET Connection_ID=:Connection_ID, mixed=:mixed ,listInterruptedSources=:listInterruptedSources WHERE ID=:id";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0,connID);
	sqlite3_bind_text(query,1,_mixed.c_str(),_mixed.size(),NULL);
	sqlite3_bind_int(query,2,int(pointer));
	sqlite3_bind_int(query,3,intID);

	if (!this->pQuery(command)) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

genError_t DataBaseHandler::getInterruptDatafromID(const genInt_t intID, connection_t* return_connID, sink_t* return_Sink_ID, source_t* return_Source_ID, bool* return_mixed, std::list<source_t>** return_listInterruptedSources) {
	sqlite3_stmt* query;
	std::string command = "SELECT Connection_ID, Sink_ID, Source_ID, mixed, listInterruptedSources FROM " + std::string(INTERRUPT_TABLE) + " WHERE ID=" + int2string(intID) + ";";

	if (SQexecute(command)!=true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (sqlite3_step(query)) {
			*return_connID = sqlite3_column_int(query,0);
			*return_Sink_ID = sqlite3_column_int(query,1);
			*return_Source_ID = sqlite3_column_int(query,2);
			*return_mixed = sqlite3_column_int(query,3);
			*return_listInterruptedSources = reinterpret_cast<std::list<source_t>*>(sqlite3_column_int(query,4));
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

genError_t DataBaseHandler::removeInterrupt(const genInt_t intID) {
	sqlite3_stmt* query;
	std::string command = "SELECT listInterruptedSources FROM " + std::string(INTERRUPT_TABLE) + " WHERE ID=" + int2string(intID) + ";";
	if (SQexecute(command) != true) {
		return GEN_DATABASE_ERROR;
	} else {
		if (sqlite3_step(query)) {
			delete reinterpret_cast<std::list<source_t>*>(sqlite3_column_int(query,0));
			command = "DELETE FROM " + std::string(INTERRUPT_TABLE) + " WHERE ID='" + int2string(intID) + "';";
			if (!this->pQuery(command)) {
				return GEN_DATABASE_ERROR;
			} else {
				return GEN_OK;
			}
		}
	}
	return GEN_UNKNOWN;
}

domain_t DataBaseHandler::peek_Domain_ID(std::string DomainName) {
	sqlite3_stmt* query;

	std::string command = "SELECT ID FROM " + std::string(DOMAIN_TABLE) + " WHERE DomainName='" + DomainName + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			return sqlite3_column_int(query,0);
		} else {
			command = "INSERT INTO " + std::string(DOMAIN_TABLE) + " (DomainName) VALUES ('" + DomainName + "')";
		}
	}

	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Source_ID(source_t Source_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT Domain_ID FROM " + std::string(SOURCE_TABLE) + " WHERE ID=" + int2string(Source_ID) + ";";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Sink_ID(sink_t Sink_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT Domain_ID FROM " + std::string(SINK_TABLE) + " WHERE ID=" + int2string(Sink_ID) + ";";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

source_t DataBaseHandler::get_Source_ID_from_Name(std::string name) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(SOURCE_TABLE) + " WHERE Name='" + name + "';";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

sourceClass_t DataBaseHandler::get_Source_Class_ID_from_Name(std::string name) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE ClassName='" + name + "';";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Name(std::string name) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(DOMAIN_TABLE) + " WHERE DomainName='" + name + "';";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

gateway_t DataBaseHandler::get_Gateway_ID_with_Domain_ID(domain_t startDomain_ID, domain_t targetDomain_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(GATEWAY_TABLE) + " WHERE DomainSource_ID=" + int2string(startDomain_ID) + " AND DomainSink_ID=" + int2string(targetDomain_ID) + ";";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

genError_t DataBaseHandler::get_Gateway_Source_Sink_Domain_ID_from_ID(gateway_t Gateway_ID, source_t* return_Source_ID, sink_t* return_Sink_ID, domain_t* return_ControlDomain_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT Source_ID, Sink_ID, ControlDomain_ID FROM " + std::string(GATEWAY_TABLE) + " WHERE ID=" + int2string(Gateway_ID) + ";";

	if (SQexecute(command)) {
		return GEN_DATABASE_ERROR;
	} else {
		if (sqlite3_step(query)) {
			*return_Source_ID = sqlite3_column_int(query,0);
			*return_Sink_ID = sqlite3_column_int(query,1);
			*return_ControlDomain_ID = sqlite3_column_int(query,2);
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

void DataBaseHandler::get_Domain_ID_Tree(bool onlyfree, RoutingTree* Tree, std::list<RoutingTreeItem*>* allItems) {
	sqlite3_stmt* query;
	int RootID = Tree->returnRootDomainID();
	RoutingTreeItem *parent = Tree->returnRootItem();
	std::string _onlyfree = "false";
	unsigned int i = 0;

	if (onlyfree) {
		_onlyfree = "true";
	}

	std::string command="SELECT ID,DomainSource_ID FROM " + std::string(GATEWAY_TABLE) + " WHERE DomainSink_ID=:id AND IsBlocked=:flag;";

	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	do {
		sqlite3_bind_int(query,0,RootID);
		sqlite3_bind_text(query,1,_onlyfree.c_str(),_onlyfree.size(),NULL);
		while (sqlite3_step(query)) {
			allItems->push_back(Tree->insertItem(sqlite3_column_int(query,1), sqlite3_column_int(query,0), parent));
		}
		std::list<RoutingTreeItem*>::iterator it=allItems->begin();
		std::advance(it,i);
		RootID = (*it)->returnDomainID();
		i++;
	} while (allItems->size() > i);
}

std::string DataBaseHandler::get_Bus_from_Domain_ID(domain_t Domain_ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT BusName FROM " + std::string(DOMAIN_TABLE) + " WHERE ID=" + int2string(Domain_ID) + ";";

	if (SQexecute(command)) {
		return std::string("");
	} else {
		sqlite3_step(query);
		return std::string((const char*)sqlite3_column_text(query,0));
	}
}

domain_t DataBaseHandler::get_Domain_ID_from_Connection_ID(connection_t ID) {
	sqlite3_stmt* query;
	std::string command = "SELECT Source_ID FROM " + std::string(CONNECTION_TABLE) + " WHERE ID=" + int2string(ID) + ";";

	if (SQexecute(command)) {
		return -1;
	}

	sqlite3_step(query);
	int SourceID = sqlite3_column_int(query,0);
	command = "SELECT Domain_ID FROM " + std::string(SOURCE_TABLE) + " WHERE ID=" + int2string(SourceID) + ";";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

void DataBaseHandler::getListofSources(std::list<SourceType>* SourceList) {
	sqlite3_stmt* query;
	SourceType sType;
	std::string command = "SELECT ID,NAME FROM " + std::string(SOURCE_TABLE) + " WHERE isGateway='false';";
	if (SQexecute(command)) {
		while (sqlite3_step(query)) {
			sType.ID = sqlite3_column_int(query,0);
			sType.name = std::string((const char*)sqlite3_column_text(query,1));
			SourceList->push_back(sType);
		}
	}
}

void DataBaseHandler::getListofSinks(std::list<SinkType>* SinkList) {
	sqlite3_stmt* query;
	SinkType sType;
	std::string command = "SELECT ID,NAME FROM " + std::string(SINK_TABLE) + ";";
	if (SQexecute(command)) {
		while (sqlite3_step(query)) {
			sType.ID = sqlite3_column_int(query,0);
			sType.name =  std::string((const char*)sqlite3_column_text(query,1));
			SinkList->push_back(sType);
		}
	}
}

void DataBaseHandler::getListofConnections(std::list<ConnectionType>* ConnectionList) {
	sqlite3_stmt* query;
	ConnectionType cType;
	std::string command = "SELECT Source_ID,Sink_ID FROM " + std::string(CONNECTION_TABLE) + ";";
	if (SQexecute(command)) {
		while (sqlite3_step(query)) {
			cType.Source_ID = sqlite3_column_int(query,0);
			cType.Sink_ID =  sqlite3_column_int(query,1);
			ConnectionList->push_back(cType);
		}
	}
}

bool DataBaseHandler::is_source_Mixed(source_t source) {
	sqlite3_stmt* query;
	int classID = 0;

	std::string command = "SELECT Class_ID FROM " + std::string(SOURCE_TABLE) + " WHERE ID='" + int2string(source) + "';";
	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			classID = sqlite3_column_int(query,0);
		}
	}
	command = "SELECT isMixed FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE ID='" + int2string(classID) + "';";

	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			char* answer=(char*)sqlite3_column_text(query,0);
			if (strcmp(answer,"true") == 0) {
				return true;
			}
		}
	}
	return false;
}

sink_t DataBaseHandler::get_Sink_ID_from_Name(std::string name) {
	sqlite3_stmt* query;
	std::string command = "SELECT ID FROM " + std::string(SINK_TABLE) + " WHERE Name='" + name + "';";

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

connection_t DataBaseHandler::getConnectionID(source_t SourceID, sink_t SinkID) {
	sqlite3_stmt* query;
	std::string command="SELECT ID FROM " + std::string(MAIN_TABLE) + " WHERE Source_ID=:sourceID AND Sink_ID=:sinkID";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0,SourceID);
	sqlite3_bind_int(query,1,SinkID);

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

connection_t DataBaseHandler::insertConnection(source_t SourceID, sink_t SinkID) {
	std::string command = "INSERT INTO " + std::string(CONNECTION_TABLE) + " (Source_ID, Sink_ID) VALUES (" + int2string(SourceID) + "," + int2string(SinkID) + ");";
	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

genError_t DataBaseHandler::removeConnection(connection_t ConnectionID) {
	std::string command = "DELETE FROM " + std::string(CONNECTION_TABLE) + " WHERE ID='" + int2string(ConnectionID) + "';";
	if (!this->pQuery(command)) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

connection_t DataBaseHandler::reserveMainConnection(source_t source, sink_t sink) {
	sqlite3_stmt* query;
	std::string command = "INSERT INTO " + std::string(MAIN_TABLE) + "(Source_ID, Sink_ID) VALUES(:Source_ID, :Sink_ID)";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0,source);
	sqlite3_bind_int(query,1,sink);
	if (!this->pQuery(command)) {
		return -1;
	} else {
		return sqlite3_last_insert_rowid(m_database);
	}
}

genError_t DataBaseHandler::updateMainConnection(connection_t connID, genRoute_t route) {
	sqlite3_stmt* query;

	//This information is not handy to be stored directly in the database. So we put it on the heap and store the pointer to it.
	genRoute_t* routeheap = new genRoute_t(route);

	std::string command = "UPDATE " + std::string(MAIN_TABLE) + " SET route=:route WHERE ID=:connID";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0,connID);
	sqlite3_bind_int(query,1,int(routeheap));

	if (!this->pQuery(command)) {
		return GEN_DATABASE_ERROR;
	} else {
		return GEN_OK;
	}
}

genError_t DataBaseHandler::getMainConnectionDatafromID(const connection_t connID, sink_t* return_sinkID, source_t* return_sourceID, genRoute_t** return_route) {
	sqlite3_stmt* query;
	std::string command = "SELECT Sink_ID, Source_ID, route FROM " + std::string(MAIN_TABLE) + " WHERE ID=" + int2string(connID) + ";";

	if (SQexecute(command)) {
		return GEN_DATABASE_ERROR;
	} else {
		if (sqlite3_step(query)) {
			*return_sinkID = sqlite3_column_int(query,0);
			*return_sourceID = sqlite3_column_int(query,1);
			*return_route = reinterpret_cast<genRoute_t*>(sqlite3_column_int(query,2));
			return GEN_OK;
		} else {
			return GEN_UNKNOWN;
		}
	}
}

connection_t DataBaseHandler::returnMainconnectionIDforSinkSourceID(sink_t sink, source_t source) {
	sqlite3_stmt* query;
	std::string command="SELECT ID FROM " + std::string(MAIN_TABLE) + " WHERE Sink_ID=:sinkID AND Source_ID=:SourceID";
	sqlite3_prepare_v2(m_database,command.c_str(),NULL,&query,NULL);
	sqlite3_bind_int(query,0,sink);
	sqlite3_bind_int(query,1,source);

	if (SQexecute(command)) {
		return -1;
	} else {
		sqlite3_step(query);
		return sqlite3_column_int(query,0);
	}
}

std::list<source_t> DataBaseHandler::getSourceIDsForSinkID(sink_t sink) {
	std::list<source_t> list;
	sqlite3_stmt* query;

	std::string command="SELECT Source_ID FROM " + std::string(MAIN_TABLE) + " WHERE Sink_ID=" + int2string(sink);

	if (SQexecute(command)) {
		while (sqlite3_step(query)) {
			list.push_back(sqlite3_column_int(query,0));
		}
	}
	return list;
}

std::list<ConnectionType> DataBaseHandler::getListAllMainConnections() {
	std::list<ConnectionType> connectionList;
	sqlite3_stmt* query;
	std::string command = "SELECT Sink_ID, Source_ID FROM " + std::string(MAIN_TABLE) + ";";

	if (SQexecute(command)) {
		while (sqlite3_step(query)) {
			ConnectionType temp;
			temp.Sink_ID = sqlite3_column_int(query,0);
			temp.Source_ID = sqlite3_column_int(query,1);
			connectionList.push_back(temp);
			DLT_LOG( AudioManager, DLT_LOG_INFO, DLT_STRING("Added Connection"), DLT_INT(temp.Sink_ID), DLT_INT(temp.Source_ID));
		}
	}
	return connectionList;
}

genError_t DataBaseHandler::removeMainConnection(connection_t connID) {
	sqlite3_stmt* query;
	std::string command = "SELECT route FROM " + std::string(MAIN_TABLE) + " WHERE ID=" + int2string(connID) + ";";
	if (SQexecute(command)) {
		if (sqlite3_step(query)) {
			delete reinterpret_cast<genRoute_t*>(sqlite3_column_int(query,0));
			command = "DELETE FROM " + std::string(MAIN_TABLE) + " WHERE ID='" + int2string(connID) + "';";
			if (SQexecute(command)!=true) {
				return GEN_DATABASE_ERROR;
			} else {
				return GEN_OK;
			}
		}
	}
	return GEN_UNKNOWN;
}
