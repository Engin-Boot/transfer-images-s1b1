#include "Definitions.h"


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
SAMP_BOOLEAN TestCmdLine(int A_argc, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    if (PrintHelp(A_argc, A_argv) == SAMP_FALSE)
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

    A_options->ListenPort = 1115;
    A_options->ResponseRequested = SAMP_FALSE;
    A_options->Username[0] = '\0';
    A_options->Password[0] = '\0';

    A_options->UseFileList = SAMP_FALSE;
    A_options->FileList[0] = '\0';

    /*
     * Loop through each argument
     */
    OptionHandling(A_argc, A_argv, A_options);
    RemoteManagement(A_options);
    if (A_options->StopImage < A_options->StartImage)
    {
        printf("Image stop number must be greater than or equal to image start number.\n");
        PrintCmdLine();
        return SAMP_FALSE;
    }

    return SAMP_TRUE;

}/* TestCmdLine() */
void RemoteManagement(STORAGE_OPTIONS* A_options)
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
bool CheckHostandPort(STORAGE_OPTIONS* A_options)
{
    if (A_options->RemoteHostname[0] && (A_options->RemotePort != -1))
    {
        return true;
    }
    return false;
}
SAMP_BOOLEAN PrintHelp(int A_argc, const char* A_argv[])
{
    for (int i = 0; i < A_argc; i++)
    {
        string str(A_argv[i]);
        transform(str.begin(), str.end(), str.begin(), ::tolower);
        if (CheckIfHelp(str, A_argc) == true)
        {
            PrintCmdLine();
            return SAMP_FALSE;
        }
    }
    return SAMP_TRUE;
}
bool CheckIfHelp(const string& str, int A_argc)
{
    if ((str == "-h") || (A_argc < 3))
    {
        return true;
    }
    return false;
}
void OptionHandling(int A_argc, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    for (int i = 1; i < A_argc; i++)
    {
        if (CheckOptions(i, A_argv, A_options) == false)
        {
            printf("Unkown option: %s\n", A_argv[i]);
        }
    }
}
bool CheckOptions(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    if (MapOptions(i, A_argv, A_options) == false && ExtraOptions(i, A_argv, A_options) == false)
    {
        return false;
    }
    return true;
}
bool ExtraOptions(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    static int argCount = 0;
    argCount++;
    bool f = false;
    typedef void (*Fnptr2)(int, const char* [], STORAGE_OPTIONS*);
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
void RemoteAE(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    strcpy(A_options->RemoteAE, A_argv[i]);
}
void StartImage(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    A_options->StartImage = A_options->StopImage = atoi(A_argv[i]);
}
void StopImage(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    A_options->StopImage = atoi(A_argv[i]);
}

bool MapOptions(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    bool f = false;
    typedef void (*Fnptr1)(int, const char* [], STORAGE_OPTIONS*);
    map<string, Fnptr1> optionmap;
    optionmap["-a"] = LocalAE;
    optionmap["-b"] = LocalPort;
    optionmap["-f"] = Filename;
    optionmap["-l"] = ServiceList;
    optionmap["-n"] = RemoteHost;
    optionmap["-p"] = RemotePort;
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
void LocalAE(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->LocalAE, A_argv[i]);
}
void LocalPort(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->ListenPort = atoi(A_argv[i]);
}
void Filename(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->UseFileList = SAMP_TRUE;
    strcpy(A_options->FileList, A_argv[i]);
}
void ServiceList(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->ServiceList, A_argv[i]);
}
void RemoteHost(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    strcpy(A_options->RemoteHostname, A_argv[i]);
}
void RemotePort(int i, const char* A_argv[], STORAGE_OPTIONS* A_options)
{
    i++;
    A_options->RemotePort = atoi(A_argv[i]);
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
void PrintCmdLine(void)
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
    printf("\n");
    printf("\tImage files must be in the current directory if -f is not used.\n");
    printf("\tImage files must be named 0.img, 1.img, 2.img, etc if -f is not used.\n");

} /* end PrintCmdLine() */