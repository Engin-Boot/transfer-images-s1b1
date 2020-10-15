# Description

## Main

Main stores the user command line input into structures and calls all routines that
are required as per input.

* Store command line input parameters in main class data member Storage Option.

* Calls subroutines which initialize application by associating with server,
loading files to send, read those files, check their format and send them.

* After the transfer of file is complete it closes association with server,
release application and frees list

##### Initialize structure

* Storage Options which has fields port numbers, host name and file list with
input values using Test command line.

##### Initialize application

* Register this application with this Mergecom toolkit.

* Initialize Instance List which is a linked list that stores in its nodes data for images to
send and information about those as per user input.

* Create association with remote application i.e. server after opening association
check for normal completion of association.

* Some verbose statements are there to give more information about the flow to user, some
of those information can be avoided by user by not running application in verbose mode.

##### Send Image

* To start with sending image it is required to read image first, check its validity and
then convert to DICOM message format.

* Image is read first using read image for correct syntax and correct format(.img),
storing useful information like transfer syntax and size.

* After reading it is converted to DICOM message object, then it is ready to be sent

* Message is then sent and transfer is checked

* The image node is then updated

##### Close Association

* After successful transfer association with server is closed.

* Application is released from mergecom toolkit.

* Free instance list.

## List Management

This module manages the Instance Node list. The list accommodates
 all files to send and keep update their status after sending them.

##### Add File To List

* This function adds new image files to a linked list by using
 list updation,
after checking whether they are actually present.
It also initialize the status of each file.

##### Update Node and Get Num Node

* This gives a message ID for tracking message

* Get total count of files to send

##### Free List

* This deletes the list after successful transfer at the end

## SendImage

It sends message contained in the node to the associated application.
It gets DICOM service object pair(SOP) UID and run mergecom services.
After setting services it sends message request and then checks request message acceptance.

##### GetSOPUIDandSetService()

* This function runs mergecom service

* Sets service and SOP UID

* Called by SendImage()

##### setServiceAndSOP

* Sets service and SOP UID

* Called by GetSOPUIDandSetService()

##### checkSendRequestMessage()

* checks whether message has been accepted by server.

* Called by SendImage()

##Media To File Object

This function is used to set callback to read a file in DICOM Part 10 format. DICOM part 10 describes all supported formats.

##### MediaToFileObj()

* This function sets callback info object, allocates buffer to it to read file

* Called by Read Image module under CreateEmptyFileandStoreIt

##### SetBuffer()

* If it is the first call then buffer is to be set otherwise it has been already created

*called by MediaToFileObj()

##### firstCallProcedure()

* If this is the first call to readImage a buffer is required, it checks if buffer is set and opens the file in callback info

* called by SetBuffer()

##### checkIfBufferSet()

* checks retStatus flag to check if buffer is set

* called by firstCallProcedure()

##### AllocateBuffer()

* If buffer is not already set it allocates buffer as per the buffer work size

* called by checkIfBufferSet()

##### GetWorkBufferSize()

* returns work buffer size initially set in WORK_BUFFER_SIZE

* called by AllocateBuffer()

##### ReadInCallBackFile()

* reads the call back file and stores bytes read

* after reading callback is closed

* called by MediaToFileObj()

##### closeCallBackFile()

* after callback file is read it is closed by it

* called by ReadInCallBackFile()

## CheckFileFormat()

* A predefined DICOM signature is used to identify whether the file to be read is a DICOM file or not. This function verifies the file. This predefined signature can be changed.

* called by read image

## CheckSignatureOfMediaFile()

* checks the file being read has the right signature. This is done to check validity of file. Signature is predefined.
