/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file Databasehandler.cpp
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
*/

#include "DatabaseHandler.h"
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <dlt/dlt.h>

#define DOMAIN_TABLE "Domains"
#define SOURCE_CLASS_TABLE "SourceClasses"
#define SINK_CLASS_TABLE "SinkClasses"
#define SOURCE_TABLE "Sources"
#define SINK_TABLE "Sinks"
#define GATEWAY_TABLE "Gateways"
#define CROSSFADER_TABLE "Crossfaders"
#define CONNECTION_TABLE "Connections"
#define MAINCONNECTION_TABLE "MainConnections"
#define INTERRUPT_TABLE "Interrupts"
#define MAIN_TABLE "MainTable"
#define SYSTEM_TABLE "SystemProperties"

DLT_IMPORT_CONTEXT(DLT_CONTEXT)

using namespace am;

const std::string databaseTables[]={
		" Domains (domainID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), busname VARCHAR(50), nodename VARCHAR(50), early BOOL, complete BOOL, state INTEGER, reserved BOOL);",
		" SourceClasses (sourceClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));",
		" SinkClasses (sinkClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));",
		" Sources (sourceID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, domainID INTEGER, name VARCHAR(50), sourceClassID INTEGER, sourceState INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, interruptState INTEGER, reserved BOOL);",
		" Sinks (sinkID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), domainID INTEGER, sinkClassID INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, muteState INTEGER, mainVolume INTEGER, reserved BOOL);",
		" Gateways (gatewayID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID INTEGER, sourceID INTEGER, domainSinkID INTEGER, domainSourceID INTEGER, controlDomainID INTEGER);",
		" Crossfaders (crossfaderID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID_A INTEGER, sinkID_B INTEGER, sourceID INTEGER, hotSink INTEGER);",
		" Connections (connectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, delay INTEGER, connectionFormat INTEGER, reserved BOOL);",
		" MainConnections (mainConnectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, connectionState INTEGER, delay INTEGER);",
		" SystemProperties (type INTEGER PRIMARY KEY, value INTEGER);"
};

/**
 * template to converts T to std::string
 * @param i the value to be converted
 * @return the string
 */
template<typename T>
inline std::string i2s(T const& x)
{
	std::ostringstream o;
	o << x;
	return o.str();
}

DatabaseHandler::DatabaseHandler(std::string databasePath)
	: mDatabase(NULL),
	  mPath(databasePath),
	  mDatabaseObserver(NULL),
	  mFirstStaticSink(true),
	  mFirstStaticSource(true),
	  mFirstStaticGateway(true),
	  mFirstStaticSinkClass(true),
	  mFirstStaticSourceClass(true),
	  mListConnectionFormat()
{

	/**
	 *\todo: this erases the database. just for testing!
	 */
	std::ifstream infile(mPath.c_str());

	if (infile)
	{
		remove(mPath.c_str());
		DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::DatabaseHandler Knocked down database"));
	}

	bool dbOpen=openDatabase();
	if (!dbOpen)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::DatabaseHandler problems opening the database!"));
		assert(!dbOpen);
	}

	createTables();
}



DatabaseHandler::~DatabaseHandler()
{
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("Closed Database"));
	sqlite3_close(mDatabase);
}

am_Error_e DatabaseHandler::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	assert(domainData.domainID==0);
	assert(!domainData.name.empty());
	assert(!domainData.busname.empty());
	assert(domainData.state>=DS_CONTROLLED && domainData.state<=DS_INDEPENDENT_RUNDOWN);

	//first check for a reserved domain
	sqlite3_stmt* query=NULL, *queryFinal;
	int eCode=0;
	std::string command="SELECT domainID FROM "+ std::string(DOMAIN_TABLE) + " WHERE name=?";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, domainData.name.c_str(),domainData.name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		command= "UPDATE "  + std::string(DOMAIN_TABLE) + " SET name=?, busname=?, nodename=?, early=?, complete=?, state=?, reserved=? WHERE domainID=" +i2s(sqlite3_column_int(query,0));
	}
	else if (eCode==SQLITE_DONE)
	{

		command= "INSERT INTO " + std::string(DOMAIN_TABLE) + " (name, busname, nodename, early, complete, state, reserved) VALUES (?,?,?,?,?,?,?)";
	}
	else
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryFinal,NULL);
	sqlite3_bind_text(queryFinal,1, domainData.name.c_str(),domainData.name.size(),SQLITE_STATIC);
	sqlite3_bind_text(queryFinal,2, domainData.busname.c_str(),domainData.busname.size(),SQLITE_STATIC);
	sqlite3_bind_text(queryFinal,3, domainData.nodename.c_str(),domainData.nodename.size(),SQLITE_STATIC);
	sqlite3_bind_int(queryFinal,4, domainData.early);
	sqlite3_bind_int(queryFinal,5, domainData.complete);
	sqlite3_bind_int(queryFinal,6, domainData.state);
	sqlite3_bind_int(queryFinal,7, 0);

	if((eCode=sqlite3_step(queryFinal))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(queryFinal))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	domainID=sqlite3_last_insert_rowid(mDatabase);
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterDomainDB entered new domain with name"), DLT_STRING(domainData.name.c_str()),
			DLT_STRING("busname:"),DLT_STRING(domainData.busname.c_str()),
			DLT_STRING("nodename:"),DLT_STRING(domainData.nodename.c_str()),
			DLT_STRING("early:"), DLT_BOOL(domainData.early),
			DLT_STRING("complete:"),DLT_BOOL(domainData.complete),
			DLT_STRING("state:"),DLT_INT(domainData.state),
			DLT_STRING("assigned ID:"),DLT_INT16(domainID));

	am_Domain_s domain=domainData;
	domain.domainID=domainID;
	if(mDatabaseObserver) mDatabaseObserver->newDomain(domain);

	return E_OK;
}



