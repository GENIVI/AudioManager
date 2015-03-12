/**
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \file CAmDatabaseHandlerSQLite.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmDatabaseHandlerSQLite.h"
#include <cassert>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "CAmDatabaseObserver.h"
#include "CAmRouter.h"
#include "CAmDltWrapper.h"
#include <sqlite3.h>

namespace am
{

/**
 * Macro to handle SQLITE errors on prepare
 */
#define MY_SQLITE_PREPARE_V2(db,zSql,nByte,ppStmt,pzTail)                                                               \
        if ((eCode = sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail)))                                              \
        {                                                                                                               \
            logError("CAmDatabaseHandler::my_sqlite_prepare_v2 on Command",zSql,"failed with errorCode:", eCode);       \
            return (E_DATABASE_ERROR);                                                                                  \
        }

#define MY_SQLITE_PREPARE_V2_BOOL(db,zSql,nByte,ppStmt,pzTail)                                                          \
        if ((eCode = sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail)))                                              \
        {                                                                                                               \
            logError("CAmDatabaseHandler::my_sqlite_prepare_v2_bool on Command",zSql,"failed with errorCode:", eCode);  \
            return (false);                                                                                             \
        }

/**
 * Macro to handle SQLITE errors bind text
 */
#define MY_SQLITE_BIND_TEXT(query,index,text,size,static_)                                                              \
        if ((eCode = sqlite3_bind_text(query, index, text, size, static_)))                                             \
        {                                                                                                               \
            logError("CAmDatabaseHandler::sqlite3_bind_text failed with errorCode:", eCode);                            \
            return (E_DATABASE_ERROR);                                                                                  \
        }

/**
 * Macro to handle SQLITE errors on bind int
 */
#define MY_SQLITE_BIND_INT(query, index, data)                                                                          \
        if((eCode = sqlite3_bind_int(query, index, data)))                                                              \
        {                                                                                                               \
            logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);                             \
            return (E_DATABASE_ERROR);                                                                                  \
        }

/**
 * Macro to handle SQLITE errors on reset
 */
#define MY_SQLITE_RESET(query)                                                                                          \
        if((eCode = sqlite3_reset(query)))                                                                              \
        {                                                                                                               \
            logError("CAmDatabaseHandler::sqlite3_reset failed with errorCode:", eCode);                                \
            return (E_DATABASE_ERROR);                                                                                  \
        }

/**
 * Macro to handle SQLITE finalize
 */
#define MY_SQLITE_FINALIZE(query)                                                                                       \
        if((eCode = sqlite3_finalize(query)))                                                                           \
        {                                                                                                               \
            logError("CAmDatabaseHandler::sqlite3_finalize failed with errorCode:", eCode);                             \
            return (E_DATABASE_ERROR);                                                                                  \
        }

#define MY_SQLITE_FINALIZE_BOOL(query)                                                                                  \
        if((eCode = sqlite3_finalize(query)))                                                                           \
        {                                                                                                               \
            logError("CAmDatabaseHandler::sqlite3_finalize failed with errorCode:", eCode);                             \
            return (true);                                                                                              \
        }

#define DOMAIN_TABLE "Domains"  //!< domain table
#define SOURCE_CLASS_TABLE "SourceClasses" //!< source class table
#define SINK_CLASS_TABLE "SinkClasses" //!< sink class table
#define SOURCE_TABLE "Sources" //!< source table
#define SINK_TABLE "Sinks" //!< sink table
#define GATEWAY_TABLE "Gateways" //!< gateway table
#define CONVERTER_TABLE "Converters" //!< converter table
#define CROSSFADER_TABLE "Crossfaders" //!< crossfader table
#define CONNECTION_TABLE "Connections" //!< connection table
#define MAINCONNECTION_TABLE "MainConnections" //!< main connection table
#define SYSTEM_TABLE "SystemProperties" //!< system properties table
/**
 * table that holds table informations
 */
const std::string databaseTables[] =
{ " Domains (domainID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), busname VARCHAR(50), nodename VARCHAR(50), early BOOL, complete BOOL, state INTEGER, reserved BOOL);", //
        " SourceClasses (sourceClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));", //
        " SinkClasses (sinkClassID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50));", //
        " Sources (sourceID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, domainID INTEGER, name VARCHAR(50), sourceClassID INTEGER, sourceState INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, interruptState INTEGER, reserved BOOL);", //
        " Sinks (sinkID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), domainID INTEGER, sinkClassID INTEGER, volume INTEGER, visible BOOL, availability INTEGER, availabilityReason INTEGER, muteState INTEGER, mainVolume INTEGER, reserved BOOL);", //
        " Gateways (gatewayID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID INTEGER, sourceID INTEGER, domainSinkID INTEGER, domainSourceID INTEGER, controlDomainID INTEGER);", //
        " Converters (converterID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID INTEGER, sourceID INTEGER, domainID INTEGER);", //
        " Crossfaders (crossfaderID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR(50), sinkID_A INTEGER, sinkID_B INTEGER, sourceID INTEGER, hotSink INTEGER);", //
        " Connections (connectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, delay INTEGER, connectionFormat INTEGER, reserved BOOL);", //
        " MainConnections (mainConnectionID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sourceID INTEGER, sinkID INTEGER, connectionState INTEGER, delay INTEGER);", //
        " SystemProperties (type INTEGER PRIMARY KEY, value INTEGER);" };

/**
 * template to converts T to std::string
 * @param x T
 * @return string
 */
template<typename T>
inline std::string i2s(T const& x)
{
    std::ostringstream o;
    o << x;
    return (o.str());
}

CAmDatabaseHandlerSQLite::CAmDatabaseHandlerSQLite():mpDatabaseObserver(NULL), //
													mFirstStaticSink(true), //
													mFirstStaticSource(true), //
													mFirstStaticGateway(true), //
													mFirstStaticConverter(true),
													mFirstStaticSinkClass(true), //
													mFirstStaticSourceClass(true), //
													mFirstStaticCrossfader(true), //
													mListConnectionFormat(),
													mpDatabase(NULL), //
													mPath(std::string(""))
{

}

CAmDatabaseHandlerSQLite::CAmDatabaseHandlerSQLite(std::string databasePath):mpDatabaseObserver(NULL), //
																			mFirstStaticSink(true), //
																			mFirstStaticSource(true), //
																			mFirstStaticGateway(true), //
																			mFirstStaticConverter(true),
																			mFirstStaticSinkClass(true), //
																			mFirstStaticSourceClass(true), //
																			mFirstStaticCrossfader(true), //
																			mListConnectionFormat(),
																			mpDatabase(NULL), //
																			mPath(databasePath)
{
    std::ifstream infile(mPath.c_str());

    if (infile)
    {
        if(remove(mPath.c_str())==0)
        {
        	logError("DatabaseHandler::DatabaseHandler Knocked down database failed !");
        }
        logInfo("DatabaseHandler::DatabaseHandler Knocked down database");
    }

    bool dbOpen = openDatabase();
    if (!dbOpen)
    {
        logInfo("DatabaseHandler::DatabaseHandler problems opening the database!");
    }

    createTables();
}

CAmDatabaseHandlerSQLite::~CAmDatabaseHandlerSQLite()
{
    logInfo("Closed Database");
    mpDatabaseObserver = NULL;
    if(mpDatabase)
    	sqlite3_close(mpDatabase);
}

