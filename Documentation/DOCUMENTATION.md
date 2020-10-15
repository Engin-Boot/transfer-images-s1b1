# Description

# Main

Main stores the user command line input into structures and calls all routines that
are required as per input.

* Store command line input parameters in main class data member Storage Option.

* Calls subroutines which initialize application by associating with server,
loading files to send, read those files, check their format and send them.

* After the transfer of file is complete it closes association with server,
release application and frees list

### Initialize structure

* Storage Options which has fields port numbers, host name and file list with
input values using Test command line.

### Initialize application

* Register this application with this Mergecom toolkit.

* Initialize Instance List which is a linked list that stores in its nodes data for images to
send and information about those as per user input.

* Create association with remote application i.e. server after opening association
check for normal completion of association.

* Some verbose statements are there to give more information about the flow to user, some
of those information can be avoided by user by not running application in verbose mode.

### Send Image

* To start with sending image it is required to read image first, check its validity and
then convert to DICOM message format.

* Image is read first using read image for correct syntax and correct format(.img),
storing useful information like transfer syntax and size.

* After reading it is converted to DICOM message object, then it is ready to be sent

* Message is then sent and transfer is checked

* The image node is then updated

### Close Association

* After successful transfer association with server is closed.

* Application is released from mergecom toolkit.

* Free instance list.

# List Management

This module manages the Instance Node list. The list accommodates
 all files to send and keep update their status after sending them.

### Add File To List

* This function adds new image files to a linked list by using list updation,
after checking whether they are actually present. It also initialize the status of each file.

### Update Node and Get Num Node

* This gives a message ID for tracking message

* Get total count of files to send

### Free List

* This deletes the list after successful transfer at the end

# Command_Line.cpp

## Overall Description

This file has all the functions required for taking the options from command line and setting the required parameters as needed.

## Functional Breakdown

### TestCmdLine()

* This is the function which is called by main() to set the parameters according to the inputted options. 
* If for any parameter an option has not been specified by user then the default value for the parameter 
is set in this function itself.

### RemoteManagement()

* This function is called by TestCmdLine() and it sets the Service List as the default Service List when user inputs the remote hostname
and port on the command line.
* This is done so that there is no failure in establishing the connection, because the user may not have the
remote system configured in the mergecom.app file.

### CheckHostandPort()

* Checks whether user has specified remote hostname and port on the command line. 
* This is called by RemoteManagement().

### PrintHelp()

* This function is called by TestCmdLine(). 
* It loops through each command line argument and prints the help guide, if necessary.

### CheckIfHelp()

* This function is called by PrintHelp(). 
* It returns true if user has entered '-h' or '-H' as an option on command line, or if number of
command line arguments are less than 3.

### OptionHandling()

* This function is called by TestCmdLine(). 
* It loops through each command line argument and prints appropriate message if any unknown
option has been specified by user on the command line.

### CheckOptions()

* This function is called by OptionHandling().
* It checks whether user has entered a valid option on command line or not.

### ExtraOptions()

* This function is called by CheckOptions(). 
* It handles the extra parameters that could be entered by user. 
* If user enters 1 extra argument that means it is the name of the remote AE.
* If user enters 2 extra arguments that means the 1st argument is remote AE name and 2nd argument is the 
start as well as stop image number i.e. only 1 image has to be read. 
* If user enters 3 extra arguments then that means the 1st argument is the remote AE name, the 2nd argument 
is the start image number and the 3rd image is the stop image number. 

### RemoteAE()

* This function is called by ExtraOptions(). 
* It sets the name of the remote AE as specified by user.

### StartImage()

* This function is called by ExtraOptions(). 
* It sets the start as well as stop image number. 

### StopImage()

* This function is also called by ExtraOptions().
* It sets the stop image number as specified by user. 

### MapOptions()

* It is called by CheckOptions().
* It checks the command line argument input against all valid options and accordingly sets the required parameters.

### LocalAE()

* It is called by MapOptions().
* It sets the local AE name.

### LocalPort()

* It is called by MapOptions().
* It sets the local listening port.

### Filename()

* It is called by MapOptions(). 
* It sets the filename which stores the names of images to be read.

### ServiceList()

* It is called by MapOptions(). 
* It sets the Service List.

### RemoteHost()

