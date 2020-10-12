#include "Definitions.h"

/****************************************************************************
 *
 *  Function    :   ReadImage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered
 *                  A_node     - The node in our list of instances
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Determine the format of a DICOM file and read it into
 *                  memory.  Note that in a production application, the
 *                  file format should be predetermined (and not have to be
 *                  "guessed" by the CheckFileFormat function).  The
 *                  format for this application was chosen to show how both
 *                  DICOM Part 10 format files and "stream" format objects
 *                  can be sent over the network.
 *
 ****************************************************************************/
SAMP_BOOLEAN ReadImage(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode* A_node)
{
    FORMAT_ENUM             format = UNKNOWN_FORMAT;
    SAMP_BOOLEAN            sampBool = SAMP_FALSE;


    format = CheckFileFormat(A_node->fname);
    if (format == MEDIA_FORMAT)
    {
        A_node->mediaFormat = SAMP_TRUE;
        sampBool = ReadFileFromMedia(A_options, A_appID, A_node->fname, &A_node->msgID, &A_node->transferSyntax, &A_node->imageBytes);
    }
    else
    {
        PrintError("Unable to determine the format of file", MC_NORMAL_COMPLETION);
        sampBool = SAMP_FALSE;
    }
    if (sampBool == SAMP_TRUE)
    {
        sampBool = ValidImageCheck(sampBool, A_node);
    }
    fflush(stdout);
    return sampBool;
}
static SAMP_BOOLEAN ValidImageCheck(SAMP_BOOLEAN sampBool, InstanceNode* A_node)
{
    MC_STATUS               mcStatus;
    mcStatus = MC_Get_Value_To_String(A_node->msgID, MC_ATT_SOP_CLASS_UID, sizeof(A_node->SOPClassUID), A_node->SOPClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for SOP Class UID failed", mcStatus);
    }

    mcStatus = MC_Get_Value_To_String(A_node->msgID, MC_ATT_SOP_INSTANCE_UID, sizeof(A_node->SOPInstanceUID), A_node->SOPInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for SOP Instance UID failed", mcStatus);
    }
    return sampBool;
}
/****************************************************************************
 *
 *  Function    :   ReadFileFromMedia
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered
 *                  A_filename - Name of file to open
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax the message was encoded
 *                               in is returned here.
 *                  A_bytesRead- Total number of bytes read in image.  Used
 *                               only for display and to calculate the
 *                               transfer rate.
 *
 *  Returns     :   SAMP_TRUE on success
 *                  SAMP_FALSE on failure to read the object
 *
 *  Description :   This function reads a file in the DICOM Part 10 (media)
 *                  file format.  Before returning, it determines the
 *                  transfer syntax the file was encoded as, and converts
 *                  the file into the tool kit's "message" file format
 *                  for use in the network routines.
 *
 ****************************************************************************/

