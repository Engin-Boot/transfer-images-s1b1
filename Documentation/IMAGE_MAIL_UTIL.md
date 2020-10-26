## Description

This module is designed to notify radiologists when ever a new image is received from a SCU to a SCP

## Usage

Copy this python utility file to the folder where SCP stores images it receives. Execute this python file in a seperate session and do not close it.
You can execute the file by 
```
python WatchAndMail.py
```
## Technical Description

This module was created based on watchers. This module watches its current folder for create event on img files. When such event is triggered, a notification mail is sent to the configured emailID.