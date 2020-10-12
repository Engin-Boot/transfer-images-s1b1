#include "Definitions.h"

bool mainclass::InitializeApplication()
{

    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */
    mcStatus = MC_Library_Initialization(NULL, NULL, NULL);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library", mcStatus);
        return (false);
    }

    /*
     *  Register this DICOM application
     */
    cout << "***ApplicationID" << applicationID << endl;
    cout << "options.localAE" << options.LocalAE << endl;
    mcStatus = MC_Register_Application(&applicationID, options.LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to register \"%s\":\n", options.LocalAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        fflush(stdout);
        return(false);
    }
    cout << "***ApplicationID" << applicationID << endl;
    cout << "options.localAE" << options.LocalAE << endl;
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
    fstatus = fscanf(fp, "%s", fname);
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
    char* remoteHostName = mainclass::checkRemoteHostName(options.RemoteHostname);
    char* ServiceList = mainclass::checkServiceList(options.ServiceList);

    return MC_Open_Association(applicationID,
        &associationID,
        options.RemoteAE,
        options.RemotePort != -1 ? &options.RemotePort : NULL,
        //options.RemoteHostname[0] ? options.RemoteHostname : NULL,
        remoteHostName,
        //options.ServiceList[0] ? options.ServiceList : NULL);
        ServiceList);

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

        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
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
     *
     * Because SendImage may not have actually sent an image even though it has returned success,
     * the calculation of performance data below may not be correct.
     */


     /*
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
        //imageStartTime = GetIntervalStart();

        if (ImageTransfer() == false)
        {
            break;
        }


    }   /* END for loop for each image */

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

    /*
     * Calculate the transfer rate.  Note, for a real performance
     * numbers, a function other than time() to measure elapsed
     * time should be used.
     */
    if (options.Verbose)
    {
        printf("Association Closed.\n");
    }

    // seconds = GetIntervalElapsed(startTime);

    printf("Data Transferred: %luMB\n", (unsigned long)(totalBytesRead / (1024 * 1024)));
    //printf("    Time Elapsed: %.3fs\n", seconds);
    //printf("   Transfer Rate: %.1fKB/s\n", ((float)totalBytesRead / seconds) / 1024.0);
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