am_Error_e DatabaseHandler::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
	assert(mainConnectionData.connectionID==0);
	assert(mainConnectionData.connectionState>=CS_CONNECTING && mainConnectionData.connectionState<=CS_SUSPENDED);
	assert(mainConnectionData.route.sinkID!=0);
	assert(mainConnectionData.route.sourceID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "INSERT INTO " + std::string(MAINCONNECTION_TABLE) + "(sourceID, sinkID, connectionState, delay) VALUES (?,?,?,-1)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, mainConnectionData.route.sourceID);
	sqlite3_bind_int(query,2, mainConnectionData.route.sinkID);
	sqlite3_bind_int(query,3, mainConnectionData.connectionState);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}


	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	connectionID=sqlite3_last_insert_rowid(mDatabase);

	//now check the connectionTable for all connections in the route. IF a particular route is not found, we return with error
	std::vector<uint16_t> listOfConnections;
	int16_t delay=0;
	command="SELECT connectionID, delay FROM "+std::string(CONNECTION_TABLE)+(" WHERE sourceID=? AND sinkID=? AND connectionFormat=?");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_RoutingElement_s>::const_iterator elementIterator=mainConnectionData.route.route.begin();
	for (;elementIterator<mainConnectionData.route.route.end();++elementIterator)
	{
		sqlite3_bind_int(query,1, elementIterator->sourceID);
		sqlite3_bind_int(query,2, elementIterator->sinkID);
		sqlite3_bind_int(query,3, elementIterator->connectionFormat);

		if((eCode=sqlite3_step(query))==SQLITE_ROW)
		{
			listOfConnections.push_back(sqlite3_column_int(query,0));
			int16_t temp_delay=sqlite3_column_int(query,1);
			if (temp_delay!=-1 && delay!=-1) delay+=temp_delay;
			else delay=-1;
		}
		else
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB did not find route for MainConnection:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now we create a table with references to the connections;
	command="CREATE TABLE MainConnectionRoute" + i2s(connectionID) + std::string("(connectionID INTEGER)");
	assert(this->sqQuery(command));

	command= "INSERT INTO MainConnectionRoute" + i2s(connectionID) + "(connectionID) VALUES (?)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<uint16_t>::iterator listConnectionIterator=listOfConnections.begin();
	for(;listConnectionIterator<listOfConnections.end();++listConnectionIterator)
	{
		sqlite3_bind_int(query,1, *listConnectionIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterMainConnectionDB entered new mainConnection with sourceID"), DLT_INT(mainConnectionData.route.sourceID),
			DLT_STRING("sinkID:"),DLT_INT16(mainConnectionData.route.sinkID),
			DLT_STRING("delay:"),DLT_INT16(delay),
			DLT_STRING("assigned ID:"),DLT_INT16(connectionID));

	if (mDatabaseObserver)
	{
		mDatabaseObserver->numberOfMainConnectionsChanged();
		mDatabaseObserver->mainConnectionStateChanged(connectionID,mainConnectionData.connectionState);
	}

	//finally, we update the delay value for the maintable
	if (delay==0) delay=-1;
	return changeDelayMainConnection(delay,connectionID);
}



am_Error_e DatabaseHandler::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
	assert(sinkData.sinkID<DYNAMIC_ID_BOUNDARY);
	assert(sinkData.domainID!=0);
	assert(!sinkData.name.empty());
	assert(sinkData.sinkClassID!=0);   // \todo: need to check if class exists?
	assert(!sinkData.listConnectionFormats.empty());
	assert(sinkData.muteState>=MS_MUTED && sinkData.muteState<=MS_UNMUTED);

	sqlite3_stmt *query=NULL, *queryFinal=NULL;
	int eCode=0;
	std::string command="SELECT sinkID FROM "+ std::string(SINK_TABLE) + " WHERE name=? AND reserved=1";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sinkData.name.c_str(),sinkData.name.size(),SQLITE_STATIC);

	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		command= "UPDATE "  + std::string(SINK_TABLE) + " SET name=?, domainID=?, sinkClassID=?, volume=?, visible=?, availability=?, availabilityReason=?, muteState=?, mainVolume=?, reserved=? WHERE sinkID=" +i2s(sqlite3_column_int(query,0));
	}
	else if (eCode==SQLITE_DONE)
	{
		//if sinkID is zero and the first Static Sink was already entered, the ID is created
		if (sinkData.sinkID==0 && !mFirstStaticSink && !existSinkName(sinkData.name))
		{
			command= "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, reserved) VALUES (?,?,?,?,?,?,?,?,?,?)";
		}
		else
		{
			//check if the ID already exists
			if(existSinkNameOrID(sinkData.sinkID,sinkData.name))
			{
				sqlite3_finalize(query);
				return E_ALREADY_EXISTS;
			}
			command= "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, reserved, sinkID) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
		}
	}
	else
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
		sqlite3_finalize(query);
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryFinal,NULL);
	sqlite3_bind_text(queryFinal,1, sinkData.name.c_str(),sinkData.name.size(),SQLITE_STATIC);
	sqlite3_bind_int(queryFinal,2, sinkData.domainID);
	sqlite3_bind_int(queryFinal,3, sinkData.sinkClassID);
	sqlite3_bind_int(queryFinal,4, sinkData.volume);
	sqlite3_bind_int(queryFinal,5, sinkData.visible);
	sqlite3_bind_int(queryFinal,6, sinkData.available.availability);
	sqlite3_bind_int(queryFinal,7, sinkData.available.availabilityReason);
	sqlite3_bind_int(queryFinal,8, sinkData.muteState);
	sqlite3_bind_int(queryFinal,9, sinkData.mainVolume);
	sqlite3_bind_int(queryFinal,10, 0);

	//if the ID is not created, we add it to the query
	if(sinkData.sinkID!=0)
	{
		sqlite3_bind_int(queryFinal,11, sinkData.sinkID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSink)
	{
		sqlite3_bind_int(queryFinal,11, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSink=false;
	}

	if((eCode=sqlite3_step(queryFinal))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
		sqlite3_finalize(queryFinal);
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(queryFinal))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now read back the sinkID
	command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE name=?";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,sinkData.name.c_str(),sinkData.name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkID=sqlite3_column_int(query,0);
	}
	else
	{
		sinkID=0;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
		sqlite3_finalize(query);
		return E_DATABASE_ERROR;
	}
	sqlite3_finalize(query);

	//now we need to create the additional tables:
	command="CREATE TABLE SinkConnectionFormat" + i2s(sinkID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SinkMainSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SinkSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SinkConnectionFormat" + i2s(sinkID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=sinkData.listConnectionFormats.begin();
	for(;connectionFormatIterator<sinkData.listConnectionFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill MainSinkSoundProperties
	command="INSERT INTO SinkMainSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator=sinkData.listMainSoundProperties.begin();
	for(;mainSoundPropertyIterator<sinkData.listMainSoundProperties.end();++mainSoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, mainSoundPropertyIterator->type);
		sqlite3_bind_int(query,2, mainSoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill SinkSoundProperties
	command="INSERT INTO SinkSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator=sinkData.listSoundProperties.begin();
	for(;SoundPropertyIterator<sinkData.listSoundProperties.end();++SoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, SoundPropertyIterator->type);
		sqlite3_bind_int(query,2, SoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkDB entered new sink with name"), DLT_STRING(sinkData.name.c_str()),
			DLT_STRING("domainID:"),DLT_INT(sinkData.domainID),
			DLT_STRING("classID:"),DLT_INT(sinkData.sinkClassID),
			DLT_STRING("volume:"),DLT_INT(sinkData.volume),
			DLT_STRING("visible:"),DLT_BOOL(sinkData.visible),
			DLT_STRING("available.availability:"),DLT_INT(sinkData.available.availability),
			DLT_STRING("available.availabilityReason:"),DLT_INT(sinkData.available.availabilityReason),
			DLT_STRING("muteState:"),DLT_INT(sinkData.muteState),
			DLT_STRING("mainVolume:"),DLT_INT(sinkData.mainVolume),
			DLT_STRING("assigned ID:"),DLT_INT16(sinkID));

	am_Sink_s sink=sinkData;
	sink.sinkID=sinkID;
	if (mDatabaseObserver!=NULL) mDatabaseObserver->newSink(sink);

	return E_OK;
}



am_Error_e DatabaseHandler::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
	//todo: implement crossfader
	(void)crossfaderData;
	(void)crossfaderID;
	return E_UNKNOWN;
}



am_Error_e DatabaseHandler::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
	assert(gatewayData.gatewayID<DYNAMIC_ID_BOUNDARY);
	assert(gatewayData.sinkID!=0);
	assert(gatewayData.sourceID!=0);
	assert(gatewayData.controlDomainID!=0);
	assert(gatewayData.domainSinkID!=0);
	assert(gatewayData.domainSourceID!=0);
	assert(!gatewayData.name.empty());
	assert(!gatewayData.convertionMatrix.empty());
	assert(!gatewayData.listSinkFormats.empty());
	assert(!gatewayData.listSourceFormats.empty());

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	//if sinkID is zero and the first Static Sink was already entered, the ID is created
	if (gatewayData.gatewayID==0 && !mFirstStaticGateway)
	{
		command= "INSERT INTO " + std::string(GATEWAY_TABLE) + "(name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID) VALUES (?,?,?,?,?,?)";
	}
	else
	{
		//check if the ID already exists
		if (existGateway(gatewayData.gatewayID)) return E_ALREADY_EXISTS;
		command= "INSERT INTO " + std::string(GATEWAY_TABLE) + "(name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID) VALUES (?,?,?,?,?,?,?)";
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, gatewayData.name.c_str(),gatewayData.name.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,2, gatewayData.sinkID);
	sqlite3_bind_int(query,3, gatewayData.sourceID);
	sqlite3_bind_int(query,4, gatewayData.domainSinkID);
	sqlite3_bind_int(query,5, gatewayData.domainSourceID);
	sqlite3_bind_int(query,6, gatewayData.controlDomainID);

	//if the ID is not created, we add it to the query
	if(gatewayData.gatewayID!=0)
	{
		sqlite3_bind_int(query,7, gatewayData.gatewayID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticGateway)
	{
		sqlite3_bind_int(query,7, DYNAMIC_ID_BOUNDARY);
		mFirstStaticGateway=false;
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	gatewayID=sqlite3_last_insert_rowid(mDatabase);

	//now the convertion matrix todo: change the map implementation sometimes to blob in sqlite
	mListConnectionFormat.insert(std::make_pair(gatewayID,gatewayData.convertionMatrix));

	command="CREATE TABLE GatewaySourceFormat" + i2s(gatewayID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE GatewaySinkFormat" + i2s(gatewayID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO GatewaySourceFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=gatewayData.listSourceFormats.begin();
	for(;connectionFormatIterator<gatewayData.listSourceFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	command="INSERT INTO GatewaySinkFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	connectionFormatIterator=gatewayData.listSinkFormats.begin();
	for(;connectionFormatIterator<gatewayData.listSinkFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}


	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterGatewayDB entered new gateway with name"), DLT_STRING(gatewayData.name.c_str()),
			DLT_STRING("sourceID:"),DLT_INT(gatewayData.sourceID),
			DLT_STRING("sinkID:"),DLT_INT(gatewayData.sinkID),
			DLT_STRING("domainSinkID:"),DLT_INT(gatewayData.domainSinkID),
			DLT_STRING("domainSourceID:"),DLT_BOOL(gatewayData.domainSourceID),
			DLT_STRING("controlDomainID:"),DLT_INT(gatewayData.controlDomainID),
			DLT_STRING("assigned ID:"),DLT_INT16(gatewayID));

	am_Gateway_s gateway=gatewayData;
	gateway.gatewayID=gatewayID;
	if(mDatabaseObserver) mDatabaseObserver->newGateway(gateway);
	return E_OK;
}



am_Error_e DatabaseHandler::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
	assert(sourceData.sourceID<DYNAMIC_ID_BOUNDARY);
	assert(sourceData.domainID!=0);
	assert(!sourceData.name.empty());
	assert(sourceData.sourceClassID!=0);   // \todo: need to check if class exists?
	assert(!sourceData.listConnectionFormats.empty());
	assert(sourceData.sourceState>=SS_ON && sourceData.sourceState<=SS_PAUSED);

	sqlite3_stmt* query=NULL, *queryFinal=NULL;;
	int eCode=0;
	std::string command="SELECT sourceID FROM "+ std::string(SOURCE_TABLE) + " WHERE name=? AND reserved=1";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sourceData.name.c_str(),sourceData.name.size(),SQLITE_STATIC);

	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		command= "UPDATE "  + std::string(SOURCE_TABLE) + " SET name=?, domainID=?, sourceClassID=?, sourceState=?, volume=?, visible=?, availability=?, availabilityReason=?, interruptState=?, reserved=? WHERE sourceID=" +i2s(sqlite3_column_int(query,0));
	}
	else if (eCode==SQLITE_DONE)
	{
		//if sinkID is zero and the first Static Sink was already entered, the ID is created
		if (sourceData.sourceID==0 && !mFirstStaticSource && !existSourceName(sourceData.name))
		{
			command= "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, reserved) VALUES (?,?,?,?,?,?,?,?,?,?)";
		}
		else
		{
			//check if the ID already exists
			if (existSourceNameOrID(sourceData.sourceID,sourceData.name))
			{
				sqlite3_finalize(query);
				return E_ALREADY_EXISTS;
			}
			command= "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, reserved, sourceID) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
		}
	}
	else
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
		sqlite3_finalize(query);
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryFinal,NULL);
	sqlite3_bind_text(queryFinal,1, sourceData.name.c_str(),sourceData.name.size(),SQLITE_STATIC);
	sqlite3_bind_int(queryFinal,2, sourceData.domainID);
	sqlite3_bind_int(queryFinal,3, sourceData.sourceClassID);
	sqlite3_bind_int(queryFinal,4, sourceData.sourceState);
	sqlite3_bind_int(queryFinal,5, sourceData.volume);
	sqlite3_bind_int(queryFinal,6, sourceData.visible);
	sqlite3_bind_int(queryFinal,7, sourceData.available.availability);
	sqlite3_bind_int(queryFinal,8, sourceData.available.availabilityReason);
	sqlite3_bind_int(queryFinal,9, sourceData.interruptState);
	sqlite3_bind_int(queryFinal,10, 0);

	//if the ID is not created, we add it to the query
	if(sourceData.sourceID!=0)
	{
		sqlite3_bind_int(queryFinal,11, sourceData.sourceID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSource)
	{
		sqlite3_bind_int(queryFinal,11, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSource=false;
	}

	if((eCode=sqlite3_step(queryFinal))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
		sqlite3_finalize(queryFinal);
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(queryFinal))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Finalize error code:"),DLT_INT(eCode));
		sqlite3_finalize(queryFinal);
		return E_DATABASE_ERROR;
	}

	//now read back the sinkID
	command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE name=?";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,sourceData.name.c_str(),sourceData.name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sourceID=sqlite3_column_int(query,0);
	}
	else
	{
		sourceID=0;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
		sqlite3_finalize(query);
		return E_DATABASE_ERROR;
	}
	sqlite3_finalize(query);


	//now we need to create the additional tables:
	command="CREATE TABLE SourceConnectionFormat" + i2s(sourceID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SourceMainSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SourceSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SourceConnectionFormat" + i2s(sourceID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=sourceData.listConnectionFormats.begin();
	for(;connectionFormatIterator<sourceData.listConnectionFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill MainSinkSoundProperties
	command="INSERT INTO SourceMainSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator=sourceData.listMainSoundProperties.begin();
	for(;mainSoundPropertyIterator<sourceData.listMainSoundProperties.end();++mainSoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, mainSoundPropertyIterator->type);
		sqlite3_bind_int(query,2, mainSoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill SinkSoundProperties
	command="INSERT INTO SourceSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator=sourceData.listSoundProperties.begin();
	for(;SoundPropertyIterator<sourceData.listSoundProperties.end();++SoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, SoundPropertyIterator->type);
		sqlite3_bind_int(query,2, SoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			sqlite3_finalize(query);
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkDB entered new source with name"), DLT_STRING(sourceData.name.c_str()),
			DLT_STRING("domainID:"),DLT_INT(sourceData.domainID),
			DLT_STRING("classID:"),DLT_INT(sourceData.sourceClassID),
			DLT_STRING("volume:"),DLT_INT(sourceData.volume),
			DLT_STRING("visible:"),DLT_BOOL(sourceData.visible),
			DLT_STRING("available.availability:"),DLT_INT(sourceData.available.availability),
			DLT_STRING("available.availabilityReason:"),DLT_INT(sourceData.available.availabilityReason),
			DLT_STRING("interruptState:"),DLT_INT(sourceData.interruptState),
			DLT_STRING("assigned ID:"),DLT_INT16(sourceID));

	am_Source_s source=sourceData;
	source.sourceID=sourceID;
	if(mDatabaseObserver) mDatabaseObserver->newSource(source);
	return E_OK;
}



am_Error_e DatabaseHandler::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const am_Route_s & route)
{
	assert(mainconnectionID!=0);
	if(!existMainConnection(mainconnectionID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	std::vector<uint16_t> listOfConnections;
	int16_t delay=0;
	command="SELECT connectionID, delay FROM "+std::string(CONNECTION_TABLE)+(" WHERE sourceID=? AND sinkID=? AND connectionFormat=?");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_RoutingElement_s>::const_iterator elementIterator=route.route.begin();
	for (;elementIterator<route.route.end();++elementIterator)
	{
		sqlite3_bind_int(query,1, elementIterator->sourceID);
		sqlite3_bind_int(query,2, elementIterator->sinkID);
		sqlite3_bind_int(query,3, elementIterator->connectionFormat);

		if((eCode=sqlite3_step(query))==SQLITE_ROW)
		{
			listOfConnections.push_back(sqlite3_column_int(query,0));
			int16_t temp_delay=sqlite3_column_int(query,1);
			if (temp_delay!=-1 && delay!=-1) delay+=temp_delay;
			else delay=-1;
		}
		else
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB did not find route for MainConnection:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now we delete the data in the table
	command="DELETE from MainConnectionRoute" + i2s(mainconnectionID);
	assert(this->sqQuery(command));

	command= "INSERT INTO MainConnectionRoute" + i2s(mainconnectionID) + "(connectionID) VALUES (?)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<uint16_t>::iterator listConnectionIterator=listOfConnections.begin();
	for(;listConnectionIterator<listOfConnections.end();++listConnectionIterator)
	{
		sqlite3_bind_int(query,1, *listConnectionIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB entered new route:"),DLT_INT(mainconnectionID));
	return E_OK;
}

am_Error_e DatabaseHandler::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
	assert(mainconnectionID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existMainConnection(mainconnectionID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET connectionState=? WHERE mainConnectionID=" + i2s(mainconnectionID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connectionState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB changed mainConnectionState of MainConnection:"),DLT_INT(mainconnectionID),DLT_STRING("to:"),DLT_INT(connectionState));

	if (mDatabaseObserver) mDatabaseObserver->mainConnectionStateChanged(mainconnectionID,connectionState);
	return E_OK;
}



am_Error_e DatabaseHandler::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SINK_TABLE) + " SET mainVolume=? WHERE sinkID=" + i2s(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, mainVolume);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB changed mainVolume of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(mainVolume));

	if(mDatabaseObserver) mDatabaseObserver->volumeChanged(sinkID,mainVolume);

	return E_OK;
}



am_Error_e DatabaseHandler::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SINK_TABLE) + " SET availability=?, availabilityReason=? WHERE sinkID=" + i2s(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, availability.availability);
	sqlite3_bind_int(query,2, availability.availabilityReason);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB changed sinkAvailability of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(availability.availability), DLT_STRING("Reason:"),DLT_INT(availability.availabilityReason));

	if (mDatabaseObserver && sourceVisible(sinkID)) mDatabaseObserver->sinkAvailabilityChanged(sinkID,availability);
	return E_OK;
}



