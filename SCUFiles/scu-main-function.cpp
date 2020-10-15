#include "Definitions.h"

bool mainclass::InitializeApplication()
{
    //Essential Initialization
    mcStatus = MC_Library_Initialization(NULL, NULL, NULL);
    if (CheckIfMCStatusNotOk(mcStatus, "Unable to initialize library"))
    {
        return (false);
    }

    /*
     *  Register this DICOM application
     */
  
    mcStatus = MC_Register_Application(&applicationID, options.LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to register \"%s\":\n", options.LocalAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        fflush(stdout);
        return(false);
    }
    return mainclass::InitializeList();
}

bool mainclass::InitializeList()
{
    if (options.UseFileList)
    {
        /* Read the command line file to create the list */
        fp = fopen(options.FileList, TEXT_READ);
        if (!fp)
        {
            printf("ERROR: Unable to open %s.\n", options.FileList);
            fflush(stdout);
            return(false);
        }
        mainclass::ReadFileByFILENAME();
    }
    else
    {
        /* Traverse through the possible names and add them to the list based on the start/stop count */
        mainclass::ReadFileFromStartStopPosition();
    }
    totalImages = GetNumNodes(instanceList);
    mainclass::VerboseBeforeConnection();
    return mainclass::CreateAssociation();
}

void mainclass::ReadFileByFILENAME()
{
    fstatus = fscanf(fp, "%512s", fname);
    while (fstatus != EOF && fstatus != 0)
    {
        ReadEachLineInFile();
    }
    fclose(fp);
}
void mainclass::ReadEachLineInFile()
{
    if (fname[0] == '#') /* skip commented out rows */
    {
        return;
    }
    sampBool = AddFileToList(&instanceList, fname);
    if (!sampBool)
    {
        printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
    }
}
void mainclass::ReadFileFromStartStopPosition()
{
    for (imageCurrent = options.StartImage; imageCurrent <= options.StopImage; imageCurrent++)
    {
        sprintf(fname, "%d.img", imageCurrent);
        sampBool = AddFileToList(&instanceList, fname);
        if (!sampBool)
        {
            printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
        }
    }
}
char* mainclass::checkRemoteHostName(char* RemoteHostName)
{
    if (RemoteHostName[0])
        return RemoteHostName;
    else
        return NULL;
}
char* mainclass::checkServiceList(char* ServiceList)
{
    if (ServiceList[0])
        return ServiceList;
    else
        return NULL;
}

MC_STATUS mainclass::OpenAssociation()
{
    char* tempremoteHostName = mainclass::checkRemoteHostName(options.RemoteHostname);
    char* tempServiceList = mainclass::checkServiceList(options.ServiceList);

    return MC_Open_Association(applicationID,
        &associationID,
        options.RemoteAE,
        options.RemotePort != -1 ? &options.RemotePort : NULL,
        tempremoteHostName,
        tempServiceList);
}
bool mainclass::CreateAssociation()
{
    mcStatus = mainclass::OpenAssociation();
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", options.RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        fflush(stdout);
        return(false);
    }

    mcStatus = MC_Get_Association_Info(associationID, &options.asscInfo);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Association_Info failed", mcStatus);
    }
    mainclass::VerboseAfterConnection();
    return true;
}
void mainclass::RemoteVerbose()
{
    if (options.RemoteHostname[0])
        printf("    Hostname: %s\n", options.RemoteHostname);
    else
        printf("    Hostname: Default in mergecom.app\n");

    if (options.RemotePort != -1)
        printf("        Port: %d\n", options.RemotePort);
    else
        printf("        Port: Default in mergecom.app\n");
}
void mainclass::VerboseBeforeConnection()
{
    printf("Opening connection to remote system:\n");
    printf("    AE title: %s\n", options.RemoteAE);
    RemoteVerbose();
    if (options.ServiceList[0])
        printf("Service List: %s\n", options.ServiceList);
    else
        printf("Service List: Default in mergecom.app\n");

    printf("   Files to Send: %d \n", totalImages);

}

