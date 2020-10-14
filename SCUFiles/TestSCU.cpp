
#define CATCH_CONFIG_MAIN
#include "Definitions.h"
#include "catch.hpp"  

//**************Unit Test scu-main-function*****************************

TEST_CASE("When a single file is to be sent and it is not present in the directory then no image is added to node and tranfer does not occur")
{
    //InitializeApplication(),InitilaizeList(),
    char fname[256];
    bool sampBool;
    mainclass testobj(fname);
    char tempfilename[] = "NameWhichDoesNotExist.img";
    strcpy(fname, tempfilename);
    testobj.options.UseFileList = SAMP_TRUE;
    strcpy(testobj.options.FileList, tempfilename);
    sampBool = testobj.InitializeList();
    REQUIRE(sampBool == false);
}

TEST_CASE("When the start stop mode of image transfer is used and stop image number is less than start image then no image is added to node and tranfer does not occur")
{
    //ReadFileFromStartStopPosition(), AddFileToList(), GetNumNodes, VerboseBeforeConnection, CreateAssociation()
    char fname[256];
    mainclass testobj(fname);
    testobj.options.StartImage = 5;
    testobj.options.StopImage = 2;

    testobj.ReadFileFromStartStopPosition();
    REQUIRE(testobj.totalImages == 0);
}


TEST_CASE("When any invalid command is given in command line as input then no assocaition is created and the program exits")
{
    char fname[256];
    mainclass testobj(fname);
    const char  arg0[] = "Invalid";
    const char  arg1[] = "Command";
    const char  arg2[] = "Invalid";
    const char* argv[] = { &arg0[0], &arg1[0], &arg2[0], NULL };
    int   argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    TestCmdLine(argc, argv, &testobj.options);
    REQUIRE(testobj.InitializeApplication() == false);
}

TEST_CASE("If no Server/SCP is present then do not make association")
{
    char fname[256];
    mainclass testobj(fname);
    const char  arg0[] = "SCU";
    const char  arg1[] = "MERGE_STORE_SCP";
    const char  arg2[] = "0";
    const char* argv[] = { &arg0[0], &arg1[0], &arg2[0], NULL };
    int   argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    TestCmdLine(argc, argv, &testobj.options);
    
    REQUIRE(testobj.OpenAssociation() != MC_NORMAL_COMPLETION);

}

TEST_CASE("When no image is sent then imagesSent count is zero")
{
    char fname[256];
    mainclass testobj(fname);
    const char  arg0[] = "SCU";
    const char  arg1[] = "MERGE_STORE_SCP";
    const char  arg2[] = "0";
    const char* argv[] = { &arg0[0], &arg1[0], &arg2[0], NULL };
    int   argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    TestCmdLine(argc, argv, &testobj.options);
   
    testobj.StartSendImage();
    REQUIRE(testobj.imagesSent == 0);
}


//************Unit Tests Send Image*********************

TEST_CASE("When a MC_Staus flag is set to false then CheckIfMCStatusNotOk() can check it and display associated error")
{
    //InitializeApplication(),InitilaizeList(),
    MC_STATUS mcStatus = MC_NOT_SUPPORTED;

    REQUIRE(CheckIfMCStatusNotOk(mcStatus, "Required Error Message") == true);
}

