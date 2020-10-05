/*************************************************************************
 *
 *       System: Merge DICOM Toolkit
 *
 *       Author: Merge Healthcare
 *
 *  Description: This is a sample Service Class User application
 *               for the Storage Service Class and the Storage Commitment
 *               service class.  The application has a number of features:
 *               - It can read in images in both the DICOM Part 10 format
 *                 and the DICOM "stream" format.
 *               - The application determines the format of the object
 *                 before reading in.
 *               - The application supports DICOM "Asychronous Window
 *                 negotiation" and will transfer asychronously if
 *                 negotiated.
 *               - The AE Title, host name, and port number of the
 *                 system being connected to can be specified on the
 *                 command line.
 *               - A verbose mode can be specified where detailed
 *                 information is displayed as the application functions.
 *               - The local AE title can be specified on the command
 *                 line.
 *               - The service list (found in the mergecom.app
 *                 configuration file) used by the application to
 *                 determine what services are negotiated can be specified
 *                 on the command line.
 *               - The application will support DICOM Part 10 formated
 *                 compressed/encapsulated if specified on the command
 *                 line.  One note, however, the standard service lists
 *                 found in the mergecom.app file must be extended with
 *                 a transfer syntax list to support these transfer
 *                 syntaxes.
 *               - If specified on the command line, the application will
 *                 send a storage commitment request to the same SCP as
 *                 it is sending images.  The storage commitment request
 *                 will be for the images included on the command line.
 *
 *************************************************************************
 *
 *                      (c) 2012 Merge Healthcare
 *            900 Walnut Ridge Drive, Hartland, WI 53029
 *
 *                      -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/

/*
 * Standard OS Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <fcntl.h>
#endif

/*
 * Merge DICOM Toolkit Includes
 */
#include "mc3media.h"
#include "mc3msg.h"
#include "mergecom.h"
#include "diction.h"
#include "mc3services.h"
#include "mc3items.h"

#include "general_util.h"

/*
 * Module constants
 */

/* DICOM VR Lengths */
#define AE_LENGTH 16
#define UI_LENGTH 64
#define SVC_LENGTH 130
#define STR_LENGTH 100
#define WORK_SIZE (64*1024)

#define TIME_OUT 30

#if defined(_WIN32)
#define BINARY_READ "rb"
#define BINARY_WRITE "wb"
#define BINARY_APPEND "rb+"
#define BINARY_READ_APPEND "a+b"
#define BINARY_CREATE "w+b"
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#else
#define BINARY_READ "r"
#define BINARY_WRITE "w"
#define BINARY_APPEND "r+"
#define BINARY_READ_APPEND "a+"
#define BINARY_CREATE "w+"
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#endif

/*
 * Module type declarations
 */

/*
 * CBinfo is used to callback functions when reading in stream objects
 * and Part 10 format objects.
 */
typedef struct CALLBACKINFO
{
    FILE*   fp;
    char    fileName[512];
    /*
     * Note! The size of this buffer impacts toolkit performance.  
     *       Higher values in general should result in increased performance of reading files
     */
    size_t  bytesRead;
    size_t  bufferLength;

    char*   buffer;
} CBinfo;

/*
 * Structure to store local application information
 */
typedef struct stor_scu_options
{
    int     StartImage;
    int     StopImage;
    int     ListenPort; /* for StorageCommit */
    int     RemotePort;

    char    RemoteAE[AE_LENGTH + 2];
    char    LocalAE[AE_LENGTH + 2];
    char    RemoteHostname[STR_LENGTH];
    char    ServiceList[SVC_LENGTH + 2];
    char    FileList[1024];
    char    Username[STR_LENGTH];
    char    Password[STR_LENGTH];

    SAMP_BOOLEAN UseFileList;
    SAMP_BOOLEAN Verbose;
    SAMP_BOOLEAN StorageCommit;
    SAMP_BOOLEAN ResponseRequested;
    SAMP_BOOLEAN StreamMode;

    AssocInfo       asscInfo;
} STORAGE_OPTIONS;


/*
 * Used to identify the format of an object
 */
typedef enum
{
    UNKNOWN_FORMAT = 0,
    MEDIA_FORMAT = 1,
    IMPLICIT_LITTLE_ENDIAN_FORMAT,
    IMPLICIT_BIG_ENDIAN_FORMAT,
    EXPLICIT_LITTLE_ENDIAN_FORMAT,
    EXPLICIT_BIG_ENDIAN_FORMAT
} FORMAT_ENUM;


/*
 * Structure to maintain list of instances sent & to be sent.
 * The structure keeps track of all instances and is used
 * in a linked list.
 */
typedef struct instance_node
{
    int    msgID;                       /* messageID of for this node */
    char   fname[1024];                 /* Name of file */
    TRANSFER_SYNTAX transferSyntax;     /* Transfer syntax of file */

    char   SOPClassUID[UI_LENGTH+2];    /* SOP Class UID of the file */
    char   serviceName[48];             /* Merge DICOM Toolkit service name for SOP Class */
    char   SOPInstanceUID[UI_LENGTH+2]; /* SOP Instance UID of the file */

    size_t       imageBytes;            /* size in bytes of the file */

    unsigned int dicomMsgID;            /* DICOM Message ID in group 0x0000 elements */
    unsigned int status;                /* DICOM status value returned for this file. */
    char   statusMeaning[STR_LENGTH];   /* Textual meaning of "status" */
    SAMP_BOOLEAN responseReceived;      /* Bool indicating we've received a response for a sent file */
    SAMP_BOOLEAN failedResponse;        /* Bool saying if a failure response message was received */
    SAMP_BOOLEAN imageSent;             /* Bool saying if the image has been sent over the association yet */
    SAMP_BOOLEAN mediaFormat;           /* Bool saying if the image was originally in media format (Part 10) */

    struct instance_node* Next;         /* Pointer to next node in list */

} InstanceNode;

/*
 *  Local Function prototypes
 */

int main(int argc,char** argv);

static SAMP_BOOLEAN TestCmdLine(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options );
static SAMP_BOOLEAN AddFileToList(InstanceNode** A_list, char* A_fname);
static SAMP_BOOLEAN UpdateNode(InstanceNode* A_node );
static void FreeList(InstanceNode** A_list );
static int GetNumNodes( InstanceNode* A_list);
static int GetNumOutstandingRequests(InstanceNode* A_list);
static SAMP_BOOLEAN StorageCommitment(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode** A_list);
static SAMP_BOOLEAN SetAndSendNActionMessage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode** A_list);
static SAMP_BOOLEAN HandleNEventAssociation(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode** A_list);
static SAMP_BOOLEAN ProcessNEventMessage(STORAGE_OPTIONS* A_options, int A_messageID, InstanceNode** A_list);
static SAMP_BOOLEAN CheckResponseMessage(int A_responseMsgID, unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength );
static FORMAT_ENUM CheckFileFormat(char* A_filename );
static SAMP_BOOLEAN ReadImage(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode* A_node);
static SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node);
static SAMP_BOOLEAN SendStream(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node);
static SAMP_BOOLEAN ReadResponseMessages(STORAGE_OPTIONS*  A_options, int A_associationID, int A_timeout, InstanceNode** A_list, InstanceNode* A_node);
static MC_STATUS NOEXP_FUNC MediaToFileObj(char* Afilename, void* AuserInfo, int* AdataSize, void** AdataBuffer, int AisFirst, int* AisLast);
static MC_STATUS NOEXP_FUNC StreamToMsgObj(int AmsgID, void* AcBinformation, int AfirstCall, int* AdataLen, void** AdataBuffer, int* AisLast);

static MC_STATUS NOEXP_FUNC FileToStream(   int AmsgID, 
                                            unsigned long Atag, 
                                            void* AuserInfo, 
                                            CALLBACK_TYPE CBtype, 
                                            unsigned long*  AdataSize,
                                            void**          AdataBuffer,
                                            int             AisFirst,
                                            int*            AisLast);
static char* Create_Inst_UID(void);
static void PrintError (char* A_string, MC_STATUS A_status);

static SAMP_BOOLEAN ReadFileFromMedia(  STORAGE_OPTIONS* A_options, 
                                        int A_appID, 
                                        char* A_filename, 
                                        int* A_msgID, 
                                        TRANSFER_SYNTAX* A_syntax, 
                                        size_t* A_bytesRead);

static SAMP_BOOLEAN ReadMessageFromFile(STORAGE_OPTIONS*    A_options,
                                        char*               A_fileName,
                                        FORMAT_ENUM         A_format,
                                        int*                A_msgID,
                                        TRANSFER_SYNTAX*    A_syntax,
                                        size_t*             A_bytesRead);

/****************************************************************************
 *
 *  Function    :   Main
 *
 *  Description :   Main routine for DICOM Storage Service Class SCU
 *
 ****************************************************************************/