bool CheckTransferSyntax(int A_syntax)
{
    //Associated with ReadFileFromMedia
    map<int, int> MapTransferSyntaxToValue;

    MapTransferSyntaxToValue[DEFLATED_EXPLICIT_LITTLE_ENDIAN] = 1;
    MapTransferSyntaxToValue[EXPLICIT_BIG_ENDIAN] = 1;
    MapTransferSyntaxToValue[EXPLICIT_LITTLE_ENDIAN] = 1;
    MapTransferSyntaxToValue[HEVC_H265_M10P_LEVEL_5_1] = 1;
    MapTransferSyntaxToValue[HEVC_H265_MP_LEVEL_5_1] = 1;
    MapTransferSyntaxToValue[IMPLICIT_BIG_ENDIAN] = 1;
    MapTransferSyntaxToValue[IMPLICIT_LITTLE_ENDIAN] = 1;
    MapTransferSyntaxToValue[JPEG_2000] = 1;
    MapTransferSyntaxToValue[JPEG_2000_LOSSLESS_ONLY] = 1;
    MapTransferSyntaxToValue[JPEG_2000_MC] = 1;
    MapTransferSyntaxToValue[JPEG_2000_MC_LOSSLESS_ONLY] = 1;
    MapTransferSyntaxToValue[JPEG_BASELINE] = 1;
    MapTransferSyntaxToValue[JPEG_EXTENDED_2_4] = 1;
    MapTransferSyntaxToValue[JPEG_EXTENDED_3_5] = 1;
    MapTransferSyntaxToValue[JPEG_SPEC_NON_HIER_6_8] = 1;
    MapTransferSyntaxToValue[JPEG_SPEC_NON_HIER_7_9] = 1;
    MapTransferSyntaxToValue[JPEG_FULL_PROG_NON_HIER_10_12] = 1;
    MapTransferSyntaxToValue[JPEG_FULL_PROG_NON_HIER_11_13] = 1;
    MapTransferSyntaxToValue[JPEG_LOSSLESS_NON_HIER_14] = 1;
    MapTransferSyntaxToValue[JPEG_LOSSLESS_NON_HIER_15] = 1;
    MapTransferSyntaxToValue[JPEG_EXTENDED_HIER_16_18] = 1;
    MapTransferSyntaxToValue[JPEG_EXTENDED_HIER_17_19] = 1;
    MapTransferSyntaxToValue[JPEG_SPEC_HIER_20_22] = 1;
    MapTransferSyntaxToValue[JPEG_SPEC_HIER_21_23] = 1;
    MapTransferSyntaxToValue[JPEG_FULL_PROG_HIER_24_26] = 1;
    MapTransferSyntaxToValue[JPEG_FULL_PROG_HIER_25_27] = 1;
    MapTransferSyntaxToValue[JPEG_LOSSLESS_HIER_28] = 1;
    MapTransferSyntaxToValue[JPEG_LOSSLESS_HIER_29] = 1;
    MapTransferSyntaxToValue[JPEG_LOSSLESS_HIER_14] = 1;
    MapTransferSyntaxToValue[JPEG_LS_LOSSLESS] = 1;
    MapTransferSyntaxToValue[JPEG_LS_LOSSY] = 1;
    MapTransferSyntaxToValue[JPIP_REFERENCED] = 1;
    MapTransferSyntaxToValue[JPIP_REFERENCED_DEFLATE] = 1;
    MapTransferSyntaxToValue[MPEG2_MPHL] = 1;
    MapTransferSyntaxToValue[MPEG2_MPML] = 1;
    MapTransferSyntaxToValue[MPEG4_AVC_H264_HP_LEVEL_4_1] = 1;
    MapTransferSyntaxToValue[MPEG4_AVC_H264_BDC_HP_LEVEL_4_1] = 1;
    MapTransferSyntaxToValue[MPEG4_AVC_H264_HP_LEVEL_4_2_2D] = 1;
    MapTransferSyntaxToValue[MPEG4_AVC_H264_HP_LEVEL_4_2_3D] = 1;
    MapTransferSyntaxToValue[MPEG4_AVC_H264_STEREO_HP_LEVEL_4_2] = 1;
    MapTransferSyntaxToValue[SMPTE_ST_2110_20_UNCOMPRESSED_PROGRESSIVE_ACTIVE_VIDEO] = 1;
    MapTransferSyntaxToValue[SMPTE_ST_2110_20_UNCOMPRESSED_INTERLACED_ACTIVE_VIDEO] = 1;
    MapTransferSyntaxToValue[SMPTE_ST_2110_30_PCM_DIGITAL_AUDIO] = 1;
    MapTransferSyntaxToValue[PRIVATE_SYNTAX_1] = 1;
    MapTransferSyntaxToValue[PRIVATE_SYNTAX_2] = 1;
    MapTransferSyntaxToValue[RLE] = 1;
    MapTransferSyntaxToValue[INVALID_TRANSFER_SYNTAX] = 0;

    return MapTransferSyntaxToValue[A_syntax];

}

void CloseCallBackInfo(CBinfo& callbackInfo)
{
    ////Associated with ReadFileFromMedia
    if (callbackInfo.fp)
        fclose(callbackInfo.fp);
    if (callbackInfo.buffer)
        free(callbackInfo.buffer);
    return;
}