TEST_CASE("When a request message is sent from SCU and it is not accepted then check to find such occurrences")
{
    char fname[256];
    mainclass testobj(fname);
    SECTION("When Association is Aborted")
    {
        MC_STATUS mcStatus = MC_ASSOCIATION_ABORTED;
        bool StatusNotOk = checkSendRequestMessage(mcStatus, testobj.node);
        REQUIRE(StatusNotOk == true);
    }
    SECTION("When Some System Error Occurs")
    {
        MC_STATUS mcStatus = MC_SYSTEM_ERROR;
        bool StatusNotOk = checkSendRequestMessage(mcStatus, testobj.node);
        REQUIRE(StatusNotOk == true);
    }
    SECTION("When any Unacceptable Service runs")
    {
        MC_STATUS mcStatus = MC_UNACCEPTABLE_SERVICE;
        bool StatusNotOk = checkSendRequestMessage(mcStatus, testobj.node);
        REQUIRE(StatusNotOk == true);
    }

}
//************Unit Tests Command_Line.cpp*********************
TEST_CASE("when arguments are less than 3 or '-h' is passed as option then CheckIfHelp() returns true")
{
    SECTION("when arguments are less than 3 then CheckIfHelp() returns true") 
    {
        REQUIRE(CheckIfHelp("",2)==true);
    }
    SECTION("when '-h' is passed as option then CheckIfHelp() returns true")
    {
        REQUIRE(CheckIfHelp("-h",4) == true);
    }
}
TEST_CASE("when arguments are less than 3 or '-h' is entered as an option then PrintHelp() returns SAMP_FALSE")
{
    const char* argv[] = { "SCU", "-h" };
    REQUIRE(PrintHelp(2, argv) == SAMP_FALSE);
}
TEST_CASE("when a valid option is passed as an argument then the task corresponding to the option is performed and true is returned by MapOptions()")
{
    SECTION("when '-a' is passed as an argument then local AE is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        strcpy(store->LocalAE, "MERGE_STORE_SCU");
        const char* argv[] = { "SCU", "-a" , "Transfer_Image_SCU"};
        REQUIRE(MapOptions(1,argv,store) == true);
    }
    SECTION("when '-b' is passed as an argument then local port is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        store->ListenPort = 1115;
        const char* argv[] = { "SCU", "-b" , "1515" };
        REQUIRE(MapOptions(1, argv, store) == true);
    }
    SECTION("when '-f' is passed as an argument then the specified filename is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        const char* argv[] = { "SCU", "-f" , "image_list.txt" };
        REQUIRE(MapOptions(1, argv, store) == true);
    }
    SECTION("when '-l' is passed as an argument then service list is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        strcpy(store->ServiceList, "Storage_SCU_Service_List");
        const char* argv[] = { "SCU", "-l" , "Storage_Service_List" };
        REQUIRE(MapOptions(1, argv, store) == true);
    }
    SECTION("when -n is passed as an argument then remote host is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        const char* argv[] = { "SCU", "-n" , "Remote_Host_SCP" };
        REQUIRE(MapOptions(1, argv, store) == true);
    }
    SECTION("when '-p' is passed as an argument then remote port is set and MapOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        store->RemotePort = -1;
        const char* argv[] = { "SCU", "-p" , "2020" };
        REQUIRE(MapOptions(1, argv, store) == true);
    }
}
TEST_CASE("when a valid extra option is passed as an argument then the task corresponding to the option is performed and true is returned by ExtraOptions()")
{
    SECTION("when 1 extra argument is passed then remote AE is set and ExtraOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        strcpy(store->RemoteAE, "MERGE_STORE_SCP");
        const char* argv[] = { "SCU", "Transfer_Image_SCP" };
        REQUIRE(ExtraOptions(1, argv, store) == true);
    }
    SECTION("when 2 extra arguments are passed then remote AE and start image parameters are set and ExtraOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        strcpy(store->RemoteAE, "MERGE_STORE_SCP");
        store->StartImage = store->StopImage = 0;
        const char* argv[] = { "SCU", "Transfer_Image_SCP" , "2" };
        REQUIRE(ExtraOptions(1, argv, store) == true);
    }
    SECTION("when 3 extra arguments are passed then remote AE, start image and stop image parameters are set and ExtraOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        strcpy(store->RemoteAE, "MERGE_STORE_SCP");
        store->StartImage = store->StopImage = 0;
        const char* argv[] = { "SCU", "Transfer_Image_SCP" , "2", "5" };
        REQUIRE(ExtraOptions(1, argv, store) == true);
    }
}
TEST_CASE("when valid options are entered then CheckOptions() returns true else false")
{
    SECTION("when valid options are entered then corresponding task is performed and CheckOptions() returns true")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        const char* argv[] = { "SCU", "-a" , "Transfer_Image_SCU" };
        REQUIRE(CheckOptions(1, argv, store) == true);
    }
    SECTION("when invalid options are entered then CheckOptions() returns false")
    {
        STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
        const char* argv[] = { "SCU", "-x" , "Transfer_Image_SCU" };
        REQUIRE(CheckOptions(1, argv, store) == false);
    }
}
TEST_CASE("when remote host name and port are given then CheckHostandPort() returns true else false")
{
    STORAGE_OPTIONS* store = new STORAGE_OPTIONS;
    const char* argv[] = { "SCU", "-n" , "Remote_Host_SCP" };
    MapOptions(1, argv, store);
    store->RemotePort = 2020;
    REQUIRE(CheckHostandPort(store) == true);
}