void mainclass::VerboseAfterConnection()
{

    if (options.Verbose)
    {
        printf("Connecting to Remote Application:\n");
        printf("  Remote AE Title:          %s\n", options.asscInfo.RemoteApplicationTitle);
        printf("  Local AE Title:           %s\n", options.asscInfo.LocalApplicationTitle);
        printf("  Host name:                %s\n", options.asscInfo.RemoteHostName);
        printf("  IP Address:               %s\n", options.asscInfo.RemoteIPAddress);
        printf("  Local Max PDU Size:       %lu\n", options.asscInfo.LocalMaximumPDUSize);
        printf("  Remote Max PDU Size:      %lu\n", options.asscInfo.RemoteMaximumPDUSize);
        printf("  Max operations invoked:   %u\n", options.asscInfo.MaxOperationsInvoked);
        printf("  Max operations performed: %u\n", options.asscInfo.MaxOperationsPerformed);
        printf("  Implementation Version:   %s\n", options.asscInfo.RemoteImplementationVersion);
        printf("  Implementation Class UID: %s\n", options.asscInfo.RemoteImplementationClassUID);

        /*
         * Print out User Identity information if negotiated
         */
        printf("  User Identity type:       None\n\n\n");
        printf("Services and transfer syntaxes negotiated:\n");

        mainclass::VerboseTransferSyntax();
    }
    else
        printf("Connected to remote system [%s]\n\n", options.RemoteAE);
}
void mainclass::VerboseTransferSyntax()
{

    mcStatus = MC_Get_First_Acceptable_Service(associationID, &servInfo);
    while (mcStatus == MC_NORMAL_COMPLETION)
    {
        printf("  %-30s: %s\n", servInfo.ServiceName, GetSyntaxDescription(servInfo.SyntaxType));
        mcStatus = MC_Get_Next_Acceptable_Service(associationID, &servInfo);
    }

    if (mcStatus != MC_END_OF_LIST)
    {
        PrintError("Warning: Unable to get service info", mcStatus);
    }
    printf("\n\n");
}

void mainclass::UpdateImageSentCount()
{

    if (node->imageSent == SAMP_TRUE)
    {
        imagesSent++;
    }
    else
    {
        node->responseReceived = SAMP_TRUE;
        node->failedResponse = SAMP_TRUE;
    }

    mcStatus = MC_Free_Message(&node->msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for request message", mcStatus);
    }   
}
bool mainclass::SendImageAndUpdateNode()
{
    sampBool = SendImage(&options, associationID, node);
    if (!sampBool)
    {
        node->imageSent = SAMP_FALSE;
        printf("Failure in sending file [%s]\n", node->fname);
        MC_Abort_Association(&associationID);
        MC_Release_Application(&applicationID);
        return false;
    }
    sampBool = UpdateNode(node);
    if (!sampBool)
    {
        printf("Warning, unable to update node with information [%s]\n", node->fname);

        MC_Abort_Association(&associationID);
        MC_Release_Application(&applicationID);
        return false;
    }
    UpdateImageSentCount();
    sampBool = ReadResponseMessages(&options, associationID, 0, &instanceList, NULL);
    if (!sampBool)
    {
        printf("Failure in reading response message, aborting association.\n");

        MC_Abort_Association(&associationID);
        MC_Release_Application(&applicationID);
        return false;
    }
    if (options.asscInfo.MaxOperationsInvoked > 0)
    {
        while (GetNumOutstandingRequests(instanceList) >= options.asscInfo.MaxOperationsInvoked)
        {
            sampBool = ReadResponseMessages(&options, associationID, 10, &instanceList, NULL);
            if (!sampBool)
            {
                printf("Failure in reading response message, aborting association.\n");
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }
        }
    }
    return true;

}
bool mainclass::ImageTransfer()
{
    /*
        * Determine the image format and read the image in.  If the
        * image is in the part 10 format, convert it into a message.
        */
    sampBool = ReadImage(&options, applicationID, node);
    if (!sampBool)
    {
        node->imageSent = SAMP_FALSE;
        printf("Can not open image file [%s]\n", node->fname);
        node = node->Next;
        return true;
    }

    totalBytesRead += node->imageBytes;

    /*
     * Send image read in with ReadImage.
     * Save image transfer information in list
     */
    if (SendImageAndUpdateNode() == false)
        return false;
    /*
     * Traverse through file list
     */
    node = node->Next;
    return true;
}
void mainclass::StartSendImage()
{
    node = instanceList;
    while (node)
    {
        if (ImageTransfer() == false)
        {
            break;
        }
        while ( GetNumOutstandingRequests( instanceList ) > 0 )
    {
        sampBool = ReadResponseMessages( &options, associationID, 10, &instanceList, NULL );
        if (!sampBool)
        {
            printf("Failure in reading response message, aborting association.\n");
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            break;
        }
    }
    } 
}

