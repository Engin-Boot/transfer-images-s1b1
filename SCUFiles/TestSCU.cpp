
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