int  main(int argc, char** argv);
int main(int argc, char** argv)
{
    SAMP_BOOLEAN            sampBool;
    STORAGE_OPTIONS         options;
    MC_STATUS               mcStatus;
    int                     applicationID = -1, associationID = -1, imageCurrent = 0;
    int                     imagesSent = 0L, totalImages = 0L, fstatus = 0;
    double                  seconds = 0.0;
    void                    *startTime = NULL, *imageStartTime = NULL;
    char                    fname[512] = {0};  /* Extra long, just in case */
    ServiceInfo             servInfo;
    size_t                  totalBytesRead = 0L;
    InstanceNode            *instanceList = NULL, *node = NULL;
    FILE*                   fp = NULL;

    /*
     * Test the command line parameters, and populate the options
     * structure with these parameters
     */
    sampBool = TestCmdLine( argc, argv, &options );
    if ( sampBool == SAMP_FALSE )
    {
        return(EXIT_FAILURE);
    }

    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */
    mcStatus = MC_Library_Initialization ( NULL, NULL, NULL );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library", mcStatus);
        return ( EXIT_FAILURE );
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
        return(EXIT_FAILURE);
    }

    /*
     * Create a linked list of all files to be transferred.
     * Retrieve the list from a specified file on the command line,
     * or generate the list from the start & stop numbers on the
     * command line
     */
    if (options.UseFileList)
    {
        /* Read the command line file to create the list */
        fp = fopen(options.FileList, TEXT_READ);
        if (!fp)
        {
            printf("ERROR: Unable to open %s.\n", options.FileList);
            fflush(stdout);
            return(EXIT_FAILURE);
        }

        for (;;) /* forever loop until break */
        {
            fstatus = fscanf(fp, "%s", fname);
            if (fstatus == EOF || fstatus == 0)
            {
                fclose(fp);
                break;
            }

            if (fname[0] == '#') /* skip commented out rows */
                continue;

            sampBool = AddFileToList( &instanceList, fname );
            if (!sampBool)
            {
                printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
            }
         }
    }
    else
    {
        /* Traverse through the possible names and add them to the list based on the start/stop count */
        for (imageCurrent = options.StartImage; imageCurrent <= options.StopImage; imageCurrent++)
        {
            sprintf(fname, "%d.img", imageCurrent);
            sampBool = AddFileToList( &instanceList, fname );
            if (!sampBool)
            {
                printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
            }
        }
    }

    totalImages = GetNumNodes( instanceList );

    if (options.Verbose)
    {
        printf("Opening connection to remote system:\n");
        printf("    AE title: %s\n", options.RemoteAE);
        if (options.RemoteHostname[0])
            printf("    Hostname: %s\n", options.RemoteHostname);
        else
            printf("    Hostname: Default in mergecom.app\n");

        if (options.RemotePort != -1)
            printf("        Port: %d\n", options.RemotePort);
        else
            printf("        Port: Default in mergecom.app\n");

        if (options.ServiceList[0])
            printf("Service List: %s\n", options.ServiceList);
        else
            printf("Service List: Default in mergecom.app\n");

        printf("   Files to Send: %d \n", totalImages);
    }

    startTime = GetIntervalStart();

    /*
     * Open the association with user identity information if specified
     * on the command line.  User Identity negotiation was defined
     * in DICOM Supplement 99.
     */
    if (strlen(options.Username))
    {
        USER_IDENTITY_TYPE identityType;
        if (strlen(options.Password))
            identityType = USERNAME_AND_PASSCODE;
        else
            identityType = USERNAME;

        /*
         *   Open association with user identity information and
         *   override hostname & port parameters if they were supplied on the command line.
         */
        mcStatus = MC_Open_Association_With_Identity(   applicationID, 
                                                        &associationID,
                                                        options.RemoteAE,
                                                        options.RemotePort != -1 ? &options.RemotePort : NULL,
                                                        options.RemoteHostname[0] ? options.RemoteHostname : NULL,
                                                        options.ServiceList[0] ? options.ServiceList : NULL,
                                                        NULL, /* Secure connections only */
                                                        NULL, /* Secure connections only */
                                                        identityType,
                                                        options.ResponseRequested ? POSITIVE_RESPONSE_REQUESTED : NO_RESPONSE_REQUESTED,
                                                        options.Username,
                                                        (unsigned short) strlen(options.Username),
                                                        identityType == USERNAME ? NULL : options.Password,
                                                        identityType == USERNAME ? 0 : (unsigned short) strlen(options.Password) );
    }
    else
        /*
         *   Open association and override hostname & port parameters if they were supplied on the command line.
         */
        mcStatus = MC_Open_Association( applicationID, 
                                        &associationID,
                                        options.RemoteAE,
                                        options.RemotePort != -1 ? &options.RemotePort : NULL,
                                        options.RemoteHostname[0] ? options.RemoteHostname : NULL,
                                        options.ServiceList[0] ? options.ServiceList : NULL );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", options.RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        fflush(stdout);
        return(EXIT_FAILURE);
    }

    mcStatus = MC_Get_Association_Info( associationID, &options.asscInfo);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Association_Info failed", mcStatus);
    }

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
        if (options.asscInfo.UserIdentityType == NO_USER_IDENTITY)
        {
            printf("  User Identity type:       None\n\n\n");
        }
        else
        {
            if (options.asscInfo.UserIdentityType == USERNAME)
                printf("  User Identity type:       Username\n");
            else if (options.asscInfo.UserIdentityType == USERNAME_AND_PASSCODE)
                printf("  User Identity type:       Username and Passcode\n");
            else if (options.asscInfo.UserIdentityType == KERBEROS_SERVICE_TICKET)
                printf("  User Identity type:       Kerberos Service Ticket\n");
            else if (options.asscInfo.UserIdentityType == SAML_ASSERTION)
                printf("  User Identity type:       SAML Assertion\n");
            else if (options.asscInfo.UserIdentityType == JSON_WEB_TOKEN)
                printf("  User Identity type:       JSON Web Token\n");
            if (options.asscInfo.PositiveResponseRequested)
            {
                printf("  Positive response requested: Yes\n");

                if (options.asscInfo.PositiveResponseReceived)
                    printf("  Positive response received: Yes\n\n\n");
                else
                    printf("  Positive response received: No\n\n\n");
            }
            else
                printf("  Positive response requested: No\n\n\n");
        }

        printf("Services and transfer syntaxes negotiated:\n");

        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
        mcStatus = MC_Get_First_Acceptable_Service(associationID,&servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
            printf("  %-30s: %s\n",servInfo.ServiceName, GetSyntaxDescription(servInfo.SyntaxType));
            mcStatus = MC_Get_Next_Acceptable_Service(associationID,&servInfo);
        }

        if (mcStatus != MC_END_OF_LIST)
        {
            PrintError("Warning: Unable to get service info",mcStatus);
        }
        printf("\n\n");
    }
    else
        printf("Connected to remote system [%s]\n\n", options.RemoteAE);

    /*
     * Check User Identity Negotiation and for response
     */
    if (options.ResponseRequested)
    {
        if (!options.asscInfo.PositiveResponseReceived)
        {
            printf("WARNING: Positive response for User Identity requested from\n\tserver, but not received.\n\n");
        }
    }
    
    fflush(stdout);

    /*
     *   Send all requested images.  Traverse through instanceList to
     *   get all files to send
     */
    node = instanceList;
    while ( node )
    {
        imageStartTime = GetIntervalStart();

        if (options.StreamMode)
        {
            /*
             * Send image in Stream mode.
             */            
            sampBool = SendStream( &options, associationID, node);
            if (!sampBool)
            {
                node->imageSent = SAMP_FALSE;
                printf("Failure in sending file [%s]\n", node->fname);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }
        }
        else
        {
            /*
            * Determine the image format and read the image in.  If the
            * image is in the part 10 format, convert it into a message.
            */
            sampBool = ReadImage( &options, applicationID, node);
            if (!sampBool)
            {
                node->imageSent = SAMP_FALSE;
                printf("Can not open image file [%s]\n", node->fname);
                node = node->Next;
                continue;
            }

            totalBytesRead += node->imageBytes;

            /*
             * Send image read in with ReadImage.
             *
             * Because SendImage may not have actually sent an image even though it has returned success, 
             * the calculation of performance data below may not be correct.
             */
            sampBool = SendImage( &options, associationID, node);
            if (!sampBool)
            {
                node->imageSent = SAMP_FALSE;
                printf("Failure in sending file [%s]\n", node->fname);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }

            /*
             * Save image transfer information in list
             */
            sampBool = UpdateNode( node );
            if (!sampBool)
            {
                printf("Warning, unable to update node with information [%s]\n", node->fname);

                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }
        }

        if ( node->imageSent == SAMP_TRUE )
        {
            imagesSent++;
        }
        else
        {
            node->responseReceived = SAMP_TRUE;
            node->failedResponse = SAMP_TRUE;
        }

        if (options.StreamMode)
        {
            sampBool = ReadResponseMessages( &options, associationID, 10, &instanceList, node );
            if (!sampBool)
            {
                printf("Failure in reading response message, aborting association.\n");
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }
        }
        else
        {
            mcStatus = MC_Free_Message(&node->msgID);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message failed for request message", mcStatus);
            }

            /*
            * The following is the core code for handling DICOM asynchronous
            * transfers.  With asynchronous communications, the SCU is allowed
            * to send multiple request messages to the server without
            * receiving a response message.  The MaxOperationsInvoked is
            * negotiated over the association, and determines how many request
            * messages the SCU can send before a response must be read.
            *
            * In this code, we first poll to see if a response message is
            * available.  This means data is readily available to be read over
            * the connection.  If there is no data to be read & asychronous
            * operations have been negotiated, and we haven't reached the max
            * number of operations invoked, we can just go ahead and send
            * the next request message.  If not, we go into the loop below
            * waiting for the response message from the server.
            *
            * This code alows network transfer speeds to improve.  Instead of
            * having to wait for a response message, the SCU can immediately
            * send the next request message so that the connection bandwidth
            * is better utilized.
            */
            sampBool = ReadResponseMessages( &options, associationID, 0, &instanceList, NULL );
            if (!sampBool)
            {
                printf("Failure in reading response message, aborting association.\n");

                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }

            /*
            * 0 for MaxOperationsInvoked means unlimited operations.  don't poll if this is the case, just
            * go to the next request to send.
            */
            if ( options.asscInfo.MaxOperationsInvoked > 0 )
            {
                while ( GetNumOutstandingRequests( instanceList ) >= options.asscInfo.MaxOperationsInvoked )
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

        seconds = GetIntervalElapsed(imageStartTime);
        if ( options.Verbose )
            printf("     Time: %.3f seconds\n\n", (float)seconds);
        else
            printf("\tSent %s image (%d of %d), elapsed time: %.3f seconds\n", node->serviceName, imagesSent, totalImages, seconds);

        /*
         * Traverse through file list
         */
        node = node->Next;

    }   /* END for loop for each image */

    /*
     * Wait for any remaining C-STORE-RSP messages.  This will only happen
     * when asynchronous communications are used.
     */
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
        printf("Association Closed.\n" );
    }

    seconds = GetIntervalElapsed(startTime);
    
    printf("Data Transferred: %luMB\n", (unsigned long) (totalBytesRead / (1024 * 1024)) );
    printf("    Time Elapsed: %.3fs\n", seconds);
    printf("   Transfer Rate: %.1fKB/s\n", ((float)totalBytesRead / seconds) / 1024.0);
    fflush(stdout);

    /*
     * Now, do Storage Commitment if app is configured to do it
     */
    if (options.StorageCommit == SAMP_TRUE)
    {
        /*
         * Set the port before calling MC_Wait_For_Association.
         */
        mcStatus = MC_Set_Int_Config_Value( TCPIP_LISTEN_PORT, options.ListenPort );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to set listen port, defaulting",mcStatus);
        }
        StorageCommitment( &options, applicationID, &instanceList );
    }

    /*
     * Release the dICOM Application
     */
    mcStatus = MC_Release_Application(&applicationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Release_Application failed", mcStatus);
    }

    /*
     * Free the node list's allocated memory
     */
    FreeList( &instanceList );

    /*
     * Release all memory used by the Merge DICOM Toolkit.
     */
    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");
    
    fflush(stdout);

    return(EXIT_SUCCESS);
}