void mainclass::CloseAssociation()
{
    /*
    * A failure on close has no real recovery.  Abort the association
    * and continue on.
    */

    mcStatus = MC_Close_Association(&associationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Close association failed", mcStatus);
        MC_Abort_Association(&associationID);
    }

    if (options.Verbose)
    {
        printf("Association Closed.\n");
    }
    printf("Data Transferred: %luMB\n", (unsigned long)(totalBytesRead / (1024 * 1024)));
    fflush(stdout);
}

void mainclass::ReleaseApplication()
{
    MC_STATUS mcStatus;
    mcStatus = MC_Release_Application(&applicationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Release_Application failed", mcStatus);
    }

    /*
     * Free the node list's allocated memory
     */
    FreeList(&instanceList);

    /*
     * Release all memory used by the Merge DICOM Toolkit.
     */
    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");
}

/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
void PrintError(const char* A_string, MC_STATUS A_status)
{
    char        prefix[30] = { 0 };
    /*
     *  Need process ID number for messages
     */
#ifdef UNIX
    sprintf(prefix, "PID %d", getpid());
#endif
    if (A_status == -1)
    {
        printf("%s\t%s\n", prefix, A_string);
    }
    else
    {
        printf("%s\t%s:\n", prefix, A_string);
        printf("%s\t\t%s\n", prefix, MC_Error_Message(A_status));
    }
}


bool CheckIfMCStatusNotOk(MC_STATUS mcStatus, const char* ErrorMessage)
{
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError(ErrorMessage, mcStatus);
        fflush(stdout);
        return true;
    }
    return false;
}



/****************************************************************************
 *
 *  Function    :   GetNumOutstandingRequests
 *
 *  Parameters  :   A_list     - Pointer to head of node list to get count for
 *
 *  Returns     :   int, num messages we're waiting for c-store responses for
 *
 *  Description :   Checks the list of instances sent over the association &
 *                  returns the number of responses we're waiting for.
 *
 ****************************************************************************/
int GetNumOutstandingRequests(InstanceNode* A_list)

{
    int            outstandingResponseMsgs = 0;
    InstanceNode* node;

    node = A_list;
    while (node)
    {
        if ((node->imageSent == SAMP_TRUE) && (node->responseReceived == SAMP_FALSE))
            outstandingResponseMsgs++;

        node = node->Next;
    }
    return outstandingResponseMsgs;
}

/****************************************************************************
 *
 *  Function    :   ReadResponseMessages
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered
 *                  A_timeout  -
 *                  A_list     - List of nodes among which to identify
 *                               the message for the request with which
 *                               the response is associated.
 *                  A_node     - A node in our list of instances.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE on failure where association must be aborted
 *
 *  Description :   Read the response message and determine which request
 *                  message is being responded to.
 *
 *                  SAMP_TRUE is returned on success, or when a recoverable
 *                  error occurs.
 *
 ****************************************************************************/
SAMP_BOOLEAN ReadResponseMessages(STORAGE_OPTIONS* A_options, int A_associationID, int A_timeout, InstanceNode** A_list, InstanceNode* A_node)
{
    MC_STATUS       mcStatus;
    SAMP_BOOLEAN    sampBool;
    int             responseMessageID;
    char* responseService;
    MC_COMMAND      responseCommand;
    static char     affectedSOPinstance[UI_LENGTH + 2];
    unsigned int    dicomMsgID;
    InstanceNode* node = (InstanceNode*)A_node;

    /*
     *  Wait for response
     */
    mcStatus = MC_Read_Message(A_associationID, A_timeout, &responseMessageID, &responseService, &responseCommand);
    if (mcStatus == MC_TIMEOUT)
        return (SAMP_TRUE);

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed", mcStatus);
        fflush(stdout);
        return (SAMP_FALSE);
    }

    mcStatus = MC_Get_Value_To_UInt(responseMessageID, MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO, &dicomMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_UInt for Message ID Being Responded To failed.  Unable to process response message.", mcStatus);
        fflush(stdout);
        return(SAMP_TRUE);
    }

    mcStatus = MC_Get_Value_To_String(responseMessageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, sizeof(affectedSOPinstance), affectedSOPinstance);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for affected SOP instance failed.  Unable to process response message.", mcStatus);
        fflush(stdout);
        return(SAMP_TRUE);
    }

    if (!A_options->StreamMode)
    {
        node = *A_list;
        while (node)
        {
            if (node->dicomMsgID == dicomMsgID)
            {
                if (!strcmp(affectedSOPinstance, node->SOPInstanceUID))
                {
                    break;
                }
            }
            node = node->Next;
        }
    }

    if (!node)
    {
        printf("Message ID Being Responded To tag does not match message sent over association: %d\n", dicomMsgID);
        MC_Free_Message(&responseMessageID);
        fflush(stdout);
        return (SAMP_TRUE);
    }

    node->responseReceived = SAMP_TRUE;

    sampBool = CheckResponseMessage(responseMessageID, &node->status, node->statusMeaning, sizeof(node->statusMeaning));
    if (!sampBool)
    {
        node->failedResponse = SAMP_TRUE;
    }

    if ((A_options->Verbose) || (node->status != C_STORE_SUCCESS))
        printf("   Status: %s\n", node->statusMeaning);

    node->failedResponse = SAMP_FALSE;

    mcStatus = MC_Free_Message(&responseMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for response message", mcStatus);
        fflush(stdout);
        return (SAMP_TRUE);
    }
    fflush(stdout);
    return (SAMP_TRUE);
}


