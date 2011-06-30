/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DataBaseHandler.h
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

#ifndef DATABASEHANDLER_H_
#define DATABASEHANDLER_H_

#define AUDIO_DATABASE "audiomanagerDB.sqlite"

#define DOMAIN_TABLE "Domains"
#define SOURCE_CLASS_TABLE "SourceClasses"
#define SINK_CLASS_TABLE "SinkClasses"
#define SOURCE_TABLE "Sources"
#define SINK_TABLE "Sinks"
#define GATEWAY_TABLE "Gateways"
#define CONNECTION_TABLE "Connections"
#define INTERRUPT_TABLE "Interrupts"
#define MAIN_TABLE "MainTable"

#include <QtSql>
#include <QString>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QFile>
#include <qfile.h>

#include "audioManagerIncludes.h"

class RoutingTreeItem;
class Router;
class SinkType;
class SourceType;
class ConnectionType;
class RoutingTree;

/**
 * \class DataBaseHandler
 * \brief The handler of the database
 * \details creates, deletes inserts and reads data in the database. Works on top of SQLite
 *
 *\fn bool DataBaseHandler::open_database()
 * \brief opens the database
 *
 * \fn DataBaseHandler::close_database()
 * \brief closes the database
 *
 * \fn bool DataBaseHandler::delete_data(QString table)
 * \brief deletes the data of a complete table
 * \param table the name of the table to be deleted
 *
 * \fn bool DataBaseHandler::create_tables()
 * \brief creates all neccessary tables
 *
 * \fn domain_t DataBaseHandler::insert_into_Domains_table(QString DomainName, QString BusName, QString NodeName, bool EarlyMode)
 * \brief inserts data into the domain table
 * \param DomainName the name to be inserted
 * \param BusName the name of the Bus that is used for the domain
 * \param NodeName the name of the RoutingAdaptor that belongs to the domain
 * \param EarlyMode true if the domain is an early domain
 * \return the domain id (unique)
 *
 * \fn sourceClass_t DataBaseHandler::insert_into_Source_Class_table(QString ClassName, volume_t VolumeOffset, bool IsInterrupt, bool IsMixed)
 * \brief inserts data into the Source Class table
 * \param ClassName name of the class
 * \param VolumeOffset the volume offset of the class
 * \param IsInterrupt true if it is an interrupt
 * \param IsMixed true if it is mixed
 * \return the unique Class ID
 *
 * \fn sinkClass_t DataBaseHandler::insert_into_Sink_Class_table(QString ClassName)
 * \brief enters new Sink Class in the Class Table
 * \param ClassName the name of the class
 * \return unique class ID
 *
 * \fn source_t DataBaseHandler::insert_into_Source_table(QString Name, sourceClass_t Class_ID, domain_t Domain_ID, bool IsGateway)
 * \brief inserts a new source into the source table
 * \param Name the name of the source
 * \param Class_ID the class ID of the source
 * \param Domain_ID the domain ID
 * \param IsGateway true if it is a gateway
 * \return returns a new source ID
 *
 * \fn sink_t DataBaseHandler::insert_into_Sink_table(QString Name, sinkClass_t Class_ID, domain_t Domain_ID, bool IsGateway)
 * \brief inserts a new Sink into the table
 * \param Name the name of the sink
 * \param Class_ID the class ID
 * \param Domain_ID the domain ID
 * \param IsGateway true if it is a gateway
 * \return the new Sink ID
 *
 * \fn gateway_t DataBaseHandler::insert_into_Gatway_table(QString Name, sink_t Sink_ID, source_t Source_ID, domain_t DomainSource_ID, domain_t DomainSink_ID, domain_t ControlDomain_ID)
 * \brief inserts a gateway into the database
 * \param Name the name of the gateway
 * \param Sink_ID the sink id of the gateway
 * \param Source_ID the source id of the gateway
 * \param DomainSource_ID the domain id where the source is in
 * \param DomainSink_ID the domain id where the sinḱ is in
 * \param ControlDomain_ID the domain which controls the gateway
 * \return the unique id of the gateway
 *
 * \fn domain_t DataBaseHandler::peek_Domain_ID(QString DomainName)
 * \brief reserves a domain ID but does not register it.
 * \details This function is used to register Gateways. The problem is that a gateway is registered from one domain that does not know if the other end of the gateway domain is already registered.
 * this can be solved via peeking a domain ID. By using this function the domain is only "reserved" not registered.
 * \param DomainName the name of the domain to be peeked
 * \return the Domain ID
 *
 * \fn domain_t DataBaseHandler::get_Domain_ID_from_Source_ID(source_t Source_ID)
 * \brief returns the domain if from a source given via an Id
 * \param Source_ID the source that the domain shall be returned of
 * \return the domain ID
 *
 * \fn domain_t DataBaseHandler::get_Domain_ID_from_Sink_ID(sink_t Sink_ID)
 * \brief returns the domain from a given sink ID
 * \param Sink_ID the sink ID
 * \return the domain ID
 *
 * \fn source_t DataBaseHandler::get_Source_ID_from_Name(QString name)
 * \brief returns the source ID from a name
 * \param name the name for witch the source ID shall be returned
 * \return the source ID
 *
 * \fn sourceClass_t DataBaseHandler::get_Source_Class_ID_from_Name(QString name)
 * \brief returns the source class ID from a given class name
 * \return the source class ID
 *
 * \fn domain_t DataBaseHandler::get_Domain_ID_from_Name(QString name)
 * \brief returns the domain ID from given domain name
 * \param name the name
 * \return the domain ID
 *
 * \fn sink_t DataBaseHandler::get_Sink_ID_from_Name(QString name)
 * \brief returns the Sink ID from a given name
 * \param name the name
 * \return the sink ID
 *
 * \fn QString DataBaseHandler::get_Bus_from_Domain_ID(domain_t Domain_ID)
 * \brief returns the bus name for a given domain ID
 * \param Domain_ID the domain ID
 * \return the name of the bus
 *
 * \fn domain_t DataBaseHandler::get_Domain_ID_from_Connection_ID(connection_t ID)
 * \brief returns the domain ID from a given Connection ID
 * \param ID the connection ID
 * \return the domain ID
 *
 * \fn bool DataBaseHandler::is_source_Mixed(source_t source)
 * \brief is used to find out if a source is mixed. Used for interrrupt sources
 * \param source the source
 * \return true if source is mixed.
 *
 * \fn gateway_t DataBaseHandler::get_Gateway_ID_with_Domain_ID(domain_t startDomain_ID, domain_t targetDomain_ID)
 * \brief returns a gateway ID for a given domain connection
 * \param  startDomain_ID the domain ID where the gateway should start
 * \param targetDomain_ID the domain ID where the gateway should stop
 * \return the gateway ID
 *
 * \fn genError_t DataBaseHandler::get_Gateway_Source_Sink_Domain_ID_from_ID(gateway_t Gateway_ID,source_t* return_Source_ID,sink_t* return_Sink_ID,domain_t* return_ControlDomain_ID)
 * \brief retruns informations about gateways
 * \param Gateway_ID the gateways ID as input
 * \param return_Source_ID call by reference source ID return value
 * \param return_Sink_ID call by reference Sink ID return value
 * \param return_ControlDomain_ID call by reference Control Domain ID return value
 *
 * \fn void DataBaseHandler::get_Domain_ID_Tree(bool onlyfree, RoutingTree* Tree, QList<RoutingTreeItem*>* allItems)
 * \brief is used to create a Tree out of RoutingTreeItems. Used for routing algorithm
 * \param onlyfree if called with true then only routes via free gateways will be returned
 * \param Tree pointer the the Routing Tree
 * \param allItems pointer to a list of all Routing Tree Items as call by reference value
 *
 * \fn void DataBaseHandler::getListofSources(QList<SourceType>* SourceList)
 * \brief returns a list of all sources
 * \param SourceList call by reference pointer to return value
 *
 * \fn void DataBaseHandler::getListofSinks(QList<SinkType>* SinkList)
 * \brief returns a list of all sinks
 * \param SinkList call by reference pointer to return value
 *
 * \fn void DataBaseHandler::getListofConnections(QList<ConnectionType>* ConnectionList)
 * \brief returns a list of all connections
 * \param ConnectionList call by reference pointer to return value
 *
 * \fn connection_t DataBaseHandler::getConnectionID(source_t SourceID,sink_t SinkID)
 * \brief returns the connection ID for a given source sink combination
 * \param SourceID the source ID
 * \param SinkID the sink ID
 * \return the connection ID, -1 if there is no connection
 *
 * \fn connection_t DataBaseHandler::insertConnection(source_t SourceID,sink_t SinkID)
 * \brief inserts a connection into the database
 * \param SourceID the source ID
 * \param SinkID the sink ID
 * \return the connection ID of the newly entered connection
 *
 * \fn genError_t DataBaseHandler::removeConnection(connection_t ConnectionID)
 * \brief removes a connection from the database
 * \param ConnectionID the connection ID
 * \return GEN_OK if everything was ok
 *
 * \fn connection_t DataBaseHandler::reserveMainConnection(source_t source,sink_t sink)
 * \brief reserves a main connection ID
 * \param source the source
 * \param sink the sink
 * \return the main connection ID of the newly entered main connection
 *
 * \fn genError_t DataBaseHandler::updateMainConnection(connection_t connID,genRoute_t route)
 * \brief updates the main connection. Enters the missing information into the main connection that was reserved before
 * \param connID connection ID
 * \param route the route information to be entered
 * \return GEN_OK on success
 *
 * \fn genError_t DataBaseHandler::getMainConnectionDatafromID(const connection_t connID, sink_t* return_sinkID,source_t* return_sourceID,genRoute_t** return_route)
 * \brief returns details for a connection ID
 * \param connID the connection ID
 * \param return_sinkID call by reference pointer to return Sink ID
 * \param return_sourceID call by reference pointer to return Source ID
 * \param return_route call by reference pointer to return pointer of the route
 * \return GEN_OK on success
 *
 * \fn connection_t DataBaseHandler::returnMainconnectionIDforSinkSourceID (sink_t sink, source_t source)
 * \brief return a main connection ID for a sink source combination
 * \param sink the sink ID
 * \param source the source ID
 * \return the connection ID
 *
 * \fn QList<source_t> DataBaseHandler::getSourceIDsForSinkID (sink_t sink)
 * \brief returns the list of source IDs that are connected to a sink ID
 * \param sink the sink ID
 * \return a list of source IDs
 *
 * \fn QList<ConnectionType> DataBaseHandler::getListAllMainConnections()
 * \brief returns a list of all mainconnections
 * \return a list of all connections
 *
 * \fn genError_t DataBaseHandler::removeMainConnection(connection_t connID)
 * \brief removes a main connection ID
 * \param connID the connection ID to be removed
 * \return GEN_OK on success
 *
 * \fn genInt_t DataBaseHandler::reserveInterrupt(sink_t Sink_ID, source_t Source_ID)
 * \brief reserve an interrupt
 * \details use this to reserve an interrupt ID before the actual connections is build up. Via DataBaseHandler::updateInterrupt the information can be complemented afterwards
 * \param Sink_ID the sink ID
 * \param Source_ID the source ID
 * \return the interrupt ID
 *
 * \fn genError_t DataBaseHandler::updateInterrupt(const genInt_t intID,connection_t connID, bool mixed, QList<source_t> listInterrruptedSources)
 * \brief use this to enter the missing information into a reserved Interrupt into the database
 * \param intID the interrupt ID
 * \param connID the connection ID that is used
 * \param mixed true if the interrupt is mixed
 * \param listInterrruptedSources the list of interrupted sources. Used to restore the old state afterwards
 * \return GEN_OK on success
 *
 * \fn genError_t DataBaseHandler::getInterruptDatafromID(const genInt_t intID, connection_t* return_connID, sink_t* return_Sink_ID, source_t* return_Source_ID, bool* return_mixed, QList<source_t>** return_listInterrruptedSources)
 * \brief returns information about interrupts from the ID
 * \param intID the interrupt ID
 * \param return_connID pointer to call by reference connection ID
 * \param return_Sink_ID pointer to call by reference sink ID
 * \param return_Source_ID pointer to call by reference source ID
 * \param return_mixed pointer to call by reference mixed value
 * \param return_listInterrruptedSources pointer to call by reference list of interrupted sources
 * \return GEN_OK on success
 *
 * \fn genError_t DataBaseHandler::removeInterrupt(const genInt_t intID)
 * \brief removes an interrupt from the database
 * \param intID the interrrupt ID to be removed
 * \return GEN_OK on success
 *
 * 	\fn void DataBaseHandler::signal_connectionChanged()
 * 	\brief this signal is emitted when connections are changed
 *
 * 	\fn void DataBaseHandler::signal_numberOfSinksChanged()
 * 	\brief this signal is emitted when the number of sinks changed
 *
 * 	\fn void DataBaseHandler::signal_numberOfSourcesChanged()
 * 	\brief this signal is emitted when the number of sources changed
 *
 */

