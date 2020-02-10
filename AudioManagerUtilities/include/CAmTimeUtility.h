/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2016, ADIT GmbH
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
 * \file CAmTimeUtility.h
 * For further information see http://www.genivi.org/.
 */

#ifndef TIMEUTILITY_H_
#define TIMEUTILITY_H_

#include <string>

namespace am
{

class CAmTimeUtility
{
public:
    static inline std::string now()
    {
        time_t     t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        struct tm *timeinfo(localtime(&t));
        char       buffer[80];
        strftime(buffer, 80, "%D %T ", timeinfo);
        return (std::string(buffer));
    }

    virtual ~CAmTimeUtility();
};

}

#endif /* TIMEUTILITY_H_ */