MC_STATUS CreateEmptyFileAndStoreIt(int& A_appID, int*& A_msgID, char*& A_filename, CBinfo& callbackInfo)
{
    //Associated with ReadFileFromMedia
    MC_STATUS mcStatusTemp;
    mcStatusTemp = MC_Create_Empty_File(A_msgID, A_filename);

    if (mcStatusTemp != MC_NORMAL_COMPLETION)
    {

        PrintError("Unable to create file object", mcStatusTemp);
        fflush(stdout);
        return(mcStatusTemp);
    }

    mcStatusTemp = MC_Open_File(A_appID, *A_msgID, &callbackInfo, MediaToFileObj);
    if (mcStatusTemp != MC_NORMAL_COMPLETION)
    {
        CloseCallBackInfo(callbackInfo);
        PrintError("MC_Open_File failed, unable to read file from media", mcStatusTemp);
        MC_Free_File(A_msgID);
        fflush(stdout);
        return(mcStatusTemp);
    }
}
static bool Transfer_Syntax_Encoding(MC_STATUS mcStatus, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax)
{
    /*
         * Get the transfer syntax UID from the file to determine if the object
         * is encoded in a compressed transfer syntax.  IE, one of the JPEG or
         * the RLE transfer syntaxes.  If we've specified on the command line
         * that we are supporting encapsulated/compressed transfer syntaxes,
         * go ahead an use the object, if not, reject it and return failure.
         *
         * Note that if encapsulated transfer syntaxes are supported, the
         * services lists in the mergecom.app file must be expanded using
         * transfer syntax lists to contain the JPEG syntaxes supported.
         * Also, the transfer syntaxes negotiated for each service should be
         * saved (as retrieved by the MC_Get_First/Next_Acceptable service
         * calls) to match with the actual syntax of the object.  If they do
         * not match the encoding of the pixel data may have to be modified
         * before the file is sent over the wire.
         */
    char        transferSyntaxUID[UI_LENGTH + 2] = { 0 };
    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_TRANSFER_SYNTAX_UID, sizeof(transferSyntaxUID), transferSyntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {

        PrintError("MC_Get_Value_To_String failed for transfer syntax UID", mcStatus);
        MC_Free_File(A_msgID);
        return false;
    }

    mcStatus = MC_Get_Enum_From_Transfer_Syntax(transferSyntaxUID, A_syntax);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {

        printf("Invalid transfer syntax UID contained in the file: %s\n", transferSyntaxUID);
        MC_Free_File(A_msgID);
        return false;
    }

    return true;
}
static bool Image_Extraction(int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, char*& A_filename, char* sopClassUID, char* sopInstanceUID, size_t& size_sopClassUID, size_t& size_sopInstanceUID)
{
    //char sopClassUID[UI_LENGTH + 2] = { 0 }, sopInstanceUID[UI_LENGTH + 2] = { 0 };
    MC_STATUS mcStatus;
    printf("Reading DICOM Part 10 format file in %s: %s\n", GetSyntaxDescription(*A_syntax), A_filename);
    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID, size_sopClassUID, sopClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {

        printf("Get MC_ATT_AFFECTED_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }

    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID, size_sopInstanceUID, sopInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Get MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }
    return true;
}
static bool Message_Creation(MC_STATUS mcStatus, int*& A_msgID, char* sopClassUID, char* sopInstanceUID)
{
    //char sopClassUID[UI_LENGTH + 2] = { 0 }, sopInstanceUID[UI_LENGTH + 2] = { 0 };
    /* form message with valid group 0 and transfer syntax */
    mcStatus = MC_Set_Value_From_String(*A_msgID, MC_ATT_AFFECTED_SOP_CLASS_UID, sopClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Set MC_ATT_AFFECTED_SOP_CLASS_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }

    mcStatus = MC_Set_Value_From_String(*A_msgID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, sopInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Set MC_ATT_AFFECTED_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }
    return true;
}
static bool Syntax_Handling(MC_STATUS mcStatus, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax)
{
    if (Transfer_Syntax_Encoding(mcStatus, A_msgID, A_syntax) == false)
    {
        return false;
    }
    /*
         * If we don't handle encapsulated transfer syntaxes, let's check the
         * image transfer syntax to be sure it is not encoded as an encapsulated
         * transfer syntax.
         */

    if (!CheckTransferSyntax(*A_syntax))
    {
        printf("Warning: Invalid transfer syntax (%s) specified\n", GetSyntaxDescription(*A_syntax));
        printf("         Not sending image.\n");
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }
    return true;
}
static bool Message_Handling(int*& A_msgID, char* sopClassUID, char* sopInstanceUID, size_t& size_sopClassUID, size_t& size_sopInstanceUID)
{
    MC_STATUS mcStatus;
    /*
         * Convert the "file" object into a "message" object.  This is done
         * because the MC_Send_Request_Message call requires that the object
         * be a "message" object.  Any of the tags in the message can be
         * accessed when the object is a "file" object.
         */
    mcStatus = MC_File_To_Message(*A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to convert file object to message object", mcStatus);
        MC_Free_File(A_msgID);
        fflush(stdout);
        return false;
    }

    if (Message_Creation(mcStatus, A_msgID, sopClassUID, sopInstanceUID) == false)
    {

        return false;
    }

    return true;
}
static bool ReadFile1(int& A_appID, char*& A_filename, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, size_t*& A_bytesRead)
{
    CBinfo      callbackInfo = { 0 };
    MC_STATUS mcStatus;
    mcStatus = CreateEmptyFileAndStoreIt(A_appID, A_msgID, A_filename, callbackInfo);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return false;
    }

    CloseCallBackInfo(callbackInfo);

    *A_bytesRead = callbackInfo.bytesRead;

    if (Syntax_Handling(mcStatus, A_msgID, A_syntax) == false)
    {
        return false;
    }
    return true;
}
static bool ReadFile2(int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, char*& A_filename)
{
    char sopClassUID[UI_LENGTH + 2] = { 0 }, sopInstanceUID[UI_LENGTH + 2] = { 0 };
    size_t size_sopClassUID = sizeof(sopClassUID);
    size_t size_sopInstanceUID = sizeof(sopInstanceUID);
    if (Image_Extraction(A_msgID, A_syntax, A_filename, sopClassUID, sopInstanceUID, size_sopClassUID, size_sopInstanceUID) == false)
    {
        return false;
    }

    if (Message_Handling(A_msgID, sopClassUID, sopInstanceUID, size_sopClassUID, size_sopInstanceUID) == false)
    {
        return false;
    }
    return true;
}

