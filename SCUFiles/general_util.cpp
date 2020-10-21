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

#include "mc3inc/general_util.h"

#if defined(CLOCK_MONOTONIC)
    /* Use clock_gettime, which returns struct timespec */
#elif defined(_WIN32) || defined(_WIN64)
    /* Use QueryPerformanceCounter, which returns LARGE_INTEGER */
#else
    /* No high-resolution timer available; use time_t */
#endif

/****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *                  This is used for display purposes.
 *
 ****************************************************************************/

#include <iterator>
#include <map>

using namespace std;
char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr = NULL;
    map<TRANSFER_SYNTAX, char*> syntax;
    syntax[DEFLATED_EXPLICIT_LITTLE_ENDIAN] = (char*)"Deflated Explicit VR Little Endian";
    syntax[EXPLICIT_BIG_ENDIAN] = (char*)"Explicit VR Big Endian"; 
    syntax[EXPLICIT_LITTLE_ENDIAN] = (char*)"Explicit VR Little Endian"; 
    syntax[HEVC_H265_M10P_LEVEL_5_1] = (char*)"HEVC/H.265 Main 10 Profile / Level 5.1"; 
    syntax[HEVC_H265_MP_LEVEL_5_1] = (char*)"HEVC/H.265 Main Profile / Level 5.1"; 
    syntax[IMPLICIT_BIG_ENDIAN] = (char*)"Implicit VR Big Endian"; 
    syntax[IMPLICIT_LITTLE_ENDIAN] = (char*)"Implicit VR Little Endian"; 
    syntax[JPEG_2000] = (char*)"JPEG 2000"; 
    syntax[JPEG_2000_LOSSLESS_ONLY] = (char*)"JPEG 2000 Lossless Only"; 
    syntax[JPEG_2000_MC] = (char*)"JPEG 2000 Part 2 Multi-component"; 
    syntax[JPEG_2000_MC_LOSSLESS_ONLY] = (char*)"JPEG 2000 Part 2 Multi-component Lossless Only"; 
    syntax[JPEG_BASELINE] = (char*)"JPEG Baseline (Process 1)"; 
    syntax[JPEG_EXTENDED_2_4] = (char*)"JPEG Extended (Process 2 & 4)"; 
    syntax[JPEG_EXTENDED_3_5] = (char*)"JPEG Extended (Process 3 & 5)";
    syntax[JPEG_SPEC_NON_HIER_6_8] = (char*)"JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; 
    syntax[JPEG_SPEC_NON_HIER_7_9] = (char*)"JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; 
    syntax[JPEG_FULL_PROG_NON_HIER_10_12] = (char*)"JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; 
    syntax[JPEG_FULL_PROG_NON_HIER_11_13] = (char*)"JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; 
    syntax[JPEG_LOSSLESS_NON_HIER_14] = (char*)"JPEG Lossless, Non-Hierarchical (Process 14)"; 
    syntax[JPEG_LOSSLESS_NON_HIER_15] = (char*)"JPEG Lossless, Non-Hierarchical (Process 15)"; 
    syntax[JPEG_EXTENDED_HIER_16_18] = (char*)"JPEG Extended, Hierarchical (Process 16 & 18)"; 
    syntax[JPEG_EXTENDED_HIER_17_19] = (char*)"JPEG Extended, Hierarchical (Process 17 & 19)"; 
    syntax[JPEG_SPEC_HIER_20_22] = (char*)"JPEG Spectral Selection Hierarchical (Process 20 & 22)"; 
    syntax[JPEG_SPEC_HIER_21_23] = (char*)"JPEG Spectral Selection Hierarchical (Process 21 & 23)"; 
    syntax[JPEG_FULL_PROG_HIER_24_26] = (char*)"JPEG Full Progression, Hierarchical (Process 24 & 26)"; 
    syntax[JPEG_FULL_PROG_HIER_25_27] = (char*)"JPEG Full Progression, Hierarchical (Process 25 & 27)"; 
    syntax[JPEG_LOSSLESS_HIER_28] = (char*)"JPEG Lossless, Hierarchical (Process 28)"; 
    syntax[JPEG_LOSSLESS_HIER_29] = (char*)"JPEG Lossless, Hierarchical (Process 29)"; 
    syntax[JPEG_LOSSLESS_HIER_14] = (char*)"JPEG Lossless, Non-Hierarchical, First-Order Prediction"; 
    syntax[JPEG_LS_LOSSLESS] = (char*)"JPEG-LS Lossless"; 
    syntax[JPEG_LS_LOSSY] = (char*)"JPEG-LS Lossy (Near Lossless)"; 
    syntax[JPIP_REFERENCED] = (char*)"JPIP Referenced"; 
    syntax[JPIP_REFERENCED_DEFLATE] = (char*)"JPIP Referenced Deflate"; 
    syntax[MPEG2_MPHL] = (char*)"MPEG2 Main Profile @ High Level"; 
    syntax[MPEG2_MPML] = (char*)"MPEG2 Main Profile @ Main Level";
    syntax[MPEG4_AVC_H264_HP_LEVEL_4_1] = (char*)"MPEG-4 AVC/H.264 High Profile / Level 4.1"; 
    syntax[MPEG4_AVC_H264_BDC_HP_LEVEL_4_1] = (char*)"MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1"; 
    syntax[MPEG4_AVC_H264_HP_LEVEL_4_2_2D] = (char*)"MPEG-4 AVC/H.264 High Profile / Level 4.2 For 2D Video"; 
    syntax[MPEG4_AVC_H264_HP_LEVEL_4_2_3D] = (char*)"MPEG-4 AVC/H.264 High Profile / Level 4.2 For 3D Video"; 
    syntax[MPEG4_AVC_H264_STEREO_HP_LEVEL_4_2] = (char*)"MPEG-4 AVC/H.264 Stereo High Profile / Level 4.2"; 
    syntax[SMPTE_ST_2110_20_UNCOMPRESSED_PROGRESSIVE_ACTIVE_VIDEO] = (char*)"SMPTE ST 2110-20 Uncompressed Progressive Active Video"; 
    syntax[SMPTE_ST_2110_20_UNCOMPRESSED_INTERLACED_ACTIVE_VIDEO] = (char*)"SMPTE ST 2110-20 Uncompressed Interlaced Active Video"; 
    syntax[SMPTE_ST_2110_30_PCM_DIGITAL_AUDIO] = (char*)"SMPTE ST 2110-30 PCM Digital Audio"; 
    syntax[PRIVATE_SYNTAX_1] = (char*)"Private Syntax 1"; 
    syntax[PRIVATE_SYNTAX_2] = (char*)"Private Syntax 2"; 
    syntax[RLE] = (char*)"RLE"; 
    syntax[INVALID_TRANSFER_SYNTAX] = (char*)"Invalid Transfer Syntax"; 

    map<TRANSFER_SYNTAX, char*>::iterator itr;
    for (itr = syntax.begin(); itr != syntax.end(); ++itr)
    {
        ptr = itr->second;
    }

    return ptr;
}


