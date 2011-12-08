/*
 * DatabaseHandler.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "DatabaseHandler.h"
#include <dlt/dlt.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

DLT_IMPORT_CONTEXT(AudioManager);

//#define DATABASE_PATH "/tmp/audiomanager.sqlite"
#define DATABASE_PATH ":memory:"

#define DOMAIN_TABLE "Domains"
#define SOURCE_CLASS_TABLE "SourceClasses"
#define SINK_CLASS_TABLE "SinkClasses"
#define SOURCE_TABLE "Sources"
#define SINK_TABLE "Sinks"
#define GATEWAY_TABLE "Gateways"
#define CONNECTION_TABLE "Connections"
#define MAINCONNECTION_TABLE "MainConnections"
#define INTERRUPT_TABLE "Interrupts"
#define MAIN_TABLE "MainTable"
#define SYSTEM_TABLE "SystemProperties"

const std::string databaseTables[]={
		" Domains (domainID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), busname VARCHAR(50), nodename VARCHAR(50), early BOOL, complete BOOL, state INTEGER);",
		" SourceClasses (sourceClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));",
		" SinkClasses (sinkClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));",
		" Sources (sourceID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, domainID INTEGER, name VARCHAR(50), sourceClassID INTEGER, sourceState INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, interruptState INTEGER);",
		" Sinks (sinkID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), domainID INTEGER, sinkClassID INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, muteState INTEGER, mainVolume INTEGER);",
		" Gateways (gatewayID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID INTEGER, sourceID INTEGER, domainSinkID INTEGER, domainSourceID INTEGER, controlDomainID INTEGER);",
		" Connections (connectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, delay INTEGER, connectionFormat INTEGER );",
		" MainConnections (mainConnectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, connectionState INTEGER, delay INTEGER);",
		" SystemProperties (type INTEGER PRIMARY KEY, value INTEGER);"
};

/**
 * template to converts T to std::string
 * @param i the value to be converted
 * @return the string
 */

std::string int2string(int i) {
	std::stringstream out;
	out << i;
	return out.str();
}


DatabaseHandler::DatabaseHandler()
	: mDatabase(NULL),
	  mPath(DATABASE_PATH),
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
		DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::DatabaseHandler Knocked down database"));
	}

	bool dbOpen=openDatabase();
	if (!dbOpen)
	{
		DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::DatabaseHandler problems opening the database!"));
		assert(!dbOpen);
	}

	createTables();
}



DatabaseHandler::~DatabaseHandler()
{
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Closed Database"));
	sqlite3_close(mDatabase);
}