/****************************************************************************
 *
 *  Function    :   CheckResponseMessage
 *
 *  Parameters  :   A_responseMsgID  - The message ID of the response message
 *                                     for which we want to check the status
 *                                     tag.
 *                  A_status         - Return argument for the response status
 *                                     value.
 *                  A_statusMeaning  - The meaning of the response status is
 *                                     returned here.
 *                  A_statusMeaningLength - The size of the buffer at
 *                                     A_statusMeaning.
 *
 *  Returns     :   SAMP_TRUE on success or warning status
 *                  SAMP_FALSE on failure status
 *
 *  Description :   Examine the status tag in the response to see if we
 *                  the C-STORE-RQ was successfully received by the SCP.
 *
 ****************************************************************************/
SAMP_BOOLEAN CheckResponseMessage(int A_responseMsgID, unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    MC_STATUS mcStatus;
    SAMP_BOOLEAN returnBool = SAMP_TRUE;

    mcStatus = MC_Get_Value_To_UInt(A_responseMsgID, MC_ATT_STATUS, A_status);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /* Problem with MC_Get_Value_To_UInt */
        PrintError("MC_Get_Value_To_UInt for response status failed", mcStatus);
        strncpy(A_statusMeaning, "Unknown Status", A_statusMeaningLength);
        fflush(stdout);
        return SAMP_FALSE;
    }

    /* MC_Get_Value_To_UInt worked.  Check the response status */

    switch (*A_status)
    {
        /* Success! */
    case C_STORE_SUCCESS:
        strncpy(A_statusMeaning, "C-STORE Success.", A_statusMeaningLength);
        break;

        /* Warnings.  Continue execution. */

    case C_STORE_WARNING_ELEMENT_COERCION:
        strncpy(A_statusMeaning, "Warning: Element Coersion... Continuing.", A_statusMeaningLength);
        break;

    case C_STORE_WARNING_INVALID_DATASET:
        strncpy(A_statusMeaning, "Warning: Invalid Dataset... Continuing.", A_statusMeaningLength);
        break;

    case C_STORE_WARNING_ELEMENTS_DISCARDED:
        strncpy(A_statusMeaning, "Warning: Elements Discarded... Continuing.", A_statusMeaningLength);
        break;

        /* Errors.  Abort execution. */

    case C_STORE_FAILURE_REFUSED_NO_RESOURCES:
        strncpy(A_statusMeaning, "ERROR: REFUSED, NO RESOURCES.  ASSOCIATION ABORTING.", A_statusMeaningLength);
        returnBool = SAMP_FALSE;
        break;

    case C_STORE_FAILURE_INVALID_DATASET:
        strncpy(A_statusMeaning, "ERROR: INVALID_DATASET.  ASSOCIATION ABORTING.", A_statusMeaningLength);
        returnBool = SAMP_FALSE;
        break;

    case C_STORE_FAILURE_CANNOT_UNDERSTAND:
        strncpy(A_statusMeaning, "ERROR: CANNOT UNDERSTAND.  ASSOCIATION ABORTING.", A_statusMeaningLength);
        returnBool = SAMP_FALSE;
        break;

    case C_STORE_FAILURE_PROCESSING_FAILURE:
        strncpy(A_statusMeaning, "ERROR: PROCESSING FAILURE.  ASSOCIATION ABORTING.", A_statusMeaningLength);
        returnBool = SAMP_FALSE;
        break;

    default:
        sprintf(A_statusMeaning, "Warning: Unknown status (0x%04x)... Continuing.", *A_status);
        returnBool = SAMP_FALSE;
        break;
    }
    fflush(stdout);

    return returnBool;
}
