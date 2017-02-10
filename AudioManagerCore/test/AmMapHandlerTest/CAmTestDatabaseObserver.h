/**
 * SPDX license identifier: MPL-2.0
 *
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
 * \file CAmDatabaseObserver.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef DATABASEOBSERVER_H_
#define DATABASEOBSERVER_H_

#include "audiomanagertypes.h"
#include <queue>
#include <vector>
#include <functional>
#include "CAmDatabaseHandlerMap.h"


namespace am
{


/**
 * This class observes the Database and notifies other classes about important events, mainly the CommandSender.
 */

class CAmDatabaseObserver: public CAmDatabaseHandlerMap::AmDatabaseObserverCallbacks
{
public:
    explicit CAmDatabaseObserver();
    ~CAmDatabaseObserver();

private:
};

}

#endif /* DATABASEOBSERVER_H_ */