am_Error_e DatabaseHandler::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	assert(domainData.domainID==0);
	assert(!domainData.name.empty());
	assert(!domainData.busname.empty());
	assert(domainData.state>=DS_CONTROLLED && domainData.state<=DS_INDEPENDENT_RUNDOWN);


	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "INSERT INTO " + std::string(DOMAIN_TABLE) + "(name, busname, nodename, early, complete, state) VALUES (?,?,?,?,?,?)";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, domainData.name.c_str(),domainData.name.size(),SQLITE_STATIC);
	sqlite3_bind_text(query,2, domainData.busname.c_str(),domainData.busname.size(),SQLITE_STATIC);
	sqlite3_bind_text(query,3, domainData.nodename.c_str(),domainData.nodename.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,4, domainData.early);
	sqlite3_bind_int(query,5, domainData.complete);
	sqlite3_bind_int(query,6, domainData.state);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterDomainDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	domainID=sqlite3_last_insert_rowid(mDatabase);
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterDomainDB entered new domain with name"), DLT_STRING(domainData.name.c_str()),
			DLT_STRING("busname:"),DLT_STRING(domainData.busname.c_str()),
			DLT_STRING("nodename:"),DLT_STRING(domainData.nodename.c_str()),
			DLT_STRING("early:"), DLT_BOOL(domainData.early),
			DLT_STRING("complete:"),DLT_BOOL(domainData.complete),
			DLT_STRING("state:"),DLT_INT(domainData.state),
			DLT_STRING("assigned ID:"),DLT_INT16(domainID));
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
	std::string command= "INSERT INTO " + std::string(MAINCONNECTION_TABLE) + "(sourceID, sinkID, connectionState) VALUES (?,?,?)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, mainConnectionData.route.sourceID);
	sqlite3_bind_int(query,2, mainConnectionData.route.sinkID);
	sqlite3_bind_int(query,3, mainConnectionData.connectionState);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}


	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	connectionID=sqlite3_last_insert_rowid(mDatabase);

	//now check the connectionTabel for all connections in the route. IF a particular route is not found, we return with error
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
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB did not find route for MainConnection:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now we create a table with references to the connections;
	command="CREATE TABLE MainConnectionRoute" + int2string(connectionID) + std::string("(connectionID INTEGER)");
	assert(this->sqQuery(command));

	command= "INSERT INTO MainConnectionRoute" + int2string(connectionID) + "(connectionID) VALUES (?)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<uint16_t>::iterator listConnectionIterator=listOfConnections.begin();
	for(;listConnectionIterator<listOfConnections.end();++listConnectionIterator)
	{
		sqlite3_bind_int(query,1, *listConnectionIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterMainConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterMainConnectionDB entered new mainConnection with sourceID"), DLT_INT(mainConnectionData.route.sourceID),
			DLT_STRING("sinkID:"),DLT_INT16(mainConnectionData.route.sinkID),
			DLT_STRING("delay:"),DLT_INT16(delay),
			DLT_STRING("assigned ID:"),DLT_INT16(connectionID));

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

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	//if sinkID is zero and the first Static Sink was already entered, the ID is created
	if (sinkData.sinkID==0 && !mFirstStaticSink)
	{
		command= "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume) VALUES (?,?,?,?,?,?,?,?,?)";
	}
	else
	{
		//check if the ID already exists
		if(existSink(sinkData.sinkID)) return E_ALREADY_EXISTS;
		command= "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, sinkID) VALUES (?,?,?,?,?,?,?,?,?,?)";
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sinkData.name.c_str(),sinkData.name.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,2, sinkData.domainID);
	sqlite3_bind_int(query,3, sinkData.sinkClassID);
	sqlite3_bind_int(query,4, sinkData.volume);
	sqlite3_bind_int(query,5, sinkData.visible);
	sqlite3_bind_int(query,6, sinkData.available.availability);
	sqlite3_bind_int(query,7, sinkData.available.availabilityReason);
	sqlite3_bind_int(query,8, sinkData.muteState);
	sqlite3_bind_int(query,9, sinkData.mainVolume);

	//if the ID is not created, we add it to the query
	if(sinkData.sinkID!=0)
	{
		sqlite3_bind_int(query,10, sinkData.sinkID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSink)
	{
		sqlite3_bind_int(query,10, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSink=false;
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sinkID=sqlite3_last_insert_rowid(mDatabase); //todo:change last_insert implementations for mulithread usage...

	//now we need to create the additional tables:
	command="CREATE TABLE SinkConnectionFormat" + int2string(sinkID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SinkMainSoundProperty" + int2string(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SinkSoundProperty" + int2string(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SinkConnectionFormat" + int2string(sinkID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=sinkData.listConnectionFormats.begin();
	for(;connectionFormatIterator<sinkData.listConnectionFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill MainSinkSoundProperties
	command="INSERT INTO SinkMainSoundProperty" + int2string(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator=sinkData.listMainSoundProperties.begin();
	for(;mainSoundPropertyIterator<sinkData.listMainSoundProperties.end();++mainSoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, mainSoundPropertyIterator->type);
		sqlite3_bind_int(query,2, mainSoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill SinkSoundProperties
	command="INSERT INTO SinkSoundProperty" + int2string(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator=sinkData.listSoundProperties.begin();
	for(;SoundPropertyIterator<sinkData.listSoundProperties.end();++SoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, SoundPropertyIterator->type);
		sqlite3_bind_int(query,2, SoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkDB entered new sink with name"), DLT_STRING(sinkData.name.c_str()),
			DLT_STRING("domainID:"),DLT_INT(sinkData.domainID),
			DLT_STRING("classID:"),DLT_INT(sinkData.sinkClassID),
			DLT_STRING("volume:"),DLT_INT(sinkData.volume),
			DLT_STRING("visible:"),DLT_BOOL(sinkData.visible),
			DLT_STRING("available.availability:"),DLT_INT(sinkData.available.availability),
			DLT_STRING("available.availabilityReason:"),DLT_INT(sinkData.available.availabilityReason),
			DLT_STRING("muteState:"),DLT_INT(sinkData.muteState),
			DLT_STRING("mainVolume:"),DLT_INT(sinkData.mainVolume),
			DLT_STRING("assigned ID:"),DLT_INT16(sinkID));

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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	gatewayID=sqlite3_last_insert_rowid(mDatabase);

	//now the convertion matrix todo: change the map implementation sometimes to blob in sqlite
	mListConnectionFormat.insert(std::make_pair(gatewayID,gatewayData.convertionMatrix));

	command="CREATE TABLE GatewaySourceFormat" + int2string(gatewayID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE GatewaySinkFormat" + int2string(gatewayID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO GatewaySourceFormat" + int2string(gatewayID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=gatewayData.listSourceFormats.begin();
	for(;connectionFormatIterator<gatewayData.listSourceFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	command="INSERT INTO GatewaySinkFormat" + int2string(gatewayID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	connectionFormatIterator=gatewayData.listSinkFormats.begin();
	for(;connectionFormatIterator<gatewayData.listSinkFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterGatewayDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}


	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterGatewayDB entered new gateway with name"), DLT_STRING(gatewayData.name.c_str()),
			DLT_STRING("sourceID:"),DLT_INT(gatewayData.sourceID),
			DLT_STRING("sinkID:"),DLT_INT(gatewayData.sinkID),
			DLT_STRING("domainSinkID:"),DLT_INT(gatewayData.domainSinkID),
			DLT_STRING("domainSourceID:"),DLT_BOOL(gatewayData.domainSourceID),
			DLT_STRING("controlDomainID:"),DLT_INT(gatewayData.controlDomainID),
			DLT_STRING("assigned ID:"),DLT_INT16(gatewayID));

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

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command;

	//if sinkID is zero and the first Static Sink was already entered, the ID is created
	if (sourceData.sourceID==0 && !mFirstStaticSource)
	{
		command= "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState) VALUES (?,?,?,?,?,?,?,?,?)";
	}
	else
	{
		//check if the ID already exists
		if (existSource(sourceData.sourceID)) return E_ALREADY_EXISTS;
		else command= "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, sourceID) VALUES (?,?,?,?,?,?,?,?,?,?)";
	}

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_text(query,1, sourceData.name.c_str(),sourceData.name.size(),SQLITE_STATIC);
	sqlite3_bind_int(query,2, sourceData.domainID);
	sqlite3_bind_int(query,3, sourceData.sourceClassID);
	sqlite3_bind_int(query,4, sourceData.sourceState);
	sqlite3_bind_int(query,5, sourceData.volume);
	sqlite3_bind_int(query,6, sourceData.visible);
	sqlite3_bind_int(query,7, sourceData.available.availability);
	sqlite3_bind_int(query,8, sourceData.available.availabilityReason);
	sqlite3_bind_int(query,9, sourceData.interruptState);

	//if the ID is not created, we add it to the query
	if(sourceData.sourceID!=0)
	{
		sqlite3_bind_int(query,10, sourceData.sourceID);
	}

	//if the first static sink is entered, we need to set it onto the boundary
	else if(mFirstStaticSource)
	{
		sqlite3_bind_int(query,10, DYNAMIC_ID_BOUNDARY);
		mFirstStaticSource=false;
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sourceID=sqlite3_last_insert_rowid(mDatabase);

	//now we need to create the additional tables:
	command="CREATE TABLE SourceConnectionFormat" + int2string(sourceID) + std::string("(soundFormat INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SourceMainSoundProperty" + int2string(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));
	command="CREATE TABLE SourceSoundProperty" + int2string(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SourceConnectionFormat" + int2string(sourceID) + std::string("(soundFormat) VALUES (?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ConnectionFormat_e>::const_iterator connectionFormatIterator=sourceData.listConnectionFormats.begin();
	for(;connectionFormatIterator<sourceData.listConnectionFormats.end();++connectionFormatIterator)
	{
		sqlite3_bind_int(query,1, *connectionFormatIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill MainSinkSoundProperties
	command="INSERT INTO SourceMainSoundProperty" + int2string(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator=sourceData.listMainSoundProperties.begin();
	for(;mainSoundPropertyIterator<sourceData.listMainSoundProperties.end();++mainSoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, mainSoundPropertyIterator->type);
		sqlite3_bind_int(query,2, mainSoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	//Fill SinkSoundProperties
	command="INSERT INTO SourceSoundProperty" + int2string(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator=sourceData.listSoundProperties.begin();
	for(;SoundPropertyIterator<sourceData.listSoundProperties.end();++SoundPropertyIterator)
	{
		sqlite3_bind_int(query,1, SoundPropertyIterator->type);
		sqlite3_bind_int(query,2, SoundPropertyIterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkDB entered new source with name"), DLT_STRING(sourceData.name.c_str()),
			DLT_STRING("domainID:"),DLT_INT(sourceData.domainID),
			DLT_STRING("classID:"),DLT_INT(sourceData.sourceClassID),
			DLT_STRING("volume:"),DLT_INT(sourceData.volume),
			DLT_STRING("visible:"),DLT_BOOL(sourceData.visible),
			DLT_STRING("available.availability:"),DLT_INT(sourceData.available.availability),
			DLT_STRING("available.availabilityReason:"),DLT_INT(sourceData.available.availabilityReason),
			DLT_STRING("interruptState:"),DLT_INT(sourceData.interruptState),
			DLT_STRING("assigned ID:"),DLT_INT16(sourceID));

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
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB did not find route for MainConnection:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//now we delete the data in the table
	command="DELETE from MainConnectionRoute" + int2string(mainconnectionID);
	assert(this->sqQuery(command));

	command= "INSERT INTO MainConnectionRoute" + int2string(mainconnectionID) + "(connectionID) VALUES (?)";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<uint16_t>::iterator listConnectionIterator=listOfConnections.begin();
	for(;listConnectionIterator<listOfConnections.end();++listConnectionIterator)
	{
		sqlite3_bind_int(query,1, *listConnectionIterator);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainConnectionRouteDB entered new route:"),DLT_INT(mainconnectionID));
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
	command = "UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET connectionState=? WHERE mainConnectionID=" + int2string(mainconnectionID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connectionState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainConnectionStateDB changed mainConnectionState of MainConnection:"),DLT_INT(mainconnectionID),DLT_STRING("to:"),DLT_INT(connectionState));
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
	command = "UPDATE " + std::string(SINK_TABLE) + " SET mainVolume=? WHERE sinkID=" + int2string(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, mainVolume);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkMainVolumeDB changed mainVolume of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(mainVolume));
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
	command = "UPDATE " + std::string(SINK_TABLE) + " SET availability=?, availabilityReason=? WHERE sinkID=" + int2string(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, availability.availability);
	sqlite3_bind_int(query,2, availability.availabilityReason);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkAvailabilityDB changed sinkAvailability of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(availability.availability), DLT_STRING("Reason:"),DLT_INT(availability.availabilityReason));
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
	command = "UPDATE " + std::string(DOMAIN_TABLE) + " SET state=? WHERE domainID=" + int2string(domainID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, domainState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changDomainStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changDomainStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changDomainStateDB changed domainState of domain:"),DLT_INT(domainID),DLT_STRING("to:"),DLT_INT(domainState));
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
	command = "UPDATE " + std::string(SINK_TABLE) + " SET muteState=? WHERE sinkID=" + int2string(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, muteState);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSinkMuteStateDB changed sinkMuteState of sink:"),DLT_INT(sinkID),DLT_STRING("to:"),DLT_INT(muteState));
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
	command = "UPDATE SinkMainSoundProperty" + int2string(sinkID)+ " SET value=? WHERE soundPropertyType=" + int2string(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}	assert(sinkID!=0);

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainSinkSoundPropertyDB changed MainSinkSoundProperty of sink:"),DLT_INT(sinkID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));
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
	command = "UPDATE SourceMainSoundProperty" + int2string(sourceID)+ " SET value=? WHERE soundPropertyType=" + int2string(soundProperty.type);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, soundProperty.value);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeMainSourceSoundPropertyDB changed MainSinkSoundProperty of source:"),DLT_INT(sourceID),DLT_STRING("type:"),DLT_INT(soundProperty.type),DLT_STRING("to:"),DLT_INT(soundProperty.value));
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
	command = "UPDATE " + std::string(SOURCE_TABLE) + " SET availability=?, availabilityReason=? WHERE sourceID=" + int2string(sourceID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, availability.availability);
	sqlite3_bind_int(query,2, availability.availabilityReason);
	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSourceAvailabilityDB changed changeSourceAvailabilityDB of source:"),DLT_INT(sourceID),DLT_STRING("to:"),DLT_INT(availability.availability), DLT_STRING("Reason:"),DLT_INT(availability.availabilityReason));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSystemPropertyDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}


	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeSystemPropertyDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::changeSystemPropertyDB changed system property"));
	return E_OK;
}



am_Error_e DatabaseHandler::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
	assert(mainConnectionID!=0);

	if (!existMainConnection(mainConnectionID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + int2string(mainConnectionID);
	std::string command1 = "DROP table MainConnectionRoute" + int2string(mainConnectionID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeMainConnectionDB removed:"),DLT_INT(mainConnectionID));
	return E_OK;
}



am_Error_e DatabaseHandler::removeSinkDB(const am_sinkID_t sinkID)
{
	assert(sinkID!=0);

	if (!existSink(sinkID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SINK_TABLE) + " WHERE sinkID=" + int2string(sinkID);
	std::string command1 = "DROP table SinkConnectionFormat" + int2string(sinkID);
	std::string command2 = "DROP table SinkMainSoundProperty" + int2string(sinkID);
	std::string command3 = "DROP table SinkSoundProperty" + int2string(sinkID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	if(!sqQuery(command2)) return E_DATABASE_ERROR;
	if(!sqQuery(command3)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSinkDB removed:"),DLT_INT(sinkID));
	return E_OK;
}



am_Error_e DatabaseHandler::removeSourceDB(const am_sourceID_t sourceID)
{
	assert(sourceID!=0);

	if (!existSource(sourceID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + int2string(sourceID);
	std::string command1 = "DROP table SourceConnectionFormat" + int2string(sourceID);
	std::string command2 = "DROP table SourceMainSoundProperty" + int2string(sourceID);
	std::string command3 = "DROP table SourceSoundProperty" + int2string(sourceID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	if(!sqQuery(command2)) return E_DATABASE_ERROR;
	if(!sqQuery(command3)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSourceDB removed:"),DLT_INT(sourceID));
	return E_OK;
}



am_Error_e DatabaseHandler::removeGatewayDB(const am_gatewayID_t gatewayID)
{
	assert(gatewayID!=0);

	if (!existGateway(gatewayID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + int2string(gatewayID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeGatewayDB removed:"),DLT_INT(gatewayID));
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
	std::string command = "DELETE from " + std::string(DOMAIN_TABLE) + " WHERE domainID=" + int2string(domainID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeDomainDB removed:"),DLT_INT(domainID));

	return E_OK;
}

am_Error_e DatabaseHandler::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
	assert(sinkClassID!=0);

	if (!existSinkClass(sinkClassID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + int2string(sinkClassID);
	std::string command1 = "DROP table SinkClassProperties" + int2string(sinkClassID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSinkClassDB removed:"),DLT_INT(sinkClassID));
	return E_OK;
}

am_Error_e DatabaseHandler::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
	assert(sourceClassID!=0);

	if (!existSourceClass(sourceClassID))
	{
		return E_NON_EXISTENT;
	}
	std::string command = "DELETE from " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + int2string(sourceClassID);
	std::string command1 = "DROP table SourceClassProperties" + int2string(sourceClassID);
	if(!sqQuery(command)) return E_DATABASE_ERROR;
	if(!sqQuery(command1)) return E_DATABASE_ERROR;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::removeSourceClassDB removed:"),DLT_INT(sourceClassID));
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
	std::string command= "SELECT sourceClassID FROM " + std::string(SOURCE_TABLE)+ " WHERE sourceID=" + (int2string(sourceID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classInfo.sourceClassID=sqlite3_column_int(query,0);
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	command= "SELECT name FROM " + std::string(SOURCE_CLASS_TABLE)+ " WHERE sourceClassID=" + (int2string(classInfo.sourceClassID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		classInfo.name=std::string((const char*)sqlite3_column_text(query,0));
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//read out Properties
	command= "SELECT classProperty, value FROM SourceClassProperties"+ int2string(classInfo.sourceClassID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(query,0);
		propertyTemp.value=sqlite3_column_int(query,1);
		classInfo.listClassProperties.push_back(propertyTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSourceClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command="UPDATE SinkClassProperties" + int2string(sinkClass.sinkClassID) + " set value=? WHERE classProperty=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sinkClass.listClassProperties.begin();
	for(;Iterator<sinkClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->value);
		sqlite3_bind_int(query,2, Iterator->classProperty);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo"));
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
	std::string command="UPDATE SourceClassProperties" + int2string(sourceClass.sourceClassID) + " set value=? WHERE classProperty=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sourceClass.listClassProperties.begin();
	for(;Iterator<sourceClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->value);
		sqlite3_bind_int(query,2, Iterator->classProperty);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::setSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo"));
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
	std::string command= "SELECT sinkClassID FROM " + std::string(SINK_TABLE)+ " WHERE sinkID=" + (int2string(sinkID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkClass.sinkClassID=sqlite3_column_int(query,0);
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	command= "SELECT name FROM " + std::string(SINK_CLASS_TABLE)+ " WHERE sinkClassID=" + (int2string(sinkClass.sinkClassID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	if((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		sinkClass.name=std::string((const char*)sqlite3_column_text(query,0));
	}

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//read out Properties
	command= "SELECT classProperty, value FROM SinkClassProperties"+ int2string(sinkClass.sinkClassID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(query,0);
		propertyTemp.value=sqlite3_column_int(query,1);
		sinkClass.listClassProperties.push_back(propertyTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getSinkClassInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID="+int2string(gatewayID);
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
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB database error with convertionFormat"));
			return E_DATABASE_ERROR;
		}
		gatewayData.convertionMatrix=iter->second;

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM GatewaySourceFormat" + int2string(gatewayData.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSourceConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSourceConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSourceConnectionFormat,0);
			gatewayData.listSourceFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSourceConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		commandConnectionFormat= "SELECT soundFormat FROM GatewaySinkFormat" + int2string(gatewayData.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSinkConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSinkConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSinkConnectionFormat,0);
			gatewayData.listSinkFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSinkConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getGatewayInfoDB SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT sinkID FROM " + std::string(SINK_TABLE)+ " WHERE domainID=" + (int2string(domainID));
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listSinkID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinksOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinksOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE domainID=" + int2string(domainID);

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listSourceID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourcesOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourcesOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
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

	std::string command= "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE controlDomainID=" +int2string(domainID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp=sqlite3_column_int(query,0);
		listGatewaysID.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGatewaysOfDomain SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGatewaysOfDomain SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	return E_OK;
}



am_Error_e DatabaseHandler::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
	listMainConnections.clear();
	sqlite3_stmt *query=NULL, *query1=NULL, *query2=NULL;
	int eCode=0, eCode1=0, eCode2=0;
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
		std::string statement=command1 + int2string(temp.connectionID);
		sqlite3_prepare_v2(mDatabase,statement.c_str(),-1,&query1,NULL);
		while((eCode1=sqlite3_step(query1))==SQLITE_ROW) //todo: check results of eCode1, eCode2
		{
			int k=sqlite3_column_int(query1,0);
			sqlite3_bind_int(query2,1,k);
			while((eCode2=sqlite3_step(query2))==SQLITE_ROW)
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainConnections SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT domainID, name, busname, nodename, early, complete, state FROM " + std::string(DOMAIN_TABLE);
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListDomains SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListDomains SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT connectionID, sourceID, sinkID, delay, connectionFormat FROM " + std::string(CONNECTION_TABLE);
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListConnections SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, sinkID FROM " + std::string(SINK_TABLE);
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
		std::string commandConnectionFormat= "SELECT soundFormat FROM SinkConnectionFormat"+ int2string(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qConnectionFormat,NULL);
		while((eCode=sqlite3_step(qConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qConnectionFormat,0);
			temp.listConnectionFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		std::string commandSoundProperty= "SELECT soundPropertyType, value FROM SinkSoundProperty" + int2string(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandSoundProperty.c_str(),-1,&qSoundProperty,NULL);
		while((eCode=sqlite3_step(qSoundProperty))==SQLITE_ROW)
		{
			tempSoundProperty.type=(am_SoundPropertyType_e)sqlite3_column_int(qSoundProperty,0);
			tempSoundProperty.value=sqlite3_column_int(qSoundProperty,1);
			temp.listSoundProperties.push_back(tempSoundProperty);
		}

		if((eCode=sqlite3_finalize(qSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out MainSoundProperties
		std::string commandMainSoundProperty= "SELECT soundPropertyType, value FROM SinkMainSoundProperty"+ int2string(temp.sinkID);
		sqlite3_prepare_v2(mDatabase,commandMainSoundProperty.c_str(),-1,&qMAinSoundProperty,NULL);
		while((eCode=sqlite3_step(qMAinSoundProperty))==SQLITE_ROW)
		{
			tempMainSoundProperty.type=(am_MainSoundPropertyType_e)sqlite3_column_int(qMAinSoundProperty,0);
			tempMainSoundProperty.value=sqlite3_column_int(qMAinSoundProperty,1);
			temp.listMainSoundProperties.push_back(tempMainSoundProperty);
		}

		if((eCode=sqlite3_finalize(qMAinSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSinks.push_back(temp);
		temp.listConnectionFormats.clear();
		temp.listMainSoundProperties.clear();
		temp.listSoundProperties.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, sourceID FROM " + std::string(SOURCE_TABLE);
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
		std::string commandConnectionFormat= "SELECT soundFormat FROM SourceConnectionFormat"+ int2string(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qConnectionFormat,NULL);
		while((eCode=sqlite3_step(qConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qConnectionFormat,0);
			temp.listConnectionFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		std::string commandSoundProperty= "SELECT soundPropertyType, value FROM SourceSoundProperty" + int2string(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandSoundProperty.c_str(),-1,&qSoundProperty,NULL);
		while((eCode=sqlite3_step(qSoundProperty))==SQLITE_ROW)
		{
			tempSoundProperty.type=(am_SoundPropertyType_e)sqlite3_column_int(qSoundProperty,0);
			tempSoundProperty.value=sqlite3_column_int(qSoundProperty,1);
			temp.listSoundProperties.push_back(tempSoundProperty);
		}

		if((eCode=sqlite3_finalize(qSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out MainSoundProperties
		std::string commandMainSoundProperty= "SELECT soundPropertyType, value FROM SourceMainSoundProperty"+ int2string(temp.sourceID);
		sqlite3_prepare_v2(mDatabase,commandMainSoundProperty.c_str(),-1,&qMAinSoundProperty,NULL);
		while((eCode=sqlite3_step(qMAinSoundProperty))==SQLITE_ROW)
		{
			tempMainSoundProperty.type=(am_MainSoundPropertyType_e)sqlite3_column_int(qMAinSoundProperty,0);
			tempMainSoundProperty.value=sqlite3_column_int(qMAinSoundProperty,1);
			temp.listMainSoundProperties.push_back(tempMainSoundProperty);
		}

		if((eCode=sqlite3_finalize(qMAinSoundProperty))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSources.push_back(temp);
		temp.listConnectionFormats.clear();
		temp.listMainSoundProperties.clear();
		temp.listSoundProperties.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
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
		command2="SELECT classProperty, value FROM SourceClassProperties"+ int2string(classTemp.sourceClassID);
		sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&subQuery,NULL);

		while((eCode1=sqlite3_step(subQuery))==SQLITE_ROW)
		{
			propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(subQuery,0);
			propertyTemp.value=sqlite3_column_int(subQuery,1);
			classTemp.listClassProperties.push_back(propertyTemp);
		}

		if(eCode1!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}

		if((eCode1=sqlite3_finalize(subQuery))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode1));
			return E_DATABASE_ERROR;
		}
		listSourceClasses.push_back(classTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
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
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways database error with convertionFormat"));
			return E_DATABASE_ERROR;
	    }
	   temp.convertionMatrix=iter->second;

		//read out the connectionFormats
		std::string commandConnectionFormat= "SELECT soundFormat FROM GatewaySourceFormat" + int2string(temp.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSourceConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSourceConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSourceConnectionFormat,0);
			temp.listSourceFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSourceConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		//read out sound properties
		commandConnectionFormat= "SELECT soundFormat FROM GatewaySinkFormat" + int2string(temp.gatewayID);
		sqlite3_prepare_v2(mDatabase,commandConnectionFormat.c_str(),-1,&qSinkConnectionFormat,NULL);
		while((eCode=sqlite3_step(qSinkConnectionFormat))==SQLITE_ROW)
		{
			tempConnectionFormat=(am_ConnectionFormat_e)sqlite3_column_int(qSinkConnectionFormat,0);
			temp.listSinkFormats.push_back(tempConnectionFormat);
		}

		if((eCode=sqlite3_finalize(qSinkConnectionFormat))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		listGateways.push_back(temp);
		temp.listSinkFormats.clear();
		temp.listSourceFormats.clear();
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListGateways SQLITE Finalize error code:"),DLT_INT(eCode));
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
		command2="SELECT classProperty, value FROM SinkClassProperties"+ int2string(classTemp.sinkClassID);
		sqlite3_prepare_v2(mDatabase,command2.c_str(),-1,&subQuery,NULL);

		while((eCode=sqlite3_step(subQuery))==SQLITE_ROW)
		{
			propertyTemp.classProperty=(am_ClassProperty_e)sqlite3_column_int(subQuery,0);
			propertyTemp.value=sqlite3_column_int(subQuery,1);
			classTemp.listClassProperties.push_back(propertyTemp);
		}

		if(eCode!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		if((eCode=sqlite3_finalize(subQuery))!=SQLITE_OK)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		listSinkClasses.push_back(classTemp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSourceClasses SQLITE Finalize error code:"),DLT_INT(eCode));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListVisibleMainConnections SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListVisibleMainConnections SQLITE Finalize error code:"),DLT_INT(eCode));
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

	std::string command= "SELECT name, sinkID, availability, availabilityReason, muteState, mainVolume, sinkClassID FROM " + std::string(SINK_TABLE) + " WHERE visible=1";
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSinks SQLITE Finalize error code:"),DLT_INT(eCode));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSources SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT soundPropertyType, value FROM SinkMainSoundProperty" + int2string(sinkID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.type=(am_MainSoundPropertyType_e)sqlite3_column_int(query,0);
		temp.value=sqlite3_column_int(query,1);
		listSoundProperties.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE Finalize error code:"),DLT_INT(eCode));
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
	std::string command= "SELECT soundPropertyType, value FROM SourceMainSoundProperty" + int2string(sourceID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		temp.type=(am_MainSoundPropertyType_e)sqlite3_column_int(query,0);
		temp.value=sqlite3_column_int(query,1);
		listSourceProperties.push_back(temp);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListMainSinkSoundProperties SQLITE Finalize error code:"),DLT_INT(eCode));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSystemProperties SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getListSystemProperties SQLITE Finalize error code:"),DLT_INT(eCode));
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

	std::string command= "SELECT delay FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + int2string(mainConnectionID);
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);

	while((eCode=sqlite3_step(query))==SQLITE_ROW)
	{
		delay=sqlite3_column_int(query,0);
	}

	if(eCode!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getTimingInformation SQLITE error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getTimingInformation SQLITE Finalize error code:"),DLT_INT(eCode));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::sqQuery SQL Query failed:"), DLT_STRING(query.c_str()), DLT_STRING("error code:"),DLT_INT(eCode));
		return false;
	}
	return true;
}

bool DatabaseHandler::openDatabase()
{
	if (sqlite3_open_v2(mPath.c_str(),&mDatabase,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::openDatabase opened database"));
		return true;
	}
	DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::openDatabase failed to open database"));
	return false;
}

am_Error_e DatabaseHandler::changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID)
{
	assert(connectionID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command="UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET delay=? WHERE mainConnectionID=?;";
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, delay);
	sqlite3_bind_int(query,2, connectionID);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeDelayMainConnection SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::changeDelayMainConnection SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}
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
	std::string command= "INSERT INTO " + std::string(CONNECTION_TABLE) + "(sinkID, sourceID, delay, connectionFormat) VALUES (?,?,?,?)";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, connection.sinkID);
	sqlite3_bind_int(query,2, connection.sourceID);
	sqlite3_bind_int(query,3, connection.delay);
	sqlite3_bind_int(query,4, connection.connectionFormat);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	connectionID=sqlite3_last_insert_rowid(mDatabase);

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterConnectionDB entered new connection sourceID:"), DLT_INT16(connection.sourceID),
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sinkClassID=sqlite3_last_insert_rowid(mDatabase); //todo:change last_insert implementations for mulithread usage...

	//now we need to create the additional tables:
	command="CREATE TABLE SinkClassProperties" + int2string(sinkClassID) + std::string("(classProperty INTEGER, value INTEGER)");
	assert(this->sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SinkClassProperties" + int2string(sinkClassID) + std::string("(classProperty,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sinkClass.listClassProperties.begin();
	for(;Iterator<sinkClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->classProperty);
		sqlite3_bind_int(query,2, Iterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSinkClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSinkClassDB entered new sinkClass"));
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	sourceClassID=sqlite3_last_insert_rowid(mDatabase); //todo:change last_insert implementations for mulithread usage...

	//now we need to create the additional tables:
	command="CREATE TABLE SourceClassProperties" + int2string(sourceClassID) + std::string("(classProperty INTEGER, value INTEGER)");
	assert(sqQuery(command));

	//fill ConnectionFormats
	command="INSERT INTO SourceClassProperties" + int2string(sourceClassID) + std::string("(classProperty,value) VALUES (?,?)");
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	std::vector<am_ClassProperty_s>::const_iterator Iterator=sourceClass.listClassProperties.begin();
	for(;Iterator<sourceClass.listClassProperties.end();++Iterator)
	{
		sqlite3_bind_int(query,1, Iterator->classProperty);
		sqlite3_bind_int(query,2, Iterator->value);
		if((eCode=sqlite3_step(query))!=SQLITE_DONE)
		{
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}
		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSourceClassDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSourceClassDB entered new sourceClass"));
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
			DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSystemProperties SQLITE Step error code:"),DLT_INT(eCode));
			return E_DATABASE_ERROR;
		}

		sqlite3_reset(query);
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterSystemProperties SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("DatabaseHandler::enterSystemProperties entered system properties"));
	return E_OK;
}

bool DatabaseHandler::existMainConnection(const am_mainConnectionID_t mainConnectionID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT mainConnectionID FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + int2string(mainConnectionID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existMainConnection database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSource(const am_sourceID_t sourceID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + int2string(sourceID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSink(const am_sinkID_t sinkID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE sinkID=" + int2string(sinkID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSink database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existDomain(const am_domainID_t domainID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE domainID=" + int2string(domainID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existDomain database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existGateway(const am_gatewayID_t gatewayID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + int2string(gatewayID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existGateway database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

am_Error_e DatabaseHandler::getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t & domainID) const
{
	assert(sourceID!=0);

	sqlite3_stmt* query=NULL;
	std::string command = "SELECT domainID FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + int2string(sourceID);
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
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::getDomainOfSource database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}


bool DatabaseHandler::existSinkClass(const am_sinkClass_t sinkClassID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sinkClassID FROM " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + int2string(sinkClassID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSinkClass database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

bool DatabaseHandler::existSourceClass(const am_sourceClass_t sourceClassID) const
{
	sqlite3_stmt* query=NULL;
	std::string command = "SELECT sourceClassID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + int2string(sourceClassID);
	int eCode=0;
	bool returnVal=true;
	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	if ((eCode=sqlite3_step(query))==SQLITE_DONE) returnVal=false;
	else if (eCode!=SQLITE_ROW)
	{
		returnVal=false;
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::existSinkClass database error!:"), DLT_INT(eCode))
	}
	sqlite3_finalize(query);
	return returnVal;
}

am_Error_e DatabaseHandler::changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
	assert(connectionID!=0);

	sqlite3_stmt* query=NULL;
	int eCode=0;
	std::string command= "UPDATE " + std::string(CONNECTION_TABLE) + " set delay=? WHERE connectionID=?";

	sqlite3_prepare_v2(mDatabase,command.c_str(),-1,&query,NULL);
	sqlite3_bind_int(query,1, delay);
	sqlite3_bind_int(query,2, connectionID);

	if((eCode=sqlite3_step(query))!=SQLITE_DONE)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Step error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	if((eCode=sqlite3_finalize(query))!=SQLITE_OK)
	{
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("DatabaseHandler::enterConnectionDB SQLITE Finalize error code:"),DLT_INT(eCode));
		return E_DATABASE_ERROR;
	}

	//todo: add DLT Info Message here
	return E_OK;
}

void DatabaseHandler::createTables()
{
	for(uint16_t i=0;i<sizeof(databaseTables)/sizeof(databaseTables[0]);i++)
	{
		assert(sqQuery("CREATE TABLE " + databaseTables[i]));
	}
}






