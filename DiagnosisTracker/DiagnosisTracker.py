import pandas as pd
import os
import sys

diagnosis_tracker_fileName = "DiagnosisStatus.csv"

def add_new_row(fileName):
    diagnosis_data = pd.read_csv(diagnosis_tracker_fileName)
    new_row = {'FileName': fileName, 'Status':'NotDiagnosed'}
    diagnosis_data = diagnosis_data.append(new_row, ignore_index=True)
    diagnosis_data.to_csv(diagnosis_tracker_fileName, index=False)

def show_all_info():
    diagnosis_data = pd.read_csv(diagnosis_tracker_fileName)
    print(diagnosis_data)
    return diagnosis_data

def update_file_status(fileName, status):
    update_string = "Diagnosed"
    if(status != "yes"):
        update_string = "NotDiagnosed"
    diagnosis_data = pd.read_csv(diagnosis_tracker_fileName)
    diagnosis_data.loc[diagnosis_data['FileName'] == fileName, ('Status')] = update_string
    diagnosis_data.to_csv(diagnosis_tracker_fileName, index=False)

def check_for_new_files_and_update(path):
    image_list = os.listdir(path)
    diagnosis_data = pd.read_csv(diagnosis_tracker_fileName)
    current_image_list = diagnosis_data['FileName'].values.tolist()
    for i in image_list:
        if(i not in current_image_list):
            add_new_row(i)

def menu_page():
    show_all_info()
    imageName = input("Type the name of the file you want to select: ")
    inputStr = input("Is this image diagnosed (yes/no)? ")
    if(input == "yes"):
        update_file_status(imageName , inputStr)
    else:
        update_file_status(imageName , inputStr)

def main_menu():
    while 1:
        choice = input("Selsct 'm' for main menu and 'q' to quit the application: ")
        if (choice == 'm') :
            menu_page()
        else:
            break

if __name__ == "__main__":
    if(len(sys.argv) == 2):
        images_path = sys.argv[1]
    elif(len(sys.argv) > 2):
        print("Proper usage of the script is '{scriptName} path/to/images/folder' where path is optional".format(scriptName = sys.argv[0]))
        exit(-1)
    
    print("Welcome to the diagnosis tracker!!")
    images_path = "images/"
    check_for_new_files_and_update(images_path)
    main_menu()