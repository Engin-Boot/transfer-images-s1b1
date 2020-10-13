/*************************************************************************
 *
 *       System: Merge DICOM Toolkit Sample Application - General Utilities Include File
 *
 *       Author: Merge Healthcare
 *
 *  Description: This module contains defintions used by applications
 *               which use the Merge DICOM C/C++ Toolkit HIS/RIS Service
 *	             functions.
 *
 *************************************************************************
 *
 *                         COPYRIGHT (C) 2012
 *                  Merge Healthcare Incorporated
 *            900 Walnut Ridge Drive, Hartland, WI 53029
 *
 *                     -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/
#ifndef MTI_GENERAL_UTIL_H
#define MTI_GENERAL_UTIL_H

#include "mc3msg.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * Boolean used to handle return values from functions
 */
typedef enum
{
    SAMP_TRUE = 1,
    SAMP_FALSE = 0
} SAMP_BOOLEAN;

typedef enum
{
    SAMP_SUCCESS = 1,
    SAMP_FAILURE = 0
} SAMP_STATUS;

extern void *GetIntervalStart(void);
extern double GetIntervalElapsed(void *start);  /* also releases start */
extern int PollInputQuitKey(void);  /* Poll stdin/console for quit key on appropriate platforms */
extern SAMP_BOOLEAN CheckValidVR(char *A_VR);

extern char *GetSyntaxDescription(TRANSFER_SYNTAX A_syntax);

#ifdef __cplusplus
}
#endif 

#endif
