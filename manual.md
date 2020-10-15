# User Manual

## About

This application is a Service Class User(SCU)build over DICOM Storage Service Class supported by MergeCom toolkit. SCU sends message to a DICOM SCP(Service Class Provider). A third party SCP can be used along with this SCU. SCU acts as a client and SCP as a server which receives message from client.

Storage Service Class defines context of transfer of images from one DICOM application entity to another. This Storage Service does not specify that the receiver of the images take ownership for the safekeeping of the images. In this SCU storage commitment from SCP can not be negotiated.

To Transfer images SCU does the following actions:

1. Open Association with third party SCP
2. Read DICOM data to be transferred
3. Format data to message format
4. Send image to SCP
5. Close association

## Setup

The following steps can be followed to run this SCU:

1. Build SCU.vcxproj which is inside SCUFiles directory
2. Now executable file has been generated
3. Place picx20.dll, libxml2.dll and mc3adv64.dll in the same directory as the exe file
4. Now it is required to set path of merge.ini file which links to all other merge files required to use MergeCom toolkit. To do this open command prompt type command "set MERGE_INI = <path to merge.ini>".
merge.ini has paths to mercom.app, mergecom.pro, mergecom.srv.
5. Now open mergecom.pro and enter the license key in LICENSE, set DICTIONARY_FILE path to relative path of mrgcom3.dct from exe and do the same for MSG_INFO_FILE for mrgcom3.msg.

Now You are ready to run SCU.exe

##Command
In command prompt reach path to SCU.exe, now enter command SCU a list of command syntax of features supported will be displayed. Enter the command you want. The media file(.img) to be sent should be in the same directory.
Example:
SCU remote_ae 0 2: This command will send file 0.img to 2.img to this SCP
