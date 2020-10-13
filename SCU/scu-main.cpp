#include "Definitions.h"
/*
 *  Local Function prototypes
 */



 /****************************************************************************
  *
  *  Function    :   Main
  *
  *  Description :   Main routine for DICOM Storage Service Class SCU
  *
  ****************************************************************************/
  //int  main(int argc, char** argv);



  //typedef bool (*FileInputTypeFunction)(STORAGE_OPTIONS& options, FILE*& fp, char* fname, int& fstatus, int& imageCurrent, InstanceNode* instanceList);
  //typedef std::map<SAMP_BOOLEAN, FileInputTypeFunction>MapToFileInputType;
  

int main(int argc, const char* argv[])
{
    SAMP_BOOLEAN            sampBool;
    /*STORAGE_OPTIONS         opt;
    MC_STATUS               mcStatus;
    int                     applicationID = -1, associationID = -1, imageCurrent = 0;
    int                     imagesSent = 0L, totalImages = 0L, fstatus = 0;
    double                  seconds = 0.0;
    void* startTime = NULL, * imageStartTime = NULL;
    char                    fname[512] = { 0 };  /* Extra long, just in case *
    ServiceInfo             servInfo;
    size_t                  totalBytesRead = 0L;
    InstanceNode* instanceList = NULL, * node = NULL;
    FILE* fp = NULL;*/
    char fname[512] = { 0 };

    /*
     * Test the command line parameters, and populate the options
     * structure with these parameters
     */

    mainclass obj(fname);

    sampBool = TestCmdLine(argc, argv, &(obj.options));

    if (sampBool == SAMP_FALSE)
    {
        return(EXIT_FAILURE);
    }

    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */


    /*
     * Create a linked list of all files to be transferred.
     * Retrieve the list from a specified file on the command line,
     * or generate the list from the start & stop numbers on the
     * command line
     */

    if (obj.InitializeApplication() == false) {

        return (EXIT_FAILURE);
    }

    /*

        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */

    fflush(stdout);

    /*

         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         *

    */

    /*STORAGE_OPTIONS         opt;
    MC_STATUS               mcStatus;
    int                     applicationID = -1, associationID = -1, imageCurrent = 0;
    int                     imagesSent = 0L, totalImages = 0L, fstatus = 0;
    double                  seconds = 0.0;
    void* startTime = NULL, * imageStartTime = NULL;
    char                    fname[512] = { 0 };  /* Extra long, just in case *
    ServiceInfo             servInfo;
    size_t                  totalBytesRead = 0L;
    InstanceNode* instanceList = NULL, * node = NULL;
    FILE* fp = NULL;*/


    /*
     *   Send all requested images.  Traverse through instanceList to
     *   get all files to send
     */

    obj.StartSendImage();



    /*
     * Send image read in with ReadImage.
     *
     * Because SendImage may not have actually sent an image even though it has returned success,
     * the calculation of performance data below may not be correct.
     */

     // seconds = GetIntervalElapsed(imageStartTime);
     //if (options.Verbose)
       //  printf("     Time: %.3f seconds\n\n", (float)seconds);
     //else
       //  printf("\tSent %s image (%d of %d), elapsed time: %.3f seconds\n", node->serviceName, imagesSent, totalImages, seconds);

     /*
      * Traverse through file list
 }   /* END for loop for each image *
 */
    obj.CloseAssociation();

    obj.ReleaseApplication();
    /*
     * A failure on close has no real recovery.  Abort the association
     * and continue on.
     */
     /*


     /*
      * Calculate the transfer rate.  Note, for a real performance
      * numbers, a function other than time() to measure elapsed
      * time should be used.
      *

     // seconds = GetIntervalElapsed(startTime);

     printf("Data Transferred: %luMB\n", (unsigned long)(totalBytesRead / (1024 * 1024)));
     //printf("    Time Elapsed: %.3fs\n", seconds);
     //printf("   Transfer Rate: %.1fKB/s\n", ((float)totalBytesRead / seconds) / 1024.0);
     fflush(stdout);

     /*
      * Release the dICOM Application
      *


     /*
      * Free the node list's allocated memory
      *
     FreeList(&instanceList);

     /*
      * Release all memory used by the Merge DICOM Toolkit.
      *
     if (MC_Library_Release() != MC_NORMAL_COMPLETION)
         printf("Error releasing the library.\n");
     */

    fflush(stdout);

    return(EXIT_SUCCESS);

}




/*
        else
        {
        /*
         * Parse through the rest of the options
         *


        }

*/

