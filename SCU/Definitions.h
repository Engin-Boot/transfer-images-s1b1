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
#include <fstream>

using namespace std;

#ifdef _WIN32
#include <fcntl.h>
#endif

/*
 * Merge DICOM Toolkit Includes
 */
#include "../mc3inc/mc3media.h"
 //#include "mc3msg.h"
#include "../mc3inc/mergecom.h"
#include "../mc3inc/diction.h"
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
int main(int argc, char** argv);

SAMP_BOOLEAN TestCmdLine(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options);
static void RemoteManagement(STORAGE_OPTIONS* A_options);
static bool CheckHostandPort(STORAGE_OPTIONS* A_options);
static SAMP_BOOLEAN PrintHelp(int A_argc, char* A_argv[]);
static bool CheckIfHelp(string str, int A_argc);
static void OptionHandling(int A_argc, char* A_argv[], STORAGE_OPTIONS* A_options);
static bool CheckOptions(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
static bool ExtraOptions(int i, char* A_argv[], STORAGE_OPTIONS* A_options);
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
static void PrintCmdLine(void);
SAMP_BOOLEAN AddFileToList(InstanceNode** A_list, char* A_fname);
static void list_updation(InstanceNode** A_list, InstanceNode* newNode);
SAMP_BOOLEAN UpdateNode(InstanceNode* A_node);
void FreeList(InstanceNode** A_list);
int GetNumNodes(InstanceNode* A_list);
static FORMAT_ENUM CheckFileFormat(char* A_filename);
SAMP_BOOLEAN ReadImage(STORAGE_OPTIONS* A_options, int A_appID, InstanceNode* A_node);
static SAMP_BOOLEAN ValidImageCheck(SAMP_BOOLEAN sampBool, InstanceNode* A_node);
SAMP_BOOLEAN SendImage(STORAGE_OPTIONS* A_options, int A_associationID, InstanceNode* A_node);
static MC_STATUS NOEXP_FUNC MediaToFileObj(char* Afilename, void* AuserInfo, int* AdataSize, void** AdataBuffer, int AisFirst, int* AisLast);
static bool Transfer_Syntax_Encoding(MC_STATUS mcStatus, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax);
static bool Image_Extraction(int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, char*& A_filename, char* sopClassUID, char* sopInstanceUID, size_t& size_sopClassUID, size_t& size_sopInstanceUID);
static bool Message_Creation(MC_STATUS mcStatus, int*& A_msgID, char* sopClassUID, char* sopInstanceUID, size_t& size_sopClassUID, size_t& size_sopInstanceUID);
static bool Syntax_Handling(MC_STATUS mcStatus, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax);
static bool Message_Handling(int*& A_msgID, char* sopClassUID, char* sopInstanceUID, size_t& size_sopClassUID, size_t& size_sopInstanceUID);
static bool ReadFile1(int& A_appID, char*& A_filename, int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, size_t*& A_bytesRead);
static bool ReadFile2(int*& A_msgID, TRANSFER_SYNTAX*& A_syntax, char*& A_filename);

void PrintError(const char* A_string, MC_STATUS A_status);

static SAMP_BOOLEAN ReadFileFromMedia(STORAGE_OPTIONS* A_options,
    int A_appID,
    char* A_filename,
    int* A_msgID,
    TRANSFER_SYNTAX* A_syntax,
    size_t* A_bytesRead);


class mainclass
{
public:
    SAMP_BOOLEAN            sampBool;
    STORAGE_OPTIONS         options;
    MC_STATUS               mcStatus;
    int                     applicationID, associationID, imageCurrent;
    int                     imagesSent, totalImages, fstatus;
    char* fname; /* Extra long, just in case */
    ServiceInfo             servInfo;
    size_t                  totalBytesRead;
    InstanceNode* instanceList, * node;
    FILE* fp = NULL;


    mainclass(char* filename)
    {

        applicationID = -1;
        associationID = -1;
        imageCurrent = 0;
        imagesSent = 0L;
        totalImages = 0L;
        fstatus = 0;
        fname = filename; /* Extra long, just in case */
        totalBytesRead = 0L;
        instanceList = NULL;
        node = NULL;
        fp = NULL;
    }

    bool InitializeApplication();
    bool InitializeList();
    void ReadFileByFILENAME();
    void ReadEachLineInFile();
    void ReadFileFromStartStopPosition();

    bool CreateAssociation();
    char* checkRemoteHostName(char* RemoteHostName);
    char* checkServiceList(char* ServiceList);
    MC_STATUS OpenAssociation();

    void StartSendImage();
    bool ImageTransfer();
    bool SendImageAndUpdateNode();
    void UpdateImageSentCount();

    void CloseAssociation();
    void ReleaseApplication();

    void VerboseBeforeConnection();
    void RemoteVerbose();
    void VerboseAfterConnection();
    void VerboseTransferSyntax();

};
