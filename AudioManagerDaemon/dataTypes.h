/*
 * Copyright (C) 2011, BMW AG
 *
 * AudioManagerDeamon
 *
 * @file dataTypes.h
 *
 * @date: 20.05.2011
 * @author: Christian Müller (christian.ei.mueller@bmw.de)
 *
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <list>
#include <string>

typedef int source_t;
typedef int sink_t;
typedef int domain_t;
typedef int gateway_t;
typedef int connection_t;
typedef int interrupt_t;
typedef int volume_t;
typedef int genHandle_t;
typedef int genInt_t;
typedef int hookprio_t;
typedef int sourceClass_t;
typedef int sinkClass_t;

/**Global defined error Type
 *
 */
typedef enum genError {
	GEN_OK, //!< GEN_OK
	GEN_UNKNOWN, //!< GEN_UNKNOWN
	GEN_OUTOFRANGE, //!< GEN_OUTOFRANGE
	GEN_NOTUSED, //!< GEN_NOTUSED
	GEN_DATABASE_ERROR
//!< GEN_DATABASE_ERROR
} genError_t;

/** the resulttype for the hooks
 *
 */
typedef enum genHookResult {
	HOOK_OK, //!< HOOK_OK
	HOOK_STOP,//!< HOOK_STOP
	HOOK_UNUSED
//!< HOOK_UNUSED
} genHookResult_t;

/** This represents one "hopp" in the route
 * TODO: change from public structs into own public classes
 */
struct genRoutingElement_t {
	source_t source;
	sink_t sink;
	domain_t Domain_ID;
};

/**This is a container for a complete route.
 * A List of "hopps" and a length.
 */
class genRoute_t {
public:
	int len;
	source_t Source_ID;
	sink_t Sink_ID;
	std::list<genRoutingElement_t> route;
};

/**This class describes the interrupt Type.
 * \var ID the ID of the Interrupt (unique)
 * \var connID the Connection ID that is used
 * \var sourceID the SourceID of the Interrupt
 * \var sinkID the sinkID of the interrupt
 * \var mixed true if interrupt is mixed into current audio
 * \var listInterruptSources the list of the interrupted sources.
 */
class interruptType_t {
public:
	genInt_t ID;
	connection_t connID;
	source_t sourceID;
	sink_t SinkID;
	bool mixed;
	std::list<source_t> listInterruptedSources;
};

class SinkType {
public:
	std::string name;
	sink_t ID;
};

class SourceType {
public:
	std::string name;
	source_t ID;
};

class ConnectionType {
public:
	source_t Source_ID;
	sink_t Sink_ID;
};

#endif /* DATATYPES_H_ */
