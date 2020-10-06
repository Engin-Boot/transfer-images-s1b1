/*************************************************************************
 *
 *       System: Merge DICOM Toolkit
 *
 *       Author: Merge Healthcare
 *
 *  Description: This contains general utilities for all the sample programs.
 *
 *************************************************************************
 *
 *                      COPYRIGHT (C) 2012
 *                   Merge Healthcare Incorporated
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

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#if !defined(_WIN32) && !defined(UNIX)
#define UNIX            1
#endif

#if defined(_WIN32)
#include <conio.h>
#endif

#include <time.h>
#include <ctype.h>
#include <stdlib.h> /* malloc() */
#include <stdio.h>
#include <string.h>
#include <memory.h>
#ifdef UNIX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h> 
#endif

#include "general_util.h"

#if defined(CLOCK_MONOTONIC)
    /* Use clock_gettime, which returns struct timespec */
#elif defined(_WIN32) || defined(_WIN64)
    /* Use QueryPerformanceCounter, which returns LARGE_INTEGER */
#else
    /* No high-resolution timer available; use time_t */
#endif

/*
void *GetIntervalStart()
{
#if defined(CLOCK_MONOTONIC)
    struct timespec *result = malloc(sizeof (struct timespec));
    if (clock_gettime(CLOCK_MONOTONIC, result) != 0)
    {
        free(result);
        result = NULL;
    }
    return result;
#elif defined(_WIN32) || defined(_WIN64)
    /* Use QueryPerformanceCounter, which returns LARGE_INTEGER *
    LARGE_INTEGER *result = malloc(sizeof (LARGE_INTEGER));
    if (!QueryPerformanceCounter(result))
    {
        free(result);
        result = NULL;
    }
#else
    /* No high-resolution timer available; use time_t *
    time_t *result = malloc(sizeof (time_t));
    (void) time(result);
#endif
    return result;
}
*/

/*
double GetIntervalElapsed(void *start_generic)
{
    if (start_generic != NULL)
    {
#if defined(CLOCK_MONOTONIC)
        struct timespec now, start = *(struct timespec *)start_generic;
        static const double nanoseconds = 1E9;    /* nanoseconds per second *
        free(start_generic);
        if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)
        {
            return (now.tv_sec + now.tv_nsec / nanoseconds - start.tv_sec - start.tv_nsec / nanoseconds);
        }
#elif defined(_WIN32)||defined(_WIN64)
        LARGE_INTEGER now;
        LARGE_INTEGER frequency;
        LARGE_INTEGER start = *(LARGE_INTEGER *) start_generic;
        free(start_generic);
        if (QueryPerformanceCounter(&now) && QueryPerformanceFrequency(&frequency))
        {
            return (now.QuadPart - start.QuadPart) / (double) frequency.QuadPart;
        }
#else
        time_t start = *(time_t *) start_generic;
        time_t now = time(NULL);
        free(start_generic);
        return difftime(now, start);
#endif
    }
    return 0.0;
}
*/


/****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *                  This is used for display purposes.
 *
 ****************************************************************************/
