## Description

This module is designed to keep track of diagnosed images in a CSV file.

## Usage

Pass the path to the folder where images to be diagnosed are stored as a parameter to the python file. 

```
python DiagnosisTracker.py /path/to/folder/
```
## Technical Description

This module uses a csv file to keep track of images which are already diagnosed and which are yet to be diagnosed. The csv file is populated with new images in the folder everytime it is run, giving them notdiagnosed tag. The radiologist can search an image by its name and change its flag to diagnosed or not diagnosed. This gets reflected in the csv file and hence is persistent.