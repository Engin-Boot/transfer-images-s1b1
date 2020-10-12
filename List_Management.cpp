#include "Definitions.h"


/****************************************************************************
 *
 *  Function    :   AddFileToList
 *
 *  Parameters  :   A_list     - List of nodes.
 *                  A_fname    - The name of file to add to the list
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Create a node in the instance list for a file to be sent
 *                  on this association.  The node is added to the end of the
 *                  list.
 *
 ****************************************************************************/
SAMP_BOOLEAN AddFileToList(InstanceNode** A_list, char* A_fname)
{
    InstanceNode* newNode;

    newNode = (InstanceNode*)malloc(sizeof(InstanceNode));
    if (!newNode)
    {
        PrintError("Unable to allocate object to store instance information", MC_NORMAL_COMPLETION);
        return (SAMP_FALSE);
    }

    memset(newNode, 0, sizeof(InstanceNode));

    strncpy(newNode->fname, A_fname, sizeof(newNode->fname));
    newNode->fname[sizeof(newNode->fname) - 1] = '\0';

    newNode->responseReceived = SAMP_FALSE;
    newNode->failedResponse = SAMP_FALSE;
    newNode->imageSent = SAMP_FALSE;
    newNode->msgID = -1;
    newNode->transferSyntax = IMPLICIT_LITTLE_ENDIAN;
    list_updation(A_list, newNode);

    return (SAMP_TRUE);
}
static void list_updation(InstanceNode** A_list, InstanceNode* newNode)
{
    InstanceNode* listNode;
    if (!*A_list)
    {
        /*
         * Nothing in the list
         */
        newNode->Next = *A_list;
        *A_list = newNode;
    }
    else
    {
        /*
         * Add to the tail of the list
         */
        listNode = *A_list;

        while (listNode->Next)
            listNode = listNode->Next;

        listNode->Next = newNode;
    }
}

/****************************************************************************
 *
 *  Function    :   UpdateNode
 *
 *  Parameters  :   A_node     - node to update
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Update an image node with info about a file transferred
 *
 ****************************************************************************/
SAMP_BOOLEAN UpdateNode(InstanceNode* A_node)
{
    MC_STATUS        mcStatus;

    /*
     * Get DICOM msgID for tracking of responses
     */
    mcStatus = MC_Get_Value_To_UInt(A_node->msgID, MC_ATT_MESSAGE_ID, &(A_node->dicomMsgID));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_UInt for Message ID failed", mcStatus);
        A_node->responseReceived = SAMP_TRUE;
        return(SAMP_FALSE);
    }

    A_node->responseReceived = SAMP_FALSE;
    A_node->failedResponse = SAMP_FALSE;
    A_node->imageSent = SAMP_TRUE;

    return (SAMP_TRUE);
}


/****************************************************************************
 *
 *  Function    :   FreeList
 *
 *  Parameters  :   A_list     - Pointer to head of node list to free.
 *
 *  Returns     :   nothing
 *
 *  Description :   Free the memory allocated for a list of nodesransferred
 *
 ****************************************************************************/
void FreeList(InstanceNode** A_list)
{
    InstanceNode* node;

    /*
     * Free the instance list
     */
    while (*A_list)
    {
        node = *A_list;
        *A_list = node->Next;

        if (node->msgID != -1)
            MC_Free_Message(&node->msgID);

        free(node);
    }
}


/****************************************************************************
 *
 *  Function    :   GetNumNodes
 *
 *  Parameters  :   A_list     - Pointer to head of node list to get count for
 *
 *  Returns     :   int, num node entries in list
 *
 *  Description :   Gets a count of the current list of instances.
 *
 ****************************************************************************/
int GetNumNodes(InstanceNode* A_list)

{
    int            numNodes = 0;
    InstanceNode* node;

    node = A_list;
    while (node)
    {
        numNodes++;
        node = node->Next;
    }

    return numNodes;
}