/********************************************************************
 *
 *  Function    :   PrintCmdLine
 *
 *  Parameters  :   none
 *
 *  Returns     :   nothing
 *
 *  Description :   Prints program usage
 *
 ********************************************************************/
static void PrintCmdLine(void)
{
    printf("\nUsage stor_scu remote_ae start stop -f filename -a local_ae -b local_port -c -n remote_host -p remote_port -l service_list -v -u username -w password -q\n");
    printf("\n");
    printf("\t remote_ae       name of remote Application Entity Title to connect with\n");
    printf("\t start           start image number (not required if -f specified)\n");
    printf("\t stop            stop image number (not required if -f specified)\n");
    printf("\t -f filename     (optional) specify a file containing a list of images to transfer\n");
    printf("\t -a local_ae     (optional) specify the local Application Title (default: MERGE_STORE_SCU)\n");
    printf("\t -b local_port   (optional) specify the local TCP listen port for commitment (default: found in the mergecom.pro file)\n");
    printf("\t -c              Do storage commitment for the transferred files\n");
    printf("\t -n remote_host  (optional) specify the remote hostname (default: found in the mergecom.app file for remote_ae)\n");
    printf("\t -p remote_port  (optional) specify the remote TCP listen port (default: found in the mergecom.app file for remote_ae)\n");
    printf("\t -l service_list (optional) specify the service list to use when negotiating (default: Storage_SCU_Service_List)\n");
    printf("\t -s              Transfer the data using stream (raw) mode\n");
    printf("\t -v              Execute in verbose mode, print negotiation information\n");
    printf("\t -u username     (optional) specify a username to negotiate as defined in DICOM Supplement 99\n");
    printf("\t                 Note: just a username can be specified, or a username and password can be specified\n");
    printf("\t                       A password alone cannot be specified.\n");
    printf("\t -w password     (optional) specify a password to negotiate as defined in DICOM Supplement 99\n");
    printf("\t -q              Positive response to user identity requested from SCP\n");
    printf("\n");
    printf("\tImage files must be in the current directory if -f is not used.\n");
    printf("\tImage files must be named 0.img, 1.img, 2.img, etc if -f is not used.\n");

} /* end PrintCmdLine() */


/*************************************************************************
 *
 *  Function    :   TestCmdLine
 *
 *  Parameters  :   Aargc   - Command line arguement count
 *                  Aargv   - Command line arguements
 *                  A_options - Local application options read in.
 *
 *  Return value:   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Test command line for valid arguements.  If problems
 *                  are found, display a message and return SAMP_FALSE
 *
 *************************************************************************/
