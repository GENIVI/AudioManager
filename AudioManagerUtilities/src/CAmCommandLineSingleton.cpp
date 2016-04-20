/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  \file CAmCommandLineSingleton.cpp
 *  For further information see http://www.genivi.org/.
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

void CAmCommandLineSingleton::deleteInstance()
{
	if (pSingleCmdLine)
		delete pSingleCmdLine;
	
	pSingleCmdLine=NULL;		
}

CAmCommandLineSingleton::~CAmCommandLineSingleton() {
	// TODO Auto-generated destructor stub
}

} /* namespace am */
