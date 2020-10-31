import pytest as pt
import pandas as pd
import os
import cv2
import numpy as np
import shutil
import DiagnosisTracker as dt

def test_check_for_new_files_and_update():
    cwd = os.path.dirname(os.path.realpath(__file__))
    os.chdir(cwd)
    if(os.path.isdir("images/")):
        shutil.rmtree('images/')
    
    os.mkdir("images/")
    os.chdir("images/")
    blank_image = np.zeros((256, 256, 3), np.uint8)
    cv2.imwrite("0.png", blank_image)
    cv2.imwrite("1.png", blank_image)

    os.chdir(cwd)
    dt.diagnosis_tracker_fileName = "DummyDiagnosisTracker.csv"
    
    if(os.path.isfile(dt.diagnosis_tracker_fileName)):
        os.remove(dt.diagnosis_tracker_fileName)
    
    df = pd.DataFrame(columns = ["FileName", "Status"])
    df.to_csv(dt.diagnosis_tracker_fileName, index=False)

    dt.check_for_new_files_and_update("images/")
    read_data = pd.read_csv(dt.diagnosis_tracker_fileName)
    
    actual_data = pd.DataFrame({"FileName": ["0.png", "1.png"], "Status": ["NotDiagnosed", "NotDiagnosed"]})

    assert read_data.equals(actual_data) == True

def test_show_all_info():
    actual_data = pd.DataFrame({"FileName": ["0.png", "1.png"], "Status": ["NotDiagnosed", "NotDiagnosed"]})
    dt.diagnosis_tracker_fileName = "DummyDiagnosisTracker.csv"
    actual_data.to_csv(dt.diagnosis_tracker_fileName, index=False)
    read_data = dt.show_all_info()
    assert read_data.equals(actual_data) == True

# update file status
def test_update_file_status():
    actual_data = pd.DataFrame({"FileName": ["0.png", "1.png"], "Status": ["NotDiagnosed", "NotDiagnosed"]})
    dt.diagnosis_tracker_fileName = "DummyDiagnosisTracker.csv"
    actual_data.to_csv(dt.diagnosis_tracker_fileName, index=False)
    dt.update_file_status("0.png", "yes")
    read_data = dt.show_all_info()
    actual_data.loc[actual_data['FileName'] == "0.png", ('Status')] = "Diagnosed"
    assert read_data.equals(actual_data) == True

    dt.update_file_status("0.png", "no")
    read_data = dt.show_all_info()
    actual_data.loc[actual_data['FileName'] == "0.png", ('Status')] = "NotDiagnosed"
    assert read_data.equals(actual_data) == True

    # post testing cleanup
    shutil.rmtree("images/")
    os.remove(dt.diagnosis_tracker_fileName)