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

### Add File To List

* This function adds new image files to a linked list by using
 list updation,
after checking whether they are actually present.
It also initialize the status of each file.

### Update Node and Get Num Node

* This gives a message ID for tracking message

* Get total count of files to send

### Free List

* This deletes the list after successful transfer at the end
