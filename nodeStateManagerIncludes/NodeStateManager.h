#ifndef NODESTATEMANAGER_H
#define NODESTATEMANAGER_H

/**********************************************************************************************************************
*
* Copyright (C) 2012 Continental Automotive Systems, Inc.
*
* Author: Jean-Pierre.Bogler@continental-corporation.com
*
* Interface between NodeStateManager and other components in the same process
*
* The file defines the interfaces and data types, which components in the same process or on the D-Bus
* can use to communicate to the NodeStateManager (NSM). Please note that there are further interfaces
* defined in XML to access the NSM via D-Bus.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Date       Author             Reason
* 2012.06.01 uidu5846  1.0.0.0  CSP_WZ#388:  Initial version of the NodeStateManager interface
* 2012.09.27 uidu5846  1.1.0.0  CSP_WZ#1194: Changed file header structure and license to be released
*                                            as open source package. Introduced 'NodeStateTypes.h' to
*                                            avoid circle includes and encapsulate type definitions.
* 2012.10.24 uidu5846  1.2.0.0  CSP_WZ#1322: Changed types of interface parameters to native types.
*                                            Since the same native types are used, no interface change.
*
**********************************************************************************************************************/

/** \ingroup SSW_LCS */
/** \defgroup SSW_NSM_TEMPLATE Node State Manager
 *  \{
 */
/** \defgroup SSW_NSM_INTERFACE API document
 *  \{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************************************************************
*
*  HEADER FILE INCLUDES
*
**********************************************************************************************************************/

#include "NodeStateTypes.h"

/**********************************************************************************************************************
*
*  CONSTANTS
*
**********************************************************************************************************************/

/**
 *  Module version, use SswVersion to interpret the value.
 *  The lower significant byte is equal 0 for released version only
 */

#define NSM_INTERFACE_VERSION    0x01020000U

/**********************************************************************************************************************
*
*  TYPE
*
**********************************************************************************************************************/

/* There are no types defined here */

/**********************************************************************************************************************
*
*  GLOBAL VARIABLES
*
**********************************************************************************************************************/

/* There are no exported global variables */


/**********************************************************************************************************************
*
*  FUNCTION PROTOTYPE
*
**********************************************************************************************************************/

/** \brief Set data (property) of the NodeStateManager.
\param[in] enData     Type of the data to set (see ::NsmDataType_e).
\param[in] pData      Pointer to the memory location containing the data.
\param[in] u32DataLen Length of the data that should be set (in byte).
\retval see ::NsmErrorStatus_e

This is a generic interface that can be used by the NSMc to write a specific data item that from the NSM. */
NsmErrorStatus_e NsmSetData(NsmDataType_e enData, unsigned char *pData, unsigned int u32DataLen);


/** \brief Get data (property) of the NodeStateManager.
\param[in]  enData     Type of the data to get (see ::NsmDataType_e).
\param[out] pData      Pointer to the memory location where the data should be stored.
\param[in]  u32DataLen Length of the data that should be stored (in byte).
\retval     A positive value indicates the number of bytes that have been written to the out buffer pData.
            A negative value indicates an error.

This is a generic interface that can be used by the NSMc to read a specific data item that from the NSM. */
int NsmGetData(NsmDataType_e enData, unsigned char *pData, unsigned int u32DataLen);


/** \brief Get version of the interface
\retval Version of the interface as defined in ::SswVersion_t

This function asks the lifecycle to perform a restart of the main controller. */
unsigned int NsmGetInterfaceVersion(void);


/**********************************************************************************************************************
*
*  MACROS
*
**********************************************************************************************************************/

/* There are no macros defined */


#ifdef __cplusplus
}
#endif
/** \} */ /* End of SSW_NSM_INTERFACE */
/** \} */ /* End of SSW_NSM_TEMPLATE  */
#endif /* NSM_NODESTATEMANAGER_H */
