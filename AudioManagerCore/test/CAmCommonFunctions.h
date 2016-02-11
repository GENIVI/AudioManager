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
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef COMMONHEADERS_H_
#define COMMONHEADERS_H_

#include "audiomanagertypes.h"

namespace am
{

static const am_CustomNotificationType_t NT_TEST_1 = 1;
static const am_CustomNotificationType_t NT_TEST_2 = 2;


class CAmCommonFunctions
{
public:

    static std::vector<am_CustomAvailabilityReason_t> getStandardConnectionFormatList();
    static std::vector<am_SoundProperty_s> getStandardSoundPropertyList();
    static std::vector<am_MainSoundProperty_s> getStandardMainSoundPropertyList();
    static std::vector<bool> getStandardConvertionMatrix();
    static std::vector<am_NotificationConfiguration_s> getStandardNotificationConfigurationList();
    bool compareSource(std::vector<am_Source_s>::iterator listIterator, const am_Source_s& sourceData);
    bool compareSink(std::vector<am_Sink_s>::iterator listIterator, const am_Sink_s& sinkData);
    bool compareGateway(std::vector<am_Gateway_s>::iterator listIterator, const am_Gateway_s& gatewayData);
    bool compareGateway1(const am_Gateway_s gateway1, const am_Gateway_s gatewayData);
    bool compareConverter(std::vector<am_Converter_s>::iterator listIterator, const am_Converter_s& gatewayData);
    bool compareConverter1(const am_Converter_s gateway1, const am_Converter_s gatewayData);
    bool compareSinkMainSink(std::vector<am_SinkType_s>::iterator listIterator, const std::vector<am_Sink_s>& sinkList);
    bool compareSinkMainSource(std::vector<am_SourceType_s>::iterator listIterator, const std::vector<am_Source_s>& sourceList);
    bool compareRoute(am_Route_s a, am_Route_s b);
    void createSink(am_Sink_s& sink) const;
    void createSource(am_Source_s& source) const;
    void createDomain(am_Domain_s& domain) const;
    void createGateway(am_Gateway_s& gateway);
    void createConnection(am_Connection_s& connection) const;
    void createConverter(am_Converter_s & converter) const;
    void connectionList2RoutingList(std::vector<am_RoutingElement_s>& routingList, const std::vector<am_Connection_s>& connectionList);

    struct sortBySinkID
    {
        bool operator()(const am_RoutingElement_s & a, const am_RoutingElement_s & b) const
        {
            return (a.sinkID < b.sinkID);
        }
    };

    struct sortByConnectionFormat
    {
        bool operator()(const am_CustomAvailabilityReason_t & a, const am_CustomAvailabilityReason_t & b) const
        {
            return (a < b);
        }
    };

    struct sortByMainSoundProperty
    {
        bool operator()(const am_MainSoundProperty_s & a, const am_MainSoundProperty_s & b) const
        {
            return (a.type > b.type);
        }
    };

    struct sortBySoundProperty
    {
        bool operator()(const am_SoundProperty_s & a, const am_SoundProperty_s & b) const
        {
            return (a.type < b.type);
        }
    };
};

}

#endif /* COMMONHEADERS_H_ */