static SAMP_BOOLEAN TestCmdLine( int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options )
{
    int       i = 0, argCount=0;

    if (A_argc < 3)
    {
        PrintCmdLine();
        return SAMP_FALSE;
    }

    /*
     * Set default values
     */
    A_options->StartImage = 0;
    A_options->StopImage = 0;

    strcpy(A_options->LocalAE, "MERGE_STORE_SCU");
    strcpy(A_options->RemoteAE, "MERGE_STORE_SCP");
    strcpy(A_options->ServiceList, "Storage_SCU_Service_List");

    A_options->RemoteHostname[0] = '\0';
    A_options->RemotePort = -1;
    
    A_options->Verbose = SAMP_FALSE;
    A_options->StorageCommit = SAMP_FALSE;
    A_options->ListenPort = 1115;
    A_options->ResponseRequested = SAMP_FALSE;
    A_options->StreamMode = SAMP_FALSE;
    A_options->Username[0] = '\0';
    A_options->Password[0] = '\0';

    A_options->UseFileList = SAMP_FALSE;
    A_options->FileList[0] = '\0';

    /*
     * Loop through each arguement
     */
    for (i = 1; i < A_argc; i++)
    {
        if ( !strcmp(A_argv[i], "-h") || !strcmp(A_argv[i], "/h") ||
             !strcmp(A_argv[i], "-H") || !strcmp(A_argv[i], "/H") ||
             !strcmp(A_argv[i], "-?") || !strcmp(A_argv[i], "/?"))
        {
            PrintCmdLine();
            return SAMP_FALSE;
        }
        else if ( !strcmp(A_argv[i], "-a") || !strcmp(A_argv[i], "-A"))
        {
            /*
             * Set the Local AE
             */
            i++;
            strcpy(A_options->LocalAE, A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-b") || !strcmp(A_argv[i], "-B"))
        {
            /*
             * Local Port Number
             */
            i++;
            A_options->ListenPort = atoi(A_argv[i]);

        }
        else if ( !strcmp(A_argv[i], "-c") || !strcmp(A_argv[i], "-C"))
        {
            /*
             * StorageCommit mode
             */
            A_options->StorageCommit = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-f") || !strcmp(A_argv[i], "-F"))
        {
            /*
             * Config file with filenames
             */
            i++;
            A_options->UseFileList = SAMP_TRUE;
            strcpy(A_options->FileList,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-l") || !strcmp(A_argv[i], "-L"))
        {
            /*
             * Service List
             */
            i++;
            strcpy(A_options->ServiceList,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-n") || !strcmp(A_argv[i], "-N"))
        {
            /*
             * Remote Host Name
             */
            i++;
            strcpy(A_options->RemoteHostname,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-p") || !strcmp(A_argv[i], "-P"))
        {
            /*
             * Remote Port Number
             */
            i++;
            A_options->RemotePort = atoi(A_argv[i]);

        }
        else if ( !strcmp(A_argv[i], "-q") || !strcmp(A_argv[i], "-Q"))
        {
            /*
             * Positive response requested from server.
             */
            A_options->ResponseRequested = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-s") || !strcmp(A_argv[i], "-S"))
        {
            /*
             * Stream mode
             */
            A_options->StreamMode = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-u") || !strcmp(A_argv[i], "-U"))
        {
            /*
             * Username
             */
            i++;
            strcpy(A_options->Username,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-v") || !strcmp(A_argv[i], "-V"))
        {
            /*
             * Verbose mode
             */
            A_options->Verbose = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-w") || !strcmp(A_argv[i], "-W"))
        {
            /*
             * Username
             */
            i++;
            strcpy(A_options->Password,A_argv[i]);
        }
        else
        {
            /*
             * Parse through the rest of the options
             */

            argCount++;
            switch (argCount)
            {
                case 1:
                    strcpy(A_options->RemoteAE, A_argv[i]);
                    break;
                case 2:
                    A_options->StartImage = A_options->StopImage = atoi(A_argv[i]);
                    break;
                case 3:
                    A_options->StopImage = atoi(A_argv[i]);
                    break;
                default:
                    printf("Unkown option: %s\n",A_argv[i]);
                    break;
            }
        }
    }

    /*
     * If the hostname & port are specified on the command line,
     * the user may not have the remote system configured in the
     * mergecom.app file.  In this case, force the default service
     * list, so we can attempt to make a connection, or else we would
     * fail.
     */
    if ( A_options->RemoteHostname[0]
    &&  !A_options->ServiceList[0]
     && ( A_options->RemotePort != -1))
    {
        strcpy(A_options->ServiceList, "Storage_SCU_Service_List");
    }


    if (A_options->StopImage < A_options->StartImage)
    {
        printf("Image stop number must be greater than or equal to image start number.\n");
        PrintCmdLine();
        return SAMP_FALSE;
    }

    return SAMP_TRUE;

}/* TestCmdLine() */


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
static SAMP_BOOLEAN AddFileToList(InstanceNode** A_list, char* A_fname)
{
    InstanceNode*    newNode;
    InstanceNode*    listNode;

    newNode = (InstanceNode*)malloc(sizeof(InstanceNode));
    if (!newNode)
    {
        PrintError("Unable to allocate object to store instance information", MC_NORMAL_COMPLETION);
        return ( SAMP_FALSE );
    }

    memset( newNode, 0, sizeof(InstanceNode) );

    strncpy(newNode->fname, A_fname, sizeof(newNode->fname));
    newNode->fname[sizeof(newNode->fname)-1] = '\0';

    newNode->responseReceived = SAMP_FALSE;
    newNode->failedResponse = SAMP_FALSE;
    newNode->imageSent = SAMP_FALSE;
    newNode->msgID = -1;
    newNode->transferSyntax = IMPLICIT_LITTLE_ENDIAN;

    if ( !*A_list )
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

        while ( listNode->Next )
            listNode = listNode->Next;

        listNode->Next = newNode;
    }

    return ( SAMP_TRUE );
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
static SAMP_BOOLEAN UpdateNode(InstanceNode* A_node)
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

    return ( SAMP_TRUE );
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
static void FreeList( InstanceNode**    A_list )
{
    InstanceNode*    node;

    /*
     * Free the instance list
     */
    while (*A_list)
    {
        node = *A_list;
        *A_list = node->Next;

        if ( node->msgID != -1 )
            MC_Free_Message(&node->msgID);

        free( node );
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
static int GetNumNodes( InstanceNode*       A_list)

{
    int            numNodes = 0;
    InstanceNode*  node;

    node = A_list;
    while (node)
    {
        numNodes++;
        node = node->Next;
    }

    return numNodes;
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
static int GetNumOutstandingRequests(InstanceNode* A_list)

{
    int            outstandingResponseMsgs = 0;
    InstanceNode*  node;

    node = A_list;
    while (node)
    {
        if ( ( node->imageSent == SAMP_TRUE ) && ( node->responseReceived == SAMP_FALSE ) )
            outstandingResponseMsgs++;

        node = node->Next;
    }
    return outstandingResponseMsgs;
}


/****************************************************************************
 *
 *  Function    :   StorageCommitment
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Perform storage commitment for a set of storage objects.
 *                  The list was created as the objects themselves were
 *                  sent.
 *
 ****************************************************************************/
static SAMP_BOOLEAN StorageCommitment(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode** A_list)
{
    int           associationID = -1, calledApplicationID = -1;
    SAMP_BOOLEAN  sampStatus;
    MC_STATUS     mcStatus;

    if (!*A_list)
    {
        printf("No objects to commit.\n");
        return ( SAMP_TRUE );
    }

    /*
     * Open association to remote AE.  Instead of using the default service
     * list, use a special service list that only includes storage
     * commitment.
     */
    mcStatus = MC_Open_Association( A_appID, &associationID, A_options->RemoteAE,
                                    A_options->RemotePort != -1 ? &A_options->RemotePort : NULL,
                                    A_options->RemoteHostname[0] ? A_options->RemoteHostname : NULL,
                                    "Storage_Commit_SCU_Service_List" );

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n",A_options->RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        return(SAMP_FALSE);
    }

    /*
     * Populate the N-ACTION message for storage commitment and sent it
     * over the network.  Also wait for a response message.
     */
    sampStatus = SetAndSendNActionMessage( A_options, associationID, A_list );
    if ( !sampStatus )
    {
        MC_Abort_Association(&associationID);
        return SAMP_FALSE;
    }
    else
    {
        /*
         * When the close association fails, there's nothing really to be
         * done.  Let's still continue on and wait for an N-EVENT-REPORT
         */
        mcStatus = MC_Close_Association( &associationID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Close association failed", mcStatus);
            MC_Abort_Association(&associationID);
        }
    }

    if (A_options->Verbose)
        printf("Waiting for N-EVENT-REPORT Association\n");


    for(;;)
    {
        /*
         * Wait as an SCU for an association from the storage commitment
         * SCP.  This association will contain an N-EVENT-REPORT-RQ message.
         */
        mcStatus = MC_Wait_For_Association( "Storage_Commit_SCU_Service_List", TIME_OUT, &calledApplicationID, &associationID);
        if (mcStatus == MC_TIMEOUT)
        {
            continue;
        }
        else if (mcStatus == MC_UNKNOWN_HOST_CONNECTED)
        {
            printf("\tUnknown host connected, association rejected \n");
            continue;
        }
        else if (mcStatus == MC_NEGOTIATION_ABORTED)
        {
            printf("\tAssociation aborted during negotiation \n");
            continue;
        }
        else if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Error on MC_Wait_For_Association for storage commitment",mcStatus);
            break;
        }

        /*
         * Handle the N-EVENT association.  We are only expecting a single
         * association, so quite after we've handled te association.
         */
        HandleNEventAssociation( A_options, associationID, A_list );
        break;
    }

    return ( SAMP_TRUE );
}



/****************************************************************************
 *
 *  Function    :   SetAndSendNActionMessage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Populate an N-ACTION-RQ message to be sent, and wait
 *                  for a response to the request.
 *
 ****************************************************************************/
static SAMP_BOOLEAN SetAndSendNActionMessage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode** A_list)
{
    MC_STATUS      mcStatus;
    int            messageID;
    int            itemID;
    InstanceNode*  node;
    int            responseMessageID;
    char*          transactionUID;
    char*          responseService;
    MC_COMMAND     responseCommand;
    int            responseStatus;

    mcStatus = MC_Open_Message( &messageID, Services.STORAGE_COMMITMENT_PUSH, N_ACTION_RQ );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Error opening Storage Commitment message",mcStatus);
        return ( SAMP_FALSE );
    }

    /*
     * Set the well-known SOP instance for storage commitment Push, as
     * listed in DICOM PS3.4, J.3.5
     */
    mcStatus = MC_Set_Value_From_String( messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID, "1.2.840.10008.1.20.1.1" );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for requested sop instance uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FALSE );
    }

    /*
     * Action ID as defined in DICOM PS3.4
     */
    mcStatus = MC_Set_Next_Value_From_Int( messageID, MC_ATT_ACTION_TYPE_ID, 1 );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to set ItemID in n-action message", mcStatus);
        MC_Free_Message( &messageID );
        return ( SAMP_FALSE );
    }

    /*
     * Set the transaction UID.  Note that in a real storage commitment
     * application, this UID should be tracked and associated with the
     * SOP instances asked for commitment with this request.  That way if
     * multiple storage commitment requests are outstanding, and an
     * N-EVENT-REPORT comes in, we can associate the message with the
     * proper storage commitment request.  Commitment or commitment
     * failure for specific objects can then be tracked.
     */
    transactionUID = Create_Inst_UID();
    mcStatus = MC_Set_Value_From_String( messageID, MC_ATT_TRANSACTION_UID, transactionUID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for transaction uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FALSE );
    }

    if (A_options->Verbose)
    {
        printf("\nSending N-Action with transaction UID: %s\n", transactionUID);
    }

    /*
     * Create an item for each SOP instance we are asking commitment for.
     * The item contains the SOP Class & Instance UIDs for the object.
     */
    node = *A_list;
    while (node)
    {
        mcStatus = MC_Open_Item( &itemID, Items.REF_SOP_MEDIA );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }

        /*
         * Set_Next_Value so that we can set multiple items within the sequence attribute.
         */
        mcStatus = MC_Set_Next_Value_From_Int( messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, itemID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set ItemID in n-action message", mcStatus);
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }

        mcStatus = MC_Set_Value_From_String( itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, node->SOPClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Class UID in n-action message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }

        mcStatus = MC_Set_Value_From_String( itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, node->SOPInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Instance UID in n-action message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }

        if (A_options->Verbose)
        {
            printf("   Object SOP Class UID: %s\n", node->SOPClassUID );
            printf("Object SOP Instance UID: %s\n\n", node->SOPInstanceUID );
        }

        node = node->Next;
    }


    /*
     * Once the message has been built, we are then able to perform the
     * N-ACTION-RQ on it.
     */
    mcStatus = MC_Send_Request_Message( A_associationID, messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to send N-ACTION-RQ message",mcStatus);
        MC_Free_Message( &messageID );
        fflush(stdout);
        return ( SAMP_FALSE );
    }

    /*
     * After sending the message, we free it.
     */
    mcStatus = MC_Free_Message( &messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-ACTION-RQ message",mcStatus);
        fflush(stdout);
        return ( SAMP_FALSE );
    }

    /*
     *  Wait for N-ACTION-RSP message.
     */
    mcStatus = MC_Read_Message(A_associationID, TIME_OUT, &responseMessageID, &responseService, &responseCommand);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed for N-ACTION-RSP", mcStatus);
        fflush(stdout);
        return ( SAMP_FALSE );
    }

    /*
     * Check the status in the response message.
     */
    mcStatus = MC_Get_Value_To_Int(responseMessageID, MC_ATT_STATUS, &responseStatus);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_Int for N-ACTION-RSP status failed",mcStatus);
        MC_Free_Message(&responseMessageID);
        fflush(stdout);
        return ( SAMP_FALSE );
    }

    switch (responseStatus)
    {
        case N_ACTION_NO_SUCH_SOP_INSTANCE:
            printf("N-ACTION-RSP failed because of invalid UID\n");
            MC_Free_Message(&responseMessageID);
            fflush(stdout);
            return ( SAMP_FALSE );
        case N_ACTION_PROCESSING_FAILURE:
            printf("N-ACTION-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            fflush(stdout);
            return ( SAMP_FALSE );

        case N_ACTION_SUCCESS:
            break;

        default:
            printf("N-ACTION-RSP failure, status=0x%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            fflush(stdout);
            return ( SAMP_FALSE );
    }

    mcStatus = MC_Free_Message( &responseMessageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-ACTION-RSP message",mcStatus);
        fflush(stdout);
        return ( SAMP_FALSE );
    }
    fflush(stdout);
    return( SAMP_TRUE );

} /* End of SetAndSendNActionMessage */



/****************************************************************************
 *
 *  Function    :   HandleNEventAssociation
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Handle a storage commitment association when expecting
 *                  an N-EVENT-REPORT-RQ message.  In a typical DICOM
 *                  application this may be handled in a child process or
 *                  a seperate thread from where the MC_Wait_For_Association
 *                  call is made.
 *
 ****************************************************************************/
static SAMP_BOOLEAN HandleNEventAssociation(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode** A_list)
{
    MC_STATUS     mcStatus;
    SAMP_BOOLEAN  sampStatus;
    RESP_STATUS   respStatus;
    int           messageID = -1, rspMessageID = -1;
    MC_COMMAND    command;
    char*         serviceName;

    if (A_options->Verbose)
        printf("Accepting N-EVENT association\n");

    /*
     * Accept the association.
     */
    mcStatus = MC_Accept_Association( A_associationID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * Make sure the association is cleaned up.
         */
        MC_Reject_Association( A_associationID, TRANSIENT_NO_REASON_GIVEN );
        PrintError("Error on MC_Accept_Association", mcStatus);
        fflush(stdout);
        return SAMP_FALSE;
    }
    
    fflush(stdout);

    for (;;)
    {
        /*
         * Note, only the requestor of an association can close the association.
         * So, we wait here in the read message call after we have received
         * the N-EVENT-REPORT for the connection to close.
         */
        mcStatus = MC_Read_Message( A_associationID, TIME_OUT, &messageID, &serviceName, &command);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            if (mcStatus == MC_TIMEOUT)
            {
                printf("Timeout occured waiting for message.  Waiting again.\n");
                continue;
            }
            else if (mcStatus == MC_ASSOCIATION_CLOSED)
            {
                printf("Association Closed.\n");
                break;
            }
            else if (mcStatus == MC_NETWORK_SHUT_DOWN
                 ||  mcStatus == MC_ASSOCIATION_ABORTED
                 ||  mcStatus == MC_INVALID_MESSAGE_RECEIVED
                 ||  mcStatus == MC_CONFIG_INFO_ERROR)
            {
                /*
                 * In this case, the association has already been closed
                 * for us.
                 */
                PrintError("Unexpected event, association aborted", mcStatus);
                break;
            }

            PrintError("Error on MC_Read_Message", mcStatus);
            MC_Abort_Association(&A_associationID);
            break;
        }

        sampStatus = ProcessNEventMessage(A_options, messageID, A_list);
        if (sampStatus == SAMP_TRUE )
            respStatus = N_EVENT_SUCCESS;
        else
            respStatus = N_EVENT_PROCESSING_FAILURE;


        mcStatus = MC_Free_Message(&messageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RSP error",mcStatus);
            fflush(stdout);
            return ( SAMP_FALSE );
        }

        /*
         * Now lests send a response message.
         */
        mcStatus = MC_Open_Message ( &rspMessageID, Services.STORAGE_COMMITMENT_PUSH, N_EVENT_REPORT_RSP );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error of N-EVENT response",mcStatus);
            fflush(stdout);
            return ( SAMP_FALSE );
        }

        mcStatus = MC_Send_Response_Message( A_associationID, respStatus, rspMessageID );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message for N_EVENT_REPORT_RSP error",mcStatus);
            MC_Free_Message(&rspMessageID);
            fflush(stdout);
            return( SAMP_FALSE );
        }

        mcStatus = MC_Free_Message(&rspMessageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RSP error",mcStatus);
            fflush(stdout);
            return ( SAMP_FALSE );
        }
    }
    fflush(stdout);
    return ( SAMP_TRUE );
}


/****************************************************************************
 *
 *  Function    :   ProcessNEventMessage
 *
 *  Parameters  :   A_options   - Pointer to structure containing input
 *                                parameters to the application
 *                  A_messageID - Association ID registered
 *                  A_list      - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Handle a storage commitment association when expecting
 *                  an N-EVENT-REPORT-RQ message.  In a typical DICOM
 *                  application this may be handled in a child process or
 *                  a seperate thread from where the MC_Wait_For_Association
 *                  call is made.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ProcessNEventMessage(STORAGE_OPTIONS* A_options, int A_messageID, InstanceNode** A_list)
{
    char          uidBuffer[UI_LENGTH + 2] = {0};
    char          sopClassUID[UI_LENGTH + 2] = {0};
    char          sopInstanceUID[UI_LENGTH + 2] = {0};
    unsigned short eventType;
    MC_STATUS     mcStatus;
    int           itemID;

    mcStatus = MC_Get_Value_To_String( A_messageID, MC_ATT_TRANSACTION_UID, sizeof(uidBuffer), uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to retreive transaction UID", mcStatus);
        uidBuffer[0] = '\0';
        fflush(stdout);
        return SAMP_FALSE;
    }

    /*
     * At this time, the transaction UID should be linked to a
     * transaction UID of a previous storage commitment request.
     * The following code can then compare the successful SOP
     * instances and failed SOP instances with those that were
     * requested.
     */
    mcStatus = MC_Get_Value_To_UShortInt( A_messageID, MC_ATT_EVENT_TYPE_ID, &eventType );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to retreive event type ID", mcStatus);
        eventType = 0;
    }

    switch( eventType )
    {
        case 1: /* SUCCESS */
            printf ( "\nN-EVENT Transaction UID: %s is SUCCESS\n", uidBuffer );

            break;
        case 2: /* FAILURE */
            printf ( "\nN-EVENT Transaction UID: %s is FAILURE\n", uidBuffer );
            /*
             * At this point, the failure list is traversed through
             * to determine which images failed for the transaction.
             * This should be compared to the originals.
             */
            if (A_options->Verbose)
                printf("    Failed to commit SOP Instances:\n");

            mcStatus = MC_Get_Next_Value_To_Int( A_messageID, MC_ATT_FAILED_SOP_SEQUENCE, &itemID );
            while ( mcStatus == MC_NORMAL_COMPLETION )
            {
                mcStatus = MC_Get_Value_To_String( itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, sizeof(sopClassUID), sopClassUID );
                if ( mcStatus != MC_NORMAL_COMPLETION )
                {
                    PrintError("Unable to get SOP Class UID in n-event-rq message", mcStatus);
                    sopClassUID[0] = '\0';
                }

                mcStatus = MC_Get_Value_To_String( itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, sizeof(sopInstanceUID), sopInstanceUID );
                if ( mcStatus != MC_NORMAL_COMPLETION )
                {
                    PrintError("Unable to get SOP Instance UID in n-event-rq message", mcStatus);
                    sopInstanceUID[0] = '\0';
            }

                if (A_options->Verbose)
                {
                    printf("       SOP Class UID: %s\n", sopClassUID );
                    printf("    SOP Instance UID: %s\n\n", sopInstanceUID );
                }

                mcStatus = MC_Get_Next_Value_To_Int( A_messageID, MC_ATT_FAILED_SOP_SEQUENCE, &itemID );
            }
            break;
        default:
            printf( "Transaction UID: %s event_report invalid event type %d\n", uidBuffer, eventType );
    }


    /*
     * We should be comparing here the original SOP instances in the
     * original transaction to what was returned here.
     */
    if (A_options->Verbose)
        printf("    Successfully commited SOP Instances:\n");

    mcStatus = MC_Get_Next_Value_To_Int( A_messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, &itemID );
    while ( mcStatus == MC_NORMAL_COMPLETION )
    {
        mcStatus = MC_Get_Value_To_String( itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, sizeof(sopClassUID), sopClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Class UID in n-event-rq message", mcStatus);
            sopClassUID[0] = '\0';
        }

        mcStatus = MC_Get_Value_To_String( itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, sizeof(sopInstanceUID), sopInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Instance UID in n-event-rq message", mcStatus);
            sopInstanceUID[0] = '\0';
        }

        if (A_options->Verbose)
        {
            printf("       SOP Class UID: %s\n", sopClassUID );
            printf("    SOP Instance UID: %s\n\n", sopInstanceUID );
        }

        mcStatus = MC_Get_Next_Value_To_Int( A_messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, &itemID );
    }
    fflush(stdout);
    return ( SAMP_TRUE );
}


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
static SAMP_BOOLEAN ReadImage(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode* A_node)
{
    FORMAT_ENUM             format = UNKNOWN_FORMAT;
    SAMP_BOOLEAN            sampBool = SAMP_FALSE;
    MC_STATUS               mcStatus;


    format = CheckFileFormat( A_node->fname );
    switch(format)
    {
        case MEDIA_FORMAT:
            A_node->mediaFormat = SAMP_TRUE;
            sampBool = ReadFileFromMedia( A_options, A_appID, A_node->fname, &A_node->msgID, &A_node->transferSyntax, &A_node->imageBytes );
            break;

        case IMPLICIT_LITTLE_ENDIAN_FORMAT:
        case IMPLICIT_BIG_ENDIAN_FORMAT:
        case EXPLICIT_LITTLE_ENDIAN_FORMAT:
        case EXPLICIT_BIG_ENDIAN_FORMAT:
            A_node->mediaFormat = SAMP_FALSE;
            sampBool = ReadMessageFromFile( A_options, A_node->fname, format, &A_node->msgID, &A_node->transferSyntax, &A_node->imageBytes );
            break;

        case UNKNOWN_FORMAT:
            PrintError("Unable to determine the format of file", MC_NORMAL_COMPLETION);
            sampBool = SAMP_FALSE;
            break;
    }

    if ( sampBool == SAMP_TRUE )
    {
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
    }
    fflush(stdout);
    return sampBool;
}

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
static SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node)
{
    MC_STATUS       mcStatus;

    A_node->imageSent = SAMP_FALSE;

    /* Get the SOP class UID and set the service */
    mcStatus = MC_Get_MergeCOM_Service(A_node->SOPClassUID, A_node->serviceName, sizeof(A_node->serviceName));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_MergeCOM_Service failed", mcStatus);
        fflush(stdout);
        return ( SAMP_TRUE );
    }

    mcStatus = MC_Set_Service_Command(A_node->msgID, A_node->serviceName, C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Service_Command failed", mcStatus);
        fflush(stdout);
        return ( SAMP_TRUE );
    }

    /* set affected SOP Instance UID */
    mcStatus = MC_Set_Value_From_String(A_node->msgID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, A_node->SOPInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for affected SOP Instance UID", mcStatus);
        fflush(stdout);
        return ( SAMP_TRUE );
    }

    /*
     *  Send the message
     */
    if (A_options->Verbose)
    {
        printf("     File: %s\n", A_node->fname);

        if ( A_node->mediaFormat )
            printf("   Format: DICOM Part 10 Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));
        else
            printf("   Format: Stream Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));

        printf("SOP Class: %s (%s)\n", A_node->SOPClassUID, A_node->serviceName);
        printf("      UID: %s\n", A_node->SOPInstanceUID);
        printf("     Size: %lu bytes\n", (unsigned long) A_node->imageBytes);
    }

    mcStatus = MC_Send_Request_Message(A_associationID, A_node->msgID);
    if ((mcStatus == MC_ASSOCIATION_ABORTED) || (mcStatus == MC_SYSTEM_ERROR) || (mcStatus == MC_UNACCEPTABLE_SERVICE))
    {
        /* At this point, the association has been dropped, or we should drop it in the case of error. */
        PrintError("MC_Send_Request_Message failed", mcStatus);
        fflush(stdout);
        return ( SAMP_FALSE );
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /* This is a failure condition we can continue with */
        PrintError("Warning: MC_Send_Request_Message failed", mcStatus);
        fflush(stdout);
        return ( SAMP_TRUE );
    }

    A_node->imageSent = SAMP_TRUE;
    fflush(stdout);

    return ( SAMP_TRUE );
}


/****************************************************************************
 *
 *  Function    :   SendStream
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered
 *                  A_node     - The node in our list of instances
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE on failure where association must be aborted
 *
 *  Description :   Send image denoted by the node in stream mode.
 *
 *                  SAMP_TRUE is returned on success, or when a recoverable
 *                  error occurs.
 *
 ****************************************************************************/
static SAMP_BOOLEAN SendStream(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node)
{
    MC_STATUS       mcStatus;
    CBinfo callbackInfo = {0};

    A_node->imageSent = SAMP_FALSE;

    /*
     *  Send the message
     */
    if (A_options->Verbose)
    {
        printf("     File: %s\n", A_node->fname);
    }

    strcpy(callbackInfo.fileName, A_node->fname);

    mcStatus = MC_Send_Request(A_associationID, NULL, &callbackInfo, FileToStream);
    if (mcStatus == MC_ASSOCIATION_ABORTED || mcStatus == MC_SYSTEM_ERROR)
    {
        /*
         * At this point, the association has been dropped, or we should
         * drop it in the case of MC_SYSTEM_ERROR.
         */
        PrintError("MC_Send_Request_Message failed", mcStatus);
        if (callbackInfo.buffer)
            free(callbackInfo.buffer);
        fflush(stdout);

        return ( SAMP_FALSE );
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * This is a failure condition we can continue with
         */
        PrintError("Warning: MC_Send_Request_Message failed", mcStatus);
        if (callbackInfo.buffer)
            free(callbackInfo.buffer);
        fflush(stdout);

        return ( SAMP_TRUE );
    }

    if (callbackInfo.buffer)
        free(callbackInfo.buffer);

    A_node->imageSent = SAMP_TRUE;
    A_node->imageBytes = callbackInfo.bytesRead;
    fflush(stdout);

    return ( SAMP_TRUE );
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
static SAMP_BOOLEAN ReadResponseMessages(STORAGE_OPTIONS*  A_options, int A_associationID, int A_timeout, InstanceNode** A_list, InstanceNode* A_node)
{
    MC_STATUS       mcStatus;
    SAMP_BOOLEAN    sampBool;
    int             responseMessageID;
    char*           responseService;
    MC_COMMAND      responseCommand;
    static char     affectedSOPinstance[UI_LENGTH+2];
    unsigned int    dicomMsgID;
    InstanceNode*   node = (InstanceNode*)A_node;

    /*
     *  Wait for response
     */
    mcStatus = MC_Read_Message(A_associationID, A_timeout, &responseMessageID, &responseService, &responseCommand);
    if (mcStatus == MC_TIMEOUT)
        return ( SAMP_TRUE );

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed", mcStatus);
        fflush(stdout);
        return ( SAMP_FALSE );
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
            if ( node->dicomMsgID == dicomMsgID )
            {
                if (!strcmp(affectedSOPinstance, node->SOPInstanceUID))
                {
                    break;
                }
            }
            node = node->Next;
        }
    }

    if ( !node )
    {
        printf( "Message ID Being Responded To tag does not match message sent over association: %d\n", dicomMsgID );
        MC_Free_Message(&responseMessageID);
        fflush(stdout);
        return ( SAMP_TRUE );
    }

    node->responseReceived = SAMP_TRUE;

    sampBool = CheckResponseMessage ( responseMessageID, &node->status, node->statusMeaning, sizeof(node->statusMeaning) );
    if (!sampBool)
    {
        node->failedResponse = SAMP_TRUE;
    }

    if ( ( A_options->Verbose ) || ( node->status != C_STORE_SUCCESS ) )
        printf("   Status: %s\n", node->statusMeaning);

    node->failedResponse = SAMP_FALSE;

    mcStatus = MC_Free_Message(&responseMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for response message", mcStatus);
        fflush(stdout);
        return ( SAMP_TRUE );
    }
    fflush(stdout);
    return ( SAMP_TRUE );
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
static SAMP_BOOLEAN CheckResponseMessage(int A_responseMsgID, unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength )
{
    MC_STATUS mcStatus;
    SAMP_BOOLEAN returnBool = SAMP_TRUE;

    mcStatus = MC_Get_Value_To_UInt ( A_responseMsgID, MC_ATT_STATUS, A_status );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        /* Problem with MC_Get_Value_To_UInt */
        PrintError ( "MC_Get_Value_To_UInt for response status failed", mcStatus );
        strncpy( A_statusMeaning, "Unknown Status", A_statusMeaningLength );
        fflush(stdout);
        return SAMP_FALSE;
    }

    /* MC_Get_Value_To_UInt worked.  Check the response status */

    switch ( *A_status )
    {
        /* Success! */
        case C_STORE_SUCCESS:
            strncpy( A_statusMeaning, "C-STORE Success.", A_statusMeaningLength );
            break;

        /* Warnings.  Continue execution. */

        case C_STORE_WARNING_ELEMENT_COERCION:
            strncpy( A_statusMeaning, "Warning: Element Coersion... Continuing.", A_statusMeaningLength );
            break;

        case C_STORE_WARNING_INVALID_DATASET:
            strncpy( A_statusMeaning, "Warning: Invalid Dataset... Continuing.", A_statusMeaningLength );
            break;

        case C_STORE_WARNING_ELEMENTS_DISCARDED:
            strncpy( A_statusMeaning, "Warning: Elements Discarded... Continuing.", A_statusMeaningLength );
            break;

        /* Errors.  Abort execution. */

        case C_STORE_FAILURE_REFUSED_NO_RESOURCES:
            strncpy( A_statusMeaning, "ERROR: REFUSED, NO RESOURCES.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_INVALID_DATASET:
            strncpy( A_statusMeaning, "ERROR: INVALID_DATASET.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_CANNOT_UNDERSTAND:
            strncpy( A_statusMeaning, "ERROR: CANNOT UNDERSTAND.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_PROCESSING_FAILURE:
            strncpy( A_statusMeaning, "ERROR: PROCESSING FAILURE.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        default:
            sprintf( A_statusMeaning, "Warning: Unknown status (0x%04x)... Continuing.", *A_status );
            returnBool = SAMP_FALSE;
            break;
    }
    fflush(stdout);

    return returnBool;
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
static SAMP_BOOLEAN ReadFileFromMedia(  STORAGE_OPTIONS*  A_options,
                                        int               A_appID,
                                        char*             A_filename,
                                        int*              A_msgID,
                                        TRANSFER_SYNTAX*  A_syntax,
                                        size_t*           A_bytesRead )
{
    CBinfo      callbackInfo = {0};
    MC_STATUS   mcStatus;
    char        transferSyntaxUID[UI_LENGTH+2] = {0}, sopClassUID[UI_LENGTH+2] = {0}, sopInstanceUID[UI_LENGTH+2] = {0};

    if (A_options->Verbose)
    {
        printf("Reading %s in DICOM Part 10 format\n", A_filename);
    }

    /*
     * Create new File object
     */
    mcStatus = MC_Create_Empty_File(A_msgID, A_filename);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to create file object",mcStatus);
        fflush(stdout);
        return( SAMP_FALSE );
    }

    /*
     * Read the file off of disk
     */
    mcStatus = MC_Open_File(A_appID, *A_msgID, &callbackInfo, MediaToFileObj);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        if (callbackInfo.fp)
            fclose(callbackInfo.fp);

        if (callbackInfo.buffer)
            free(callbackInfo.buffer);

        PrintError("MC_Open_File failed, unable to read file from media", mcStatus);
        MC_Free_File(A_msgID);
        fflush(stdout);
        return( SAMP_FALSE );
    }

    if (callbackInfo.fp)
        fclose(callbackInfo.fp);

    if (callbackInfo.buffer)
        free(callbackInfo.buffer);

    *A_bytesRead = callbackInfo.bytesRead;

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
    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_TRANSFER_SYNTAX_UID, sizeof(transferSyntaxUID), transferSyntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String failed for transfer syntax UID", mcStatus);
        MC_Free_File(A_msgID);
        return SAMP_FALSE;
    }

    mcStatus = MC_Get_Enum_From_Transfer_Syntax(transferSyntaxUID, A_syntax);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Invalid transfer syntax UID contained in the file: %s\n", transferSyntaxUID);
        MC_Free_File(A_msgID);
        return SAMP_FALSE;
    }

    /*
     * If we don't handle encapsulated transfer syntaxes, let's check the
     * image transfer syntax to be sure it is not encoded as an encapsulated
     * transfer syntax.
     */
    switch (*A_syntax)
    {
        case DEFLATED_EXPLICIT_LITTLE_ENDIAN:
        case EXPLICIT_BIG_ENDIAN:
        case EXPLICIT_LITTLE_ENDIAN:
        case HEVC_H265_M10P_LEVEL_5_1:
        case HEVC_H265_MP_LEVEL_5_1:
        case IMPLICIT_BIG_ENDIAN:
        case IMPLICIT_LITTLE_ENDIAN:
        case JPEG_2000:
        case JPEG_2000_LOSSLESS_ONLY:
        case JPEG_2000_MC:
        case JPEG_2000_MC_LOSSLESS_ONLY:
        case JPEG_BASELINE:
        case JPEG_EXTENDED_2_4:
        case JPEG_EXTENDED_3_5:
        case JPEG_SPEC_NON_HIER_6_8:
        case JPEG_SPEC_NON_HIER_7_9:
        case JPEG_FULL_PROG_NON_HIER_10_12:
        case JPEG_FULL_PROG_NON_HIER_11_13:
        case JPEG_LOSSLESS_NON_HIER_14:
        case JPEG_LOSSLESS_NON_HIER_15:
        case JPEG_EXTENDED_HIER_16_18:
        case JPEG_EXTENDED_HIER_17_19:
        case JPEG_SPEC_HIER_20_22:
        case JPEG_SPEC_HIER_21_23:
        case JPEG_FULL_PROG_HIER_24_26:
        case JPEG_FULL_PROG_HIER_25_27:
        case JPEG_LOSSLESS_HIER_28:
        case JPEG_LOSSLESS_HIER_29:
        case JPEG_LOSSLESS_HIER_14:
        case JPEG_LS_LOSSLESS:
        case JPEG_LS_LOSSY:
        case JPIP_REFERENCED:
        case JPIP_REFERENCED_DEFLATE:
        case MPEG2_MPHL:
        case MPEG2_MPML:
        case MPEG4_AVC_H264_HP_LEVEL_4_1:
        case MPEG4_AVC_H264_BDC_HP_LEVEL_4_1:
        case MPEG4_AVC_H264_HP_LEVEL_4_2_2D:
        case MPEG4_AVC_H264_HP_LEVEL_4_2_3D:
        case MPEG4_AVC_H264_STEREO_HP_LEVEL_4_2:
        case SMPTE_ST_2110_20_UNCOMPRESSED_PROGRESSIVE_ACTIVE_VIDEO:
        case SMPTE_ST_2110_20_UNCOMPRESSED_INTERLACED_ACTIVE_VIDEO:
        case SMPTE_ST_2110_30_PCM_DIGITAL_AUDIO:
        case PRIVATE_SYNTAX_1:
        case PRIVATE_SYNTAX_2:
        case RLE:
            break;

        case INVALID_TRANSFER_SYNTAX:
            
            printf("Warning: Invalid transfer syntax (%s) specified\n", GetSyntaxDescription(*A_syntax));
            printf("         Not sending image.\n");
            MC_Free_File(A_msgID);
            fflush(stdout);

            return SAMP_FALSE;
    }

    if (A_options->Verbose)
        printf("Reading DICOM Part 10 format file in %s: %s\n", GetSyntaxDescription(*A_syntax), A_filename);

    mcStatus = MC_Get_Value_To_String( *A_msgID, MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID, sizeof(sopClassUID), sopClassUID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Get MC_ATT_AFFECTED_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    mcStatus = MC_Get_Value_To_String( *A_msgID, MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID, sizeof(sopInstanceUID), sopInstanceUID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Get MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    /*
     * Convert the "file" object into a "message" object.  This is done
     * because the MC_Send_Request_Message call requires that the object
     * be a "message" object.  Any of the tags in the message can be
     * accessed when the object is a "file" object.
     */
    mcStatus = MC_File_To_Message( *A_msgID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to convert file object to message object", mcStatus);
        MC_Free_File(A_msgID);
        fflush(stdout);
        return( SAMP_FALSE );
    }

    /* form message with valid group 0 and transfer syntax */
    mcStatus = MC_Set_Value_From_String(*A_msgID, MC_ATT_AFFECTED_SOP_CLASS_UID, sopClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Set MC_ATT_AFFECTED_SOP_CLASS_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    mcStatus = MC_Set_Value_From_String(*A_msgID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, sopInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Set MC_ATT_AFFECTED_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
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
 *  Function    :   ReadMessageFromFile
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_filename - Name of file to open
 *                  A_format   - Enum containing the format of the object
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax read is returned here.
 *                  A_bytesRead- Total number of bytes read in image.  Used
 *                               only for display and to calculate the
 *                               transfer rate.
 *
 *  Returns     :   SAMP_TRUE  on success
 *                  SAMP_FALSE on failure to open the file
 *
 *  Description :   This function reads a file in the DICOM "stream" format.
 *                  This format contains no DICOM part 10 header information.
 *                  The transfer syntax of the object is contained in the
 *                  A_format parameter.
 *
 *                  When this function returns failure, the caller should
 *                  not do any cleanup, A_msgID will not contain a valid
 *                  message ID.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadMessageFromFile(STORAGE_OPTIONS*  A_options,
                                        char*             A_filename,
                                        FORMAT_ENUM       A_format,
                                        int*              A_msgID,
                                        TRANSFER_SYNTAX*  A_syntax,
                                        size_t*           A_bytesRead )
{
    MC_STATUS       mcStatus;
    unsigned long   errorTag = 0;
    CBinfo          callbackInfo = {0};
    int             retStatus = 0;

    /*
     * Determine the format
     */
    switch( A_format )
    {
        case IMPLICIT_LITTLE_ENDIAN_FORMAT:
            *A_syntax = IMPLICIT_LITTLE_ENDIAN;
            break;
        case IMPLICIT_BIG_ENDIAN_FORMAT:
            *A_syntax = IMPLICIT_BIG_ENDIAN;
            break;
        case EXPLICIT_LITTLE_ENDIAN_FORMAT:
            *A_syntax = EXPLICIT_LITTLE_ENDIAN;
            break;
        case EXPLICIT_BIG_ENDIAN_FORMAT:
            *A_syntax = EXPLICIT_BIG_ENDIAN;
            break;
        default:
            return SAMP_FALSE;
    }

    if (A_options->Verbose)
        printf("Reading DICOM \"stream\" format file in %s: %s\n", GetSyntaxDescription(*A_syntax), A_filename);

    /*
     * Open an empty message object to load the image into
     */
    mcStatus = MC_Open_Empty_Message(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open empty message", mcStatus);
        fflush(stdout);
        return SAMP_FALSE;
    }

    /*
     * Open and stream message from file
     */
    callbackInfo.fp = fopen(A_filename, BINARY_READ);
    if (!callbackInfo.fp)
    {
        printf("ERROR: Unable to open %s.\n", A_filename);
        MC_Free_Message(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    retStatus = setvbuf(callbackInfo.fp, (char *)NULL, _IOFBF, 32768);
    if ( retStatus != 0 )
    {
        printf("WARNING:  Unable to set IO buffering on input file.\n");
    }

    if (callbackInfo.bufferLength == 0)
    {
        int length = 0;
        
        mcStatus = MC_Get_Int_Config_Value(WORK_BUFFER_SIZE, &length);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            length = WORK_SIZE;
        }
        callbackInfo.bufferLength =  length;
    }

    callbackInfo.buffer = malloc( callbackInfo.bufferLength );
    if( callbackInfo.buffer == NULL )
    {
        printf("ERROR: failed to allocate file read buffer [%d] kb", (int)callbackInfo.bufferLength);
        fflush(stdout);
        return SAMP_FALSE;
    }

    mcStatus = MC_Stream_To_Message(*A_msgID, 0x00080000, 0xFFFFFFFF, *A_syntax, &errorTag, (void*) &callbackInfo, StreamToMsgObj);
    
    if (callbackInfo.fp)
        fclose(callbackInfo.fp);

    if (callbackInfo.buffer)
        free(callbackInfo.buffer);

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Stream_To_Message error, possible wrong transfer syntax guessed", mcStatus);
        MC_Free_Message(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    *A_bytesRead = callbackInfo.bytesRead;
    fflush(stdout);

    return SAMP_TRUE;

} /* ReadMessageFromFile() */


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
static MC_STATUS NOEXP_FUNC MediaToFileObj( char*     A_filename,
                                            void*     A_userInfo,
                                            int*      A_dataSize,
                                            void**    A_dataBuffer,
                                            int       A_isFirst,
                                            int*      A_isLast )
{

    CBinfo*         callbackInfo = (CBinfo*)A_userInfo;
    size_t          bytes_read;
    int             retStatus;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
        callbackInfo->fp = fopen(A_filename, BINARY_READ);

        retStatus = setvbuf(callbackInfo->fp, (char *)NULL, _IOFBF, 32768);
        if ( retStatus != 0 )
        {
            printf("WARNING:  Unable to set IO buffering on input file.\n");
        }

        if (callbackInfo->bufferLength == 0)
        {
            MC_STATUS mcStatus = MC_NORMAL_COMPLETION;
            int length = 0;
        
            mcStatus = MC_Get_Int_Config_Value(WORK_BUFFER_SIZE, &length);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                length = 64*1024;
            }
            callbackInfo->bufferLength = length;
        }

        callbackInfo->buffer = malloc( callbackInfo->bufferLength );
        if( callbackInfo->buffer == NULL )
        {
            printf("Error: failed to allocate file read buffer [%d] kb", (int)callbackInfo->bufferLength);
            return MC_CANNOT_COMPLY;
        }
    }

    if (!callbackInfo->fp)
       return MC_CANNOT_COMPLY;

    bytes_read = fread(callbackInfo->buffer, 1, callbackInfo->bufferLength, callbackInfo->fp);
    if (ferror(callbackInfo->fp))
    {
        free( callbackInfo->buffer );
        callbackInfo->buffer = NULL;

        return MC_CANNOT_COMPLY;
    }

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytes_read;
    callbackInfo->bytesRead += bytes_read;

    return MC_NORMAL_COMPLETION;

} /* MediaToFileObj() */


/*************************************************************************
 *
 *  Function    :  StreamToMsgObj
 *
 *  Parameters  :  A_msgID         - Message ID of message being read
 *                 A_CBinformation - user information passwd to callback
 *                 A_isFirst       - flag to tell if this is the first call
 *                 A_dataSize      - length of data read
 *                 A_dataBuffer    - buffer where read data is stored
 *                 A_isLast        - flag to tell if this is the last call
 *
 *  Returns     :  MC_NORMAL_COMPLETION on success
 *                 any other return value on failure.
 *
 *  Description :  Reads input stream data from a file, and places the data
 *                 into buffer to be used by the MC_Stream_To_Message function.
 *
 **************************************************************************/
static MC_STATUS NOEXP_FUNC StreamToMsgObj( int        A_msgID,
                                            void*      A_CBinformation,
                                            int        A_isFirst,
                                            int*       A_dataSize,
                                            void**     A_dataBuffer,
                                            int*       A_isLast)
{
    size_t          bytesRead;
    CBinfo*         callbackInfo = (CBinfo*)A_CBinformation;

    if (A_isFirst)
        callbackInfo->bytesRead = 0L;

    bytesRead = fread(callbackInfo->buffer, 1, callbackInfo->bufferLength, callbackInfo->fp);
    if (ferror(callbackInfo->fp))
    {
        perror("\tRead error when streaming message from file.\n");
        return MC_CANNOT_COMPLY;
    }

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytesRead;
    callbackInfo->bytesRead += bytesRead;

    return MC_NORMAL_COMPLETION;
} /* StreamToMsgObj() */


/*************************************************************************
 *
 *  Function    :  FileToStream
 *
 *  Parameters  :  AmsgID      - Message ID of message being read
 *                 Atag        - unused, included only to conform to the
 *                               prototype required by the consumer of this
 *                               callback
 *                 AuserInfo   - user information passed to callback
 *                 AdataSize   - length of data read
 *                 AdataBuffer - buffer where read data is stored
 *                 AisFirst    - flag to tell if this is the first call
 *                 AisLast     - flag to tell if this is the last call
 *
 *  Returns     :  MC_NORMAL_COMPLETION on success
 *                 any other return value on failure.
 *
 *  Description :  Reads input stream data from a file, and places the data
 *                 into buffer to be used by the MC_Send_Request function.
 *
 **************************************************************************/
static MC_STATUS NOEXP_FUNC FileToStream( int             AmsgID,
                                          unsigned long   Atag,
                                          void*           AuserInfo,
                                          CALLBACK_TYPE   CBtype,
                                          unsigned long*  AdataSize,
                                          void**          AdataBuffer,
                                          int             AisFirst,
                                          int*            AisLast)
{
    size_t  bytes_read = 0;
    CBinfo  *cbInfo = (CBinfo*)AuserInfo;

    if (cbInfo == (CBinfo*)NULL)
        return MC_CANNOT_COMPLY;

    if( AisFirst )
    {
        cbInfo->fp = fopen( cbInfo->fileName, BINARY_READ );
        if( !cbInfo->fp ) 
        {
            printf("FileToStream error: can't open file [%s]", cbInfo->fileName);
            fflush(stdout);
            return MC_CANNOT_COMPLY;
        }

        fseek( cbInfo->fp, 0L, SEEK_SET );

        if (cbInfo->bufferLength == 0)
        {
            MC_STATUS mcStatus = MC_NORMAL_COMPLETION;
            int length = 0;
        
            mcStatus = MC_Get_Int_Config_Value(WORK_BUFFER_SIZE, &length);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                length = 64*1024;
            }
            cbInfo->bufferLength = length;
        }

        cbInfo->buffer = malloc( cbInfo->bufferLength );
        if( cbInfo->buffer == NULL )
        {
            printf("FileToStream error: failed to allocate file read buffer [%d] kb", (int)cbInfo->bufferLength);
            fflush(stdout);
            return MC_CANNOT_COMPLY;
        }
    }

    if ( CBtype == REQUEST_FOR_DATA_WITH_OFFSET )
    {
        unsigned long offset = *AdataSize;

        if (cbInfo->fp == (FILE*)NULL)
            cbInfo->fp = fopen( cbInfo->fileName, BINARY_READ );

        if( !cbInfo->fp ) 
        {
            printf("FileToStream error: can't open file [%s]", cbInfo->fileName);
            fflush(stdout);
            return MC_CANNOT_COMPLY;
        }

        if (offset >= 0)
            fseek( cbInfo->fp, offset, SEEK_SET );
    }

    bytes_read = fread( cbInfo->buffer, 1, cbInfo->bufferLength, cbInfo->fp );
    if( bytes_read == 0 )
    {
        if( !feof( cbInfo->fp ) )
        {
            printf("FileToStream error: failed to read file [%s]", cbInfo->fileName);
            fflush(stdout);
            return MC_CANNOT_COMPLY;
        }

        *AisLast = 1;

        free( cbInfo->buffer );
        cbInfo->buffer = NULL;
    }
    else
    {
        *AisLast = feof( cbInfo->fp ) ? 1 : 0;
    }

    *AdataSize = (unsigned long)bytes_read;
    *AdataBuffer = cbInfo->buffer;
    cbInfo->bytesRead += bytes_read;

    if (*AisLast)
    {
        fclose(cbInfo->fp);
        cbInfo->fp = NULL;
    }

    return MC_NORMAL_COMPLETION;;
}

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
static FORMAT_ENUM CheckFileFormat( char*    A_filename )
{
    FILE*            fp;
    char             signature[5] = "\0\0\0\0\0";
    char             vR[3] = "\0\0\0";

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



    if ( (fp = fopen(A_filename, BINARY_READ)) != NULL)
    {
        if (fseek(fp, 128, SEEK_SET) == 0)
        {
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
        }

        fseek(fp, 0, SEEK_SET);

        /*
         * Now try and determine the format if it is not media
         */
        if (fread(&group.groupNumber, 1, sizeof(group.groupNumber), fp) !=
            sizeof(group.groupNumber))
        {
            printf("ERROR: reading Group Number\n");
            return UNKNOWN_FORMAT;
        }
        if (fread(&elementNumber, 1, sizeof(elementNumber), fp) !=
            sizeof(elementNumber))
        {
            printf("ERROR: reading Element Number\n");
            return UNKNOWN_FORMAT;
        }

        if (fread(vR, 1, 2, fp) != 2)
        {
            printf("ERROR: reading VR\n");
            return UNKNOWN_FORMAT;
        }

        /*
         * See if this is a valid VR, if not then this is implicit VR
         */
        if (CheckValidVR(vR))
        {
            /*
             * we know that this is an explicit endian, but we don't
             *  know which endian yet.
             */
            if (!strcmp(vR, "OB")
             || !strcmp(vR, "OW")
             || !strcmp(vR, "OL")
             || !strcmp(vR, "OD")
             || !strcmp(vR, "OF")
             || !strcmp(vR, "UC")
             || !strcmp(vR, "UR")
             || !strcmp(vR, "UT")
             || !strcmp(vR, "UN")
             || !strcmp(vR, "SQ"))
            {
                /*
                 * need to read the next 2 bytes which should be set to 0
                 */
                if (fread(vR, 1, 2, fp) != 2)
                    printf("ERROR: reading VR\n");
                else if (vR[0] == '\0' && vR[1] == '\0')
                {
                    /*
                     * the next 32 bits is the length
                     */
                    if (fread(&along.valueLength, 1, sizeof(along.valueLength),
                        fp) != sizeof(along.valueLength))
                        printf("ERROR: reading Value Length\n");
                    fclose(fp);

                    /*
                     * Make the assumption that if this tag has a value, the
                     * length of the value is going to be small, and thus the
                     * high order 2 bytes will be set to 0.  If the first
                     * bytes read are 0, then this is a big endian syntax.
                     *
                     * If the length of the tag is zero, we look at the
                     * group number field.  Most DICOM objects start at
                     * group 8. Test for big endian format with the group 8
                     * in the second byte, or else defailt to little endian
                     * because it is more common.
                     */
                    if (along.valueLength)
                    {
                        if ( along.l[0] == '\0' && along.l[1] == '\0')
                            return EXPLICIT_BIG_ENDIAN_FORMAT;
                        return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                    }
                    else
                    {
                        if (group.i[1] == 8)
                            return EXPLICIT_BIG_ENDIAN_FORMAT;
                        return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                    }
                }
                else
                {
                    printf("ERROR: Data Element not correct format\n");
                    fclose(fp);
                }
            }
            else
            {
                /*
                 * the next 16 bits is the length
                 */
                if (fread(&aint.shorterValueLength, 1,
                    sizeof(aint.shorterValueLength), fp) !=
                    sizeof(aint.shorterValueLength))
                    printf("ERROR: reading short Value Length\n");
                fclose(fp);

                /*
                 * Again, make the assumption that if this tag has a value,
                 * the length of the value is going to be small, and thus the
                 * high order byte will be set to 0.  If the first byte read
                 * is 0, and it has a length then this is a big endian syntax.
                 * Because there is a chance the first tag may have a length
                 * greater than 16 (forcing both bytes to be non-zero,
                 * unless we're sure, use the group length to test, and then
                 * default to explicit little endian.
                 */
                if  (aint.shorterValueLength
                 && (aint.i[0] == '\0'))
                    return EXPLICIT_BIG_ENDIAN_FORMAT;
                else
                {
                    if (group.i[1] == 8)
                        return EXPLICIT_BIG_ENDIAN_FORMAT;
                    return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                }
            }
        }
        else
        {
            /*
             * What we read was not a valid VR, so it must be implicit
             * endian, or maybe format error
             */
            if (fseek(fp, -2L, SEEK_CUR) != 0)
            {
                printf("ERROR: seeking in file\n");
                return UNKNOWN_FORMAT;
            }

            /*
             * the next 32 bits is the length
             */
            if (fread(&along.valueLength, 1, sizeof(along.valueLength), fp) !=
                sizeof(along.valueLength))
                printf("ERROR: reading Value Length\n");
            fclose(fp);

            /*
             * This is a big assumption, if this tag length is a
             * big number, the Endian must be little endian since
             * we assume the length should be small for the first
             * few tags in this message.
             */
            if (along.valueLength)
            {
                if ( along.l[0] == '\0' && along.l[1] == '\0' )
                    return IMPLICIT_BIG_ENDIAN_FORMAT;
                return IMPLICIT_LITTLE_ENDIAN_FORMAT;
            }
            else
            {
                if (group.i[1] == 8)
                    return IMPLICIT_BIG_ENDIAN_FORMAT;
                return IMPLICIT_LITTLE_ENDIAN_FORMAT;
            }
        }
    }
    return UNKNOWN_FORMAT;
} /* CheckFileFormat() */

/****************************************************************************
 *
 *  Function    :   Create_Inst_UID
 *
 *  Parameters  :   none
 *
 *  Returns     :   A pointer to a new UID
 *
 *  Description :   This function creates a new UID for use within this
 *                  application.  Note that this is not a valid method
 *                  for creating UIDs within DICOM because the "base UID"
 *                  is not valid.
 *                  UID Format:
 *                  <baseuid>.<deviceidentifier>.<serial number>.<process id>
 *                       .<current date>.<current time>.<counter>
 *
 ****************************************************************************/
static char * Create_Inst_UID()
{
    static short UID_CNTR = 0;
    static char  deviceType[] = "1";
    static char  serial[] = "1";
    static char  Sprnt_uid[68];
    char         creationDate[68];
    char         creationTime[68];
    time_t       timeReturn;
    struct tm*   timePtr;
#ifdef UNIX
    unsigned long pid = getpid();
#endif


    timeReturn = time(NULL);
    timePtr = localtime(&timeReturn);

    sprintf(creationDate, "%d%d%d", (timePtr->tm_year + 1900), (timePtr->tm_mon + 1), timePtr->tm_mday);
    sprintf(creationTime, "%d%d%d", timePtr->tm_hour, timePtr->tm_min, timePtr->tm_sec);

#ifdef UNIX
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%d.%s.%s.%d", deviceType, serial, pid, creationDate, creationTime, UID_CNTR++);
#else
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%s.%s.%d", deviceType, serial, creationDate, creationTime, UID_CNTR++);
#endif

    return(Sprnt_uid);
}


/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(char* A_string, MC_STATUS A_status)
{
    char        prefix[30] = {0};
    /*
     *  Need process ID number for messages
     */
#ifdef UNIX
    sprintf(prefix, "PID %d", getpid() );
#endif
    if (A_status == -1)
    {
        printf("%s\t%s\n",prefix,A_string);
    }
    else
    {
        printf("%s\t%s:\n",prefix,A_string);
        printf("%s\t\t%s\n", prefix,MC_Error_Message(A_status));
    }
}