am_Error_e CAmDatabaseHandlerSQLite::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    assert(domainData.domainID==0);
    assert(!domainData.name.empty());
    assert(!domainData.busname.empty());
    assert(domainData.state>=DS_UNKNOWN && domainData.state<=DS_MAX);

    //first check for a reserved domain
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    domainID=0;
    std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, domainData.name.c_str(), domainData.name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        domainID = sqlite3_column_int(query, 0);
        command = "UPDATE " + std::string(DOMAIN_TABLE) + " SET name=?, busname=?, nodename=?, early=?, complete=?, state=?, reserved=? WHERE domainID=" + i2s(sqlite3_column_int(query, 0));
    }
    else if (eCode == SQLITE_DONE)
    {

        command = "INSERT INTO " + std::string(DOMAIN_TABLE) + " (name, busname, nodename, early, complete, state, reserved) VALUES (?,?,?,?,?,?,?)";
    }
    else
    {
        logError("DatabaseHandler::enterDomainDB SQLITE Step error code:", eCode);
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, domainData.name.c_str(), domainData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_TEXT(query, 2, domainData.busname.c_str(), domainData.busname.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_TEXT(query, 3, domainData.nodename.c_str(), domainData.nodename.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 4, domainData.early)
    MY_SQLITE_BIND_INT(query, 5, domainData.complete)
    MY_SQLITE_BIND_INT(query, 6, domainData.state)
    MY_SQLITE_BIND_INT(query, 7, 0)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterDomainDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    if (domainID==0)
        domainID = sqlite3_last_insert_rowid(mpDatabase);
    logInfo("DatabaseHandler::enterDomainDB entered new domain with name=", domainData.name, "busname=", domainData.busname, "nodename=", domainData.nodename, "assigned ID:", domainID);

    am_Domain_s domain = domainData;
    domain.domainID = domainID;
    if (mpDatabaseObserver)
        mpDatabaseObserver->newDomain(domain);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    assert(mainConnectionData.mainConnectionID==0);
    assert(mainConnectionData.connectionState>=CS_UNKNOWN && mainConnectionData.connectionState<=CS_MAX);
    assert(mainConnectionData.sinkID!=0);
    assert(mainConnectionData.sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    int16_t delay = 0;
    std::string command = "INSERT INTO " + std::string(MAINCONNECTION_TABLE) + "(sourceID, sinkID, connectionState, delay) VALUES (?,?,?,-1)";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, mainConnectionData.sourceID)
    MY_SQLITE_BIND_INT(query, 2, mainConnectionData.sinkID)
    MY_SQLITE_BIND_INT(query, 3, mainConnectionData.connectionState)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    connectionID = sqlite3_last_insert_rowid(mpDatabase);

    //now check the connectionTable for all connections in the route. IF connectionID exist
    command = "SELECT delay FROM " + std::string(CONNECTION_TABLE) + (" WHERE connectionID=?");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_connectionID_t>::const_iterator elementIterator = mainConnectionData.listConnectionID.begin();
    for (; elementIterator < mainConnectionData.listConnectionID.end(); ++elementIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *elementIterator)

        if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
        {
            int16_t temp_delay = sqlite3_column_int(query, 1);
            if (temp_delay != -1 && delay != -1)
                delay += temp_delay;
            else
                delay = -1;
        }
        else
        {
            logError("DatabaseHandler::enterMainConnectionDB did not find route for MainConnection: ", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    //now we create a table with references to the connections;
    command = "CREATE TABLE MainConnectionRoute" + i2s(connectionID) + std::string("(connectionID INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    command = "INSERT INTO MainConnectionRoute" + i2s(connectionID) + "(connectionID) VALUES (?)";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_connectionID_t>::const_iterator listConnectionIterator(mainConnectionData.listConnectionID.begin());
    for (; listConnectionIterator < mainConnectionData.listConnectionID.end(); ++listConnectionIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *listConnectionIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterMainConnectionDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::enterMainConnectionDB entered new mainConnection with sourceID", mainConnectionData.sourceID, "sinkID:", mainConnectionData.sinkID, "delay:", delay, "assigned ID:", connectionID);

    if (mpDatabaseObserver)
    {
        am_MainConnectionType_s mainConnection;
        mainConnection.mainConnectionID = connectionID;
        mainConnection.connectionState = mainConnectionData.connectionState;
        mainConnection.delay = delay;
        mainConnection.sinkID = mainConnectionData.sinkID;
        mainConnection.sourceID = mainConnectionData.sourceID;
        mpDatabaseObserver->newMainConnection(mainConnection);
        mpDatabaseObserver->mainConnectionStateChanged(connectionID, mainConnectionData.connectionState);
    }

    //finally, we update the delay value for the maintable
    if (delay == 0)
        delay = -1;
    return (changeDelayMainConnection(delay, connectionID));
}

am_Error_e CAmDatabaseHandlerSQLite::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    assert(sinkData.sinkID<DYNAMIC_ID_BOUNDARY);
    assert(sinkData.domainID!=0);
    assert(!sinkData.name.empty());
    assert(sinkData.sinkClassID!=0);
    //todo: need to check if class exists?
    assert(!sinkData.listConnectionFormats.empty());
    assert(sinkData.muteState>=MS_UNKNOWN && sinkData.muteState<=MS_MAX);

    sqlite3_stmt *query = NULL;
    int eCode = 0;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE name=? AND reserved=1";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sinkData.name.c_str(), sinkData.name.size(), SQLITE_STATIC)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        command = "UPDATE " + std::string(SINK_TABLE) + " SET name=?, domainID=?, sinkClassID=?, volume=?, visible=?, availability=?, availabilityReason=?, muteState=?, mainVolume=?, reserved=? WHERE sinkID=" + i2s(sqlite3_column_int(query, 0));
    }
    else if (eCode == SQLITE_DONE)
    {
        //if sinkID is zero and the first Static Sink was already entered, the ID is created
        if (sinkData.sinkID == 0 && !mFirstStaticSink && !existSinkName(sinkData.name))
        {
            command = "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, reserved) VALUES (?,?,?,?,?,?,?,?,?,?)";
        }
        else
        {
            //check if the ID already exists
            if (existSinkNameOrID(sinkData.sinkID, sinkData.name))
            {
                MY_SQLITE_FINALIZE(query)
                return (E_ALREADY_EXISTS);
            }
            command = "INSERT INTO " + std::string(SINK_TABLE) + "(name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, reserved, sinkID) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
        }
    }
    else
    {
        logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sinkData.name.c_str(), sinkData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 2, sinkData.domainID)
    MY_SQLITE_BIND_INT(query, 3, sinkData.sinkClassID)
    MY_SQLITE_BIND_INT(query, 4, sinkData.volume)
    MY_SQLITE_BIND_INT(query, 5, sinkData.visible)
    MY_SQLITE_BIND_INT(query, 6, sinkData.available.availability)
    MY_SQLITE_BIND_INT(query, 7, sinkData.available.availabilityReason)
    MY_SQLITE_BIND_INT(query, 8, sinkData.muteState)
    MY_SQLITE_BIND_INT(query, 9, sinkData.mainVolume)
    MY_SQLITE_BIND_INT(query, 10, 0)

    //if the ID is not created, we add it to the query
    if (sinkData.sinkID != 0)
    {
        MY_SQLITE_BIND_INT(query, 11, sinkData.sinkID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticSink)
    {
        MY_SQLITE_BIND_INT(query, 11, DYNAMIC_ID_BOUNDARY)
        mFirstStaticSink = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //now read back the sinkID
    command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sinkData.name.c_str(), sinkData.name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkID = sqlite3_column_int(query, 0);
    }
    else
    {
        sinkID = 0;
        logError("DatabaseHandler::existSink database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    //now we need to create the additional tables:
    command = "CREATE TABLE SinkConnectionFormat" + i2s(sinkID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE SinkSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE SinkNotificationConfiguration" + i2s(sinkID) + std::string("(type INTEGER, status INTEGER, parameter INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO SinkConnectionFormat" + i2s(sinkID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = sinkData.listConnectionFormats.begin();
    for (; connectionFormatIterator < sinkData.listConnectionFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    //Fill SinkSoundProperties
    command = "INSERT INTO SinkSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator = sinkData.listSoundProperties.begin();
    for (; SoundPropertyIterator < sinkData.listSoundProperties.end(); ++SoundPropertyIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, SoundPropertyIterator->type)
        MY_SQLITE_BIND_INT(query, 2, SoundPropertyIterator->value)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    //Fill NotificationConfigurations
    command = "INSERT INTO SinkNotificationConfiguration" + i2s(sinkID) + std::string("(type,status,parameter) VALUES (?,?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_NotificationConfiguration_s>::const_iterator NotificationConfigurationIterator(sinkData.listNotificationConfigurations.begin());
    for (; NotificationConfigurationIterator < sinkData.listNotificationConfigurations.end(); ++NotificationConfigurationIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, NotificationConfigurationIterator->type)
        MY_SQLITE_BIND_INT(query, 2, NotificationConfigurationIterator->status)
        MY_SQLITE_BIND_INT(query, 3, NotificationConfigurationIterator->parameter)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    if (sinkData.visible == true)
    {
        command = "CREATE TABLE SinkMainSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
        if (!this->sqQuery(command))
            return (E_DATABASE_ERROR);

        //Fill MainSinkSoundProperties
        command = "INSERT INTO SinkMainSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator = sinkData.listMainSoundProperties.begin();
        for (; mainSoundPropertyIterator < sinkData.listMainSoundProperties.end(); ++mainSoundPropertyIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, mainSoundPropertyIterator->type)
            MY_SQLITE_BIND_INT(query, 2, mainSoundPropertyIterator->value)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)

        //now we got MainNotificationConfigurations as well
        command = "CREATE TABLE SinkMainNotificationConfiguration" + i2s(sinkID) + std::string("(type INTEGER, status INTEGER, parameter INTEGER)");
        if (!this->sqQuery(command))
            return (E_DATABASE_ERROR);

        command = "INSERT INTO SinkMainNotificationConfiguration" + i2s(sinkID) + std::string("(type,status,parameter) VALUES (?,?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_NotificationConfiguration_s>::const_iterator mainNotificationConfigurationIterator(sinkData.listMainNotificationConfigurations.begin());
        for (; mainNotificationConfigurationIterator < sinkData.listMainNotificationConfigurations.end(); ++mainNotificationConfigurationIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, mainNotificationConfigurationIterator->type)
            MY_SQLITE_BIND_INT(query, 2, mainNotificationConfigurationIterator->status)
            MY_SQLITE_BIND_INT(query, 3, mainNotificationConfigurationIterator->parameter)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)
    }

    logInfo("DatabaseHandler::enterSinkDB entered new sink with name", sinkData.name, "domainID:", sinkData.domainID, "classID:", sinkData.sinkClassID, "volume:", sinkData.volume, "assigned ID:", sinkID);
    am_Sink_s sink = sinkData;
    sink.sinkID = sinkID;
    if (mpDatabaseObserver != NULL)
        mpDatabaseObserver->newSink(sink);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    assert(crossfaderData.crossfaderID<DYNAMIC_ID_BOUNDARY);
    assert(crossfaderData.hotSink>=HS_UNKNOWN && crossfaderData.hotSink<=HS_MAX);
    assert(!crossfaderData.name.empty());
    assert(existSink(crossfaderData.sinkID_A));
    assert(existSink(crossfaderData.sinkID_B));
    assert(existSource(crossfaderData.sourceID));

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    if (crossfaderData.crossfaderID == 0 && !mFirstStaticCrossfader)
    {
        command = "INSERT INTO " + std::string(CROSSFADER_TABLE) + "(name, sinkID_A, sinkID_B, sourceID, hotSink) VALUES (?,?,?,?,?)";
    }
    else
    {
        //check if the ID already exists
        if (existCrossFader(crossfaderData.crossfaderID))
            return (E_ALREADY_EXISTS);
        command = "INSERT INTO " + std::string(CROSSFADER_TABLE) + "(name, sinkID_A, sinkID_B, sourceID, hotSink, crossfaderID) VALUES (?,?,?,?,?,?)";
    }

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    MY_SQLITE_BIND_TEXT(query, 1, crossfaderData.name.c_str(), crossfaderData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 2, crossfaderData.sinkID_A)
    MY_SQLITE_BIND_INT(query, 3, crossfaderData.sinkID_B)
    MY_SQLITE_BIND_INT(query, 4, crossfaderData.sourceID)
    MY_SQLITE_BIND_INT(query, 5, crossfaderData.hotSink)

    //if the ID is not created, we add it to the query
    if (crossfaderData.crossfaderID != 0)
    {
        MY_SQLITE_BIND_INT(query, 6, crossfaderData.crossfaderID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticCrossfader)
    {
        MY_SQLITE_BIND_INT(query, 6, DYNAMIC_ID_BOUNDARY)
        mFirstStaticCrossfader = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterCrossfaderDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //now read back the crossfaderID
    command = "SELECT crossfaderID FROM " + std::string(CROSSFADER_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, crossfaderData.name.c_str(), crossfaderData.name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        crossfaderID = sqlite3_column_int(query, 0);
    }
    else
    {
        crossfaderID = 0;
        logError("DatabaseHandler::enterCrossfaderDB database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterCrossfaderDB entered new crossfader with name=", crossfaderData.name, "sinkA= ", crossfaderData.sinkID_A, "sinkB=", crossfaderData.sinkID_B, "source=", crossfaderData.sourceID, "assigned ID:", crossfaderID);

    am_Crossfader_s crossfader(crossfaderData);
    crossfader.crossfaderID = crossfaderID;
    if (mpDatabaseObserver)
        mpDatabaseObserver->newCrossfader(crossfader);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
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

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    if (gatewayData.gatewayID == 0 && !mFirstStaticGateway)
    {
        command = "INSERT INTO " + std::string(GATEWAY_TABLE) + "(name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID) VALUES (?,?,?,?,?,?)";
    }
    else
    {
        //check if the ID already exists
        if (existGateway(gatewayData.gatewayID))
            return (E_ALREADY_EXISTS);
        command = "INSERT INTO " + std::string(GATEWAY_TABLE) + "(name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID) VALUES (?,?,?,?,?,?,?)";
    }

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, gatewayData.name.c_str(), gatewayData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 2, gatewayData.sinkID)
    MY_SQLITE_BIND_INT(query, 3, gatewayData.sourceID)
    MY_SQLITE_BIND_INT(query, 4, gatewayData.domainSinkID)
    MY_SQLITE_BIND_INT(query, 5, gatewayData.domainSourceID)
    MY_SQLITE_BIND_INT(query, 6, gatewayData.controlDomainID)

    //if the ID is not created, we add it to the query
    if (gatewayData.gatewayID != 0)
    {
        MY_SQLITE_BIND_INT(query, 7, gatewayData.gatewayID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticGateway)
    {
        MY_SQLITE_BIND_INT(query, 7, DYNAMIC_ID_BOUNDARY)
        mFirstStaticGateway = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterGatewayDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    gatewayID = sqlite3_last_insert_rowid(mpDatabase);

    //now the convertion matrix todo: change the map implementation sometimes to blob in sqlite
    mListConnectionFormat.insert(std::make_pair(gatewayID, gatewayData.convertionMatrix));

    command = "CREATE TABLE GatewaySourceFormat" + i2s(gatewayID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE GatewaySinkFormat" + i2s(gatewayID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO GatewaySourceFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = gatewayData.listSourceFormats.begin();
    for (; connectionFormatIterator < gatewayData.listSourceFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterGatewayDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    command = "INSERT INTO GatewaySinkFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    connectionFormatIterator = gatewayData.listSinkFormats.begin();
    for (; connectionFormatIterator < gatewayData.listSinkFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterGatewayDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterGatewayDB entered new gateway with name", gatewayData.name, "sourceID:", gatewayData.sourceID, "sinkID:", gatewayData.sinkID, "assigned ID:", gatewayID);
    am_Gateway_s gateway = gatewayData;
    gateway.gatewayID = gatewayID;
    if (mpDatabaseObserver)
        mpDatabaseObserver->newGateway(gateway);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterConverterDB(const am_Converter_s & converterData, am_converterID_t & converterID)
{
    assert(converterData.converterID<DYNAMIC_ID_BOUNDARY);
    assert(converterData.sinkID!=0);
    assert(converterData.sourceID!=0);
    assert(converterData.domainID!=0);
    assert(!converterData.name.empty());
    assert(!converterData.convertionMatrix.empty());
    assert(!converterData.listSinkFormats.empty());
    assert(!converterData.listSourceFormats.empty());

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    if (converterData.converterID == 0 && !mFirstStaticConverter)
    {
        command = "INSERT INTO " + std::string(CONVERTER_TABLE) + "(name, sinkID, sourceID, domainID) VALUES (?,?,?,?)";
    }
    else
    {
        //check if the ID already exists
        if (existConverter(converterData.converterID))
            return (E_ALREADY_EXISTS);
        command = "INSERT INTO " + std::string(CONVERTER_TABLE) + "(name, sinkID, sourceID, domainID,  converterID) VALUES (?,?,?,?,?)";
    }

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, converterData.name.c_str(), converterData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 2, converterData.sinkID)
    MY_SQLITE_BIND_INT(query, 3, converterData.sourceID)
    MY_SQLITE_BIND_INT(query, 4, converterData.domainID)

    //if the ID is not created, we add it to the query
    if (converterData.converterID != 0)
    {
        MY_SQLITE_BIND_INT(query, 5, converterData.converterID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticConverter)
    {
        MY_SQLITE_BIND_INT(query, 5, DYNAMIC_ID_BOUNDARY)
		mFirstStaticConverter = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterConverterDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    converterID = sqlite3_last_insert_rowid(mpDatabase);

    //now the convertion matrix todo: change the map implementation sometimes to blob in sqlite
    mListConnectionFormat.insert(std::make_pair(converterID, converterData.convertionMatrix));

    command = "CREATE TABLE ConverterSourceFormat" + i2s(converterID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE ConverterSinkFormat" + i2s(converterID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO ConverterSourceFormat" + i2s(converterID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = converterData.listSourceFormats.begin();
    for (; connectionFormatIterator < converterData.listSourceFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterConverterDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    command = "INSERT INTO ConverterSinkFormat" + i2s(converterID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    connectionFormatIterator = converterData.listSinkFormats.begin();
    for (; connectionFormatIterator < converterData.listSinkFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterConverterDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterConverterDB entered new gateway with name", converterData.name, "sourceID:", converterData.sourceID, "sinkID:", converterData.sinkID, "assigned ID:", converterID);
    am_Converter_s converter = converterData;
    converter.converterID = converterID;
    if (mpDatabaseObserver)
        mpDatabaseObserver->newConverter(converter);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    assert(sourceData.sourceID<DYNAMIC_ID_BOUNDARY);
    assert(sourceData.domainID!=0);
    assert(!sourceData.name.empty());
    assert(sourceData.sourceClassID!=0);
    // \todo: need to check if class exists?
    assert(!sourceData.listConnectionFormats.empty());
    assert(sourceData.sourceState>=SS_UNKNNOWN && sourceData.sourceState<=SS_MAX);

    sqlite3_stmt* query = NULL;
    ;
    int eCode = 0;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE name=? AND reserved=1";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sourceData.name.c_str(), sourceData.name.size(), SQLITE_STATIC)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        command = "UPDATE " + std::string(SOURCE_TABLE) + " SET name=?, domainID=?, sourceClassID=?, sourceState=?, volume=?, visible=?, availability=?, availabilityReason=?, interruptState=?, reserved=? WHERE sourceID=" + i2s(sqlite3_column_int(query, 0));
    }
    else if (eCode == SQLITE_DONE)
    {
        //if sinkID is zero and the first Static Sink was already entered, the ID is created
        if (sourceData.sourceID == 0 && !mFirstStaticSource && !existSourceName(sourceData.name))
        {
            command = "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, reserved) VALUES (?,?,?,?,?,?,?,?,?,?)";
        }
        else
        {
            //check if the ID already exists
            if (existSourceNameOrID(sourceData.sourceID, sourceData.name))
            {
                MY_SQLITE_FINALIZE(query)
                return (E_ALREADY_EXISTS);
            }
            command = "INSERT INTO " + std::string(SOURCE_TABLE) + "(name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, reserved, sourceID) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
        }
    }
    else
    {
        logError("DatabaseHandler::enterSourceDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sourceData.name.c_str(), sourceData.name.size(), SQLITE_STATIC)
    MY_SQLITE_BIND_INT(query, 2, sourceData.domainID)
    MY_SQLITE_BIND_INT(query, 3, sourceData.sourceClassID)
    MY_SQLITE_BIND_INT(query, 4, sourceData.sourceState)
    MY_SQLITE_BIND_INT(query, 5, sourceData.volume)
    MY_SQLITE_BIND_INT(query, 6, sourceData.visible)
    MY_SQLITE_BIND_INT(query, 7, sourceData.available.availability)
    MY_SQLITE_BIND_INT(query, 8, sourceData.available.availabilityReason)
    MY_SQLITE_BIND_INT(query, 9, sourceData.interruptState)
    MY_SQLITE_BIND_INT(query, 10, 0)

    //if the ID is not created, we add it to the query
    if (sourceData.sourceID != 0)
    {
        MY_SQLITE_BIND_INT(query, 11, sourceData.sourceID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticSource)
    {
        MY_SQLITE_BIND_INT(query, 11, DYNAMIC_ID_BOUNDARY)
        mFirstStaticSource = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterSourceDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //now read back the sinkID
    command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sourceData.name.c_str(), sourceData.name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sourceID = sqlite3_column_int(query, 0);
    }
    else
    {
        sourceID = 0;
        logError("DatabaseHandler::existSink database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    //now we need to create the additional tables:
    command = "CREATE TABLE SourceConnectionFormat" + i2s(sourceID) + std::string("(soundFormat INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE SourceSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);
    command = "CREATE TABLE SourceNotificationConfiguration" + i2s(sourceID) + std::string("(type INTEGER, status INTEGER, parameter INTEGER)");
        if (!this->sqQuery(command))
            return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO SourceConnectionFormat" + i2s(sourceID) + std::string("(soundFormat) VALUES (?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = sourceData.listConnectionFormats.begin();
    for (; connectionFormatIterator != sourceData.listConnectionFormats.end(); ++connectionFormatIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSourceDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }
    MY_SQLITE_FINALIZE(query)

    //Fill SinkSoundProperties
    command = "INSERT INTO SourceSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator = sourceData.listSoundProperties.begin();
    for (; SoundPropertyIterator != sourceData.listSoundProperties.end(); ++SoundPropertyIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, SoundPropertyIterator->type)
        MY_SQLITE_BIND_INT(query, 2, SoundPropertyIterator->value)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    //Fill NotificationConfigurations
    command = "INSERT INTO SourceNotificationConfiguration" + i2s(sourceID) + std::string("(type,status,parameter) VALUES (?,?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_NotificationConfiguration_s>::const_iterator NotificationConfigurationIterator(sourceData.listNotificationConfigurations.begin());
    for (; NotificationConfigurationIterator < sourceData.listNotificationConfigurations.end(); ++NotificationConfigurationIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, NotificationConfigurationIterator->type)
        MY_SQLITE_BIND_INT(query, 2, NotificationConfigurationIterator->status)
        MY_SQLITE_BIND_INT(query, 3, NotificationConfigurationIterator->parameter)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    if (sourceData.visible == true)
    {
        command = "CREATE TABLE SourceMainSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType INTEGER, value INTEGER)");
        if (!this->sqQuery(command))
            return (E_DATABASE_ERROR);

        //Fill MainSinkSoundProperties
        command = "INSERT INTO SourceMainSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator = sourceData.listMainSoundProperties.begin();
        for (; mainSoundPropertyIterator != sourceData.listMainSoundProperties.end(); ++mainSoundPropertyIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, mainSoundPropertyIterator->type)
            MY_SQLITE_BIND_INT(query, 2, mainSoundPropertyIterator->value)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::enterSourceDB SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)

        //now we got MainNotificationConfigurations as well
        command = "CREATE TABLE SourceMainNotificationConfiguration" + i2s(sourceID) + std::string("(type INTEGER, status INTEGER, parameter INTEGER)");
        if (!this->sqQuery(command))
            return (E_DATABASE_ERROR);

        command = "INSERT INTO SourceMainNotificationConfiguration" + i2s(sourceID) + std::string("(type,status,parameter) VALUES (?,?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_NotificationConfiguration_s>::const_iterator mainNotificationConfigurationIterator(sourceData.listMainNotificationConfigurations.begin());
        for (; mainNotificationConfigurationIterator != sourceData.listMainNotificationConfigurations.end(); ++mainNotificationConfigurationIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, mainNotificationConfigurationIterator->type)
            MY_SQLITE_BIND_INT(query, 2, mainNotificationConfigurationIterator->status)
            MY_SQLITE_BIND_INT(query, 3, mainNotificationConfigurationIterator->parameter)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::enterSinkDB SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)
    }

    logInfo("DatabaseHandler::enterSourceDB entered new source with name", sourceData.name, "domainID:", sourceData.domainID, "classID:", sourceData.sourceClassID, "visible:", sourceData.visible, "assigned ID:", sourceID);

    am_Source_s source = sourceData;
    source.sourceID = sourceID;
    if (mpDatabaseObserver)
        mpDatabaseObserver->newSource(source);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID)
{
    assert(mainconnectionID!=0);
    if (!existMainConnection(mainconnectionID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    int16_t delay = 0;
    command = "SELECT delay FROM " + std::string(CONNECTION_TABLE) + (" WHERE connectionID=?");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_connectionID_t>::const_iterator elementIterator = listConnectionID.begin();
    for (; elementIterator < listConnectionID.end(); ++elementIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *elementIterator)

        if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
        {
            int16_t temp_delay = sqlite3_column_int(query, 1);
            if (temp_delay != -1 && delay != -1)
                delay += temp_delay;
            else
                delay = -1;
        }
        else
        {
            logError("DatabaseHandler::changeMainConnectionRouteDB did not find route for MainConnection: ", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    //now we delete the data in the table
    command = "DELETE from MainConnectionRoute" + i2s(mainconnectionID);
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    command = "INSERT INTO MainConnectionRoute" + i2s(mainconnectionID) + "(connectionID) VALUES (?)";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_connectionID_t>::const_iterator listConnectionIterator(listConnectionID.begin());
    for (; listConnectionIterator != listConnectionID.end(); ++listConnectionIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, *listConnectionIterator)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::changeMainConnectionRouteDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    if (changeDelayMainConnection(delay,mainconnectionID)!=E_OK)
        logError("DatabaseHandler::changeMainConnectionRouteDB error while changing mainConnectionDelay to ", delay);

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeMainConnectionRouteDB entered new route:", mainconnectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
    assert(mainconnectionID!=0);
    assert(connectionState>=CS_UNKNOWN && connectionState<=CS_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existMainConnection(mainconnectionID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET connectionState=? WHERE mainConnectionID=" + i2s(mainconnectionID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, connectionState)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainConnectionStateDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeMainConnectionStateDB changed mainConnectionState of MainConnection:", mainconnectionID, "to:", connectionState);

    if (mpDatabaseObserver)
        mpDatabaseObserver->mainConnectionStateChanged(mainconnectionID, connectionState);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SINK_TABLE) + " SET mainVolume=? WHERE sinkID=" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, mainVolume)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSinkMainVolumeDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSinkMainVolumeDB changed mainVolume of sink:", sinkID, "to:", mainVolume);

    if (mpDatabaseObserver)
        mpDatabaseObserver->volumeChanged(sinkID, mainVolume);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);
    assert(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SINK_TABLE) + " SET availability=?, availabilityReason=? WHERE sinkID=" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, availability.availability)
    MY_SQLITE_BIND_INT(query, 2, availability.availabilityReason)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSinkAvailabilityDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    assert(sinkID!=0);
    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSinkAvailabilityDB changed sinkAvailability of sink:", sinkID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (mpDatabaseObserver && sourceVisible(sinkID))
        mpDatabaseObserver->sinkAvailabilityChanged(sinkID, availability);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
    assert(domainID!=0);
    assert(domainState>=DS_UNKNOWN && domainState<=DS_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(DOMAIN_TABLE) + " SET state=? WHERE domainID=" + i2s(domainID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, domainState)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changDomainStateDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changDomainStateDB changed domainState of domain:", domainID, "to:", domainState);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);
    assert(muteState>=MS_UNKNOWN && muteState<=MS_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SINK_TABLE) + " SET muteState=? WHERE sinkID=" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, muteState)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSinkMuteStateDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    assert(sinkID!=0);
    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSinkMuteStateDB changed sinkMuteState of sink:", sinkID, "to:", muteState);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sinkMuteStateChanged(sinkID, muteState);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SinkMainSoundProperty" + i2s(sinkID) + " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, soundProperty.value)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSinkSoundPropertyDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    assert(sinkID!=0);
    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeMainSinkSoundPropertyDB changed MainSinkSoundProperty of sink:", sinkID, "type:", soundProperty.type, "to:", soundProperty.value);
    if (mpDatabaseObserver)
        mpDatabaseObserver->mainSinkSoundPropertyChanged(sinkID, soundProperty);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SourceMainSoundProperty" + i2s(sourceID) + " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, soundProperty.value)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSourceSoundPropertyDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeMainSourceSoundPropertyDB changed MainSinkSoundProperty of source:", sourceID, "type:", soundProperty.type, "to:", soundProperty.value);

    if (mpDatabaseObserver)
        mpDatabaseObserver->mainSourceSoundPropertyChanged(sourceID, soundProperty);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);
    assert(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SOURCE_TABLE) + " SET availability=?, availabilityReason=? WHERE sourceID=" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, availability.availability)
    MY_SQLITE_BIND_INT(query, 2, availability.availabilityReason)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSourceAvailabilityDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeSourceAvailabilityDB changed changeSourceAvailabilityDB of source:", sourceID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (mpDatabaseObserver && sourceVisible(sourceID))
        mpDatabaseObserver->sourceAvailabilityChanged(sourceID, availability);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "UPDATE " + std::string(SYSTEM_TABLE) + " set value=? WHERE type=?";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, property.value)
    MY_SQLITE_BIND_INT(query, 2, property.type)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSystemPropertyDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeSystemPropertyDB changed system property");

    if (mpDatabaseObserver)
        mpDatabaseObserver->systemPropertyChanged(property);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
    assert(mainConnectionID!=0);

    if (!existMainConnection(mainConnectionID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
    std::string command1 = "DROP table MainConnectionRoute" + i2s(mainConnectionID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command1))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeMainConnectionDB removed:", mainConnectionID);
    if (mpDatabaseObserver)
    {
        mpDatabaseObserver->mainConnectionStateChanged(mainConnectionID, CS_DISCONNECTED);
        mpDatabaseObserver->removedMainConnection(mainConnectionID);
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeSinkDB(const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    bool visible = sinkVisible(sinkID);

    std::string command = "DELETE from " + std::string(SINK_TABLE) + " WHERE sinkID=" + i2s(sinkID);
    std::string command1 = "DROP table SinkConnectionFormat" + i2s(sinkID);
    std::string command2 = "DROP table SinkSoundProperty" + i2s(sinkID);
    std::string command3 = "DROP table SinkMainSoundProperty" + i2s(sinkID);
    std::string command4 = "DROP table SinkNotificationConfiguration" + i2s(sinkID);
    std::string command5 = "DROP table SinkMainNotificationConfiguration" + i2s(sinkID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command1))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command2))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command4))
        return (E_DATABASE_ERROR);
    if (visible) //only drop table if it ever existed
    {
        if (!sqQuery(command3))
            return (E_DATABASE_ERROR);
        if (!sqQuery(command5))
            return (E_DATABASE_ERROR);
    }
    logInfo("DatabaseHandler::removeSinkDB removed:", sinkID);

    if (mpDatabaseObserver != NULL)
        mpDatabaseObserver->removedSink(sinkID, visible);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeSourceDB(const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    bool visible = sourceVisible(sourceID);

    std::string command = "DELETE from " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
    std::string command1 = "DROP table SourceConnectionFormat" + i2s(sourceID);
    std::string command2 = "DROP table SourceMainSoundProperty" + i2s(sourceID);
    std::string command3 = "DROP table SourceSoundProperty" + i2s(sourceID);
    std::string command4 = "DROP table SourceNotificationConfiguration" + i2s(sourceID);
    std::string command5 = "DROP table SourceMainNotificationConfiguration" + i2s(sourceID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command1))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command3))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command4))
        return (E_DATABASE_ERROR);

    if(visible)
    {
        if (!sqQuery(command2))
            return (E_DATABASE_ERROR);
        if (!sqQuery(command5))
            return (E_DATABASE_ERROR);
    }
    logInfo("DatabaseHandler::removeSourceDB removed:", sourceID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removedSource(sourceID, visible);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeGatewayDB(const am_gatewayID_t gatewayID)
{
    assert(gatewayID!=0);

    if (!existGateway(gatewayID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + i2s(gatewayID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeGatewayDB removed:", gatewayID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeGateway(gatewayID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeConverterDB(const am_converterID_t converterID)
{
    assert(converterID!=0);

    if (!existConverter(converterID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(CONVERTER_TABLE) + " WHERE converterID=" + i2s(converterID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeConverterDB removed:", converterID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeConverter(converterID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{
    assert(crossfaderID!=0);

    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(CROSSFADER_TABLE) + " WHERE crossfaderID=" + i2s(crossfaderID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeCrossfaderDB removed:", crossfaderID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeCrossfader(crossfaderID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeDomainDB(const am_domainID_t domainID)
{
    assert(domainID!=0);

    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(DOMAIN_TABLE) + " WHERE domainID=" + i2s(domainID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeDomainDB removed:", domainID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeDomain(domainID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
    assert(sinkClassID!=0);

    if (!existSinkClass(sinkClassID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + i2s(sinkClassID);
    std::string command1 = "DROP table SinkClassProperties" + i2s(sinkClassID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command1))
        return (E_DATABASE_ERROR);

    logInfo("DatabaseHandler::removeSinkClassDB removed:", sinkClassID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSinkClassesChanged();

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
    assert(sourceClassID!=0);

    if (!existSourceClass(sourceClassID))
    {
        return (E_NON_EXISTENT);
    }
    std::string command = "DELETE from " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + i2s(sourceClassID);
    std::string command1 = "DROP table SourceClassProperties" + i2s(sourceClassID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    if (!sqQuery(command1))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeSourceClassDB removed:", sourceClassID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSourceClassesChanged();
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::removeConnection(const am_connectionID_t connectionID)
{
    assert(connectionID!=0);

    std::string command = "DELETE from " + std::string(CONNECTION_TABLE) + " WHERE connectionID=" + i2s(connectionID);
    if (!sqQuery(command))
        return (E_DATABASE_ERROR);
    logInfo("DatabaseHandler::removeConnection removed:", connectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_ClassProperty_s propertyTemp;
    std::string command = "SELECT sourceClassID FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + (i2s(sourceID));
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        classInfo.sourceClassID = sqlite3_column_int(query, 0);
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSourceClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    command = "SELECT name FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + (i2s(classInfo.sourceClassID));
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        classInfo.name = std::string((const char*) sqlite3_column_text(query, 0));
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSourceClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //read out Properties
    command = "SELECT classProperty, value FROM SourceClassProperties" + i2s(classInfo.sourceClassID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        propertyTemp.classProperty = (am_CustomClassProperty_t) sqlite3_column_int(query, 0);
        propertyTemp.value = sqlite3_column_int(query, 1);
        classInfo.listClassProperties.push_back(propertyTemp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSourceClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s & sinkData) const
{

    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    sqlite3_stmt* query = NULL, *qConnectionFormat = NULL, *qSoundProperty = NULL, *qMAinSoundProperty = NULL, *qMainNotification = NULL, *qNotification = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;
    am_SoundProperty_s tempSoundProperty;
    am_MainSoundProperty_s tempMainSoundProperty;
    am_NotificationConfiguration_s tempNotificationConfiguration,tempMainNotification;
    std::string command = "SELECT name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 and sinkID=" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkData.name = std::string((const char*) sqlite3_column_text(query, 0));
        sinkData.domainID = sqlite3_column_int(query, 1);
        sinkData.sinkClassID = sqlite3_column_int(query, 2);
        sinkData.volume = sqlite3_column_int(query, 3);
        sinkData.visible = sqlite3_column_int(query, 4);
        sinkData.available.availability = (am_Availability_e) sqlite3_column_int(query, 5);
        sinkData.available.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 6);
        sinkData.muteState = (am_MuteState_e) sqlite3_column_int(query, 7);
        sinkData.mainVolume = sqlite3_column_int(query, 8);
        sinkData.sinkID = sqlite3_column_int(query, 9);

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM SinkConnectionFormat" + i2s(sinkID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
            sinkData.listConnectionFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qConnectionFormat)

        //read out sound properties
        std::string commandSoundProperty = "SELECT soundPropertyType, value FROM SinkSoundProperty" + i2s(sinkID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandSoundProperty.c_str(), -1, &qSoundProperty, NULL)
        while ((eCode = sqlite3_step(qSoundProperty)) == SQLITE_ROW)
        {
            tempSoundProperty.type = (am_CustomSoundPropertyType_t) sqlite3_column_int(qSoundProperty, 0);
            tempSoundProperty.value = sqlite3_column_int(qSoundProperty, 1);
            sinkData.listSoundProperties.push_back(tempSoundProperty);
        }

        MY_SQLITE_FINALIZE(qSoundProperty)

        std::string notificationCommand = "SELECT type, status, parameter FROM SinkNotificationConfiguration" + i2s(sinkID);
        MY_SQLITE_PREPARE_V2(mpDatabase, notificationCommand.c_str(), -1, &qNotification, NULL)

        while ((eCode = sqlite3_step(qNotification)) == SQLITE_ROW)
        {
            tempNotificationConfiguration.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qNotification, 0));
            tempNotificationConfiguration.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qNotification, 1));
            tempNotificationConfiguration.parameter= static_cast<int16_t>(sqlite3_column_int(qNotification, 2));
            sinkData.listNotificationConfigurations.push_back(tempNotificationConfiguration);
        }
        MY_SQLITE_FINALIZE(qNotification)

        if (sinkData.visible)
        {

            //read out MainSoundProperties
            std::string commandMainSoundProperty = "SELECT soundPropertyType, value FROM SinkMainSoundProperty" + i2s(sinkID);
            MY_SQLITE_PREPARE_V2(mpDatabase, commandMainSoundProperty.c_str(), -1, &qMAinSoundProperty, NULL)
            while ((eCode = sqlite3_step(qMAinSoundProperty)) == SQLITE_ROW)
            {
                tempMainSoundProperty.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(qMAinSoundProperty, 0);
                tempMainSoundProperty.value = sqlite3_column_int(qMAinSoundProperty, 1);
                sinkData.listMainSoundProperties.push_back(tempMainSoundProperty);
            }

            MY_SQLITE_FINALIZE(qMAinSoundProperty)

            std::string mainNotificationCommand = "SELECT type, status, parameter FROM SinkMainNotificationConfiguration" + i2s(sinkID);
            MY_SQLITE_PREPARE_V2(mpDatabase, mainNotificationCommand.c_str(), -1, &qMainNotification, NULL)

            while ((eCode = sqlite3_step(qMainNotification)) == SQLITE_ROW)
            {
                tempMainNotification.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qMainNotification, 0));
                tempMainNotification.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qMainNotification, 1));
                tempMainNotification.parameter= static_cast<int16_t>(sqlite3_column_int(qMainNotification, 2));
                sinkData.listMainNotificationConfigurations.push_back(tempMainNotification);
            }
            MY_SQLITE_FINALIZE(qMainNotification)
        }
    }

    else if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s & sourceData) const
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    sqlite3_stmt* query = NULL, *qConnectionFormat = NULL, *qSoundProperty = NULL, *qMAinSoundProperty = NULL, *qMainNotification = NULL, *qNotification = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;
    am_SoundProperty_s tempSoundProperty;
    am_MainSoundProperty_s tempMainSoundProperty;
    am_NotificationConfiguration_s tempNotificationConfiguration,tempMainNotification;
    std::string command = "SELECT name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND sourceID=" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sourceData.name = std::string((const char*) sqlite3_column_text(query, 0));
        sourceData.domainID = sqlite3_column_int(query, 1);
        sourceData.sourceClassID = sqlite3_column_int(query, 2);
        sourceData.sourceState = (am_SourceState_e) sqlite3_column_int(query, 3);
        sourceData.volume = sqlite3_column_int(query, 4);
        sourceData.visible = sqlite3_column_int(query, 5);
        sourceData.available.availability = (am_Availability_e) sqlite3_column_int(query, 6);
        sourceData.available.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 7);
        sourceData.interruptState = (am_InterruptState_e) sqlite3_column_int(query, 8);
        sourceData.sourceID = sqlite3_column_int(query, 9);

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM SourceConnectionFormat" + i2s(sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
            sourceData.listConnectionFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qConnectionFormat)

        //read out sound properties
        std::string commandSoundProperty = "SELECT soundPropertyType, value FROM SourceSoundProperty" + i2s(sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandSoundProperty.c_str(), -1, &qSoundProperty, NULL);
        while ((eCode = sqlite3_step(qSoundProperty)) == SQLITE_ROW)
        {
            tempSoundProperty.type = (am_CustomSoundPropertyType_t) sqlite3_column_int(qSoundProperty, 0);
            tempSoundProperty.value = sqlite3_column_int(qSoundProperty, 1);
            sourceData.listSoundProperties.push_back(tempSoundProperty);
        }

        MY_SQLITE_FINALIZE(qSoundProperty)

        std::string notificationCommand = "SELECT type, status, parameter FROM SourceNotificationConfiguration" + i2s(sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, notificationCommand.c_str(), -1, &qNotification, NULL)

        while ((eCode = sqlite3_step(qNotification)) == SQLITE_ROW)
        {
            tempNotificationConfiguration.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qNotification, 0));
            tempNotificationConfiguration.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qNotification, 1));
            tempNotificationConfiguration.parameter= static_cast<int16_t>(sqlite3_column_int(qNotification, 2));
            sourceData.listNotificationConfigurations.push_back(tempNotificationConfiguration);
        }
        MY_SQLITE_FINALIZE(qNotification)

        if (sourceData.visible)
        {

            //read out MainSoundProperties
            std::string commandMainSoundProperty = "SELECT soundPropertyType, value FROM SourceMainSoundProperty" + i2s(sourceID);
            MY_SQLITE_PREPARE_V2(mpDatabase, commandMainSoundProperty.c_str(), -1, &qMAinSoundProperty, NULL)
            while ((eCode = sqlite3_step(qMAinSoundProperty)) == SQLITE_ROW)
            {
                tempMainSoundProperty.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(qMAinSoundProperty, 0);
                tempMainSoundProperty.value = sqlite3_column_int(qMAinSoundProperty, 1);
                sourceData.listMainSoundProperties.push_back(tempMainSoundProperty);
            }

            MY_SQLITE_FINALIZE(qMAinSoundProperty)

            std::string mainNotificationCommand = "SELECT type, status, parameter FROM SourceMainNotificationConfiguration" + i2s(sourceID);
            MY_SQLITE_PREPARE_V2(mpDatabase, mainNotificationCommand.c_str(), -1, &qMainNotification, NULL)

            while ((eCode = sqlite3_step(qMainNotification)) == SQLITE_ROW)
            {
                tempMainNotification.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qMainNotification, 0));
                tempMainNotification.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qMainNotification, 1));
                tempMainNotification.parameter= static_cast<int16_t>(sqlite3_column_int(qMainNotification, 2));
                sourceData.listMainNotificationConfigurations.push_back(tempMainNotification);
            }
            MY_SQLITE_FINALIZE(qMainNotification)

        }
    }
    else if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSourceInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s & mainConnectionData) const
{
    assert(mainConnectionID!=0);
    if (!existMainConnection(mainConnectionID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt *query = NULL, *query1 = NULL;
    int eCode = 0;
    am_MainConnection_s temp;
    std::string command = "SELECT mainConnectionID, sourceID, sinkID, connectionState, delay FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
    std::string command1 = "SELECT connectionID FROM MainConnectionRoute";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        mainConnectionData.mainConnectionID = sqlite3_column_int(query, 0);
        mainConnectionData.sourceID = sqlite3_column_int(query, 1);
        mainConnectionData.sinkID = sqlite3_column_int(query, 2);
        mainConnectionData.connectionState = (am_ConnectionState_e) sqlite3_column_int(query, 3);
        mainConnectionData.delay = sqlite3_column_int(query, 4);
        std::string statement = command1 + i2s(mainConnectionID);
        MY_SQLITE_PREPARE_V2(mpDatabase, statement.c_str(), -1, &query1, NULL)
        while ((eCode = sqlite3_step(query1)) == SQLITE_ROW)
        {
            mainConnectionData.listConnectionID.push_back(sqlite3_column_int(query1, 0));
        }
        MY_SQLITE_FINALIZE(query1)
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getMainConnectionInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkClassInfoDB(const am_SinkClass_s& sinkClass)
{
    assert(sinkClass.sinkClassID!=0);
    assert(!sinkClass.listClassProperties.empty());

    sqlite3_stmt* query = NULL;
    int eCode = 0;

    //check if the ID already exists
    if (!existSinkClass(sinkClass.sinkClassID))
        return (E_NON_EXISTENT);

    //fill ConnectionFormats
    std::string command = "UPDATE SinkClassProperties" + i2s(sinkClass.sinkClassID) + " set value=? WHERE classProperty=?;";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_ClassProperty_s>::const_iterator Iterator = sinkClass.listClassProperties.begin();
    for (; Iterator < sinkClass.listClassProperties.end(); ++Iterator)
    {
        MY_SQLITE_BIND_INT(query, 1, Iterator->value)
        MY_SQLITE_BIND_INT(query, 2, Iterator->classProperty)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::changeMainSourceNotificationConfigurationDB SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_FINALIZE(query)
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceClassInfoDB(const am_SourceClass_s& sourceClass)
{
    assert(sourceClass.sourceClassID!=0);
    assert(!sourceClass.listClassProperties.empty());

    sqlite3_stmt* query = NULL;
    int eCode = 0;

    //check if the ID already exists
    if (!existSourceClass(sourceClass.sourceClassID))
        return (E_NON_EXISTENT);

    //fill ConnectionFormats
    std::string command = "UPDATE SourceClassProperties" + i2s(sourceClass.sourceClassID) + " set value=? WHERE classProperty=?;";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_ClassProperty_s>::const_iterator Iterator = sourceClass.listClassProperties.begin();
    for (; Iterator < sourceClass.listClassProperties.end(); ++Iterator)
    {
        MY_SQLITE_BIND_INT(query, 1, Iterator->value)
        MY_SQLITE_BIND_INT(query, 2, Iterator->classProperty)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::setSinkClassInfoDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_ClassProperty_s propertyTemp;
    std::string command = "SELECT sinkClassID FROM " + std::string(SINK_TABLE) + " WHERE sinkID=" + (i2s(sinkID));
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkClass.sinkClassID = sqlite3_column_int(query, 0);
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    command = "SELECT name FROM " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + (i2s(sinkClass.sinkClassID));
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkClass.name = std::string((const char*) sqlite3_column_text(query, 0));
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //read out Properties
    command = "SELECT classProperty, value FROM SinkClassProperties" + i2s(sinkClass.sinkClassID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        propertyTemp.classProperty = (am_CustomClassProperty_t) sqlite3_column_int(query, 0);
        propertyTemp.value = sqlite3_column_int(query, 1);
        sinkClass.listClassProperties.push_back(propertyTemp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkClassInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
    assert(gatewayID!=0);
    if (!existGateway(gatewayID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL, *qSinkConnectionFormat = NULL, *qSourceConnectionFormat = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;
    std::string command = "SELECT name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + i2s(gatewayID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        gatewayData.name = std::string((const char*) sqlite3_column_text(query, 0));
        gatewayData.sinkID = sqlite3_column_int(query, 1);
        gatewayData.sourceID = sqlite3_column_int(query, 2);
        gatewayData.domainSinkID = sqlite3_column_int(query, 3);
        gatewayData.domainSourceID = sqlite3_column_int(query, 4);
        gatewayData.controlDomainID = sqlite3_column_int(query, 5);
        gatewayData.gatewayID = sqlite3_column_int(query, 6);

        //convertionMatrix:
        ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
        iter = mListConnectionFormat.find(gatewayData.gatewayID);
        if (iter == mListConnectionFormat.end())
        {
            logError("DatabaseHandler::getGatewayInfoDB database error with convertionFormat");
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        gatewayData.convertionMatrix = iter->second;

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM GatewaySourceFormat" + i2s(gatewayData.gatewayID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSourceConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSourceConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSourceConnectionFormat, 0);
            gatewayData.listSourceFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSourceConnectionFormat)

        //read out sound properties
        commandConnectionFormat = "SELECT soundFormat FROM GatewaySinkFormat" + i2s(gatewayData.gatewayID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSinkConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSinkConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSinkConnectionFormat, 0);
            gatewayData.listSinkFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSinkConnectionFormat)

    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getGatewayInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::getConverterInfoDB(const am_converterID_t converterID, am_Converter_s& converterData) const
{
    assert(converterID!=0);
    if (!existConverter(converterID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL, *qSinkConnectionFormat = NULL, *qSourceConnectionFormat = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;
    std::string command = "SELECT name, sinkID, sourceID, domainID, converterID FROM " + std::string(CONVERTER_TABLE) + " WHERE converterID=" + i2s(converterID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
    	converterData.name = std::string((const char*) sqlite3_column_text(query, 0));
    	converterData.sinkID = sqlite3_column_int(query, 1);
    	converterData.sourceID = sqlite3_column_int(query, 2);
    	converterData.domainID = sqlite3_column_int(query, 3);
    	converterData.converterID = sqlite3_column_int(query, 4);

        //convertionMatrix:
        ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
        iter = mListConnectionFormat.find(converterData.converterID);
        if (iter == mListConnectionFormat.end())
        {
            logError("DatabaseHandler::getConverterInfoDB database error with convertionFormat");
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        converterData.convertionMatrix = iter->second;

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM ConverterSourceFormat" + i2s(converterData.converterID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSourceConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSourceConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSourceConnectionFormat, 0);
            converterData.listSourceFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSourceConnectionFormat)

        //read out sound properties
        commandConnectionFormat = "SELECT soundFormat FROM ConverterSinkFormat" + i2s(converterData.converterID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSinkConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSinkConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSinkConnectionFormat, 0);
            converterData.listSinkFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSinkConnectionFormat)

    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getConverterInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
    assert(crossfaderID!=0);
    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT name, sinkID_A, sinkID_B, sourceID, hotSink,crossfaderID FROM " + std::string(CROSSFADER_TABLE) + " WHERE crossfaderID=" + i2s(crossfaderID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        crossfaderData.name = std::string((const char*) sqlite3_column_text(query, 0));
        crossfaderData.sinkID_A = sqlite3_column_int(query, 1);
        crossfaderData.sinkID_B = sqlite3_column_int(query, 2);
        crossfaderData.sourceID = sqlite3_column_int(query, 3);
        crossfaderData.hotSink = static_cast<am_HotSink_e>(sqlite3_column_int(query, 4));
        crossfaderData.crossfaderID = sqlite3_column_int(query, 5);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getCrossfaderInfoDB SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
    assert(domainID!=0);
    listSinkID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_sinkID_t temp;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND domainID=" + (i2s(domainID));
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp = sqlite3_column_int(query, 0);
        listSinkID.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSinksOfDomain SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
    assert(domainID!=0);
    listSourceID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_sourceID_t temp;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND domainID=" + i2s(domainID);

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp = sqlite3_column_int(query, 0);
        listSourceID.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSourcesOfDomain SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listCrossfader) const
{
    assert(domainID!=0);
    listCrossfader.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_crossfaderID_t temp;

    std::string command = "SELECT c.crossfaderID FROM " + std::string(CROSSFADER_TABLE) + " c," + std::string(SOURCE_TABLE) + " s WHERE c.sourceID=s.sourceID AND s.domainID=" + i2s(domainID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp = sqlite3_column_int(query, 0);
        listCrossfader.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListCrossfadersOfDomain SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
    assert(domainID!=0);
    listGatewaysID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_gatewayID_t temp;

    std::string command = "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE controlDomainID=" + i2s(domainID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp = sqlite3_column_int(query, 0);
        listGatewaysID.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListGatewaysOfDomain SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListConvertersOfDomain(const am_domainID_t domainID, std::vector<am_converterID_t>& listConvertersID) const
{
    assert(domainID!=0);
    listConvertersID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_gatewayID_t temp;

    std::string command = "SELECT converterID FROM " + std::string(CONVERTER_TABLE) + " WHERE domainID=" + i2s(domainID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp = sqlite3_column_int(query, 0);
        listConvertersID.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListConvertersOfDomain SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
    listMainConnections.clear();
    sqlite3_stmt *query = NULL, *query1 = NULL;
    int eCode = 0;
    am_MainConnection_s temp;
    std::string command = "SELECT mainConnectionID, sourceID, sinkID, connectionState, delay FROM " + std::string(MAINCONNECTION_TABLE);
    std::string command1 = "SELECT connectionID FROM MainConnectionRoute";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.mainConnectionID = sqlite3_column_int(query, 0);
        temp.sourceID = sqlite3_column_int(query, 1);
        temp.sinkID = sqlite3_column_int(query, 2);
        temp.connectionState = (am_ConnectionState_e) sqlite3_column_int(query, 3);
        temp.delay = sqlite3_column_int(query, 4);
        std::string statement = command1 + i2s(temp.mainConnectionID);
        MY_SQLITE_PREPARE_V2(mpDatabase, statement.c_str(), -1, &query1, NULL)
        while ((eCode = sqlite3_step(query1)) == SQLITE_ROW)
        {
            temp.listConnectionID.push_back(sqlite3_column_int(query1, 0));
        }
        listMainConnections.push_back(temp);
        MY_SQLITE_FINALIZE(query1)
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListMainConnections SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
    listDomains.clear();
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_Domain_s temp;
    std::string command = "SELECT domainID, name, busname, nodename, early, complete, state FROM " + std::string(DOMAIN_TABLE) + " WHERE reserved=0";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.domainID = sqlite3_column_int(query, 0);
        temp.name = std::string((const char*) sqlite3_column_text(query, 1));
        temp.busname = std::string((const char*) sqlite3_column_text(query, 2));
        temp.nodename = std::string((const char*) sqlite3_column_text(query, 3));
        temp.early = sqlite3_column_int(query, 4);
        temp.complete = sqlite3_column_int(query, 5);
        temp.state = (am_DomainState_e) sqlite3_column_int(query, 6);
        listDomains.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListDomains SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
    listConnections.clear();
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_Connection_s temp;
    std::string command = "SELECT connectionID, sourceID, sinkID, delay, connectionFormat FROM " + std::string(CONNECTION_TABLE) + " WHERE reserved=0";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.connectionID = sqlite3_column_int(query, 0);
        temp.sourceID = sqlite3_column_int(query, 1);
        temp.sinkID = sqlite3_column_int(query, 2);
        temp.delay = sqlite3_column_int(query, 3);
        temp.connectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(query, 4);
        listConnections.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListConnections SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
    listSinks.clear();
    return enumerateSinks([&](const am_Sink_s & sink){ listSinks.push_back(sink);});
}

am_Error_e CAmDatabaseHandlerSQLite::getListSources(std::vector<am_Source_s> & listSources) const
{
    listSources.clear();
    return enumerateSources([&](const am_Source_s & source){ listSources.push_back(source);});
}

am_Error_e CAmDatabaseHandlerSQLite::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    listSourceClasses.clear();

    sqlite3_stmt* query = NULL, *subQuery = NULL;
    int eCode = 0, eCode1;
    am_SourceClass_s classTemp;
    am_ClassProperty_s propertyTemp;

    std::string command = "SELECT sourceClassID, name FROM " + std::string(SOURCE_CLASS_TABLE);
    std::string command2;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        classTemp.sourceClassID = sqlite3_column_int(query, 0);
        classTemp.name = std::string((const char*) sqlite3_column_text(query, 1));

        //read out Properties
        command2 = "SELECT classProperty, value FROM SourceClassProperties" + i2s(classTemp.sourceClassID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command2.c_str(), -1, &subQuery, NULL)

        while ((eCode1 = sqlite3_step(subQuery)) == SQLITE_ROW)
        {
            propertyTemp.classProperty = (am_CustomClassProperty_t) sqlite3_column_int(subQuery, 0);
            propertyTemp.value = sqlite3_column_int(subQuery, 1);
            classTemp.listClassProperties.push_back(propertyTemp);
        }

        if (eCode1 != SQLITE_DONE)
        {
            logError("DatabaseHandler::getListSourceClasses SQLITE error code:", eCode1);
            MY_SQLITE_FINALIZE(query)
            MY_SQLITE_FINALIZE(subQuery)
            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_FINALIZE(subQuery)

        listSourceClasses.push_back(classTemp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSourceClasses SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(subQuery)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
    listCrossfaders.clear();
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_Crossfader_s tempData;
    std::string command = "SELECT name, sinkID_A, sinkID_B, sourceID, hotSink,crossfaderID FROM " + std::string(CROSSFADER_TABLE);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        tempData.name = std::string((const char*) sqlite3_column_text(query, 0));
        tempData.sinkID_A = sqlite3_column_int(query, 1);
        tempData.sinkID_B = sqlite3_column_int(query, 2);
        tempData.sourceID = sqlite3_column_int(query, 3);
        tempData.hotSink = static_cast<am_HotSink_e>(sqlite3_column_int(query, 4));
        tempData.crossfaderID = sqlite3_column_int(query, 5);
        listCrossfaders.push_back(tempData);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListCrossfaders SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
    listGateways.clear();
    return enumerateGateways([&](const am_Gateway_s & gateway){ listGateways.push_back(gateway);});
}

am_Error_e CAmDatabaseHandlerSQLite::getListConverters(std::vector<am_Converter_s> & listConverters) const
{
	listConverters.clear();
	return enumerateConverters([&](const am_Converter_s & converter){ listConverters.push_back(converter);});
}

am_Error_e CAmDatabaseHandlerSQLite::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    listSinkClasses.clear();

    sqlite3_stmt* query = NULL, *subQuery = NULL;
    int eCode = 0;
    am_SinkClass_s classTemp;
    am_ClassProperty_s propertyTemp;

    std::string command = "SELECT sinkClassID, name FROM " + std::string(SINK_CLASS_TABLE);
    std::string command2;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        classTemp.sinkClassID = sqlite3_column_int(query, 0);
        classTemp.name = std::string((const char*) sqlite3_column_text(query, 1));

        //read out Properties
        command2 = "SELECT classProperty, value FROM SinkClassProperties" + i2s(classTemp.sinkClassID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command2.c_str(), -1, &subQuery, NULL)

        while ((eCode = sqlite3_step(subQuery)) == SQLITE_ROW)
        {
            propertyTemp.classProperty = (am_CustomClassProperty_t) sqlite3_column_int(subQuery, 0);
            propertyTemp.value = sqlite3_column_int(subQuery, 1);
            classTemp.listClassProperties.push_back(propertyTemp);
        }

        if (eCode != SQLITE_DONE)
        {
            logError("DatabaseHandler::getListSourceClasses SQLITE error code:", eCode);

            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_FINALIZE(subQuery)

        listSinkClasses.push_back(classTemp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSourceClasses SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListVisibleMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
    listConnections.clear();
    sqlite3_stmt *query = NULL;
    int eCode = 0;
    am_MainConnectionType_s temp;

    std::string command = "SELECT mainConnectionID, sourceID, sinkID, connectionState, delay FROM " + std::string(MAINCONNECTION_TABLE);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.mainConnectionID = sqlite3_column_int(query, 0);
        temp.sourceID = sqlite3_column_int(query, 1);
        temp.sinkID = sqlite3_column_int(query, 2);
        temp.connectionState = (am_ConnectionState_e) sqlite3_column_int(query, 3);
        temp.delay = sqlite3_column_int(query, 4);
        listConnections.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListVisibleMainConnections SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSinks(std::vector<am_SinkType_s> & listMainSinks) const
{
    listMainSinks.clear();
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_SinkType_s temp;

    std::string command = "SELECT name, sinkID, availability, availabilityReason, muteState, mainVolume, sinkClassID FROM " + std::string(SINK_TABLE) + " WHERE visible=1 AND reserved=0";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.name = std::string((const char*) sqlite3_column_text(query, 0));
        temp.sinkID = sqlite3_column_int(query, 1);
        temp.availability.availability = (am_Availability_e) sqlite3_column_int(query, 2);
        temp.availability.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 3);
        temp.muteState = (am_MuteState_e) sqlite3_column_int(query, 4);
        temp.volume = sqlite3_column_int(query, 5);
        temp.sinkClassID = sqlite3_column_int(query, 6);
        listMainSinks.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSinks SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSources(std::vector<am_SourceType_s> & listMainSources) const
{
    listMainSources.clear();
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_SourceType_s temp;
    std::string command = "SELECT name, sourceClassID, availability, availabilityReason, sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE visible=1";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.name = std::string((const char*) sqlite3_column_text(query, 0));
        temp.sourceClassID = sqlite3_column_int(query, 1);
        temp.availability.availability = (am_Availability_e) sqlite3_column_int(query, 2);
        temp.availability.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 3);
        temp.sourceID = sqlite3_column_int(query, 4);

        listMainSources.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSources SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listSoundProperties.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_MainSoundProperty_s temp;
    std::string command = "SELECT soundPropertyType, value FROM SinkMainSoundProperty" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(query, 0);
        temp.value = sqlite3_column_int(query, 1);
        listSoundProperties.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListMainSinkSoundProperties SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listSourceProperties.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_MainSoundProperty_s temp;
    std::string command = "SELECT soundPropertyType, value FROM SourceMainSoundProperty" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(query, 0);
        temp.value = sqlite3_column_int(query, 1);
        listSourceProperties.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListMainSourceSoundProperties SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listSoundproperties.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_SoundProperty_s temp;
    std::string command = "SELECT soundPropertyType, value FROM SinkSoundProperty" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(query, 0);
        temp.value = sqlite3_column_int(query, 1);
        listSoundproperties.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSinkSoundProperties SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listSoundproperties.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_SoundProperty_s temp;
    std::string command = "SELECT soundPropertyType, value FROM SourceSoundProperty" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(query, 0);
        temp.value = sqlite3_column_int(query, 1);
        listSoundproperties.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSourceSoundProperties SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
    listSystemProperties.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_SystemProperty_s temp;
    std::string command = "SELECT type, value FROM " + std::string(SYSTEM_TABLE);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type = (am_CustomSystemPropertyType_t) sqlite3_column_int(query, 0);
        temp.value = sqlite3_column_int(query, 1);
        listSystemProperties.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSystemProperties SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getListSinkConnectionFormats(const am_sinkID_t sinkID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
    listConnectionFormats.clear();
    sqlite3_stmt *qConnectionFormat = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;
    std::string commandConnectionFormat = "SELECT soundFormat FROM SinkConnectionFormat" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
    while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
    {
        tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
        listConnectionFormats.push_back(tempConnectionFormat);
    }

    MY_SQLITE_FINALIZE(qConnectionFormat)

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getListSourceConnectionFormats(const am_sourceID_t sourceID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
    listConnectionFormats.clear();
    sqlite3_stmt* qConnectionFormat = NULL;
    int eCode = 0;
    am_CustomConnectionFormat_t tempConnectionFormat;

    //read out the connectionFormats
    std::string commandConnectionFormat = "SELECT soundFormat FROM SourceConnectionFormat" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
    while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
    {
        tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
        listConnectionFormats.push_back(tempConnectionFormat);
    }

    MY_SQLITE_FINALIZE(qConnectionFormat)

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getListGatewayConnectionFormats(const am_gatewayID_t gatewayID, std::vector<bool> & listConnectionFormat) const
{
    ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
    iter = mListConnectionFormat.find(gatewayID);
    if (iter == mListConnectionFormat.end())
    {
        logError("DatabaseHandler::getListGatewayConnectionFormats database error with convertionFormat");

        return (E_DATABASE_ERROR);
    }
    listConnectionFormat = iter->second;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getListConverterConnectionFormats(const am_converterID_t converterID, std::vector<bool> & listConnectionFormat) const
{
    ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
    iter = mListConnectionFormat.find(converterID);
    if (iter == mListConnectionFormat.end())
    {
        logError("DatabaseHandler::getListConverterConnectionFormats database error with convertionFormat");

        return (E_DATABASE_ERROR);
    }
    listConnectionFormat = iter->second;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
    assert(mainConnectionID!=0);
    delay = -1;
    sqlite3_stmt *query = NULL;
    int eCode = 0;

    std::string command = "SELECT delay FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        delay = sqlite3_column_int(query, 0);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getTimingInformation SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    if (delay == -1)
        return (E_NOT_POSSIBLE);

    return (E_OK);
}

bool CAmDatabaseHandlerSQLite::sqQuery(const std::string& query)
{
    sqlite3_stmt* statement;
    int eCode = 0;
    if ((eCode = sqlite3_exec(mpDatabase, query.c_str(), NULL, &statement, NULL)) != SQLITE_OK)
    {
        logError("DatabaseHandler::sqQuery SQL Query failed:", query.c_str(), "error code:", eCode);
        return (false);
    }
    return (true);
}

bool CAmDatabaseHandlerSQLite::openDatabase()
{
    if (sqlite3_open_v2(mPath.c_str(), &mpDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK)
    {
        logInfo("DatabaseHandler::openDatabase opened database");
        return (true);
    }
    logError("DatabaseHandler::openDatabase failed to open database");
    return (false);
}

am_Error_e CAmDatabaseHandlerSQLite::changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID)
{
    assert(connectionID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT mainConnectionID FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE delay=? AND mainConnectionID=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, delay)
    MY_SQLITE_BIND_INT(query, 2, connectionID)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        MY_SQLITE_FINALIZE(query)
        return (E_OK);
    }
    command = "UPDATE " + std::string(MAINCONNECTION_TABLE) + " SET delay=? WHERE mainConnectionID=?;";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, delay)
    MY_SQLITE_BIND_INT(query, 2, connectionID)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeDelayMainConnection SQLITE Step error code:", eCode);

        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    if (mpDatabaseObserver)
        mpDatabaseObserver->timingInformationChanged(connectionID, delay);

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID)
{
    assert(connection.connectionID==0);
    assert(connection.sinkID!=0);
    assert(connection.sourceID!=0);
    //connection format is not checked, because it's project specific

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "INSERT INTO " + std::string(CONNECTION_TABLE) + "(sinkID, sourceID, delay, connectionFormat, reserved) VALUES (?,?,?,?,?)";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, connection.sinkID)
    MY_SQLITE_BIND_INT(query, 2, connection.sourceID)
    MY_SQLITE_BIND_INT(query, 3, connection.delay)
    MY_SQLITE_BIND_INT(query, 4, connection.connectionFormat)
    MY_SQLITE_BIND_INT(query, 5, true)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterConnectionDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    connectionID = sqlite3_last_insert_rowid(mpDatabase);

    logInfo("DatabaseHandler::enterConnectionDB entered new connection sourceID=", connection.sourceID, "sinkID=", connection.sinkID, "sourceID=", connection.sourceID, "connectionFormat=", connection.connectionFormat, "assigned ID=", connectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    assert(sinkClass.sinkClassID<DYNAMIC_ID_BOUNDARY);
    assert(!sinkClass.name.empty());

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    //if sinkID is zero and the first Static Sink was already entered, the ID is created
    if (sinkClass.sinkClassID == 0 && !mFirstStaticSinkClass)
    {
        command = "INSERT INTO " + std::string(SINK_CLASS_TABLE) + "(name) VALUES (?)";
    }
    else
    {
        //check if the ID already exists
        if (existSinkClass(sinkClass.sinkClassID))
            return (E_ALREADY_EXISTS);
        command = "INSERT INTO " + std::string(SINK_CLASS_TABLE) + "(name, sinkClassID) VALUES (?,?)";
    }

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sinkClass.name.c_str(), sinkClass.name.size(), SQLITE_STATIC)

    //if the ID is not created, we add it to the query
    if (sinkClass.sinkClassID != 0)
    {
        MY_SQLITE_BIND_INT(query, 2, sinkClass.sinkClassID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticSinkClass)
    {
        MY_SQLITE_BIND_INT(query, 2, DYNAMIC_ID_BOUNDARY)
        mFirstStaticSinkClass = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterSinkClassDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    sinkClassID = sqlite3_last_insert_rowid(mpDatabase); //todo:change last_insert implementations for mulithread usage...

    //now we need to create the additional tables:
    command = "CREATE TABLE SinkClassProperties" + i2s(sinkClassID) + std::string("(classProperty INTEGER, value INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO SinkClassProperties" + i2s(sinkClassID) + std::string("(classProperty,value) VALUES (?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_ClassProperty_s>::const_iterator Iterator = sinkClass.listClassProperties.begin();
    for (; Iterator < sinkClass.listClassProperties.end(); ++Iterator)
    {
        MY_SQLITE_BIND_INT(query, 1, Iterator->classProperty)
        MY_SQLITE_BIND_INT(query, 2, Iterator->value)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSinkClassDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterSinkClassDB entered new sinkClass");
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSinkClassesChanged();
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    assert(sourceClass.sourceClassID<DYNAMIC_ID_BOUNDARY);
    assert(!sourceClass.name.empty());

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    //if sinkID is zero and the first Static Sink was already entered, the ID is created
    if (sourceClass.sourceClassID == 0 && !mFirstStaticSourceClass)
    {
        command = "INSERT INTO " + std::string(SOURCE_CLASS_TABLE) + "(name) VALUES (?)";
    }
    else
    {
        //check if the ID already exists
        if (existSourceClass(sourceClass.sourceClassID))
            return (E_ALREADY_EXISTS);
        command = "INSERT INTO " + std::string(SOURCE_CLASS_TABLE) + "(name, sourceClassID) VALUES (?,?)";
    }

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, sourceClass.name.c_str(), sourceClass.name.size(), SQLITE_STATIC)

    //if the ID is not created, we add it to the query
    if (sourceClass.sourceClassID != 0)
    {
        MY_SQLITE_BIND_INT(query, 2, sourceClass.sourceClassID)
    }

    //if the first static sink is entered, we need to set it onto the boundary
    else if (mFirstStaticSourceClass)
    {
        MY_SQLITE_BIND_INT(query, 2, DYNAMIC_ID_BOUNDARY)
        mFirstStaticSourceClass = false;
    }

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::enterSourceClassDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    sourceClassID = sqlite3_last_insert_rowid(mpDatabase); //todo:change last_insert implementations for mulithread usage...

    //now we need to create the additional tables:
    command = "CREATE TABLE SourceClassProperties" + i2s(sourceClassID) + std::string("(classProperty INTEGER, value INTEGER)");
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    //fill ConnectionFormats
    command = "INSERT INTO SourceClassProperties" + i2s(sourceClassID) + std::string("(classProperty,value) VALUES (?,?)");
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    std::vector<am_ClassProperty_s>::const_iterator Iterator = sourceClass.listClassProperties.begin();
    for (; Iterator < sourceClass.listClassProperties.end(); ++Iterator)
    {
        MY_SQLITE_BIND_INT(query, 1, Iterator->classProperty)
        MY_SQLITE_BIND_INT(query, 2, Iterator->value)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSourceClassDB SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterSourceClassDB entered new sourceClass");

    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSourceClassesChanged();
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enterSystemProperties(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
    assert(!listSystemProperties.empty());
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::vector<am_SystemProperty_s>::const_iterator listIterator = listSystemProperties.begin();
    std::string command = "DELETE  FROM " + std::string(SYSTEM_TABLE);
    if (!this->sqQuery(command))
        return (E_DATABASE_ERROR);

    command = "INSERT INTO " + std::string(SYSTEM_TABLE) + " (type, value) VALUES (?,?)";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    for (; listIterator < listSystemProperties.end(); ++listIterator)
    {
        MY_SQLITE_BIND_INT(query, 1, listIterator->type)
        MY_SQLITE_BIND_INT(query, 2, listIterator->value)

        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::enterSystemProperties SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_RESET(query)
    }

    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::enterSystemProperties entered system properties");
    return (E_OK);
}

/**
 * checks for a certain mainConnection
 * @param mainConnectionID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existMainConnection(const am_mainConnectionID_t mainConnectionID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT mainConnectionID FROM " + std::string(MAINCONNECTION_TABLE) + " WHERE mainConnectionID=" + i2s(mainConnectionID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existMainConnection database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks for a certain Source
 * @param sourceID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existSource(const am_sourceID_t sourceID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND sourceID=" + i2s(sourceID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSource database error!:", eCode);
    }
    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a source name or ID exists
 * @param sourceID the sourceID
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerSQLite::existSourceNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND (name=? OR sourceID=?)";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_text(query, 1, name.c_str(), name.size(), SQLITE_STATIC)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_text failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_bind_int(query, 2, sourceID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSource database error!:", eCode);
    }
    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a name exits
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerSQLite::existSourceName(const std::string & name) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 AND name=?";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_text(query, 1, name.c_str(), name.size(), SQLITE_STATIC)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_text failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSource database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks for a certain Sink
 * @param sinkID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existSink(const am_sinkID_t sinkID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND sinkID=" + i2s(sinkID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSink database error!:", eCode);
    }
    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a sink with the ID or the name exists
 * @param sinkID the ID
 * @param name the name
 * @return true if it exists.
 */
bool CAmDatabaseHandlerSQLite::existSinkNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND (name=? OR sinkID=?)";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_text(query, 1, name.c_str(), name.size(), SQLITE_STATIC)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_text failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_bind_int(query, 2, sinkID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSink database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a sink with the name exists
 * @param name the name
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existSinkName(const std::string & name) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND name=?";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_text(query, 1, name.c_str(), name.size(), SQLITE_STATIC)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_text failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSink database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks for a certain domain
 * @param domainID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existDomain(const am_domainID_t domainID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE reserved=0 AND domainID=" + i2s(domainID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existDomain database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks for certain gateway
 * @param gatewayID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existGateway(const am_gatewayID_t gatewayID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT gatewayID FROM " + std::string(GATEWAY_TABLE) + " WHERE gatewayID=" + i2s(gatewayID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existGateway database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

bool CAmDatabaseHandlerSQLite::existConverter(const am_converterID_t converterID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT converterID FROM " + std::string(CONVERTER_TABLE) + " WHERE converterID=" + i2s(converterID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existConverter database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

am_Error_e CAmDatabaseHandlerSQLite::getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t & domainID) const
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    std::string command = "SELECT domainID FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
    int eCode = 0;
    am_Error_e returnVal = E_DATABASE_ERROR;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        domainID = sqlite3_column_int(query, 0);
        returnVal = E_OK;
    }
    else
    {
        logError("DatabaseHandler::getDomainOfSource database error!:", eCode);
    }

    MY_SQLITE_FINALIZE(query)
    return (returnVal);
}

am_Error_e am::CAmDatabaseHandlerSQLite::getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t & domainID) const
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    std::string command = "SELECT domainID FROM " + std::string(SINK_TABLE) + " WHERE sinkID=" + i2s(sinkID);
    int eCode = 0;
    am_Error_e returnVal = E_DATABASE_ERROR;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        domainID = sqlite3_column_int(query, 0);
        returnVal = E_OK;
    }
    else
    {
        logError("DatabaseHandler::getDomainOfSink database error!:", eCode);
    }

    MY_SQLITE_FINALIZE(query)
    return (returnVal);
}

/**
 * checks for certain SinkClass
 * @param sinkClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existSinkClass(const am_sinkClass_t sinkClassID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sinkClassID FROM " + std::string(SINK_CLASS_TABLE) + " WHERE sinkClassID=" + i2s(sinkClassID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSinkClass database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks for certain sourceClass
 * @param sourceClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerSQLite::existSourceClass(const am_sourceClass_t sourceClassID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sourceClassID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE sourceClassID=" + i2s(sourceClassID);
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existSinkClass database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

am_Error_e CAmDatabaseHandlerSQLite::changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    assert(connectionID!=0);

    sqlite3_stmt *query = NULL, *queryMainConnectionSubIDs = NULL;
    int eCode = 0, eCode1 = 0;
    std::string command = "UPDATE " + std::string(CONNECTION_TABLE) + " set delay=? WHERE connectionID=?";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, delay)
    MY_SQLITE_BIND_INT(query, 2, connectionID)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeConnectionTimingInformation SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    //now we need to find all mainConnections that use the changed connection and update their timing

    int tempMainConnectionID;
    //first get all route tables for all mainconnections
    command = "SELECT name FROM sqlite_master WHERE type ='table' and name LIKE 'MainConnectionRoute%'";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        //now check if the connection ID is in this table
        std::string tablename = std::string((const char*) sqlite3_column_text(query, 0));
        std::string command2 = "SELECT connectionID FROM " + tablename + " WHERE connectionID=" + i2s(connectionID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command2.c_str(), -1, &queryMainConnectionSubIDs, NULL)
        if ((eCode1 = sqlite3_step(queryMainConnectionSubIDs)) == SQLITE_ROW)
        {
            //if the connection ID is in, recalculate the mainconnection delay
            std::stringstream(tablename.substr(tablename.find_first_not_of("MainConnectionRoute"))) >> tempMainConnectionID;
            changeDelayMainConnection(calculateMainConnectionDelay(tempMainConnectionID), tempMainConnectionID);
        }
        else if (eCode1 != SQLITE_DONE)
        {
            logError("DatabaseHandler::changeConnectionTimingInformation SQLITE error code:", eCode1);

            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_FINALIZE(queryMainConnectionSubIDs)
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeConnectionTimingInformation SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeConnectionFinal(const am_connectionID_t connectionID)
{
    assert(connectionID!=0);

    sqlite3_stmt *query = NULL;
    int eCode = 0;
    std::string command = "UPDATE " + std::string(CONNECTION_TABLE) + " set reserved=0 WHERE connectionID=?";

    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, connectionID)

    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeConnectionFinal SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_timeSync_t CAmDatabaseHandlerSQLite::calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const
{
    assert(mainConnectionID!=0);
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT sum(Connections.delay),min(Connections.delay) FROM " + std::string(CONNECTION_TABLE) + ",MainConnectionRoute" + i2s(mainConnectionID) + " WHERE MainConnectionRoute" + i2s(mainConnectionID) + ".connectionID = Connections.connectionID";
    int eCode = 0;
    am_timeSync_t delay = 0;
    am_timeSync_t min = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        delay = sqlite3_column_int(query, 0);
        min = sqlite3_column_int(query, 1);
    }
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::calculateMainConnectionDelay SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    if ((eCode = sqlite3_finalize(query)) != SQLITE_OK)
    {
        logError("DatabaseHandler::calculateMainConnectionDelay SQLITE Finalize error code:", eCode);
        return (E_DATABASE_ERROR);
    }
    if (min < 0)
        delay = -1;
    return (delay);

}

/**
 * registers the Observer at the Database
 * @param iObserver pointer to the observer
 */
void CAmDatabaseHandlerSQLite::registerObserver(CAmDatabaseObserver *iObserver)
{
    assert(iObserver!=NULL);
    mpDatabaseObserver = iObserver;
}

/**
 * gives information about the visibility of a source
 * @param sourceID the sourceID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerSQLite::sourceVisible(const am_sourceID_t sourceID) const
{
    assert(sourceID!=0);
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT visible FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
    int eCode = 0;
    bool returnVal = false;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        returnVal = (bool) sqlite3_column_int(query, 0);
    }
    else if (eCode != SQLITE_DONE)
    {
        returnVal = false;
        logError("DatabaseHandler::sourceVisible database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * gives information about the visibility of a sink
 * @param sinkID the sinkID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerSQLite::sinkVisible(const am_sinkID_t sinkID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT visible FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 AND sinkID=" + i2s(sinkID);
    int eCode = 0;
    bool returnVal = false;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        returnVal = sqlite3_column_int(query, 0);
    }
    else if (eCode != SQLITE_DONE)
    {
        returnVal = false;
        logError("DatabaseHandler::sinkVisible database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a connection already exists.
 * Only takes sink, source and format information for search!
 * @param connection the connection to be checked
 * @return true if connections exists
 */
bool CAmDatabaseHandlerSQLite::existConnection(const am_Connection_s & connection) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT connectionID FROM " + std::string(CONNECTION_TABLE) + " WHERE sinkID=? AND sourceID=? AND connectionFormat=? AND reserved=0";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_int(query, 1, connection.sinkID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_bind_int(query, 2, connection.sourceID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_bind_int(query, 3, connection.connectionFormat)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existMainConnection database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a connection with the given ID exists
 * @param connectionID
 * @return true if connection exits
 */
bool CAmDatabaseHandlerSQLite::existConnectionID(const am_connectionID_t connectionID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT connectionID FROM " + std::string(CONNECTION_TABLE) + " WHERE connectionID=? AND reserved=0";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_int(query, 1, connectionID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existMainConnection database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

/**
 * checks if a CrossFader exists
 * @param crossfaderID the ID of the crossfader to be checked
 * @return true if exists
 */
bool CAmDatabaseHandlerSQLite::existCrossFader(const am_crossfaderID_t crossfaderID) const
{
    sqlite3_stmt* query = NULL;
    std::string command = "SELECT crossfaderID FROM " + std::string(CROSSFADER_TABLE) + " WHERE crossfaderID=?";
    int eCode = 0;
    bool returnVal = true;
    MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_bind_int(query, 1, crossfaderID)))
    {
        logError("CAmDatabaseHandler::sqlite3_bind_int failed with errorCode:", eCode);
        return (false);
    }

    if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
        returnVal = false;
    else if (eCode != SQLITE_ROW)
    {
        returnVal = false;
        logError("DatabaseHandler::existMainConnection database error!:", eCode);
    }

    MY_SQLITE_FINALIZE_BOOL(query)
    return (returnVal);
}

am_Error_e CAmDatabaseHandlerSQLite::getSoureState(const am_sourceID_t sourceID, am_SourceState_e & sourceState) const
{
    assert(sourceID!=0);
    sqlite3_stmt* query = NULL;
    sourceState = SS_UNKNNOWN;
    std::string command = "SELECT sourceState FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
    int eCode = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sourceState = (am_SourceState_e) sqlite3_column_int(query, 0);
    }
    else if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
    {
        logError("DatabaseHandler::getSoureState database error!:", eCode);
    }
    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState)
{
    assert(sourceID!=0);
    assert(sourceState>=SS_UNKNNOWN && sourceState<=SS_MAX);
    sqlite3_stmt* query = NULL;
    std::string command = "UPDATE " + std::string(SOURCE_TABLE) + " SET sourceState=? WHERE sourceID=" + i2s(sourceID);
    int eCode = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, sourceState)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSourceState SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSinkVolume(const am_sinkID_t sinkID, am_volume_t & volume) const
{
    assert(sinkID!=0);
    sqlite3_stmt* query = NULL;
    volume = -1;
    std::string command = "SELECT volume FROM " + std::string(SINK_TABLE) + " WHERE sinkID=" + i2s(sinkID);
    int eCode = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        volume = sqlite3_column_int(query, 0);
    }
    else if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkVolume database error!:", eCode);
    }
    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSourceVolume(const am_sourceID_t sourceID, am_volume_t & volume) const
{
    assert(sourceID!=0);
    sqlite3_stmt* query = NULL;
    volume = -1;
    std::string command = "SELECT volume FROM " + std::string(SOURCE_TABLE) + " WHERE sourceID=" + i2s(sourceID);
    int eCode = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        volume = sqlite3_column_int(query, 0);
    }
    else if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
    {
        logError("DatabaseHandler::getSourceVolume database error!:", eCode);
    }
    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existent, but not shown in sequences

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT value FROM SinkMainSoundProperty" + i2s(sinkID) + " WHERE soundPropertyType=" + i2s(propertyType);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        value = sqlite3_column_int(query, 0);
    }
    else
    {
        logError("DatabaseHandler::getDomainState database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existent, but not shown in sequences

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT value FROM SourceMainSoundProperty" + i2s(sourceID) + " WHERE soundPropertyType=" + i2s(propertyType);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        value = sqlite3_column_int(query, 0);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkSoundPropertyValue SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existent, but not shown in sequences

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT value FROM SinkSoundProperty" + i2s(sinkID) + " WHERE soundPropertyType=" + i2s(propertyType);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        value = sqlite3_column_int(query, 0);
    }
    else
    {
        logError("DatabaseHandler::getDomainState database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existent, but not shown in sequences

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT value FROM SourceSoundProperty" + i2s(sourceID) + " WHERE soundPropertyType=" + i2s(propertyType);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        value = sqlite3_column_int(query, 0);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getSinkSoundPropertyValue SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getDomainState(const am_domainID_t domainID, am_DomainState_e& state) const
{
    assert(domainID!=0);
    sqlite3_stmt* query = NULL;
    state = DS_UNKNOWN;
    std::string command = "SELECT domainState FROM " + std::string(DOMAIN_TABLE) + " WHERE domainID=" + i2s(domainID);
    int eCode = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        state = (am_DomainState_e) sqlite3_column_int(query, 0);
    }
    else if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
    {
        logError("DatabaseHandler::getDomainState database error!:", eCode);
    }
    MY_SQLITE_FINALIZE(query)
    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::peekDomain(const std::string & name, am_domainID_t & domainID)
{
    domainID=0;
    sqlite3_stmt* query = NULL, *queryInsert = NULL;
    std::string command = "SELECT domainID FROM " + std::string(DOMAIN_TABLE) + " WHERE name=?";
    int eCode = 0, eCode1 = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, name.c_str(), name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        domainID = sqlite3_column_int(query, 0);
    }
    else if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::peekDomain database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    else
    {
        command = "INSERT INTO " + std::string(DOMAIN_TABLE) + " (name,reserved) VALUES (?,?)";
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &queryInsert, NULL)
        MY_SQLITE_BIND_TEXT(queryInsert, 1, name.c_str(), name.size(), SQLITE_STATIC)
        MY_SQLITE_BIND_INT(queryInsert, 2, 1)
        //reservation flag
        if ((eCode1 = sqlite3_step(queryInsert)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::peekDomain SQLITE Step error code:", eCode1);
            MY_SQLITE_FINALIZE(queryInsert)
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_FINALIZE(queryInsert)

        domainID = sqlite3_last_insert_rowid(mpDatabase);
    }
    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::peekSink(const std::string & name, am_sinkID_t & sinkID)
{
    sqlite3_stmt* query = NULL, *queryInsert = NULL;
    std::string command = "SELECT sinkID FROM " + std::string(SINK_TABLE) + " WHERE name=?";
    int eCode = 0, eCode1 = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, name.c_str(), name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkID = sqlite3_column_int(query, 0);
    }
    else if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::peekSink database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    else
    {
        if (mFirstStaticSink)
        {
            command = "INSERT INTO " + std::string(SINK_TABLE) + " (name,reserved,sinkID) VALUES (?,?," + i2s(DYNAMIC_ID_BOUNDARY) + ")";
            mFirstStaticSink = false;
        }
        else
        {
            command = "INSERT INTO " + std::string(SINK_TABLE) + " (name,reserved) VALUES (?,?)";
        }
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &queryInsert, NULL)
        MY_SQLITE_BIND_TEXT(queryInsert, 1, name.c_str(), name.size(), SQLITE_STATIC)
        MY_SQLITE_BIND_INT(queryInsert, 2, 1)
        //reservation flag
        if ((eCode1 = sqlite3_step(queryInsert)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::peekSink SQLITE Step error code:", eCode1);
            MY_SQLITE_FINALIZE(queryInsert)
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_FINALIZE(queryInsert)

        sinkID = sqlite3_last_insert_rowid(mpDatabase);
    }
    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
    sqlite3_stmt* query = NULL, *queryInsert = NULL;
    std::string command = "SELECT sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE name=?";
    int eCode = 0, eCode1 = 0;
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, name.c_str(), name.size(), SQLITE_STATIC)
    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sourceID = sqlite3_column_int(query, 0);
    }
    else if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::peekSink database error!:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    else
    {
        if (mFirstStaticSource)
        {
            command = "INSERT INTO " + std::string(SOURCE_TABLE) + " (name,reserved,sourceID) VALUES (?,?," + i2s(DYNAMIC_ID_BOUNDARY) + ")";
            mFirstStaticSource = false;
        }
        else
        {
            command = "INSERT INTO " + std::string(SOURCE_TABLE) + " (name,reserved) VALUES (?,?)";
        }
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &queryInsert, NULL)
        MY_SQLITE_BIND_TEXT(queryInsert, 1, name.c_str(), name.size(), SQLITE_STATIC)
        MY_SQLITE_BIND_INT(queryInsert, 2, 1)
        //reservation flag
        if ((eCode1 = sqlite3_step(queryInsert)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::peekSink SQLITE Step error code:", eCode1);
            MY_SQLITE_FINALIZE(queryInsert)
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }

        MY_SQLITE_FINALIZE(queryInsert)
        sourceID = sqlite3_last_insert_rowid(mpDatabase);
    }

    MY_SQLITE_FINALIZE(query)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SINK_TABLE) + " SET volume=? WHERE sinkID=" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, volume)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSinkVolume SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSinkVolume changed volume of sink:", sinkID, "to:", volume);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(SOURCE_TABLE) + " SET volume=? WHERE sourceID=" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, volume)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSourceVolume SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSourceVolume changed volume of source=:", sourceID, "to:", volume);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SourceSoundProperty" + i2s(sourceID) + " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, soundProperty.value)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSourceSoundPropertyDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSourceSoundPropertyDB changed SourceSoundProperty of source:", sourceID, "type:", soundProperty.type, "to:", soundProperty.value);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SinkSoundProperty" + i2s(sinkID) + " SET value=? WHERE soundPropertyType=" + i2s(soundProperty.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, soundProperty.value)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeSinkSoundPropertyDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    assert(sinkID!=0);

    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeSinkSoundPropertyDB changed SinkSoundProperty of sink:", sinkID, "type:", soundProperty.type, "to:", soundProperty.value);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink)
{
    assert(crossfaderID!=0);
    assert(hotsink>=HS_UNKNOWN && hotsink>=HS_MAX);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE " + std::string(CROSSFADER_TABLE) + " SET hotsink=? WHERE crossfaderID=" + i2s(crossfaderID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, hotsink)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeCrossFaderHotSink SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)
    logInfo("DatabaseHandler::changeCrossFaderHotSink changed hotsink of crossfader=", crossfaderID, "to:", hotsink);
    return (E_OK);
}

bool CAmDatabaseHandlerSQLite::isComponentConnected(const am_Gateway_s & gateway) const
{
	sqlite3_stmt* query = NULL;
	int eCode = 0;
	std::string command;
	bool returnVal = true;
    command = "SELECT 1 FROM " + std::string(CONNECTION_TABLE) + " c WHERE c.sinkID = ? OR c.sourceID = ?";

	MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, gateway.sinkID)
    MY_SQLITE_BIND_INT(query, 2, gateway.sourceID)
	if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
	{
		returnVal = false;
	}
	else if (eCode != SQLITE_ROW)
	{
		returnVal = false;
		logError("DatabaseHandler::isComponentConnected database error!:", eCode);
	}
	MY_SQLITE_FINALIZE(query)
	return returnVal;
}

bool CAmDatabaseHandlerSQLite::isComponentConnected(const am_Converter_s & converter) const
{
	sqlite3_stmt* query = NULL;
	int eCode = 0;
	std::string command;
	bool returnVal = true;
	command = "SELECT 1 FROM " + std::string(CONNECTION_TABLE) + " c WHERE c.sinkID = ? OR c.sourceID = ?";

	MY_SQLITE_PREPARE_V2_BOOL(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, converter.sinkID)
    MY_SQLITE_BIND_INT(query, 2, converter.sourceID)
	if ((eCode = sqlite3_step(query)) == SQLITE_DONE)
	{
		returnVal = false;
	}
	else if (eCode != SQLITE_ROW)
	{
		returnVal = false;
		logError("DatabaseHandler::isComponentConnected database error!:", eCode);
	}
	MY_SQLITE_FINALIZE(query)

	return returnVal;
}

am_Error_e am::CAmDatabaseHandlerSQLite::peekSinkClassID(const std::string & name, am_sinkClass_t & sinkClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);

    am_Error_e returnVal = E_NON_EXISTENT;
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT sinkClassID FROM " + std::string(SINK_CLASS_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, name.c_str(), name.size(), SQLITE_STATIC)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sinkClassID = sqlite3_column_int(query, 0);
        returnVal = E_OK;
    }
    else if (eCode != SQLITE_DONE)
    {
        sinkClassID = 0;
        logError("DatabaseHandler::peekSinkClassID SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        returnVal = E_DATABASE_ERROR;
    }

    MY_SQLITE_FINALIZE(query)
    return (returnVal);
}

am_Error_e am::CAmDatabaseHandlerSQLite::peekSourceClassID(const std::string & name, am_sourceClass_t & sourceClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);

    am_Error_e returnVal = E_NON_EXISTENT;
    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command = "SELECT sourceClassID FROM " + std::string(SOURCE_CLASS_TABLE) + " WHERE name=?";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_TEXT(query, 1, name.c_str(), name.size(), SQLITE_STATIC)

    if ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        sourceClassID = sqlite3_column_int(query, 0);
        returnVal = E_OK;
    }
    else if (eCode != SQLITE_DONE)
    {
        sourceClassID = 0;
        logError("DatabaseHandler::peekSourceClassID SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        returnVal = E_DATABASE_ERROR;
    }

    MY_SQLITE_FINALIZE(query)
    return (returnVal);
}


am_Error_e CAmDatabaseHandlerSQLite::changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;
    am_sourceClass_t sourceClassOut(sourceClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    //check if sinkClass needs to be changed
    if (sourceClassID!=0)
    {
        command = "UPDATE"+ std::string(SOURCE_TABLE)+ " SET sourceClassID=? WHERE sourceID=" + i2s(sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        MY_SQLITE_BIND_INT(query, 1, sourceClassID)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::changeSource SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_FINALIZE(query);
    }

    else //we need to read out the active one
    {
        command = "SELECT sourceClassID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0 and sourceID=" + i2s(sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

        while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
        {
            sourceClassOut = sqlite3_column_int(query, 0);
        }
        MY_SQLITE_FINALIZE(query)
    }

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
        //first we drop the table
        command = "DELETE from SourceSoundProperty" + i2s(sourceID);
        if (!sqQuery(command))
            return (E_DATABASE_ERROR);

        //then we'll have a new one
        //Fill SinkSoundProperties
        command = "INSERT INTO SourceSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator = listSoundProperties.begin();
        for (; SoundPropertyIterator < listSoundProperties.end(); ++SoundPropertyIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, SoundPropertyIterator->type)
            MY_SQLITE_BIND_INT(query, 2, SoundPropertyIterator->value)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::changeSource SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
        //first clear the table
        command = "DELETE from SourceConnectionFormat" + i2s(sourceID);
         if (!sqQuery(command))
             return (E_DATABASE_ERROR);

        //fill ConnectionFormats
       command = "INSERT INTO SourceConnectionFormat" + i2s(sourceID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listConnectionFormats.begin();
       for (; connectionFormatIterator < listConnectionFormats.end(); ++connectionFormatIterator)
       {
           MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
           if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
           {
               logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
               MY_SQLITE_FINALIZE(query)
               return (E_DATABASE_ERROR);
           }
           MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (!listMainSoundProperties.empty() && sourceVisible(sourceID))
    {
        command = "DELETE from SourceMainSoundProperty" + i2s(sourceID);
         if (!sqQuery(command))
             return (E_DATABASE_ERROR);

         //Fill MainSinkSoundProperties
         command = "INSERT INTO SourceMainSoundProperty" + i2s(sourceID) + std::string("(soundPropertyType,value) VALUES (?,?)");
         MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
         std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator = listMainSoundProperties.begin();
         for (; mainSoundPropertyIterator < listMainSoundProperties.end(); ++mainSoundPropertyIterator)
         {
             MY_SQLITE_BIND_INT(query, 1, mainSoundPropertyIterator->type)
             MY_SQLITE_BIND_INT(query, 2, mainSoundPropertyIterator->value)
             if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
             {
                 logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
                 MY_SQLITE_FINALIZE(query)
                 return (E_DATABASE_ERROR);
             }
             MY_SQLITE_RESET(query)
         }
         MY_SQLITE_FINALIZE(query)
    }
    else //read out the properties
    {
        getListMainSourceSoundProperties(sourceID,listMainSoundPropertiesOut);
    }

    logInfo("DatabaseHandler::changeSource changed changeSink of source:", sourceID);

    if (mpDatabaseObserver != NULL)
    {
        mpDatabaseObserver->sourceUpdated(sourceID,sourceClassOut,listMainSoundPropertiesOut,sourceVisible(sourceID));
    }

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;
    am_sinkClass_t sinkClassOut(sinkClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    //check if sinkClass needs to be changed
    if (sinkClassID!=0)
    {
        command = "UPDATE"+ std::string(SINK_TABLE)+ " SET sinkClassID=? WHERE sinkID=" + i2s(sinkID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        MY_SQLITE_BIND_INT(query, 1, sinkClassID)
        if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
        {
            logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
            MY_SQLITE_FINALIZE(query)
            return (E_DATABASE_ERROR);
        }
        MY_SQLITE_FINALIZE(query);
    }

    else //we need to read out the active one
    {
        command = "SELECT sinkClassID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0 and sinkID=" + i2s(sinkID);
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

        while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
        {
            sinkClassOut = sqlite3_column_int(query, 0);
        }
        MY_SQLITE_FINALIZE(query)
    }

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
        //first we drop the table
        command = "DELETE from SinkSoundProperty" + i2s(sinkID);
        if (!sqQuery(command))
            return (E_DATABASE_ERROR);

        //then we'll have a new one
        //Fill SinkSoundProperties
        command = "INSERT INTO SinkSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
        MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
        std::vector<am_SoundProperty_s>::const_iterator SoundPropertyIterator = listSoundProperties.begin();
        for (; SoundPropertyIterator < listSoundProperties.end(); ++SoundPropertyIterator)
        {
            MY_SQLITE_BIND_INT(query, 1, SoundPropertyIterator->type)
            MY_SQLITE_BIND_INT(query, 2, SoundPropertyIterator->value)
            if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
            {
                logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
                MY_SQLITE_FINALIZE(query)
                return (E_DATABASE_ERROR);
            }
            MY_SQLITE_RESET(query)
        }
        MY_SQLITE_FINALIZE(query)
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
        //first clear the table
        command = "DELETE from SinkConnectionFormat" + i2s(sinkID);
         if (!sqQuery(command))
             return (E_DATABASE_ERROR);

        //fill ConnectionFormats
       command = "INSERT INTO SinkConnectionFormat" + i2s(sinkID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listConnectionFormats.begin();
       for (; connectionFormatIterator < listConnectionFormats.end(); ++connectionFormatIterator)
       {
           MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
           if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
           {
               logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
               MY_SQLITE_FINALIZE(query)
               return (E_DATABASE_ERROR);
           }
           MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (!listMainSoundProperties.empty() && sinkVisible(sinkID))
    {
        command = "DELETE from SinkMainSoundProperty" + i2s(sinkID);
         if (!sqQuery(command))
             return (E_DATABASE_ERROR);

         //Fill MainSinkSoundProperties
         command = "INSERT INTO SinkMainSoundProperty" + i2s(sinkID) + std::string("(soundPropertyType,value) VALUES (?,?)");
         MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
         std::vector<am_MainSoundProperty_s>::const_iterator mainSoundPropertyIterator = listMainSoundProperties.begin();
         for (; mainSoundPropertyIterator < listMainSoundProperties.end(); ++mainSoundPropertyIterator)
         {
             MY_SQLITE_BIND_INT(query, 1, mainSoundPropertyIterator->type)
             MY_SQLITE_BIND_INT(query, 2, mainSoundPropertyIterator->value)
             if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
             {
                 logError("DatabaseHandler::changeSink SQLITE Step error code:", eCode);
                 MY_SQLITE_FINALIZE(query)
                 return (E_DATABASE_ERROR);
             }
             MY_SQLITE_RESET(query)
         }
         MY_SQLITE_FINALIZE(query)
    }
    else //read out the properties
    {
        getListMainSinkSoundProperties(sinkID,listMainSoundPropertiesOut);
    }

    logInfo("DatabaseHandler::changeSink changed changeSink of sink:", sinkID);

    if (mpDatabaseObserver != NULL)
    {
        mpDatabaseObserver->sinkUpdated(sinkID,sinkClassOut,listMainSoundPropertiesOut,sinkVisible(sinkID));
    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listMainNotificationConfigurations.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_NotificationConfiguration_s temp;
    std::string command = "SELECT type, status, parameter FROM SinkMainNotificationConfiguration" + i2s(sinkID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(query, 0));
        temp.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(query, 1));
        temp.parameter= static_cast<int16_t>(sqlite3_column_int(query, 2));
        listMainNotificationConfigurations.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSinkMainNotificationConfigurations SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerSQLite::getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listMainNotificationConfigurations.clear();

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    am_NotificationConfiguration_s temp;
    std::string command = "SELECT type, status, parameter FROM SourceMainNotificationConfiguration" + i2s(sourceID);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(query, 0));
        temp.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(query, 1));
        temp.parameter= static_cast<int16_t>(sqlite3_column_int(query, 2));
        listMainNotificationConfigurations.push_back(temp);
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSourceMainNotificationConfigurations SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SinkMainNotificationConfiguration" + i2s(sinkID) + " SET status=?, parameter=? WHERE type=" + i2s(mainNotificationConfiguration.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, mainNotificationConfiguration.status)
    MY_SQLITE_BIND_INT(query, 2, mainNotificationConfiguration.parameter)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSinkNotificationConfigurationDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sinkMainNotificationConfigurationChanged(sinkID, mainNotificationConfiguration);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SourceMainNotificationConfiguration" + i2s(sourceID) + " SET status=?, parameter=? WHERE type=" + i2s(mainNotificationConfiguration.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, mainNotificationConfiguration.status)
    MY_SQLITE_BIND_INT(query, 2, mainNotificationConfiguration.parameter)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSourceNotificationConfigurationDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeMainSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sourceMainNotificationConfigurationChanged(sourceID, mainNotificationConfiguration);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    assert(gatewayID!=0);

   sqlite3_stmt* query = NULL;
   int eCode = 0;
   std::string command;

   if (!existGateway(gatewayID))
   {
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
       //clear Database
       command = "DELETE from GatewaySourceFormat" + i2s(gatewayID);
       if (!sqQuery(command))
           return (E_DATABASE_ERROR);

       //fill ConnectionFormats
       command = "INSERT INTO GatewaySourceFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listSourceConnectionFormats.begin();
       for (; connectionFormatIterator < listSourceConnectionFormats.end(); ++connectionFormatIterator)
       {
           MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
           if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
           {
               logError("DatabaseHandler::enterGatewayDB SQLITE Step error code:", eCode);
               MY_SQLITE_FINALIZE(query)
               return (E_DATABASE_ERROR);
           }
           MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
   }

   if (!listSinkConnectionFormats.empty())
   {
       //clear Database
       command = "DELETE from GatewaySinkFormat" + i2s(gatewayID);
       if (!sqQuery(command))
           return (E_DATABASE_ERROR);

       command = "INSERT INTO GatewaySinkFormat" + i2s(gatewayID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listSinkConnectionFormats.begin();
       for (; connectionFormatIterator < listSinkConnectionFormats.end(); ++connectionFormatIterator)
       {
         MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
         if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
         {
             logError("DatabaseHandler::enterGatewayDB SQLITE Step error code:", eCode);
             MY_SQLITE_FINALIZE(query)
             return (E_DATABASE_ERROR);
         }
         MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(gatewayID, convertionMatrix));
   }

   logInfo("DatabaseHandler::changeGatewayDB changed Gateway with ID", gatewayID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    assert(converterID!=0);

   sqlite3_stmt* query = NULL;
   int eCode = 0;
   std::string command;

   if (!existConverter(converterID))
   {
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
       //clear Database
       command = "DELETE from ConverterSourceFormat" + i2s(converterID);
       if (!sqQuery(command))
           return (E_DATABASE_ERROR);

       //fill ConnectionFormats
       command = "INSERT INTO ConverterSourceFormat" + i2s(converterID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listSourceConnectionFormats.begin();
       for (; connectionFormatIterator < listSourceConnectionFormats.end(); ++connectionFormatIterator)
       {
           MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
           if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
           {
               logError("DatabaseHandler::enterConverterDB SQLITE Step error code:", eCode);
               MY_SQLITE_FINALIZE(query)
               return (E_DATABASE_ERROR);
           }
           MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
   }

   if (!listSinkConnectionFormats.empty())
   {
       //clear Database
       command = "DELETE from ConverterSinkFormat" + i2s(converterID);
       if (!sqQuery(command))
           return (E_DATABASE_ERROR);

       command = "INSERT INTO ConverterSinkFormat" + i2s(converterID) + std::string("(soundFormat) VALUES (?)");
       MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
       std::vector<am_CustomConnectionFormat_t>::const_iterator connectionFormatIterator = listSinkConnectionFormats.begin();
       for (; connectionFormatIterator < listSinkConnectionFormats.end(); ++connectionFormatIterator)
       {
         MY_SQLITE_BIND_INT(query, 1, *connectionFormatIterator)
         if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
         {
             logError("DatabaseHandler::enterConverterDB SQLITE Step error code:", eCode);
             MY_SQLITE_FINALIZE(query)
             return (E_DATABASE_ERROR);
         }
         MY_SQLITE_RESET(query)
       }
       MY_SQLITE_FINALIZE(query)
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(converterID, convertionMatrix));
   }

   logInfo("DatabaseHandler::changeConverterDB changed Gateway with ID", converterID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s notificationConfiguration)
{
    assert(sinkID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SinkNotificationConfiguration" + i2s(sinkID) + " SET status=?, parameter=? WHERE type=" + i2s(notificationConfiguration.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, notificationConfiguration.status)
    MY_SQLITE_BIND_INT(query, 2, notificationConfiguration.parameter)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSinkNotificationConfigurationDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: inform obsever here...
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::changeSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s notificationConfiguration)
{
    assert(sourceID!=0);

    sqlite3_stmt* query = NULL;
    int eCode = 0;
    std::string command;

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    command = "UPDATE SourceNotificationConfiguration" + i2s(sourceID) + " SET status=?, parameter=? WHERE type=" + i2s(notificationConfiguration.type);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)
    MY_SQLITE_BIND_INT(query, 1, notificationConfiguration.status)
    MY_SQLITE_BIND_INT(query, 2, notificationConfiguration.parameter)
    if ((eCode = sqlite3_step(query)) != SQLITE_DONE)
    {
        logError("DatabaseHandler::changeMainSourceNotificationConfigurationDB SQLITE Step error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }
    MY_SQLITE_FINALIZE(query)

    logInfo("DatabaseHandler::changeSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: implement observer function
    return (E_OK);
}

void CAmDatabaseHandlerSQLite::createTables()
{
    for (uint16_t i = 0; i < sizeof(databaseTables) / sizeof(databaseTables[0]); i++)
    {
        if (!sqQuery("CREATE TABLE " + databaseTables[i]))
            throw std::runtime_error("CAmDatabaseHandler Could not create tables!");
    }
}

am_Error_e CAmDatabaseHandlerSQLite::enumerateSources(std::function<void(const am_Source_s & element)> cb) const
{
    sqlite3_stmt* query = NULL, *qConnectionFormat = NULL, *qSoundProperty = NULL, *qMAinSoundProperty = NULL, *qNotification(NULL), *qMainNotification(NULL);
    int eCode = 0;
    am_Source_s temp;
    am_CustomConnectionFormat_t tempConnectionFormat;
    am_SoundProperty_s tempSoundProperty;
    am_MainSoundProperty_s tempMainSoundProperty;
    am_NotificationConfiguration_s tempNotificationConfiguration;
    std::string command = "SELECT name, domainID, sourceClassID, sourceState, volume, visible, availability, availabilityReason, interruptState, sourceID FROM " + std::string(SOURCE_TABLE) + " WHERE reserved=0";
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.name = std::string((const char*) sqlite3_column_text(query, 0));
        temp.domainID = sqlite3_column_int(query, 1);
        temp.sourceClassID = sqlite3_column_int(query, 2);
        temp.sourceState = (am_SourceState_e) sqlite3_column_int(query, 3);
        temp.volume = sqlite3_column_int(query, 4);
        temp.visible = sqlite3_column_int(query, 5);
        temp.available.availability = (am_Availability_e) sqlite3_column_int(query, 6);
        temp.available.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 7);
        temp.interruptState = (am_InterruptState_e) sqlite3_column_int(query, 8);
        temp.sourceID = sqlite3_column_int(query, 9);

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM SourceConnectionFormat" + i2s(temp.sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
            temp.listConnectionFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qConnectionFormat)

        //read out sound properties
        std::string commandSoundProperty = "SELECT soundPropertyType, value FROM SourceSoundProperty" + i2s(temp.sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandSoundProperty.c_str(), -1, &qSoundProperty, NULL)
        while ((eCode = sqlite3_step(qSoundProperty)) == SQLITE_ROW)
        {
            tempSoundProperty.type = (am_CustomSoundPropertyType_t) sqlite3_column_int(qSoundProperty, 0);
            tempSoundProperty.value = sqlite3_column_int(qSoundProperty, 1);
            temp.listSoundProperties.push_back(tempSoundProperty);
        }

        MY_SQLITE_FINALIZE(qSoundProperty)

        std::string notificationCommand = "SELECT type, status, parameter FROM SourceNotificationConfiguration" + i2s(temp.sourceID);
        MY_SQLITE_PREPARE_V2(mpDatabase, notificationCommand.c_str(), -1, &qNotification, NULL)

        while ((eCode = sqlite3_step(qNotification)) == SQLITE_ROW)
        {
            tempNotificationConfiguration.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qNotification, 0));
            tempNotificationConfiguration.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qNotification, 1));
            tempNotificationConfiguration.parameter= static_cast<int16_t>(sqlite3_column_int(qNotification, 2));
            temp.listNotificationConfigurations.push_back(tempNotificationConfiguration);
        }
        MY_SQLITE_FINALIZE(qNotification)

        //read out MainSoundProperties if source is visible
        if(temp.visible)
        {
            std::string commandMainSoundProperty = "SELECT soundPropertyType, value FROM SourceMainSoundProperty" + i2s(temp.sourceID);
            MY_SQLITE_PREPARE_V2(mpDatabase, commandMainSoundProperty.c_str(), -1, &qMAinSoundProperty, NULL)
            while ((eCode = sqlite3_step(qMAinSoundProperty)) == SQLITE_ROW)
            {
                tempMainSoundProperty.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(qMAinSoundProperty, 0);
                tempMainSoundProperty.value = sqlite3_column_int(qMAinSoundProperty, 1);
                temp.listMainSoundProperties.push_back(tempMainSoundProperty);
            }

            MY_SQLITE_FINALIZE(qMAinSoundProperty)

            std::string mainNotificationCommand = "SELECT type, status, parameter FROM SourceMainNotificationConfiguration" + i2s(temp.sourceID);
            MY_SQLITE_PREPARE_V2(mpDatabase, mainNotificationCommand.c_str(), -1, &qMainNotification, NULL)

            while ((eCode = sqlite3_step(qMainNotification)) == SQLITE_ROW)
            {
                tempNotificationConfiguration.type =  static_cast<am_CustomNotificationType_t>(sqlite3_column_int(qMainNotification, 0));
                tempNotificationConfiguration.status = static_cast<am_NotificationStatus_e>(sqlite3_column_int(qMainNotification, 1));
                tempNotificationConfiguration.parameter= static_cast<int16_t>(sqlite3_column_int(qMainNotification, 2));
                temp.listMainNotificationConfigurations.push_back(tempNotificationConfiguration);
            }
            MY_SQLITE_FINALIZE(qMainNotification)
        }


        cb(temp);
        temp.listConnectionFormats.clear();
        temp.listMainSoundProperties.clear();
        temp.listSoundProperties.clear();
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListSources SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const
{
	sqlite3_stmt* query = NULL, *qConnectionFormat = NULL, *qSoundProperty = NULL, *qNotificationConfiguration= NULL, *qMAinSoundProperty = NULL, *qMainNotificationConfiguration= NULL;
	int eCode = 0;
	am_Sink_s temp;
	am_CustomConnectionFormat_t tempConnectionFormat;
	am_SoundProperty_s tempSoundProperty;
	am_MainSoundProperty_s tempMainSoundProperty;
	am_NotificationConfiguration_s tempNotificationConfiguration;
	am_NotificationConfiguration_s tempMainNotificationConfiguration;
	std::string command = "SELECT name, domainID, sinkClassID, volume, visible, availability, availabilityReason, muteState, mainVolume, sinkID FROM " + std::string(SINK_TABLE) + " WHERE reserved=0";
	MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

	while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
	{
		temp.name = std::string((const char*) sqlite3_column_text(query, 0));
		temp.domainID = sqlite3_column_int(query, 1);
		temp.sinkClassID = sqlite3_column_int(query, 2);
		temp.volume = sqlite3_column_int(query, 3);
		temp.visible = sqlite3_column_int(query, 4);
		temp.available.availability = (am_Availability_e) sqlite3_column_int(query, 5);
		temp.available.availabilityReason = (am_CustomAvailabilityReason_t) sqlite3_column_int(query, 6);
		temp.muteState = (am_MuteState_e) sqlite3_column_int(query, 7);
		temp.mainVolume = sqlite3_column_int(query, 8);
		temp.sinkID = sqlite3_column_int(query, 9);

		//read out the connectionFormats
		std::string commandConnectionFormat = "SELECT soundFormat FROM SinkConnectionFormat" + i2s(temp.sinkID);
		MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qConnectionFormat, NULL)
		while ((eCode = sqlite3_step(qConnectionFormat)) == SQLITE_ROW)
		{
			tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qConnectionFormat, 0);
			temp.listConnectionFormats.push_back(tempConnectionFormat);
		}

		MY_SQLITE_FINALIZE(qConnectionFormat)

		//read out sound properties
		std::string commandSoundProperty = "SELECT soundPropertyType, value FROM SinkSoundProperty" + i2s(temp.sinkID);
		MY_SQLITE_PREPARE_V2(mpDatabase, commandSoundProperty.c_str(), -1, &qSoundProperty, NULL)
		while ((eCode = sqlite3_step(qSoundProperty)) == SQLITE_ROW)
		{
			tempSoundProperty.type = (am_CustomSoundPropertyType_t) sqlite3_column_int(qSoundProperty, 0);
			tempSoundProperty.value = sqlite3_column_int(qSoundProperty, 1);
			temp.listSoundProperties.push_back(tempSoundProperty);
		}

		MY_SQLITE_FINALIZE(qSoundProperty)

		//read out notifications
		std::string commandNotificationConfiguration = "SELECT type, status, parameter FROM SinkNotificationConfiguration" + i2s(temp.sinkID);
		MY_SQLITE_PREPARE_V2(mpDatabase, commandNotificationConfiguration.c_str(), -1, &qNotificationConfiguration, NULL)
		while ((eCode = sqlite3_step(qNotificationConfiguration)) == SQLITE_ROW)
		{
			tempNotificationConfiguration.type = static_cast<am_CustomNotificationType_t> (sqlite3_column_int(qNotificationConfiguration, 0));
			tempNotificationConfiguration.status = static_cast<am_NotificationStatus_e> (sqlite3_column_int(qNotificationConfiguration, 1));
			tempNotificationConfiguration.parameter = static_cast<int16_t> (sqlite3_column_int(qNotificationConfiguration, 2));
			temp.listNotificationConfigurations.push_back(tempNotificationConfiguration);
		}

		MY_SQLITE_FINALIZE(qNotificationConfiguration)

		//read out MainSoundProperties if sink is visible
		if(temp.visible)
		{
			std::string commandMainSoundProperty = "SELECT soundPropertyType, value FROM SinkMainSoundProperty" + i2s(temp.sinkID);
			MY_SQLITE_PREPARE_V2(mpDatabase, commandMainSoundProperty.c_str(), -1, &qMAinSoundProperty, NULL)
			while ((eCode = sqlite3_step(qMAinSoundProperty)) == SQLITE_ROW)
			{
				tempMainSoundProperty.type = (am_CustomMainSoundPropertyType_t) sqlite3_column_int(qMAinSoundProperty, 0);
				tempMainSoundProperty.value = sqlite3_column_int(qMAinSoundProperty, 1);
				temp.listMainSoundProperties.push_back(tempMainSoundProperty);
			}

			MY_SQLITE_FINALIZE(qMAinSoundProperty)

			//and mainNotificationConfigurations
			std::string commandMainNotificationConfiguration = "SELECT type, status, parameter FROM SinkMainNotificationConfiguration" + i2s(temp.sinkID);
			MY_SQLITE_PREPARE_V2(mpDatabase, commandMainNotificationConfiguration.c_str(), -1, &qMainNotificationConfiguration, NULL)
			while ((eCode = sqlite3_step(qMainNotificationConfiguration)) == SQLITE_ROW)
			{
				tempMainNotificationConfiguration.type = static_cast <am_CustomNotificationType_t> (sqlite3_column_int(qMainNotificationConfiguration, 0));
				tempMainNotificationConfiguration.status = static_cast <am_NotificationStatus_e> (sqlite3_column_int(qMainNotificationConfiguration, 1));
				tempMainNotificationConfiguration.parameter = static_cast <uint16_t>(sqlite3_column_int(qMainNotificationConfiguration, 2));
				temp.listMainNotificationConfigurations.push_back(tempMainNotificationConfiguration);
			}

			MY_SQLITE_FINALIZE(qMainNotificationConfiguration)
		}

		cb(temp);
		temp.listConnectionFormats.clear();
		temp.listMainSoundProperties.clear();
		temp.listSoundProperties.clear();
	}

	if (eCode != SQLITE_DONE)
	{
		logError("DatabaseHandler::getListSinks SQLITE error code:", eCode);
		MY_SQLITE_FINALIZE(query)
		return (E_DATABASE_ERROR);
	}

	MY_SQLITE_FINALIZE(query)
	return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const
{
    sqlite3_stmt* query = NULL, *qSinkConnectionFormat = NULL, *qSourceConnectionFormat = NULL;
    int eCode = 0;
    am_Gateway_s temp;
    am_CustomConnectionFormat_t tempConnectionFormat;

    std::string command = "SELECT name, sinkID, sourceID, domainSinkID, domainSourceID, controlDomainID, gatewayID FROM " + std::string(GATEWAY_TABLE);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.name = std::string((const char*) sqlite3_column_text(query, 0));
        temp.sinkID = sqlite3_column_int(query, 1);
        temp.sourceID = sqlite3_column_int(query, 2);
        temp.domainSinkID = sqlite3_column_int(query, 3);
        temp.domainSourceID = sqlite3_column_int(query, 4);
        temp.controlDomainID = sqlite3_column_int(query, 5);
        temp.gatewayID = sqlite3_column_int(query, 6);

        //convertionMatrix:
        ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
        iter = mListConnectionFormat.find(temp.gatewayID);
        if (iter == mListConnectionFormat.end())
        {
            logError("DatabaseHandler::getListGateways database error with convertionFormat");

            return (E_DATABASE_ERROR);
        }
        temp.convertionMatrix = iter->second;

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM GatewaySourceFormat" + i2s(temp.gatewayID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSourceConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSourceConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSourceConnectionFormat, 0);
            temp.listSourceFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSourceConnectionFormat)

        //read out sound properties
        commandConnectionFormat = "SELECT soundFormat FROM GatewaySinkFormat" + i2s(temp.gatewayID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSinkConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSinkConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSinkConnectionFormat, 0);
            temp.listSinkFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSinkConnectionFormat)

        cb(temp);
        temp.listSinkFormats.clear();
        temp.listSourceFormats.clear();
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListGateways SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerSQLite::enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const
{
    sqlite3_stmt* query = NULL, *qSinkConnectionFormat = NULL, *qSourceConnectionFormat = NULL;
    int eCode = 0;
    am_Converter_s temp;
    am_CustomConnectionFormat_t tempConnectionFormat;

    std::string command = "SELECT name, sinkID, sourceID, domainID, converterID FROM " + std::string(CONVERTER_TABLE);
    MY_SQLITE_PREPARE_V2(mpDatabase, command.c_str(), -1, &query, NULL)

    while ((eCode = sqlite3_step(query)) == SQLITE_ROW)
    {
        temp.name = std::string((const char*) sqlite3_column_text(query, 0));
        temp.sinkID = sqlite3_column_int(query, 1);
        temp.sourceID = sqlite3_column_int(query, 2);
        temp.domainID = sqlite3_column_int(query, 3);
        temp.converterID = sqlite3_column_int(query, 4);

        //convertionMatrix:
        ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
        iter = mListConnectionFormat.find(temp.converterID);
        if (iter == mListConnectionFormat.end())
        {
            logError("DatabaseHandler::getListConverters database error with convertionFormat");

            return (E_DATABASE_ERROR);
        }
        temp.convertionMatrix = iter->second;

        //read out the connectionFormats
        std::string commandConnectionFormat = "SELECT soundFormat FROM ConverterSourceFormat" + i2s(temp.converterID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSourceConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSourceConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSourceConnectionFormat, 0);
            temp.listSourceFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSourceConnectionFormat)

        //read out sound properties
        commandConnectionFormat = "SELECT soundFormat FROM ConverterSinkFormat" + i2s(temp.converterID);
        MY_SQLITE_PREPARE_V2(mpDatabase, commandConnectionFormat.c_str(), -1, &qSinkConnectionFormat, NULL)
        while ((eCode = sqlite3_step(qSinkConnectionFormat)) == SQLITE_ROW)
        {
            tempConnectionFormat = (am_CustomConnectionFormat_t) sqlite3_column_int(qSinkConnectionFormat, 0);
            temp.listSinkFormats.push_back(tempConnectionFormat);
        }

        MY_SQLITE_FINALIZE(qSinkConnectionFormat)

        cb(temp);
        temp.listSinkFormats.clear();
        temp.listSourceFormats.clear();
    }

    if (eCode != SQLITE_DONE)
    {
        logError("DatabaseHandler::getListConverters SQLITE error code:", eCode);
        MY_SQLITE_FINALIZE(query)
        return (E_DATABASE_ERROR);
    }

    MY_SQLITE_FINALIZE(query)

    return (E_OK);
}

}
