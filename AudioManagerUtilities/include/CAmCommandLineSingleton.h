/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, BMW AG
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2015
 *
 * \file CAmCommandLineSingleton.h
 * For further information see http://www.genivi.org/.
 *
 */


#ifndef AUDIOMANAGERUTILITIES_INCLUDE_CAMCOMMANDLINESINGLETON_H_
#define AUDIOMANAGERUTILITIES_INCLUDE_CAMCOMMANDLINESINGLETON_H_

#include "tclap/CmdLine.h"

namespace am {

class CAmCommandLineSingleton {
public:
	static TCLAP::CmdLine* instanciateOnce(const std::string& message,
			const char delimiter = ' ',
			const std::string& version = "none",
			bool helpAndVersion = true);
			
	static TCLAP::CmdLine* instance();
	
	static void deleteInstance();
	
private:

	CAmCommandLineSingleton();
	virtual ~CAmCommandLineSingleton();
};

} /* namespace am */

#endif /* AUDIOMANAGERUTILITIES_INCLUDE_CAMCOMMANDLINESINGLETON_H_ */
