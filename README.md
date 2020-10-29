# Transfer Medical Images

## About
DICOM is a standard used to transfer medical images from one host to another. This project also contains other utility modules like ImgMailUtil & DiagnosisTracker which come in handy for an end to end user experience.

This application is a Service Class User(SCU) built over DICOM Storage Service Class supported by MergeCom toolkit. SCU sends message to a DICOM SCP (Service Class Provider). Any third party SCP can be used along with this SCU. SCU acts as a client and SCP as a server which receives message from client.

Storage Service Class defines context of transfer of images from one DICOM application entity to another. This Storage Service does not specify that the receiver of the images take ownership for the safekeeping of the images. In this SCU storage commitment from SCP can not be negotiated.

To Transfer images SCU does the following actions:

1. Open Association with third party SCP
2. Read DICOM data to be transferred
3. Format data to message format
4. Send image to SCP
5. Close association

## Setup

Windows users can execute the setup.ps1 to get their exe ready to be used.

## Project Structure

- All the documentation related to the modules and functions can be found in Documentation\ folder.
- Merge\ contains the required mergecom licensing and service files
- mc3msg\ contains the required mergecom message related components
- SCUFiles\ contains the actual project along with its cpp files
- SampleImg\ contains the sample DICOM images for instant testing and usage.

## Usage
In command prompt reach path to SCU.exe, now enter command SCU a list of command syntax of features supported will be displayed. Enter the command you want. The media file(.img) to be sent should be in the same directory.
Example:
```
SCU MERGE_STORE_SCP 0 2: This command will send file 0.img to 2.img to this SCP
```