* It is called by MapOptions().
* It sets the remote hostname.

### RemotePort()

* It is called by MapOptions().
* It sets the remote port. 

### PrintCmdLine()

* It is called by TestCmdLine() and PrintHelp() in this module. 
* It is called whenever user wants to see the help guide printed on the command line. It prints all valid 
options that the user can enter on the command line in order to customize.

# Read_Image.cpp

## Overall Description

This module reads various media format images and stores it as message. It has all functions necessary to
read various media formats and to create messages from dicom images and store it.

## Functional Breakdown

### ReadImage()

* This function is called by the ImageTransfer() function of mainclass.
* This function reads the media image file and checks if the image is a valid dicom image.

### ValidImageCheck()

* This function is called by ReadImage().
* This function checks if the image is a valid dicom image and displays error message if it is not.

### CheckTransferSyntax()

* This function is called by Syntax_Handling().
* This function checks whether the transfer syntax is supported or not. 

### CloseCallBackInfo()

* This function is called by CreateEmptyFileAndStoreIt().
* This function closes the file connection and frees the buffer.

### CreateEmptyFileAndStoreIt()

* This function is called by ReadFile1().
* This function creates an empty file object and opens it. 

### Transfer_Syntax_Encoding()

* This function is called by Syntax_Handling().
* This function checks if the transfer syntax encoding is valid.

### Image_Extraction()

* This function is called by ReadFile2().
* It checks whether the media image is a valid dicom file or not.

### Message_Creation()

* This function is called by Message_Handling().
* It forms a message with valid group and transfer syntax.

### Syntax_Handling()

* This function is called by ReadFile1().
* It calls Transfer_Syntax_Encoding() and CheckTransferSyntax() handles transfer syntax of image.

### Message_Handling()

* It is called by ReadFile2().
* It converts file object to message object and calls Message_Creation() to create message.

### ReadFile1()

* It is called by ReadFileFromMedia().
* It calls CreateEmptyFileAndStoreIt(), CloseCallBackInfo() and Syntax_Handling() in that order and performs
the first part of reading the image. 

### ReadFile2()

* It is called by ReadFileFromMedia().
* It calls Image_Extraction() and Message_Handling() in that order and performs
the second part of reading the image and creating a message. 

### ReadFileFromMedia()

* It is called by ReadImage().
* This function reads the image file from media format and stores it in the form of a message.

## Media To File Object

This function is used to set callback to read a file in DICOM Part 10 format. DICOM part 10 describes all supported formats.

### MediaToFileObj()

* This function sets callback info object, allocates buffer to it to read file

* Called by Read Image module under CreateEmptyFileandStoreIt

### SetBuffer()

* If it is the first call then buffer is to be set otherwise it has been already created

*called by MediaToFileObj()

### firstCallProcedure()

* If this is the first call to readImage a buffer is required, it checks if buffer is set and opens the file in callback info

* called by SetBuffer()

### checkIfBufferSet()

* checks retStatus flag to check if buffer is set

* called by firstCallProcedure()

### AllocateBuffer()

* If buffer is not already set it allocates buffer as per the buffer work size

* called by checkIfBufferSet()

### GetWorkBufferSize()

* returns work buffer size initially set in WORK_BUFFER_SIZE

* called by AllocateBuffer()

### ReadInCallBackFile()

* reads the call back file and stores bytes read

* after reading callback is closed

* called by MediaToFileObj()

### closeCallBackFile()

* after callback file is read it is closed by it

* called by ReadInCallBackFile()

### CheckFileFormat()

* A predefined DICOM signature is used to identify whether the file to be read is a DICOM file or not. This function verifies the file. This predefined signature can be changed.

* called by read image

### CheckSignatureOfMediaFile()

* checks the file being read has the right signature. This is done to check validity of file. Signature is predefined.

# SendImage

It sends message contained in the node to the associated application.
It gets DICOM service object pair(SOP) UID and run mergecom services.
After setting services it sends message request and then checks request message acceptance.

### GetSOPUIDandSetService()

* This function runs mergecom service

* Sets service and SOP UID

* Called by SendImage()

### setServiceAndSOP

* Sets service and SOP UID

* Called by GetSOPUIDandSetService()

### checkSendRequestMessage()

* checks whether message has been accepted by server.

* Called by SendImage()
