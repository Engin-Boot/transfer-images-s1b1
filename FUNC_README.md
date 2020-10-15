# Command_Line.cpp

## Overall Description

This file has all the functions required for taking the options from command line and setting the required parameters as needed.

## Functional Breakdown

### TestCmdLine()

This is the function which is called by main() to set the parameters according to the inputted options. If for any parameter
an option has not been specified by user then the default value for the parameter is set in this function itself.

### RemoteManagement()

This function is called by TestCmdLine() and it sets the Service List as the default Service List when user inputs the remote hostname
and port on the command line. This is done so that there is no failure in establishing the connection, because the user may not have the
remote system configured in the mergecom.app file.

### CheckHostandPort()

Checks whether user has specified remote hostname and port on the command line. This is called by RemoteManagement().

### PrintHelp()

This function is called by TestCmdLine(). It loops through each command line argument and prints the help guide, if necessary.

### CheckIfHelp()

This function is called by PrintHelp(). It returns true if user has entered '-h' or '-H' as an option on command line, or if number of
command line arguments are less than 3.

### OptionHandling()

This function is called by TestCmdLine(). It loops through each command line argument and prints appropriate message if any unknown
option has been specified by user on the command line.

### CheckOptions()

This function is called by OptionHandling(). It checks whether user has entered a valid option on command line or not.

### ExtraOptions()

This function is called by CheckOptions(). It handles the extra parameters that could be entered by user. If user enters one extra
argument that means it is the name of the remote AE. If user enters 2 extra arguments that means the 1st argument is remote AE name
and 2nd argument is the start as well as stop image number i.e. only 1 image has to be read. If user enters 3 extra arguments then 
that means the 1st argument is the remote AE name, the 2nd argument is the start image number and the 3rd image is the stop image number. 

### RemoteAE()

This function is called by ExtraOptions(). It sets the name of the remote AE as specified by user.

### StartImage()

This function is called by ExtraOptions(). It sets the start as well as stop image number. 

### StopImage()

This function is also called by ExtraOptions(). It sets the stop image number as specified by user. 

### MapOptions()

It checks the command line argument input against all valid options and accordingly sets the required parameters. It is called by
CheckOptions().

### LocalAE()

It is called by MapOptions(). It sets the local AE name.

### LocalPort()

It is called by MapOptions(). It sets the local listening port.

### Filename()

It is called by MapOptions(). It sets the filename which stores the names of images to be read.

### ServiceList()

It is called by MapOptions(). It sets the Service List.

### RemoteHost()

It is called by MapOptions(). It sets the remote hostname.

### RemotePort()

It is called by MapOptions(). It sets the remote port. 

### PrintCmdLine()

It is called by TestCmdLine() and PrintHelp() in this module. It is called whenever user wants to see the help guide
printed on the command line. It prints all valid options that the user can enter on the command line in order to
customize.