static SAMP_BOOLEAN ReadFileFromMedia(STORAGE_OPTIONS* A_options,
    int               A_appID,
    char* A_filename,
    int* A_msgID,
    TRANSFER_SYNTAX* A_syntax,
    size_t* A_bytesRead)
{



    /*
     * Create new File object
     */
    if (ReadFile1(A_appID, A_filename, A_msgID, A_syntax, A_bytesRead) == false)
    {
        return SAMP_FALSE;
    }
    /*
     * Read the file off of disk
    */
    if (ReadFile2(A_msgID, A_syntax, A_filename) == false)
    {
        return SAMP_FALSE;
    }


    /*
     *  SCU service should set Transfer Syntax of DICOM send message explicitly to guarantee that SCP service would get
     *  DICOM receive message with correct Transfer Syntax.
     *
     *  Corresponding DICOM service used for DICOM association to transfer the DICOM message
     *  should be configured properly in mergecom.app configuration file.
     */
     /*
         mcStatus = MC_Set_Message_Transfer_Syntax(*A_msgID, *A_syntax);
         if (mcStatus != MC_NORMAL_COMPLETION)
         {
             printf("Set MC_Set_Message_Transfer_Syntax failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
             MC_Free_File(A_msgID);
             return SAMP_FALSE;
         }
     */
    fflush(stdout);

    return SAMP_TRUE;
} /* ReadFileFromMedia() */


/****************************************************************************
 *
 *  Function    :   MediaToFileObj
 *
 *  Parameters  :   A_fileName   - Filename to open for reading
 *                  A_userInfo   - Pointer to an object used to preserve
 *                                 data between calls to this function.
 *                  A_dataSize   - Number of bytes read
 *                  A_dataBuffer - Pointer to buffer of data read
 *                  A_isFirst    - Set to non-zero value on first call
 *                  A_isLast     - Set to 1 when file has been completely
 *                                 read
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success
 *                  any other MC_STATUS value on failure.
 *
 *  Description :   Callback function used by MC_Open_File to read a file
 *                  in the DICOM Part 10 (media) format.
 *
 ****************************************************************************/
int GetWorkBufferSize()
{
    MC_STATUS mcStatus = MC_NORMAL_COMPLETION;
    int length = 0;

    mcStatus = MC_Get_Int_Config_Value(WORK_BUFFER_SIZE, &length);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        length = 64 * 1024;
    }
    return length;
}

bool AllocateBuffer(CBinfo*& callbackInfo)
{
    if (callbackInfo->bufferLength == 0)
    {

        int length = GetWorkBufferSize();

        callbackInfo->bufferLength = length;
    }

    callbackInfo->buffer = (char*)(malloc(callbackInfo->bufferLength));
    if (callbackInfo->buffer == NULL)
    {
        printf("Error: failed to allocate file read buffer [%d] kb", (int)callbackInfo->bufferLength);
        return false;
    }
    return true;
}

void checkIfBufferSet(int retStatus)
{
    if (retStatus != 0)
    {
        printf("WARNING:  Unable to set IO buffering on input file.\n");
    }
}

