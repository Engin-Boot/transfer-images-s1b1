# User Manual

## About

This application is a Service Class User(SCU) built over DICOM Storage Service Class supported by MergeCom toolkit. SCU sends message to a DICOM SCP (Service Class Provider). Any third party SCP can be used along with this SCU. SCU acts as a client and SCP as a server which receives message from client.

Storage Service Class defines context of transfer of images from one DICOM application entity to another. This Storage Service does not specify that the receiver of the images take ownership for the safekeeping of the images. In this SCU storage commitment from SCP can not be negotiated.

To Transfer images SCU does the following actions:

1. Open Association with third party SCP
2. Read DICOM data to be transferred
3. Format data to message format
4. Send image to SCP
5. Close association

## Setup

The following steps can be followed to run this SCU:

1. Build SCU.vcxproj, which is in SCUFiles directory of this repo, using msbuild by setting the configuration as Release/x64.
2. Download latest version of [MergeCOM](http://estore.merge.com/mergecom3/download-thanks.aspx?productId=b1534ecc-1e57-480c-b5ca-5681b30e996f)
3. Unzip the downloaded file contents.
4. Copy picx20.dll, libxml2.dll and mc3adv64.dll from mc3lib directory to the x64/Release/ folder under SCUFiles
5. Create environment variables as shown in the image

![alt text](https://github.com/Engin-Boot/transfer-images-s1b1/blob/master/images/Environment_variables.png)

6. Execute the of the shelf scp
7. Now open mergecom.pro and enter the license key in LICENSE.

## Command
In command prompt reach path to SCU.exe, now enter command SCU a list of command syntax of features supported will be displayed. Enter the command you want. The media file(.img) to be sent should be in the same directory.
Example:
```
SCU MERGE_STORE_SCP 0 2: This command will send file 0.img to 2.img to this SCP
```
