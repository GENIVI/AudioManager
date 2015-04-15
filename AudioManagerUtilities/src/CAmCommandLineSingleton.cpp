/*
 * CAmCommandLineSingleton.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: genius
 */

#include "CAmCommandLineSingleton.h"
#include <cassert>

namespace am {

static TCLAP::CmdLine* pSingleCmdLine = NULL;

CAmCommandLineSingleton::CAmCommandLineSingleton() {
	// TODO Auto-generated constructor stub

}

TCLAP::CmdLine* CAmCommandLineSingleton::instanciateOnce(const std::string& message,
		const char delimiter,
		const std::string& version,
		bool helpAndVersion)
{
	if(NULL==pSingleCmdLine)
	{
		pSingleCmdLine = new TCLAP::CmdLine(message,delimiter,version,helpAndVersion);
	}
	return pSingleCmdLine;
}

TCLAP::CmdLine* CAmCommandLineSingleton::instance()
{
	assert(NULL!=pSingleCmdLine);
	return pSingleCmdLine;
}

CAmCommandLineSingleton::~CAmCommandLineSingleton() {
	// TODO Auto-generated destructor stub
}

} /* namespace am */
