#include "Definitions.h"
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
        printf("Message ID Being Responded To tag does not match message sent over association: %u\n", dicomMsgID);
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
void checkForSuccessResponse(unsigned int *A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    if (A_status != (unsigned int*)C_STORE_SUCCESS)
    {
        return;
    }
    strncpy(A_statusMeaning, "C-STORE Success.", A_statusMeaningLength);

}

void checkForElementCoercionWarningResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    if (*A_status != C_STORE_WARNING_ELEMENT_COERCION)
    {
        return;
    }
    strncpy(A_statusMeaning, "Warning: Element Coersion... Continuing.", A_statusMeaningLength);
}

void checkForInvalidDatasetWarningResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    if (*A_status != C_STORE_WARNING_INVALID_DATASET)
    {
        return;
    }
    strncpy(A_statusMeaning, "Warning: Invalid Dataset... Continuing.", A_statusMeaningLength);
}

void checkForElementsDiscardedWarningResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    if (*A_status != C_STORE_WARNING_ELEMENTS_DISCARDED)
    {
        return;
    }
    strncpy(A_statusMeaning, "Warning: Elements Discarded... Continuing.", A_statusMeaningLength);
}

void checkForRefusedErrorResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    SAMP_BOOLEAN returnBool = SAMP_TRUE;
    if (*A_status != C_STORE_FAILURE_REFUSED_NO_RESOURCES)
    {
        return;
    }
    strncpy(A_statusMeaning, "ERROR: REFUSED, NO RESOURCES.  ASSOCIATION ABORTING.", A_statusMeaningLength);
    returnBool = SAMP_FALSE;
}

void checkForInvalidDatasetErrorResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    SAMP_BOOLEAN returnBool = SAMP_TRUE;
    if (*A_status != C_STORE_FAILURE_INVALID_DATASET)
    {
        return;
    }
    strncpy(A_statusMeaning, "ERROR: INVALID_DATASET.  ASSOCIATION ABORTING.", A_statusMeaningLength);
    returnBool = SAMP_FALSE;
}

void checkForUnUnderstandableErrorResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    SAMP_BOOLEAN returnBool = SAMP_TRUE;
    if (*A_status != C_STORE_FAILURE_CANNOT_UNDERSTAND)
    {
        return;
    }
    strncpy(A_statusMeaning, "ERROR: CANNOT UNDERSTAND.  ASSOCIATION ABORTING.", A_statusMeaningLength);
    returnBool = SAMP_FALSE;
}

void checkForprocessingFailureErrorResponse(unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength)
{
    SAMP_BOOLEAN returnBool = SAMP_TRUE;
    if (*A_status != C_STORE_FAILURE_PROCESSING_FAILURE)
    {
        return;
    }
    strncpy(A_statusMeaning, "ERROR: PROCESSING FAILURE.  ASSOCIATION ABORTING.", A_statusMeaningLength);
    returnBool = SAMP_FALSE;
}

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

    checkForSuccessResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForUnUnderstandableErrorResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForInvalidDatasetWarningResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForElementsDiscardedWarningResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForRefusedErrorResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForInvalidDatasetErrorResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForUnUnderstandableErrorResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    checkForprocessingFailureErrorResponse(A_status, A_statusMeaning, A_statusMeaningLength);
    fflush(stdout);

    return returnBool;
}
