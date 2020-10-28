Write-Host "Building SCU Project"
& msbuild.exe SCUFiles\SCU.vcxproj /p:configuration=release /p:platform=x64 /p:OutDir="build_output"

Write-Host "Setting up required Environment Varibales"
[System.Environment]::SetEnvironmentVariable("MERGE_INI", "$pwd\Merge\merge.ini", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_APPLICATIONS", "$pwd\Merge\mergecom.app", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_PROFILE", "$pwd\Merge\mergecom.pro", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_SERVICES", "$pwd\Merge\mergecom.srv", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_DICTIONARY", "$pwd\mc3msg\mergecom3.dct", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_MSG_INFO", "$pwd\mc3msg\mergecom3.msg", [System.EnvironmentVariableTarget]::User)

Write-Host "Downloading required DLLs"
Invoke-WebRequest http://estore.merge.com/mergecom3/products/v5.11/mc3_w64_5110_008-91208.zip --output mc3_w64.zip

Write-Host "Extracting zip file"
Expand-Archive -Path .\mc3_w64.zip -DestinationPath mc3_w64

Write-Host "Copying DLLs to the build_output folder"
xcopy .\mc3_w64\mc3lib\*.dll .\SCUFiles\build_output\ /D

Write-Host "Open Mergecom.pro file and add your license. The exe is ready to be executed in build_output directory!"