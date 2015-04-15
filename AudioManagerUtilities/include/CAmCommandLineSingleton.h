/*
 * CAmCommandLineSingleton.h
 *
 *  Created on: Apr 15, 2015
 *      Author: genius
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
private:
	CAmCommandLineSingleton();
	virtual ~CAmCommandLineSingleton();
};

} /* namespace am */

#endif /* AUDIOMANAGERUTILITIES_INCLUDE_CAMCOMMANDLINESINGLETON_H_ */
