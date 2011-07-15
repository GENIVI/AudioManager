/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file RoutingReceive.cpp
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

#include "RoutingReceive.h"

void RoutingReceiver::register_Databasehandler(DataBaseHandler* handler_) {
	handler = handler_;
}

int RoutingReceiver::registerDomain(char* name, char* busname, char* node, bool earlymode) {
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Domain Registered: "), DLT_STRING(name));
	return handler->insert_into_Domains_table(std::string(name), std::string(busname), std::string(node), earlymode);
}

int RoutingReceiver::registerSource(char* name, char* audioclass, char* domain) {
	int id_class = handler->get_Source_Class_ID_from_Name(std::string(audioclass));
	int id_domain = handler->get_Domain_ID_from_Name(std::string(domain));

	if (id_class == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Class unknown: "), DLT_STRING(audioclass));
		return -1;
	} else if (id_class == -1) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Database Error: "));
		return -1;
	}
	if (id_domain == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Domain unknown: "));
		return -1;
	} else if (id_class == -1) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Database Error: "));
		return -1;
	}

	return handler->insert_into_Source_table(std::string(name), id_class, id_domain, false);
}
int RoutingReceiver::registerSink(char* name, char* sinkclass, char* domain) {
	//TODO: Integrate Sink Classes
	(void) sinkclass;
	int id_sinkclass = 1;
	int id_domain = handler->get_Domain_ID_from_Name(std::string(domain));

	if (id_domain == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Domain unknown: "));
		return -1;
	}

	return handler->insert_into_Sink_table(std::string(name), id_sinkclass, id_domain, false);
}
int RoutingReceiver::registerGateway(char* name, char* sink, char* source, char *domainSource, char* domainSink, char* controlDomain) {

	int domainSourceID = handler->get_Domain_ID_from_Name(std::string(domainSource));
	if (domainSourceID == 0) {
		domainSourceID = handler->peek_Domain_ID(std::string(domainSource));
	}
	int domainSinkID = handler->get_Domain_ID_from_Name(std::string(domainSink));
	if (domainSinkID == 0) {
		domainSinkID = handler->peek_Domain_ID(std::string(domainSink));
	}
	int domainControlID = handler->get_Domain_ID_from_Name(std::string(controlDomain));

	if (domainSourceID == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Domain Source unknown: "));
		return -1;
	}
	if (domainSink == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Domain Sink unknown: "));
		return -1;
	}
	if (domainControlID == 0) {
		DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Domain Control unknown: "));
		return -1;
	}

	int sourceId = handler->insert_into_Source_table(std::string(source), 0, domainSourceID, true);
	int sinkID = handler->insert_into_Sink_table(std::string(sink), 0, domainSinkID, true);
	return handler->insert_into_Gatway_table(std::string(name), sinkID, sourceId, domainSourceID, domainSinkID, domainControlID);

}

int RoutingReceiver::peekDomain(char* name) {
	return handler->peek_Domain_ID(std::string(name));
}

void RoutingReceiver::ackConnect(genHandle_t handle, genError_t error) {
	//emit signal_ackConnect(handle, error);
}