am_Error_e DatabaseHandler::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
	assert(domainID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existDomain(domainID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(DOMAIN_TABLE) + " SET state=? WHERE domainID=" + i2s(domainID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, domainState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changDomainStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changDomainStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changDomainStateDB changed domainState of domain:"),DLT_INT(domainID),DLT_STRING("to:"),DLT_INT(domainState));
	return E_OK;
}



am_Error_e DatabaseHandler::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SINK_TABLE) + " SET muteState=? WHERE sinkID=" + i2s(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, muteState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB changed sinkMuteState of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(muteState));

	if(mDatabaseObserver) mDatabaseObserver->sinkMuteStateChanged(sinkID,muteState);

	return E_OK;
}



am_Error_e DatabaseHandler::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
	//todo: add checks if soundproperty exists!
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE SinkMainSoundProperty" + i2s(sinkID)+ " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB changed MainSinkSoundProperty of sink:"),DLT_INT(sinkID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));
	if (mDatabaseObserver) mDatabaseObserver->mainSinkSoundPropertyChanged(sinkID,soundProperty);
	return E_OK;
}



am_Error_e DatabaseHandler::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
	//todo: add checks if soundproperty exists!
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE SourceMainSoundProperty" + i2s(sourceID)+ " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB changed MainSinkSoundProperty of source:"),DLT_INT(sourceID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));

	if(mDatabaseObserver) mDatabaseObserver->mainSourceSoundPropertyChanged(sourceID,soundProperty);
	return E_OK;
}



am_Error_e DatabaseHandler::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SOURCE_TABLE) + " SET availability=?, availabilityReason=? WHERE sourceID=" + i2s(sourceID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, availability.availability);
	sqlite3_bind_int(query,2, availability.availabilityReason);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB changed changeSourceAvailabilityDB of source:"),DLT_INT(sourceID),DLT_STRING("to:"),DLT_INT(availability.availability), DLT_STRING("Reason:"),DLT_INT(availability.availabilityReason));

	if (mDatabaseObserver && sourceVisible(sourceID)) mDatabaseObserver->sourceAvailabilityChanged(sourceID,availability);
	return E_OK;
}



am_Error_e DatabaseHandler::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command="UPDATE " + std::string(SYSTEM_TABLE) + " set value=? WHERE type=?";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, property.value);
	sqlite3_bind_int(query,2, property.type);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSystemPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}


	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSystemPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSystemPropertyDB changed system property"));

	if(mDatabaseObserver) mDatabaseObserver->systemPropertyChanged(property);

	return E_OK;
}