class DataBaseHandler: public QObject {
Q_OBJECT
public:
	DataBaseHandler();
	virtual ~DataBaseHandler();

	bool open_database();
	void close_database();
	bool delete_data(QString table);
	bool create_tables();

	domain_t insert_into_Domains_table(QString DomainName, QString BusName, QString NodeName, bool EarlyMode);
	sourceClass_t insert_into_Source_Class_table(QString ClassName, volume_t VolumeOffset, bool IsInterrupt, bool IsMixed);
	sinkClass_t insert_into_Sink_Class_table(QString ClassName);
	source_t insert_into_Source_table(QString Name, sourceClass_t Class_ID, domain_t Domain_ID, bool IsGateway);
	sink_t insert_into_Sink_table(QString Name, sinkClass_t Class_ID, domain_t Domain_ID, bool IsGateway);
	gateway_t insert_into_Gatway_table(QString Name, sink_t Sink_ID, source_t Source_ID, domain_t DomainSource_ID, domain_t DomainSink_ID, domain_t ControlDomain_ID);

	domain_t peek_Domain_ID(QString DomainName);
	domain_t get_Domain_ID_from_Source_ID(source_t Source_ID);
	domain_t get_Domain_ID_from_Sink_ID(sink_t Sink_ID);

	source_t get_Source_ID_from_Name(QString name);
	sourceClass_t get_Source_Class_ID_from_Name(QString name);
	domain_t get_Domain_ID_from_Name(QString name);
	sink_t get_Sink_ID_from_Name(QString name);
	QString get_Bus_from_Domain_ID(domain_t Domain_ID);
	domain_t get_Domain_ID_from_Connection_ID(connection_t ID);

