#include "Definitions.h"

/****************************************************************************
 *
 *  Function    :   SendImage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered
 *                  A_node     - The node in our list of instances
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE on failure where association must be aborted
 *
 *  Description :   Send message containing the image in the node over
 *                  the association.
 *
 *                  SAMP_TRUE is returned on success, or when a recoverable
 *                  error occurs.
 *
 ****************************************************************************/


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

bool setServiceAndSOP(InstanceNode* A_node)
{
    MC_STATUS mcStatus;
    mcStatus = MC_Set_Service_Command(A_node->msgID, A_node->serviceName, C_STORE_RQ);
    if (CheckIfMCStatusNotOk(mcStatus, "MC_Set_Service_Command failed"))
    {
        return true;
    }
    mcStatus = MC_Set_Value_From_String(A_node->msgID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, A_node->SOPInstanceUID);
    if (CheckIfMCStatusNotOk(mcStatus, "MC_Set_Value_From_String failed for affected SOP Instance UID"))
    {
        return true;
    }
    return false;
}
bool GetSOPUIDAndSetService(InstanceNode* A_node)
{    //Gives Error MC_Release_Application failed: Application ID parameter is invalid
    MC_STATUS mcStatus;
    mcStatus = MC_Get_MergeCOM_Service(A_node->SOPClassUID, A_node->serviceName, sizeof(A_node->serviceName));

    if (CheckIfMCStatusNotOk(mcStatus, "MC_Get_MergeCOM_Service failed"))
    {
        return true;
    }
    if (setServiceAndSOP(A_node))
    {
        return true;
    }

    return false;

}

bool checkSendRequestMessage(MC_STATUS mcStatus, InstanceNode*& A_node)
{
    std::map<MC_STATUS, string> SendRequestMap;

    SendRequestMap[MC_ASSOCIATION_ABORTED] = "MC_Send_Request_Message failed";
    SendRequestMap[MC_SYSTEM_ERROR] = "MC_Send_Request_Message failed";
    SendRequestMap[MC_UNACCEPTABLE_SERVICE] = "MC_Send_Request_Message failed";


    if (SendRequestMap.find(mcStatus) != SendRequestMap.end()) {
        //CheckIfMCStatusNotOk(mcStatus, "MC_Send_Request_Message failed");
        return CheckIfMCStatusNotOk(mcStatus, "MC_Send_Request_Message failed");;
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /* This is a failure condition we can continue with */

        CheckIfMCStatusNotOk(mcStatus, "Warning: MC_Send_Request_Message failed");
        return (false);
    }

    A_node->imageSent = SAMP_TRUE;
    fflush(stdout);
    return false;
}

SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node)
{
    MC_STATUS       mcStatus;

    A_node->imageSent = SAMP_FALSE;

    /* Get the SOP class UID and set the service */

    /* set affected SOP Instance UID */

    if (GetSOPUIDAndSetService(A_node))
    {
        return SAMP_TRUE;
    }

    /*
     *  Send the message
     */
    printf("     File: %s\n", A_node->fname);
    printf("   Format: DICOM Part 10 Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));
    printf("SOP Class: %s (%s)\n", A_node->SOPClassUID, A_node->serviceName);
    printf("      UID: %s\n", A_node->SOPInstanceUID);
    printf("     Size: %lu bytes\n", (unsigned long)A_node->imageBytes);


    mcStatus = MC_Send_Request_Message(A_associationID, A_node->msgID);

    if (checkSendRequestMessage(mcStatus, A_node))
        return (SAMP_FALSE);

    return (SAMP_TRUE);
}
