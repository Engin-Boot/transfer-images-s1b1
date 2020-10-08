/*
 * Standard OS Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <map>

using namespace std;

#ifdef _WIN32
#include <fcntl.h>
#endif

 /*
  * Merge DICOM Toolkit Includes
  */
#include "mc3media.h"
  //#include "mc3msg.h"
#include "mergecom.h"
#include "diction.h"
//#include "mc3services.h"
//#include "mc3items.h"

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
    FILE* fp;
    char    fileName[512];
    /*
     * Note! The size of this buffer impacts toolkit performance.
     *       Higher values in general should result in increased performance of reading files
     */
    size_t  bytesRead;
    size_t  bufferLength;

    char* buffer;
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

    char   SOPClassUID[UI_LENGTH + 2];    /* SOP Class UID of the file */
    char   serviceName[48];             /* Merge DICOM Toolkit service name for SOP Class */
    char   SOPInstanceUID[UI_LENGTH + 2]; /* SOP Instance UID of the file */

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

int main(int argc, char** argv);

static SAMP_BOOLEAN TestCmdLine(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options);
static void RemoteManagement(STORAGE_OPTIONS* A_options);
static bool CheckHostandPort(STORAGE_OPTIONS* A_options);
static SAMP_BOOLEAN PrintHelp(int A_argc, char* A_argv[]);
static bool CheckIfHelp(string str, int A_argc);
static void OptionHandling(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options);
static bool CheckOptions(int argCount, int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static bool ExtraOptions(int argCount, int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void RemoteAE(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void StartImage(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void StopImage(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static bool MapOptions(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void LocalAE(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void LocalPort(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void Filename(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void ServiceList(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void RemoteHost(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void RemotePort(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static void Verbose(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static SAMP_BOOLEAN AddFileToList(InstanceNode** A_list, char* A_fname);
static void list_updation(InstanceNode** A_list, InstanceNode* newNode);
static SAMP_BOOLEAN UpdateNode(InstanceNode* A_node);
static void FreeList(InstanceNode** A_list);
static int GetNumNodes(InstanceNode* A_list);
static FORMAT_ENUM CheckFileFormat(char* A_filename);
static SAMP_BOOLEAN ReadImage(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode* A_node);
static SAMP_BOOLEAN ValidImageCheck(SAMP_BOOLEAN sampBool, InstanceNode* A_node);
static SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node);
static MC_STATUS NOEXP_FUNC MediaToFileObj(char* Afilename, void* AuserInfo, int* AdataSize, void** AdataBuffer, int AisFirst, int* AisLast);

static void PrintError(const char* A_string, MC_STATUS A_status);

static SAMP_BOOLEAN ReadFileFromMedia(STORAGE_OPTIONS* A_options,
    int A_appID,
    char* A_filename,
    int* A_msgID,
    TRANSFER_SYNTAX* A_syntax,
    size_t* A_bytesRead);


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
    void* startTime = NULL, * imageStartTime = NULL;
    char                    fname[512] = { 0 };  /* Extra long, just in case */
    ServiceInfo             servInfo;
    size_t                  totalBytesRead = 0L;
    InstanceNode* instanceList = NULL, * node = NULL;
    FILE* fp = NULL;

    /*
     * Test the command line parameters, and populate the options
     * structure with these parameters
     */
    sampBool = TestCmdLine(argc, argv, &options);
    if (sampBool == SAMP_FALSE)
    {
        return(EXIT_FAILURE);
    }

    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */
    mcStatus = MC_Library_Initialization(NULL, NULL, NULL);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library", mcStatus);
        return (EXIT_FAILURE);
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

            sampBool = AddFileToList(&instanceList, fname);
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
            sampBool = AddFileToList(&instanceList, fname);
            if (!sampBool)
            {
                printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
            }
        }
    }

    totalImages = GetNumNodes(instanceList);

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

    //startTime = GetIntervalStart();

    mcStatus = MC_Open_Association(applicationID,
        &associationID,
        options.RemoteAE,
        options.RemotePort != -1 ? &options.RemotePort : NULL,
        options.RemoteHostname[0] ? options.RemoteHostname : NULL,
        options.ServiceList[0] ? options.ServiceList : NULL);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", options.RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        fflush(stdout);
        return(EXIT_FAILURE);
    }

    mcStatus = MC_Get_Association_Info(associationID, &options.asscInfo);
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
        printf("  User Identity type:       None\n\n\n");

        printf("Services and transfer syntaxes negotiated:\n");

        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
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
    else
        printf("Connected to remote system [%s]\n\n", options.RemoteAE);


    fflush(stdout);

    /*
     *   Send all requested images.  Traverse through instanceList to
     *   get all files to send
     */
    node = instanceList;
    while (node)
    {
        //imageStartTime = GetIntervalStart();

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
            continue;
        }

        totalBytesRead += node->imageBytes;

        /*
         * Send image read in with ReadImage.
         *
         * Because SendImage may not have actually sent an image even though it has returned success,
         * the calculation of performance data below may not be correct.
         */
        sampBool = SendImage(&options, associationID, node);
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
        sampBool = UpdateNode(node);
        if (!sampBool)
        {
            printf("Warning, unable to update node with information [%s]\n", node->fname);

            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            break;
        }
        // }

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

        // seconds = GetIntervalElapsed(imageStartTime);
        //if (options.Verbose)
          //  printf("     Time: %.3f seconds\n\n", (float)seconds);
        //else
          //  printf("\tSent %s image (%d of %d), elapsed time: %.3f seconds\n", node->serviceName, imagesSent, totalImages, seconds);

        /*
         * Traverse through file list
         */
        node = node->Next;

    }   /* END for loop for each image */

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
    FreeList(&instanceList);

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
    printf("\nUsage SCU remote_ae start stop -f filename -a local_ae -b local_port -n remote_host -p remote_port -l service_list -v \n");
    printf("\n");
    printf("\t remote_ae       name of remote Application Entity Title to connect with\n");
    printf("\t start           start image number (not required if -f specified)\n");
    printf("\t stop            stop image number (not required if -f specified)\n");
    printf("\t -f filename     (optional) specify a file containing a list of images to transfer\n");
    printf("\t -a local_ae     (optional) specify the local Application Title (default: MERGE_STORE_SCU)\n");
    printf("\t -b local_port   (optional) specify the local TCP listen port for commitment (default: found in the mergecom.pro file)\n");
    printf("\t -n remote_host  (optional) specify the remote hostname (default: found in the mergecom.app file for remote_ae)\n");
    printf("\t -p remote_port  (optional) specify the remote TCP listen port (default: found in the mergecom.app file for remote_ae)\n");
    printf("\t -l service_list (optional) specify the service list to use when negotiating (default: Storage_SCU_Service_List)\n");
    printf("\t -v              Execute in verbose mode, print negotiation information\n");
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
static SAMP_BOOLEAN TestCmdLine(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    if (PrintHelp(A_argc,A_argv) == SAMP_FALSE)
    {
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
     * Loop through each argument
     */
    OptionHandling(A_argc,A_argv,A_options);
    RemoteManagement(A_options);
    if (A_options->StopImage < A_options->StartImage)
    {
        printf("Image stop number must be greater than or equal to image start number.\n");
        PrintCmdLine();
        return SAMP_FALSE;
    }

    return SAMP_TRUE;

}/* TestCmdLine() */

static void RemoteManagement(STORAGE_OPTIONS* A_options)
{
    /*
        * If the hostname & port are specified on the command line,
        * the user may not have the remote system configured in the
        * mergecom.app file.  In this case, force the default service
        * list, so we can attempt to make a connection, or else we would
        * fail.
        */

    if (!A_options->ServiceList[0] && CheckHostandPort(A_options))
    {
        strcpy(A_options->ServiceList, "Storage_SCU_Service_List");
    }
}
static bool CheckHostandPort(STORAGE_OPTIONS* A_options)
{
    if (A_options->RemoteHostname[0] && (A_options->RemotePort != -1))
    {
        return true;
    }
    return false;
}
static SAMP_BOOLEAN PrintHelp(int A_argc, char* A_argv[])
{
    for (int i = 1; i < A_argc; i++)
    {
        string str(A_argv[i]);
        transform(str.begin(), str.end(), str.begin(), ::tolower);
        if (CheckIfHelp(str,A_argc)==true)
        {
            PrintCmdLine();
            return SAMP_FALSE;
        }
    }
    return SAMP_TRUE;
} 
static bool CheckIfHelp(string str, int A_argc)
{
    if ((str == "-h") || (A_argc < 3))
        return true;
    return false;
}
static void OptionHandling(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    int       argCount = 0;
    for (int i = 1; i < A_argc; i++)
    {
        if (CheckOptions(argCount, i, A_argv, A_options)==false)
        {
            printf("Unkown option: %s\n", A_argv[i]);
        }
    }
}
static bool CheckOptions(int argCount, int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    if (MapOptions(i, A_argv, A_options) == false && ExtraOptions(argCount,i,A_argv,A_options) == false)
    {
        return false;
    }
    return true;
}
static bool ExtraOptions(int argCount, int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    argCount++;
    bool f = false;
    typedef void (*Fnptr2)(int, char* [], STORAGE_OPTIONS*);
    map<int, Fnptr2> extraoptions;
    extraoptions[1] = RemoteAE;
    extraoptions[2] = StartImage;
    extraoptions[3] = StopImage;
    map<int, Fnptr2>::iterator itr;
    for (itr = extraoptions.begin(); itr != extraoptions.end(); ++itr)
    {
        if (argCount == itr->first)
        {
            f = true;
            extraoptions[argCount](i, A_argv, A_options);
        }
    }
    return f;

}
static void RemoteAE(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    strcpy(A_options->RemoteAE, A_argv[i]);
}
static void StartImage(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    A_options->StartImage = A_options->StopImage = atoi(A_argv[i]);
}
static void StopImage(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    A_options->StopImage = atoi(A_argv[i]);
}

static bool MapOptions(int i, char* A_argv[],STORAGE_OPTIONS* A_options)
{
    bool f = false;
    typedef void (*Fnptr1)(int, char* [], STORAGE_OPTIONS*);
    map<string, Fnptr1> optionmap;
    optionmap["-a"] = LocalAE;
    optionmap["-b"] = LocalPort;
    optionmap["-f"] = Filename;
    optionmap["-l"] = ServiceList;
    optionmap["-n"] = RemoteHost;
    optionmap["-p"] = RemotePort;
    optionmap["-v"] = Verbose;
    map<string, Fnptr1>::iterator itr;
    string str(A_argv[i]);
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    for (itr = optionmap.begin(); itr != optionmap.end(); ++itr)
    {
        if (str == itr->first)
        {
            f = true;
            optionmap[str](i, A_argv, A_options);
        }
    }
    return f;
}
static void LocalAE(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->LocalAE, A_argv[i]);
}
static void LocalPort(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->ListenPort = atoi(A_argv[i]);
}
static void Filename(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->UseFileList = SAMP_TRUE;
    strcpy(A_options->FileList, A_argv[i]);
}
static void ServiceList(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->ServiceList, A_argv[i]);
}
static void RemoteHost(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->RemoteHostname, A_argv[i]);
}
static void RemotePort(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->RemotePort = atoi(A_argv[i]);
}
static void Verbose(int i, char* A_argv[], STORAGE_OPTIONS* A_options)
{
    A_options->Verbose = SAMP_TRUE;
}

/*
        else
        {
        /*
         * Parse through the rest of the options
         *

         
        }
        
*/

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
    list_updation(A_list,newNode);
    
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
static void FreeList(InstanceNode** A_list)
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
static int GetNumNodes(InstanceNode* A_list)

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
static SAMP_BOOLEAN ValidImageCheck(SAMP_BOOLEAN sampBool,InstanceNode* A_node)
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

bool checkSendRequestMessage(MC_STATUS mcStatus, InstanceNode* A_node)
{
    std::map<MC_STATUS, string> SendRequestMap;

    SendRequestMap[MC_ASSOCIATION_ABORTED] = "MC_Send_Request_Message failed";
    SendRequestMap[MC_SYSTEM_ERROR] = "MC_Send_Request_Message failed";
    SendRequestMap[MC_UNACCEPTABLE_SERVICE] = "MC_Send_Request_Message failed";


    if (SendRequestMap.find(mcStatus) != SendRequestMap.end()) {
        CheckIfMCStatusNotOk(mcStatus, "MC_Send_Request_Message failed");
        return true;
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

static SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node)
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
    if (A_options->Verbose)
    {
        printf("     File: %s\n", A_node->fname);

        if (A_node->mediaFormat)
            printf("   Format: DICOM Part 10 Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));
        else
            printf("   Format: Stream Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));

        printf("SOP Class: %s (%s)\n", A_node->SOPClassUID, A_node->serviceName);
        printf("      UID: %s\n", A_node->SOPInstanceUID);
        printf("     Size: %lu bytes\n", (unsigned long)A_node->imageBytes);
    }

    mcStatus = MC_Send_Request_Message(A_associationID, A_node->msgID);

    if (checkSendRequestMessage(mcStatus, A_node))
        return (SAMP_FALSE);

    return (SAMP_TRUE);
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
static SAMP_BOOLEAN ReadFileFromMedia(STORAGE_OPTIONS* A_options,
    int               A_appID,
    char* A_filename,
    int* A_msgID,
    TRANSFER_SYNTAX* A_syntax,
    size_t* A_bytesRead)
{
    CBinfo      callbackInfo = { 0 };
    MC_STATUS   mcStatus;
    char        transferSyntaxUID[UI_LENGTH + 2] = { 0 }, sopClassUID[UI_LENGTH + 2] = { 0 }, sopInstanceUID[UI_LENGTH + 2] = { 0 };

    if (A_options->Verbose)
    {
        printf("Reading %s in DICOM Part 10 format\n", A_filename);
    }

    /*
     * Create new File object
     */


     /*
      * Read the file off of disk
     */
    mcStatus = CreateEmptyFileAndStoreIt(A_appID, A_msgID, A_filename, callbackInfo);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return(SAMP_FALSE);
    }

    CloseCallBackInfo(callbackInfo);

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

    if (!CheckTransferSyntax(*A_syntax))
    {
        printf("Warning: Invalid transfer syntax (%s) specified\n", GetSyntaxDescription(*A_syntax));
        printf("         Not sending image.\n");
        MC_Free_File(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    if (A_options->Verbose)
        printf("Reading DICOM Part 10 format file in %s: %s\n", GetSyntaxDescription(*A_syntax), A_filename);

    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID, sizeof(sopClassUID), sopClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Get MC_ATT_AFFECTED_SOP_INSTANCE_UID failed. Error %d (%s)\n", (int)mcStatus, MC_Error_Message(mcStatus));
        MC_Free_File(A_msgID);
        fflush(stdout);
        return SAMP_FALSE;
    }

    mcStatus = MC_Get_Value_To_String(*A_msgID, MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID, sizeof(sopInstanceUID), sopInstanceUID);
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
    mcStatus = MC_File_To_Message(*A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to convert file object to message object", mcStatus);
        MC_Free_File(A_msgID);
        fflush(stdout);
        return(SAMP_FALSE);
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


/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(const char* A_string, MC_STATUS A_status)
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