char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr = NULL;

    switch (A_syntax)
    {
        case DEFLATED_EXPLICIT_LITTLE_ENDIAN: ptr = "Deflated Explicit VR Little Endian"; break;
        case EXPLICIT_BIG_ENDIAN:             ptr = "Explicit VR Big Endian"; break;
        case EXPLICIT_LITTLE_ENDIAN:          ptr = "Explicit VR Little Endian"; break;
        case HEVC_H265_M10P_LEVEL_5_1:        ptr = "HEVC/H.265 Main 10 Profile / Level 5.1"; break;
        case HEVC_H265_MP_LEVEL_5_1:          ptr = "HEVC/H.265 Main Profile / Level 5.1"; break;
        case IMPLICIT_BIG_ENDIAN:             ptr = "Implicit VR Big Endian"; break;
        case IMPLICIT_LITTLE_ENDIAN:          ptr = "Implicit VR Little Endian"; break;
        case JPEG_2000:                       ptr = "JPEG 2000"; break;
        case JPEG_2000_LOSSLESS_ONLY:         ptr = "JPEG 2000 Lossless Only"; break;
        case JPEG_2000_MC:                    ptr = "JPEG 2000 Part 2 Multi-component"; break;
        case JPEG_2000_MC_LOSSLESS_ONLY:      ptr = "JPEG 2000 Part 2 Multi-component Lossless Only"; break;
        case JPEG_BASELINE:                   ptr = "JPEG Baseline (Process 1)"; break;
        case JPEG_EXTENDED_2_4:               ptr = "JPEG Extended (Process 2 & 4)"; break;
        case JPEG_EXTENDED_3_5:               ptr = "JPEG Extended (Process 3 & 5)"; break;
        case JPEG_SPEC_NON_HIER_6_8:          ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; break;
        case JPEG_SPEC_NON_HIER_7_9:          ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; break;
        case JPEG_FULL_PROG_NON_HIER_10_12:   ptr = "JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; break;
        case JPEG_FULL_PROG_NON_HIER_11_13:   ptr = "JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; break;
        case JPEG_LOSSLESS_NON_HIER_14:       ptr = "JPEG Lossless, Non-Hierarchical (Process 14)"; break;
        case JPEG_LOSSLESS_NON_HIER_15:       ptr = "JPEG Lossless, Non-Hierarchical (Process 15)"; break;
        case JPEG_EXTENDED_HIER_16_18:        ptr = "JPEG Extended, Hierarchical (Process 16 & 18)"; break;
        case JPEG_EXTENDED_HIER_17_19:        ptr = "JPEG Extended, Hierarchical (Process 17 & 19)"; break;
        case JPEG_SPEC_HIER_20_22:            ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
        case JPEG_SPEC_HIER_21_23:            ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
        case JPEG_FULL_PROG_HIER_24_26:       ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
        case JPEG_FULL_PROG_HIER_25_27:       ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
        case JPEG_LOSSLESS_HIER_28:           ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
        case JPEG_LOSSLESS_HIER_29:           ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
        case JPEG_LOSSLESS_HIER_14:           ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
        case JPEG_LS_LOSSLESS:                ptr = "JPEG-LS Lossless"; break;
        case JPEG_LS_LOSSY:                   ptr = "JPEG-LS Lossy (Near Lossless)"; break;
        case JPIP_REFERENCED:                 ptr =  "JPIP Referenced"; break;
        case JPIP_REFERENCED_DEFLATE:         ptr =  "JPIP Referenced Deflate"; break;
        case MPEG2_MPHL:                      ptr = "MPEG2 Main Profile @ High Level"; break;
        case MPEG2_MPML:                      ptr = "MPEG2 Main Profile @ Main Level"; break;
        case MPEG4_AVC_H264_HP_LEVEL_4_1:     ptr =  "MPEG-4 AVC/H.264 High Profile / Level 4.1"; break;
        case MPEG4_AVC_H264_BDC_HP_LEVEL_4_1: ptr =  "MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1"; break;
        case MPEG4_AVC_H264_HP_LEVEL_4_2_2D:  ptr =  "MPEG-4 AVC/H.264 High Profile / Level 4.2 For 2D Video"; break;
        case MPEG4_AVC_H264_HP_LEVEL_4_2_3D:  ptr =  "MPEG-4 AVC/H.264 High Profile / Level 4.2 For 3D Video"; break;
        case MPEG4_AVC_H264_STEREO_HP_LEVEL_4_2: ptr =  "MPEG-4 AVC/H.264 Stereo High Profile / Level 4.2"; break;
        case SMPTE_ST_2110_20_UNCOMPRESSED_PROGRESSIVE_ACTIVE_VIDEO: ptr = "SMPTE ST 2110-20 Uncompressed Progressive Active Video"; break;
        case SMPTE_ST_2110_20_UNCOMPRESSED_INTERLACED_ACTIVE_VIDEO: ptr = "SMPTE ST 2110-20 Uncompressed Interlaced Active Video"; break;
        case SMPTE_ST_2110_30_PCM_DIGITAL_AUDIO: ptr = "SMPTE ST 2110-30 PCM Digital Audio"; break;
        case PRIVATE_SYNTAX_1:                ptr = "Private Syntax 1"; break;
        case PRIVATE_SYNTAX_2:                ptr = "Private Syntax 2"; break;
        case RLE:                             ptr = "RLE"; break;
        case INVALID_TRANSFER_SYNTAX:         ptr = "Invalid Transfer Syntax"; break;
    }
    return ptr;
}


