#include "Definitions.h"
 /****************************************************************************
  *
  *  Function    :   Main
  *
  *  Description :   Main routine for DICOM Storage Service Class SCU
  *
  ****************************************************************************/
int main(int argc, const char* argv[])
{
    SAMP_BOOLEAN            sampBool;
    char fname[512] = { 0 };
    mainclass obj(fname);
    /*
    * Test the command line parameters, and populate the options
    * structure with these parameters
    */

    sampBool = TestCmdLine(argc, argv, &(obj.options));
    if (sampBool == SAMP_FALSE)
    {
        return(EXIT_FAILURE);
    }

    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */


    /*
     * Register application, create a linked list of all files to be transferred.
     * Retrieve the list from a specified file on the command line,
     * or generate the list from the start & stop numbers on the
     * command line
     * Then create association with server application
     */
    if (obj.InitializeApplication() == false) {

        return (EXIT_FAILURE);
    }

    fflush(stdout);

    /*
     *   Send all requested images.  Traverse through instanceList to
     *   get all files to send.
     *   Read all images
     *   Send all images that are ready to be sent 
     */

    obj.StartSendImage();

    /*
    * Abort Association, free all nodes and Release Application
    * Release all memory used by the Merge DICOM Toolkit.
    */

   
    obj.CloseAssociation();
    obj.ReleaseApplication();

    fflush(stdout);
    return(EXIT_SUCCESS);
}