	bool is_source_Mixed(source_t source);

	gateway_t get_Gateway_ID_with_Domain_ID(domain_t startDomain_ID, domain_t targetDomain_ID);
	genError_t get_Gateway_Source_Sink_Domain_ID_from_ID(gateway_t Gateway_ID, source_t* return_Source_ID, sink_t* return_Sink_ID, domain_t* return_ControlDomain_ID);
	void get_Domain_ID_Tree(bool onlyfree, RoutingTree* Tree, QList<RoutingTreeItem*>* allItems);

	void getListofSources(QList<SourceType>* SourceList);
	void getListofSinks(QList<SinkType>* SinkList);
	void getListofConnections(QList<ConnectionType>* ConnectionList);

	connection_t getConnectionID(source_t SourceID, sink_t SinkID);
	connection_t insertConnection(source_t SourceID, sink_t SinkID);
	genError_t removeConnection(connection_t ConnectionID);
	connection_t reserveMainConnection(source_t source, sink_t sink);
	genError_t updateMainConnection(connection_t connID, genRoute_t route);
	genError_t getMainConnectionDatafromID(const connection_t connID, sink_t* return_sinkID, source_t* return_sourceID, genRoute_t** return_route);
	connection_t returnMainconnectionIDforSinkSourceID(sink_t sink, source_t source);
	QList<source_t> getSourceIDsForSinkID(sink_t sink);
	QList<ConnectionType> getListAllMainConnections();
	genError_t removeMainConnection(connection_t connID);
	genInt_t reserveInterrupt(sink_t Sink_ID, source_t Source_ID);
	genError_t updateInterrupt(const genInt_t intID, connection_t connID, bool mixed, QList<source_t> listInterrruptedSources);
	genError_t getInterruptDatafromID(const genInt_t intID, connection_t* return_connID, sink_t* return_Sink_ID, source_t* return_Source_ID, bool* return_mixed, QList<source_t>** return_listInterrruptedSources);
	genError_t removeInterrupt(const genInt_t intID);

signals:
	void signal_connectionChanged();
	void signal_numberOfSinksChanged();
	void signal_numberOfSourcesChanged();

private:
	QSqlDatabase m_database; //!< pointer to database
};

#endif /* DATABASEHANDLER_H_ */