am_Error_e DatabaseHandler::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
	assert(mainConnectionID!=0);

	if (!existMainConnection(mainConnectionID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
	std::string command1 = "DROP table MainConnectionRoute" + i2s(mainConnectionID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeMainConnectionDB removed:"),DLT_INT(mainConnectionID));
	if (mDatabaseObserver) {
		mDatabaseObserver->mainConnectionStateChanged(mainConnectionID,CS_DISCONNECTED);
		mDatabaseObserver->numberOfMainConnectionsChanged();
	}
	return E_OK;
}



am_Error_e DatabaseHandler::removeSinkDB(const am_sinkID_t sinkID)
{
	assert(sinkID!=0);

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SINK_TABLE) + " WHERE sinkID=" + i2s(sinkID);
	std::string command1 = "DROP table SinkConnectionFormat" + i2s(sinkID);
	std::string command2 = "DROP table SinkMainSoundProperty" + i2s(sinkID);
	std::string command3 = "DROP table SinkSoundProperty" + i2s(sinkID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	if(!sqQuery(command2)) return E_DATABASE_ERROR;
	if(!sqQuery(command3)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSinkDB removed:"),DLT_INT(sinkID));

	if (mDatabaseObserver!=NULL) mDatabaseObserver->removedSink(sinkID);

	return E_OK;
}



am_Error_e DatabaseHandler::removeSourceDB(const am_sourceID_t sourceID)
{
	assert(sourceID!=0);

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
	std::string command1 = "DROP table SourceConnectionFormat" + i2s(sourceID);
	std::string command2 = "DROP table SourceMainSoundProperty" + i2s(sourceID);
	std::string command3 = "DROP table SourceSoundProperty" + i2s(sourceID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	if(!sqQuery(command2)) return E_DATABASE_ERROR;
	if(!sqQuery(command3)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSourceDB removed:"),DLT_INT(sourceID));
	if(mDatabaseObserver) mDatabaseObserver->removedSource(sourceID);
	return E_OK;
}



am_Error_e DatabaseHandler::removeGatewayDB(const am_gatewayID_t gatewayID)
{
	assert(gatewayID!=0);

	if (!existGateway(gatewayID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + i2s(gatewayID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeGatewayDB removed:"),DLT_INT(gatewayID));
	if(mDatabaseObserver) mDatabaseObserver->removeGateway(gatewayID);
	return E_OK;
}



am_Error_e DatabaseHandler::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{
	//todo: implement crossdfader
	(void)crossfaderID;
	return E_UNKNOWN;
}



am_Error_e DatabaseHandler::removeDomainDB(const am_domainID_t domainID)
{
	assert(domainID!=0);

	if (!existDomain(domainID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(DOMAIN_TABLE) + " WHERE domainID=" + i2s(domainID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeDomainDB removed:"),DLT_INT(domainID));
	if(mDatabaseObserver) mDatabaseObserver->removeDomain(domainID);
	return E_OK;
}

am_Error_e DatabaseHandler::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
	assert(sinkClassID!=0);

	if (!existSinkClass(sinkClassID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + i2s(sinkClassID);
	std::string command1 = "DROP table SinkClassProperties" + i2s(sinkClassID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSinkClassDB removed:"),DLT_INT(sinkClassID));
	if (mDatabaseObserver) mDatabaseObserver->numberOfSinkClassesChanged();

	return E_OK;
}

am_Error_e DatabaseHandler::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
	assert(sourceClassID!=0);

	if (!existSourceClass(sourceClassID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + i2s(sourceClassID);
	std::string command1 = "DROP table SourceClassProperties" + i2s(sourceClassID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSourceClassDB removed:"),DLT_INT(sourceClassID));

	if (mDatabaseObserver) mDatabaseObserver->numberOfSourceClassesChanged();
	return E_OK;
}

am_Error_e DatabaseHandler::removeConnection(const am_connectionID_t connectionID)
{
	assert(connectionID!=0);

	std::string command = "DELETE from " + std::string(CONNECTION_TABLE) + " WHERE connectionID=" + i2s(connectionID);
	std::string command1 = "DROP table SourceClassProperties" + i2s(connectionID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeConnection removed:"),DLT_INT(connectionID));

	return E_OK;
}


am_Error_e DatabaseHandler::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{
	assert(sourceID!=0);

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_ClassProperty_s propertyTemp;
	std::string command= "SELECT sourceClassID FROM " + std::string(SOURCE_TABLE)+ " WHERE sourceID=" + (i2s(sourceID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classInfo.sourceClassID=sqlite3_column_int(query,0);
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	command= "SELECT name FROM " + std::string(SOURCE_CLASS_TABLE)+ " WHERE sourceClassID=" + (i2s(classInfo.sourceClassID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classInfo.name=std::string((const char*)sqlite3_column_text(query,0));
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//read out Properties
	command= "SELECT classProperty, value FROM SourceClassProperties"+ i2s(classInfo.sourceClassID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(query,0);
		propertyTemp.value=sqlite3_column_int(query,1);
		classInfo.listClassProperties.push_back(propertyTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	return E_OK;
}



am_Error_e DatabaseHandler::changeSinkClassInfoDB(const am_SinkClass_s& sinkClass)
{
	assert(sinkClass.sinkClassID!=0);
	assert(!sinkClass.listClassProperties.empty());


	sqlite3_stmt* query=NULL;
	int eCode=0;

	//check if the ID already exists
	if(!existSinkClass(sinkClass.sinkClassID)) return E_NON_EXISTENT;

	//fill ConnectionFormats
	std::string command="UPDATE SinkClassProperties" + i2s(sinkClass.sinkClassID) + " set value=? WHERE classProperty=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sinkClass.listClassProperties.begin();
	for(;Iterator<sinkClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->value);
		sqlite3_bind_int(query,2, Iterator->classProperty);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo"));
	return E_OK;
}



am_Error_e DatabaseHandler::changeSourceClassInfoDB(const am_SourceClass_s& sourceClass)
{
	assert(sourceClass.sourceClassID!=0);
	assert(!sourceClass.listClassProperties.empty());


	sqlite3_stmt* query=NULL;
	int eCode=0;

	//check if the ID already exists
	if(!existSourceClass(sourceClass.sourceClassID)) return E_NON_EXISTENT;

	//fill ConnectionFormats
	std::string command="UPDATE SourceClassProperties" + i2s(sourceClass.sourceClassID) + " set value=? WHERE classProperty=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sourceClass.listClassProperties.begin();
	for(;Iterator<sourceClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->value);
		sqlite3_bind_int(query,2, Iterator->classProperty);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo"));
	return E_OK;
}



am_Error_e DatabaseHandler::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
	assert(sinkID!=0);

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_ClassProperty_s propertyTemp;
	std::string command= "SELECT sinkClassID FROM " + std::string(SINK_TABLE)+ " WHERE sinkID=" + (i2s(sinkID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkClass.sinkClassID=sqlite3_column_int(query,0);
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	command= "SELECT name FROM " + std::string(SINK_CLASS_TABLE)+ " WHERE sinkClassID=" + (i2s(sinkClass.sinkClassID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkClass.name=std::string((const char*)sqlite3_column_text(query,0));
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//read out Properties
	command= "SELECT classProperty, value FROM SinkClassProperties"+ i2s(sinkClass.sinkClassID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(query,0);
		propertyTemp.value=sqlite3_column_int(query,1);
		sinkClass.listClassProperties.push_back(propertyTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	return E_OK;
}



am_Error_e DatabaseHandler::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
	assert(gatewayID!=0);
	if (!existGateway(gatewayID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL, *qSinkConnectionFormat=NULL, *qSourceConnectionFormat=NULL;
	int eCode=0;
	am_ConnectionFormat_e tempConnectionFormat;
	std::string command= "SELECT name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID="+i2s(gatewayID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		gatewayData.name=std::string((const char*)sqlite3_column_text(query,0));
		gatewayData.sinkID=sqlite3_column_int(query,1);
		gatewayData.sourceID=sqlite3_column_int(query,2);
		gatewayData.domainSinkID=sqlite3_column_int(query,3);
		gatewayData.domainSourceID=sqlite3_column_int(query,4);
		gatewayData.controlDomainID=sqlite3_column_int(query,5);
		gatewayData.gatewayID=sqlite3_column_int(query,6);

		//convertionMatrix:
		ListConnectionFormat::const_iterator iter=mListConnectionFormat.begin();
		iter=mListConnectionFormat.find(gatewayData.gatewayID);
		if (iter == mListConnectionFormat.end())
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB database error with convertionFormat"));
			return E_DATABASE_ERROR;
		}
		gatewayData.convertionMatrix=iter->second;

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM GatewaySourceFormat" + i2s(gatewayData.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSourceConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSourceConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSourceConnectionFormat,0);
			gatewayData.listSourceFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSourceConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		commandConnectionFormat= "SELECT soundFormat FROM GatewaySinkFormat" + i2s(gatewayData.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSinkConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSinkConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSinkConnectionFormat,0);
			gatewayData.listSinkFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSinkConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;

}



am_Error_e DatabaseHandler::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
	//todo: implement crossfader
	(void)crossfaderID;
	(void)crossfaderData;
	return E_UNKNOWN;
}



am_Error_e DatabaseHandler::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
	assert(domainID!=0);
	listSinkID.clear();
	if (!existDomain(domainID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_sinkID_t temp;
	std::string command= "SELECT sinkID FROM " + std::string(SINK_TABLE)+ " WHERE reserved=0 AND domainID=" + (i2s(domainID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listSinkID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinksOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinksOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
	assert(domainID!=0);
	listSourceID.clear();
	if (!existDomain(domainID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_sourceID_t temp;
	std::string command= "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND domainID=" + i2s(domainID);

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listSourceID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourcesOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourcesOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listGatewaysID) const
{
	//todo: implement crossfader
	(void)listGatewaysID;
	(void)domainID;
	return E_UNKNOWN;

}



am_Error_e DatabaseHandler::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
	assert(domainID!=0);
	listGatewaysID.clear();
	if (!existDomain(domainID))
	{
		return E_NON_EXISTENT;
	}
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_gatewayID_t temp;

	std::string command= "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE controlDomainID=" +i2s(domainID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listGatewaysID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGatewaysOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGatewaysOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
	listMainConnections.clear();
	sqlite3_stmt *query=NULL, *query1=NULL, *query2=NULL;
	int eCode=0;
	am_MainConnection_s temp;
	am_RoutingElement_s tempRoute;

	std::string command= "SELECT mainConnectionID, sourceID, sinkID, connectionState, delay FROM " + std::string(MAINCONNECTION_TABLE);
	std::string command1= "SELECT connectionID FROM MainConnectionRoute";
	std::string command2= "SELECT sourceID, sinkID, connectionFormat FROM " + std::string(CONNECTION_TABLE) + " WHERE connectionID=?";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&query2,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.connectionID=sqlite3_column_int(query,0);
		temp.route.sourceID=sqlite3_column_int(query,1);
		temp.route.sinkID=sqlite3_column_int(query,2);
		temp.connectionState=(am_ConnectionState_e)sqlite3_column_int(query,3);
		temp.delay=sqlite3_column_int(query,4);
		std::string statement=command1 + i2s(temp.connectionID);
		sqlite3_prepare_v2(mDatabase,statement.c_str(),-1,&query1,NULL);
		while((eCode=sqlite3_step(query1))==SQLITE_ROW) //todo: check results of eCode1, eCode2
		{
			int k=sqlite3_column_int(query1,0);
			sqlite3_bind_int(query2,1,k);
			while((eCode=sqlite3_step(query2))==SQLITE_ROW)
			{
				tempRoute.sourceID=sqlite3_column_int(query2,0);
				tempRoute.sinkID=sqlite3_column_int(query2,1);
				tempRoute.connectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(query2,2);
				getDomainOfSource(tempRoute.sourceID,tempRoute.domainID);
				temp.route.route.push_back(tempRoute);
			}
			sqlite3_reset(query2);
		}
		listMainConnections.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainConnections SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
	listDomains.clear();
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_Domain_s temp;
	std::string command= "SELECT domainID, name, busname, nodename, early, complete, state FROM " + std::string(DOMAIN_TABLE) + " WHERE reserved=0";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.domainID=sqlite3_column_int(query,0);
		temp.name=std::string((const char*)sqlite3_column_text(query,1));
		temp.busname=std::string((const char*)sqlite3_column_text(query,2));
		temp.nodename=std::string((const char*)sqlite3_column_text(query,3));
		temp.early=sqlite3_column_int(query,4);
		temp.complete=sqlite3_column_int(query,5);
		temp.state=(am_DomainState_e)sqlite3_column_int(query,6);
		listDomains.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListDomains SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListDomains SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
	listConnections.clear();
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_Connection_s temp;
	std::string command= "SELECT connectionID, sourceID, sinkID, delay, connectionFormat FROM " + std::string(CONNECTION_TABLE)+" WHERE reserved=0";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.connectionID=sqlite3_column_int(query,0);
		temp.sourceID=sqlite3_column_int(query,1);
		temp.sinkID=sqlite3_column_int(query,2);
		temp.delay=sqlite3_column_int(query,3);
		temp.connectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(query,4);
		listConnections.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListConnections SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
	listSinks.clear();
	sqlite3_stmt* query=NULL, *qConnectionFormat=NULL, *qSoundProperty=NULL, *qMAinSoundProperty=NULL;
	int eCode=0;
	am_Sink_s temp;
	am_ConnectionFormat_e tempConnectionFormat;
	am_SoundProperty_s tempSoundProperty;
	am_MainSoundProperty_s tempMainSoundProperty;
	std::string command= "SELECT name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, sinkID FROM " + std::string(SINK_TABLE)+ " WHERE reserved=0";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.name=std::string((const char*)sqlite3_column_text(query,0));
		temp.domainID=sqlite3_column_int(query,1);
		temp.sinkClassID=sqlite3_column_int(query,2);
		temp.volume=sqlite3_column_int(query,3);
		temp.visible=sqlite3_column_int(query,4);
		temp.available.availability=(am_Availablility_e)sqlite3_column_int(query,5);
		temp.available.availabilityReason=(am_AvailabilityReason_e)sqlite3_column_int(query,6);
		temp.muteState=(am_MuteState_e)sqlite3_column_int(query,7);
		temp.mainVolume=sqlite3_column_int(query,8);
		temp.sinkID=sqlite3_column_int(query,9);

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM SinkConnectionFormat"+ i2s(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qConnectionFormat,NULL);
		while((eCode=sqlite3_step(qConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qConnectionFormat,0);
			temp.listConnectionFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		std::string commandSoundProperty= "SELECT soundPropertyType, value FROM SinkSoundProperty" + i2s(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandSoundProperty.c_str(),-1,&qSoundProperty,NULL);
		while((eCode=sqlite3_step(qSoundProperty))==SQLITE_ROW)
		{
			tempSoundProperty.type=(am_SoundPropertyType_e)sqlite3_column_int(qSoundProperty,0);
			tempSoundProperty.value=sqlite3_column_int(qSoundProperty,1);
			temp.listSoundProperties.push_back(tempSoundProperty);
		}

		if((eCode=sqlite3_finalize(qSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out MainSoundProperties
		std::string commandMainSoundProperty= "SELECT soundPropertyType, value FROM SinkMainSoundProperty"+ i2s(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandMainSoundProperty.c_str(),-1,&qMAinSoundProperty,NULL);
		while((eCode=sqlite3_step(qMAinSoundProperty))==SQLITE_ROW)
		{
			tempMainSoundProperty.type=(am_MainSoundPropertyType_e)sqlite3_column_int(qMAinSoundProperty,0);
			tempMainSoundProperty.value=sqlite3_column_int(qMAinSoundProperty,1);
			temp.listMainSoundProperties.push_back(tempMainSoundProperty);
		}

		if((eCode=sqlite3_finalize(qMAinSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSinks.push_back(temp);
		temp.listConnectionFormats.clear();
		temp.listMainSoundProperties.clear();
		temp.listSoundProperties.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSources(std::vector<am_Source_s> & listSources) const
{
	listSources.clear();
	sqlite3_stmt* query=NULL, *qConnectionFormat=NULL, *qSoundProperty=NULL, *qMAinSoundProperty=NULL;
	int eCode=0;
	am_Source_s temp;
	am_ConnectionFormat_e tempConnectionFormat;
	am_SoundProperty_s tempSoundProperty;
	am_MainSoundProperty_s tempMainSoundProperty;
	std::string command= "SELECT name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, sourceID FROM " + std::string(SOURCE_TABLE) +" WHERE reserved=0";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.name=std::string((const char*)sqlite3_column_text(query,0));
		temp.domainID=sqlite3_column_int(query,1);
		temp.sourceClassID=sqlite3_column_int(query,2);
		temp.sourceState=(am_SourceState_e)sqlite3_column_int(query,3);
		temp.volume=sqlite3_column_int(query,4);
		temp.visible=sqlite3_column_int(query,5);
		temp.available.availability=(am_Availablility_e)sqlite3_column_int(query,6);
		temp.available.availabilityReason=(am_AvailabilityReason_e)sqlite3_column_int(query,7);
		temp.interruptState=(am_InterruptState_e)sqlite3_column_int(query,8);
		temp.sourceID=sqlite3_column_int(query,9);

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM SourceConnectionFormat"+ i2s(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qConnectionFormat,NULL);
		while((eCode=sqlite3_step(qConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qConnectionFormat,0);
			temp.listConnectionFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		std::string commandSoundProperty= "SELECT soundPropertyType, value FROM SourceSoundProperty" + i2s(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandSoundProperty.c_str(),-1,&qSoundProperty,NULL);
		while((eCode=sqlite3_step(qSoundProperty))==SQLITE_ROW)
		{
			tempSoundProperty.type=(am_SoundPropertyType_e)sqlite3_column_int(qSoundProperty,0);
			tempSoundProperty.value=sqlite3_column_int(qSoundProperty,1);
			temp.listSoundProperties.push_back(tempSoundProperty);
		}

		if((eCode=sqlite3_finalize(qSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out MainSoundProperties
		std::string commandMainSoundProperty= "SELECT soundPropertyType, value FROM SourceMainSoundProperty"+ i2s(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandMainSoundProperty.c_str(),-1,&qMAinSoundProperty,NULL);
		while((eCode=sqlite3_step(qMAinSoundProperty))==SQLITE_ROW)
		{
			tempMainSoundProperty.type=(am_MainSoundPropertyType_e)sqlite3_column_int(qMAinSoundProperty,0);
			tempMainSoundProperty.value=sqlite3_column_int(qMAinSoundProperty,1);
			temp.listMainSoundProperties.push_back(tempMainSoundProperty);
		}

		if((eCode=sqlite3_finalize(qMAinSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSources.push_back(temp);
		temp.listConnectionFormats.clear();
		temp.listMainSoundProperties.clear();
		temp.listSoundProperties.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
	listSourceClasses.clear();

	sqlite3_stmt* query=NULL, *subQuery=NULL;
	int eCode=0, eCode1;
	am_SourceClass_s classTemp;
	am_ClassProperty_s propertyTemp;

	std::string command= "SELECT sourceClassID, name FROM " + std::string(SOURCE_CLASS_TABLE);
	std::string command2;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classTemp.sourceClassID=sqlite3_column_int(query,0);
		classTemp.name=std::string((const char*)sqlite3_column_text(query,1));

		//read out Properties
		command2="SELECT classProperty, value FROM SourceClassProperties"+ i2s(classTemp.sourceClassID);
		sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&subQuery,NULL);

		while((eCode1=sqlite3_step(subQuery))==SQLITE_ROW)
		{
			propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(subQuery,0);
			propertyTemp.value=sqlite3_column_int(subQuery,1);
			classTemp.listClassProperties.push_back(propertyTemp);
		}

		if(eCode1!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}

		if((eCode1=sqlite3_finalize(subQuery))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
		listSourceClasses.push_back(classTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
	//todo: implement crossfaders
	(void)listCrossfaders;
	return E_UNKNOWN;
}



am_Error_e DatabaseHandler::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
	listGateways.clear();
	sqlite3_stmt* query=NULL, *qSinkConnectionFormat=NULL, *qSourceConnectionFormat=NULL;
	int eCode=0;
	am_Gateway_s temp;
	am_ConnectionFormat_e tempConnectionFormat;

	std::string command= "SELECT name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID FROM " + std::string(GATEWAY_TABLE);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.name=std::string((const char*)sqlite3_column_text(query,0));
		temp.sinkID=sqlite3_column_int(query,1);
		temp.sourceID=sqlite3_column_int(query,2);
		temp.domainSinkID=sqlite3_column_int(query,3);
		temp.domainSourceID=sqlite3_column_int(query,4);
		temp.controlDomainID=sqlite3_column_int(query,5);
		temp.gatewayID=sqlite3_column_int(query,6);

		//convertionMatrix:
		ListConnectionFormat::const_iterator iter=mListConnectionFormat.begin();
		iter=mListConnectionFormat.find(temp.gatewayID);
	    if (iter == mListConnectionFormat.end())
	    {
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways database error with convertionFormat"));
			return E_DATABASE_ERROR;
	    }
	   temp.convertionMatrix=iter->second;

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM GatewaySourceFormat" + i2s(temp.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSourceConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSourceConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSourceConnectionFormat,0);
			temp.listSourceFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSourceConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		commandConnectionFormat= "SELECT soundFormat FROM GatewaySinkFormat" + i2s(temp.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSinkConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSinkConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSinkConnectionFormat,0);
			temp.listSinkFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSinkConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		listGateways.push_back(temp);
		temp.listSinkFormats.clear();
		temp.listSourceFormats.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
	listSinkClasses.clear();

	sqlite3_stmt* query=NULL, *subQuery=NULL;
	int eCode=0;
	am_SinkClass_s classTemp;
	am_ClassProperty_s propertyTemp;

	std::string command= "SELECT sinkClassID, name FROM " + std::string(SINK_CLASS_TABLE);
	std::string command2;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classTemp.sinkClassID=sqlite3_column_int(query,0);
		classTemp.name=std::string((const char*)sqlite3_column_text(query,1));

		//read out Properties
		command2="SELECT classProperty, value FROM SinkClassProperties"+ i2s(classTemp.sinkClassID);
		sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&subQuery,NULL);

		while((eCode=sqlite3_step(subQuery))==SQLITE_ROW)
		{
			propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(subQuery,0);
			propertyTemp.value=sqlite3_column_int(subQuery,1);
			classTemp.listClassProperties.push_back(propertyTemp);
		}

		if(eCode!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		if((eCode=sqlite3_finalize(subQuery))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSinkClasses.push_back(classTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListVisibleMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
	listConnections.clear();
	sqlite3_stmt *query=NULL;
	int eCode=0;
	am_MainConnectionType_s temp;

	std::string command= "SELECT mainConnectionID, sourceID, sinkID, connectionState, delay FROM " + std::string(MAINCONNECTION_TABLE);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.mainConnectionID=sqlite3_column_int(query,0);
		temp.sourceID=sqlite3_column_int(query,1);
		temp.sinkID=sqlite3_column_int(query,2);
		temp.connectionState=(am_ConnectionState_e)sqlite3_column_int(query,3);
		temp.delay=sqlite3_column_int(query,4);
		listConnections.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListVisibleMainConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListVisibleMainConnections SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainSinks(std::vector<am_SinkType_s> & listMainSinks) const
{
	listMainSinks.clear();
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_SinkType_s temp;

	std::string command= "SELECT name, sinkID, availability, availabilityReason, muteState, mainVolume, sinkClassID FROM " + std::string(SINK_TABLE) + " WHERE visible=1 AND reserved=0";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.name=std::string((const char*)sqlite3_column_text(query,0));
		temp.sinkID=sqlite3_column_int(query,1);
		temp.availability.availability=(am_Availablility_e)sqlite3_column_int(query,2);
		temp.availability.availabilityReason=(am_AvailabilityReason_e)sqlite3_column_int(query,3);
		temp.muteState=(am_MuteState_e)sqlite3_column_int(query,4);
		temp.volume=sqlite3_column_int(query,5);
		temp.sinkClassID=sqlite3_column_int(query,6);
		listMainSinks.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainSources(std::vector<am_SourceType_s> & listMainSources) const
{
	listMainSources.clear();
	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_SourceType_s temp;
	std::string command= "SELECT name, sourceClassID, availability, availabilityReason, sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE visible=1";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.name=std::string((const char*)sqlite3_column_text(query,0));
		temp.sourceClassID=sqlite3_column_int(query,1);
		temp.availability.availability=(am_Availablility_e)sqlite3_column_int(query,2);
		temp.availability.availabilityReason=(am_AvailabilityReason_e)sqlite3_column_int(query,3);
		temp.sourceID=sqlite3_column_int(query,4);

		listMainSources.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
	assert(sinkID!=0);
	if (!existSink(sinkID)) return E_DATABASE_ERROR; // todo: here we could change to non existen, but not shown in sequences
	listSoundProperties.clear();

	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_MainSoundProperty_s temp;
	std::string command= "SELECT soundPropertyType, value FROM SinkMainSoundProperty" + i2s(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.type=(am_MainSoundPropertyType_e)sqlite3_column_int(query,0);
		temp.value=sqlite3_column_int(query,1);
		listSoundProperties.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
	assert(sourceID!=0);
	if (!existSource(sourceID)) return E_DATABASE_ERROR; // todo: here we could change to non existen, but not shown in sequences
	listSourceProperties.clear();

	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_MainSoundProperty_s temp;
	std::string command= "SELECT soundPropertyType, value FROM SourceMainSoundProperty" + i2s(sourceID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.type=(am_MainSoundPropertyType_e)sqlite3_column_int(query,0);
		temp.value=sqlite3_column_int(query,1);
		listSourceProperties.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
	listSystemProperties.clear();

	sqlite3_stmt* query=NULL;
	int eCode=0;
	am_SystemProperty_s temp;
	std::string command= "SELECT type, value FROM " + std::string(SYSTEM_TABLE);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.type=(am_SystemPropertyType_e)sqlite3_column_int(query,0);
		temp.value=sqlite3_column_int(query,1);
		listSystemProperties.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSystemProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSystemProperties SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
	assert(mainConnectionID!=0);
	delay=-1;
	sqlite3_stmt *query=NULL;
	int eCode=0;

	std::string command= "SELECT delay FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		delay=sqlite3_column_int(query,0);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getTimingInformation SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getTimingInformation SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if (delay==-1) return E_NOT_POSSIBLE;

	return E_OK;
}

bool DatabaseHandler::sqQuery(const std::string& query)
{
	sqlite3_stmt* statement;
	int eCode=0;
	if ((eCode=sqlite3_exec(mDatabase,query.c_str(),NULL,&statement,NULL))!= SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::sqQuery SQL Query failed:"), DLT_STRING(query.c_str()), DLT_STRING("error code:"),DLT_INT(eCode));
		return false;
	}
	return true;
}

bool DatabaseHandler::openDatabase()
{
	if (sqlite3_open_v2(mPath.c_str(),&mDatabase,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::openDatabase opened database"));
		return true;
	}
	DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::openDatabase failed to open database"));
	return false;
}

am_Error_e DatabaseHandler::changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID)
{
	assert(connectionID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command ="SELECT mainConnectionID FROM "+ std::string(MAINCONNECTION_TABLE) +" WHERE delay=? AND mainConnectionID=?";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, delay);
	sqlite3_bind_int(query,2, connectionID);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		sqlite3_finalize(query);
		return E_OK;
	}
	command="UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET delay=? WHERE mainConnectionID=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, delay);
	sqlite3_bind_int(query,2, connectionID);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeDelayMainConnection SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeDelayMainConnection SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if(mDatabaseObserver) mDatabaseObserver->timingInformationChanged(connectionID,delay);

	return E_OK;
}

am_Error_e DatabaseHandler::enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID)
{
	assert(connection.connectionID==0);
	assert(connection.sinkID!=0);
	assert(connection.sourceID!=0);
	//connection format is not checked, because it's project specific

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "INSERT INTO " + std::string(CONNECTION_TABLE) + "(sinkID, sourceID, delay, connectionFormat, reserved) VALUES (?,?,?,?,?)";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connection.sinkID);
	sqlite3_bind_int(query,2, connection.sourceID);
	sqlite3_bind_int(query,3, connection.delay);
	sqlite3_bind_int(query,4, connection.connectionFormat);
	sqlite3_bind_int(query,5, true);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	connectionID=sqlite3_last_insert_rowid(mDatabase);

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterConnectionDB entered new connection sourceID:"), DLT_INT16(connection.sourceID),
			DLT_STRING("sinkID:"),DLT_INT16(connection.sinkID),
			DLT_STRING("sourceID:"),DLT_INT16(connection.sourceID),
			DLT_STRING("delay:"), DLT_INT16(connection.delay),
			DLT_STRING("connectionFormat:"),DLT_INT16(connection.connectionFormat),
			DLT_STRING("assigned ID:"),DLT_INT16(connectionID));
	return E_OK;
}

am_Error_e DatabaseHandler::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
	assert(sinkClass.sinkClassID<DYNAMIC_ID_BOUNDARY);
	assert(!sinkClass.listClassProperties.empty());
	assert(!sinkClass.name.empty());

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	//if sinkID is zero and the first Static Sink was already entered, the ID is created
	if (sinkClass.sinkClassID==0 && !mFirstStaticSinkClass)
	{
		command= "INSERT INTO " + std::string(SINK_CLASS_TABLE) + "(name) VALUES (?)";
	}
	else
	{
		//check if the ID already exists
		if(existSinkClass(sinkClass.sinkClassID)) return E_ALREADY_EXISTS;
		command= "INSERT INTO " + std::string(SINK_CLASS_TABLE) + "(name, sinkClassID) VALUES (?,?)";
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sinkClass.name.c_str(),sinkClass.name.size(),SQLITE_STATIC);

	//if the ID is not created, we add it to the query
	if(sinkClass.sinkClassID!=0)
	{
		sqlite3_bind_int(query,2, sinkClass.sinkClassID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSinkClass)
	{
		sqlite3_bind_int(query,2, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSinkClass=false;
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sinkClassID=sqlite3_last_insert_rowid(mDatabase); //todo:change last_insert implementations for mulithread usage...

	//now we need to create the additional tables:
	command="CREATE TABLE SinkClassProperties" + i2s(sinkClassID) + std::string("(classProperty INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SinkClassProperties" + i2s(sinkClassID) + std::string("(classProperty,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sinkClass.listClassProperties.begin();
	for(;Iterator<sinkClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->classProperty);
		sqlite3_bind_int(query,2, Iterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkClassDB entered new sinkClass"));
	if (mDatabaseObserver) mDatabaseObserver->numberOfSinkClassesChanged();
	return E_OK;
}

am_Error_e DatabaseHandler::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
	assert(sourceClass.sourceClassID<DYNAMIC_ID_BOUNDARY);
	assert(!sourceClass.listClassProperties.empty());
	assert(!sourceClass.name.empty());

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	//if sinkID is zero and the first Static Sink was already entered, the ID is created
	if (sourceClass.sourceClassID==0 && !mFirstStaticSourceClass)
	{
		command= "INSERT INTO " + std::string(SOURCE_CLASS_TABLE) + "(name) VALUES (?)";
	}
	else
	{
		//check if the ID already exists
		if(existSourceClass(sourceClass.sourceClassID)) return E_ALREADY_EXISTS;
		command= "INSERT INTO " + std::string(SOURCE_CLASS_TABLE) + "(name, sourceClassID) VALUES (?,?)";
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sourceClass.name.c_str(),sourceClass.name.size(),SQLITE_STATIC);

	//if the ID is not created, we add it to the query
	if(sourceClass.sourceClassID!=0)
	{
		sqlite3_bind_int(query,2, sourceClass.sourceClassID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSourceClass)
	{
		sqlite3_bind_int(query,2, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSourceClass=false;
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sourceClassID=sqlite3_last_insert_rowid(mDatabase); //todo:change last_insert implementations for mulithread usage...

	//now we need to create the additional tables:
	command="CREATE TABLE SourceClassProperties" + i2s(sourceClassID) + std::string("(classProperty INTEGER, value INTEGER)");
	assert(sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SourceClassProperties" + i2s(sourceClassID) + std::string("(classProperty,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sourceClass.listClassProperties.begin();
	for(;Iterator<sourceClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->classProperty);
		sqlite3_bind_int(query,2, Iterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSourceClassDB entered new sourceClass"));

	if (mDatabaseObserver) mDatabaseObserver->numberOfSourceClassesChanged();
	return E_OK;
}

am_Error_e DatabaseHandler::enterSystemProperties(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::vector<am_SystemProperty_s>::const_iterator listIterator =listSystemProperties.begin();
	std::string command= "DELETE * FROM " + std::string(SYSTEM_TABLE);
	sqQuery(command);

	command="INSERT INTO " + std::string(SYSTEM_TABLE) + " (type, value) VALUES (?,?)";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	for(;listIterator<listSystemProperties.end();++listIterator)
	{
		sqlite3_bind_int(query,1, listIterator->type);
		sqlite3_bind_int(query,2, listIterator->value);

		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSystemProperties SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSystemProperties SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSystemProperties entered system properties"));
	return E_OK;
}

bool DatabaseHandler::existMainConnection(const am_mainConnectionID_t mainConnectionID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT mainConnectionID FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existMainConnection database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSource(const am_sourceID_t sourceID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND sourceID=" + i2s(sourceID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSourceNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND (name=? OR sourceID=?)";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,2,sourceID);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSourceName(const std::string & name) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND name=?";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSink(const am_sinkID_t sinkID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND sinkID=" + i2s(sinkID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSinkNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND (name=? OR sinkID=?)";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,2,sinkID);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSinkName(const std::string & name) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND name=?";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existDomain(const am_domainID_t domainID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE reserved=0 AND domainID=" + i2s(domainID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existDomain database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existGateway(const am_gatewayID_t gatewayID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + i2s(gatewayID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existGateway database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

am_Error_e DatabaseHandler::getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t & domainID) const
{
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	std::string command = "SELECT domainID FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
	int eCode=0;
	am_Error_e returnVal=E_DATABASE_ERROR;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		domainID=sqlite3_column_int(query,0);
		returnVal=E_OK;
	}
	else
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getDomainOfSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}


bool DatabaseHandler::existSinkClass(const am_sinkClass_t sinkClassID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkClassID FROM " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + i2s(sinkClassID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSinkClass database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSourceClass(const am_sourceClass_t sourceClassID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceClassID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + i2s(sourceClassID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSinkClass database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

am_Error_e DatabaseHandler::changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
	assert(connectionID!=0);

	sqlite3_stmt *query=NULL, *queryMainConnections, *queryMainConnectionSubIDs;
	int eCode=0, eCode1=0;
	std::string command= "UPDATE " + std::string(CONNECTION_TABLE) + " set delay=? WHERE connectionID=?";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, delay);
	sqlite3_bind_int(query,2, connectionID);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionTimingInformation SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionTimingInformation SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now we need to find all mainConnections that use the changed connection and update their timing
	am_timeSync_t tempDelay=0;
	am_Error_e error;

	int tempMainConnectionID;
	//first get all route tables for all mainconnections
	command= "SELECT name FROM sqlite_master WHERE type ='table' and name LIKE 'MainConnectionRoute%'";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryMainConnections,NULL);

	while((eCode=sqlite3_step(queryMainConnections))==SQLITE_ROW)
	{
		//now check if the connection ID is in this table
		std::string tablename=std::string((const char*)sqlite3_column_text(queryMainConnections,0));
		std::string command2="SELECT connectionID FROM " + tablename + " WHERE connectionID="+i2s(connectionID);
		sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&queryMainConnectionSubIDs,NULL);
		if((eCode1=sqlite3_step(queryMainConnectionSubIDs))==SQLITE_ROW)
		{
			//if the connection ID is in, recalculate the mainconnection delay
			std::stringstream(tablename.substr(tablename.find_first_not_of("MainConnectionRoute"))) >> tempMainConnectionID;
			changeDelayMainConnection(calculateMainConnectionDelay(tempMainConnectionID),tempMainConnectionID);
		}
		else if(eCode1!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionTimingInformation SQLITE error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionTimingInformation SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(queryMainConnections))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionTimingInformation SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}

am_Error_e DatabaseHandler::changeConnectionFinal(const am_connectionID_t connectionID)
{
	assert(connectionID!=0);

	sqlite3_stmt *query=NULL;
	int eCode=0;
	std::string command= "UPDATE " + std::string(CONNECTION_TABLE) + " set reserved=0 WHERE connectionID=?";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connectionID);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionFinal SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeConnectionFinal SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	return E_OK;
}

am_timeSync_t DatabaseHandler::calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const
{
	assert (mainConnectionID!=0);
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sum(Connections.delay),min(Connections.delay) FROM "+ std::string(CONNECTION_TABLE)+",MainConnectionRoute"+ i2s(mainConnectionID)+" WHERE MainConnectionRoute"+ i2s(mainConnectionID)+".connectionID = Connections.connectionID" ;
	int eCode=0;
	am_timeSync_t delay=0;
	am_timeSync_t min=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		delay=sqlite3_column_int(query,0);
		min=sqlite3_column_int(query,1);
	}
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::calculateMainConnectionDelay SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::calculateMainConnectionDelay SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if (min<0) delay=-1;
	return delay;

}

void DatabaseHandler::registerObserver(DatabaseObserver *iObserver)
{
	assert(iObserver!=NULL);
	mDatabaseObserver=iObserver;
}

bool DatabaseHandler::sourceVisible(const am_sourceID_t sourceID) const
{
	assert(sourceID!=0);
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT visible FROM "+ std::string(SOURCE_TABLE) +" WHERE sourceID="+ i2s(sourceID);
	int eCode=0;
	bool returnVal=false;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		returnVal=(bool)sqlite3_column_int(query,0);
	}
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::sourceVisible database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::sinkVisible(const am_sinkID_t sinkID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT visible FROM "+ std::string(SINK_TABLE) +" WHERE reserved=0 AND sinkID="+ i2s(sinkID);
	int eCode=0;
	bool returnVal=false;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		returnVal=sqlite3_column_int(query,0);
	}
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::sinkVisible database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existConnection(const am_Connection_s connection)
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT connectionID FROM " + std::string(CONNECTION_TABLE) + " WHERE sinkID=? AND sourceID=? AND connectionFormat=? AND reserved=0";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connection.sinkID);
	sqlite3_bind_int(query,2, connection.sourceID);
	sqlite3_bind_int(query,3, connection.connectionFormat);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existMainConnection database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}


bool DatabaseHandler::existConnectionID(const am_connectionID_t connectionID)
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT connectionID FROM " + std::string(CONNECTION_TABLE) + " WHERE connectionID=? AND reserved=0";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connectionID);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existMainConnection database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existcrossFader(const am_crossfaderID_t crossfaderID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT crossfaderID FROM " + std::string(CROSSFADER_TABLE) + " WHERE crossfaderID=?";
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, crossfaderID);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existMainConnection database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

am_Error_e DatabaseHandler::getSoureState(const am_sourceID_t sourceID, am_SourceState_e & sourceState) const
{
	assert(sourceID!=0);
	sqlite3_stmt* query=NULL;
	sourceState=SS_MIN;
	std::string command = "SELECT sourceState FROM "+ std::string(SOURCE_TABLE) +" WHERE sourceID="+ i2s(sourceID);
	int eCode=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sourceState=(am_SourceState_e)sqlite3_column_int(query,0);
	}
	else if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSoureState database error!:"), DLT_INT(eCode))

	}
	sqlite3_finalize(query);
	return E_OK;
}

am_Error_e DatabaseHandler::changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState)
{
	assert(sourceID!=0);
	sqlite3_stmt* query=NULL;
	std::string command = "UPDATE " + std::string(SOURCE_TABLE) +" SET sourceState=? WHERE sourceID="+ i2s(sourceID);
	int eCode=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1,sourceState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceState SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceState SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	return E_OK;
}

am_Error_e DatabaseHandler::getSinkVolume(const am_sinkID_t sinkID, am_volume_t & volume) const
{
	assert(sinkID!=0);
	sqlite3_stmt* query=NULL;
	volume=-1;
	std::string command = "SELECT volume FROM "+ std::string(SINK_TABLE) +" WHERE sinkID="+ i2s(sinkID);
	int eCode=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		volume=sqlite3_column_int(query,0);
	}
	else if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkVolume database error!:"), DLT_INT(eCode))

	}
	sqlite3_finalize(query);
	return E_OK;
}

am_Error_e DatabaseHandler::getSourceVolume(const am_sourceID_t sourceID, am_volume_t & volume) const
{
	assert(sourceID!=0);
	sqlite3_stmt* query=NULL;
	volume=-1;
	std::string command = "SELECT volume FROM "+ std::string(SOURCE_TABLE) +" WHERE sourceID="+ i2s(sourceID);
	int eCode=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		volume=sqlite3_column_int(query,0);
	}
	else if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceVolume database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return E_OK;
}


am_Error_e DatabaseHandler::getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_SoundPropertyType_e propertyType, uint16_t & value) const
{
	assert(sinkID!=0);
	if (!existSink(sinkID)) return E_DATABASE_ERROR; // todo: here we could change to non existent, but not shown in sequences

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "SELECT value FROM SinkSoundProperty" + i2s(sinkID) +" WHERE soundPropertyType=" +i2s(propertyType);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		value=sqlite3_column_int(query,0);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkSoundPropertyValue SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkSoundPropertyValue SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}

am_Error_e DatabaseHandler::getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_SoundPropertyType_e propertyType, uint16_t & value) const
{
	assert(sourceID!=0);
	if (!existSource(sourceID)) return E_DATABASE_ERROR; // todo: here we could change to non existent, but not shown in sequences

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "SELECT value FROM SourceSoundProperty" + i2s(sourceID) +" WHERE soundPropertyType=" +i2s(propertyType);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		value=sqlite3_column_int(query,0);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkSoundPropertyValue SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkSoundPropertyValue SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}

am_Error_e DatabaseHandler::getDomainState(const am_domainID_t domainID, am_DomainState_e state) const
{
	assert(domainID!=0);
	sqlite3_stmt* query=NULL;
	state=DS_MIN;
	std::string command = "SELECT domainState FROM "+ std::string(DOMAIN_TABLE) +" WHERE domainID="+ i2s(domainID);
	int eCode=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		state=(am_DomainState_e)sqlite3_column_int(query,0);
	}
	else if ((eCode=sqlite3_step(query))==SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getDomainState database error!:"), DLT_INT(eCode))

	}
	sqlite3_finalize(query);
	return E_OK;

}

am_Error_e DatabaseHandler::peekDomain(const std::string & name, am_domainID_t & domainID)
{
	sqlite3_stmt* query=NULL, *queryInsert=NULL;
	std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE name=?";
	int eCode=0, eCode1=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		domainID=sqlite3_column_int(query,0);
	}
	else if (eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekDomain database error!:"), DLT_INT(eCode))
		return E_DATABASE_ERROR;
	}
	else
	{
		command= "INSERT INTO " + std::string(DOMAIN_TABLE) + " (name,reserved) VALUES (?,?)";
		sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryInsert,NULL);
		sqlite3_bind_text(queryInsert,1,name.c_str(),name.size(),SQLITE_STATIC);
		sqlite3_bind_int(queryInsert,2,1); //reservation flag
		if((eCode1=sqlite3_step(queryInsert))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekDomain SQLITE Step error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}

		if((eCode1=sqlite3_finalize(queryInsert))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekDomain SQLITE Finalize error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
		domainID=sqlite3_last_insert_rowid(mDatabase);
	}
	sqlite3_finalize(query);
	return E_OK;
}

am_Error_e DatabaseHandler::peekSink(const std::string & name, am_sinkID_t & sinkID)
{
	sqlite3_stmt* query=NULL, *queryInsert=NULL;
	std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE name=?";
	int eCode=0, eCode1=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkID=sqlite3_column_int(query,0);
	}
	else if (eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekSink database error!:"), DLT_INT(eCode))
		return E_DATABASE_ERROR;
	}
	else
	{
		if (mFirstStaticSink)
		{
			command= "INSERT INTO " + std::string(SINK_TABLE) + " (name,reserved,sinkID) VALUES (?,?," + i2s(DYNAMIC_ID_BOUNDARY) + ")";
			mFirstStaticSink=false;
		}
		else
		{
			command= "INSERT INTO " + std::string(SINK_TABLE) + " (name,reserved) VALUES (?,?)";
		}
		sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryInsert,NULL);
		sqlite3_bind_text(queryInsert,1,name.c_str(),name.size(),SQLITE_STATIC);
		sqlite3_bind_int(queryInsert,2,1); //reservation flag
		if((eCode1=sqlite3_step(queryInsert))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekSink SQLITE Step error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}

		if((eCode1=sqlite3_finalize(queryInsert))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekDomain SQLITE Finalize error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
		sinkID=sqlite3_last_insert_rowid(mDatabase);
	}
	sqlite3_finalize(query);
	return E_OK;
}



am_Error_e DatabaseHandler::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
	sqlite3_stmt* query=NULL, *queryInsert=NULL;
	std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE name=?";
	int eCode=0, eCode1=0;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1,name.c_str(),name.size(),SQLITE_STATIC);
	if ((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sourceID=sqlite3_column_int(query,0);
	}
	else if (eCode!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekSink database error!:"), DLT_INT(eCode))
		return E_DATABASE_ERROR;
	}
	else
	{
		if (mFirstStaticSource)
		{
			command= "INSERT INTO " + std::string(SOURCE_TABLE) + " (name,reserved,sourceID) VALUES (?,?," + i2s(DYNAMIC_ID_BOUNDARY) + ")";
			mFirstStaticSource=false;
		}
		else
		{
			command= "INSERT INTO " + std::string(SOURCE_TABLE) + " (name,reserved) VALUES (?,?)";
		}
		sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&queryInsert,NULL);
		sqlite3_bind_text(queryInsert,1,name.c_str(),name.size(),SQLITE_STATIC);
		sqlite3_bind_int(queryInsert,2,1); //reservation flag
		if((eCode1=sqlite3_step(queryInsert))!=SQLITE_DONE)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekSink SQLITE Step error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}

		if((eCode1=sqlite3_finalize(queryInsert))!=SQLITE_OK)
		{
			DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::peekDomain SQLITE Finalize error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
		sourceID=sqlite3_last_insert_rowid(mDatabase);
	}
	sqlite3_finalize(query);
	return E_OK;
}

am_Error_e DatabaseHandler::changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume)
{
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SINK_TABLE) + " SET volume=? WHERE sinkID=" + i2s(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, volume);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkVolume SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkVolume SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkVolume changed volume of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(volume));

	return E_OK;
}

am_Error_e DatabaseHandler::changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume)
{
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(SOURCE_TABLE) + " SET volume=? WHERE sourceID=" + i2s(sourceID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, volume);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceVolume SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceVolume SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSourceVolume changed volume of source=:"),DLT_INT(sourceID),DLT_STRING("to:"),DLT_INT(volume));

	return E_OK;
}

am_Error_e DatabaseHandler::changeSourceSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
	//todo: add checks if soundproperty exists!
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE SourceSoundProperty" + i2s(sourceID)+ " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSourceSoundPropertyDB changed SourceSoundProperty of source:"),DLT_INT(sourceID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));

	return E_OK;
}

am_Error_e DatabaseHandler::changeSinkSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
	//todo: add checks if soundproperty exists!
	assert(sinkID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE SinkSoundProperty" + i2s(sinkID)+ " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkSoundPropertyDB changed MainSinkSoundProperty of sink:"),DLT_INT(sinkID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));

	return E_OK;
}

am_Error_e DatabaseHandler::changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink)
{
	assert(crossfaderID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	if (!existcrossFader(crossfaderID))
	{
		return E_NON_EXISTENT;
	}
	command = "UPDATE " + std::string(CROSSFADER_TABLE) + " SET hotsink=? WHERE crossfaderID=" + i2s(crossfaderID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, hotsink);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeCrossFaderHotSink SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(DLT_CONTEXT, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeCrossFaderHotSink SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeCrossFaderHotSink changed hotsink of crossfader="),DLT_INT(crossfaderID),DLT_STRING("to:"),DLT_INT(hotsink));

	return E_OK;
}

void DatabaseHandler::createTables()
{
	for(uint16_t i=0;i<sizeof(databaseTables)/sizeof(databaseTables[0]);i++)
	{
		assert(sqQuery("CREATE TABLE " + databaseTables[i]));
	}
}