bool firstCallProcedure(char*& A_filename, CBinfo*& callbackInfo, int& retStatus, int& A_isFirst)
{
    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
        callbackInfo->fp = fopen(A_filename, BINARY_READ);

        retStatus = setvbuf(callbackInfo->fp, (char*)NULL, _IOFBF, 32768);
        checkIfBufferSet(retStatus);

        if (AllocateBuffer(callbackInfo) == false)
            return false;

    }
}

bool SetBuffer(char*& A_filename, CBinfo*& callbackInfo, int& retStatus, int& A_isFirst, void* A_userInfo)
{
    if (!A_userInfo)
        return false;

    if (firstCallProcedure(A_filename, callbackInfo, retStatus, A_isFirst) == false)
    {
        return false;
    }

    return true;
}

bool closeCallBackFile(CBinfo*& callbackInfo, int*& A_isLast)
{
    if (ferror(callbackInfo->fp))
    {
        free(callbackInfo->buffer);
        callbackInfo->buffer = NULL;

        return false;
    }


    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;

        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    return true;
}

bool ReadInCallBackFile(CBinfo*& callbackInfo, size_t& bytes_read, int*& A_isLast)
{
    if (!callbackInfo->fp)
        return MC_CANNOT_COMPLY;

    bytes_read = fread(callbackInfo->buffer, 1, callbackInfo->bufferLength, callbackInfo->fp);

    if (closeCallBackFile(callbackInfo, A_isLast) == false)
        return false;

    return true;
}

static MC_STATUS NOEXP_FUNC MediaToFileObj(char* A_filename,
    void* A_userInfo,
    int* A_dataSize,
    void** A_dataBuffer,
    int       A_isFirst,
    int* A_isLast)
{

    CBinfo* callbackInfo = (CBinfo*)A_userInfo;
    size_t          bytes_read;
    int             retStatus;

    if (SetBuffer(A_filename, callbackInfo, retStatus, A_isFirst, A_userInfo) == false)
        return MC_CANNOT_COMPLY;

    if (ReadInCallBackFile(callbackInfo, bytes_read, A_isLast) == false)
        return MC_CANNOT_COMPLY;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytes_read;
    callbackInfo->bytesRead += bytes_read;

    return MC_NORMAL_COMPLETION;

} /* MediaToFileObj() */


/****************************************************************************
 *
 *  Function    :    CheckFileFormat
 *
 *  Parameters  :    Afilename      file name of the image which is being
 *                                  checked for a format.
 *
 *  Returns     :    FORMAT_ENUM    enumeration of possible return values
 *
 *  Description :    Tries to determing the messages transfer syntax.
 *                   This function is not fool proof!  It is mainly
 *                   useful for testing puposes, and as an example as
 *                   to how to determine an images format.  This code
 *                   should probably not be used in production equipment,
 *                   unless the format of objects is known ahead of time,
 *                   and it is guarenteed that this algorithm works on those
 *                   objects.
 *
 ****************************************************************************/

FORMAT_ENUM CheckSignatureOfMediaFile(FILE*& fp)
{
    //Associated with CheckFileFormat checks signature of Media file 
    char signature[5] = "\0\0\0\0";

    /*
     * Read the signature, only 4 bytes
     */
    if (fread(signature, 1, 4, fp) == 4)
    {
        /*
         * if it is the signature, return true.  The file is
         * definately in the DICOM Part 10 format.
         */
        if (!strcmp(signature, "DICM"))
        {
            fclose(fp);
            return MEDIA_FORMAT;
        }
    }

    return UNKNOWN_FORMAT;

}

static FORMAT_ENUM CheckFileFormat(char* A_filename)
{
    FILE* fp;

    //char             vR[3] = "\0\0";

    /*
    union
    {
        unsigned short   groupNumber;
        char      i[2];
    } group;

    unsigned short   elementNumber;

    union
    {
        unsigned short  shorterValueLength;
        char      i[2];
    } aint;

    union
    {
        unsigned long   valueLength;
        char      l[4];
    } along;

    */
    if ((fp = fopen(A_filename, BINARY_READ)) != NULL)
    {
        if (fseek(fp, 128, SEEK_SET) == 0)
        {
            return CheckSignatureOfMediaFile(fp);
        }

    }
    /*
     * Now try and determine the format if it is not media
     */

    return UNKNOWN_FORMAT;
} /* CheckFileFormat() */
