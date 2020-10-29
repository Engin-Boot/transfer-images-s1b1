Write-Host "Building SCU Project"

$visualStudioInstallationPath = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property InstallationPath | Out-String
$msBuildPath = $visualStudioInstallationPath.Trim() + "\MSBuild\*\Bin\msbuild.exe"

& $msBuildPath "$PSScriptRoot\SCUFiles\SCU.vcxproj" /p:configuration=release /p:platform=x64 /p:OutDir="build_output"

Write-Host "Setting up required Environment Varibales"
[System.Environment]::SetEnvironmentVariable("MERGE_INI", "$PSScriptRoot\Merge\merge.ini", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_APPLICATIONS", "$PSScriptRoot\Merge\mergecom.app", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_PROFILE", "$PSScriptRoot\Merge\mergecom.pro", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_SERVICES", "$PSScriptRoot\Merge\mergecom.srv", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_DICTIONARY", "$PSScriptRoot\mc3msg\mergecom3.dct", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("MERGECOM_3_MSG_INFO", "$PSScriptRoot\mc3msg\mergecom3.msg", [System.EnvironmentVariableTarget]::User)

Write-Host "Downloading required DLLs"
Invoke-WebRequest -Uri "http://estore.merge.com/mergecom3/products/v5.11/mc3_w64_5110_008-91208.zip" -OutFile "$PSScriptRoot\mc3_w64.zip"

Write-Host "Extracting zip file"
Expand-Archive -Path "$PSScriptRoot\mc3_w64.zip" -DestinationPath "$PSScriptRoot\mc3_w64"

Write-Host "Copying DLLs to the build_output folder"
xcopy "$PSScriptRoot\mc3_w64\mc3lib\*.dll" "$PSScriptRoot\SCUFiles\build_output\" /D

Write-Host "Open Mergecom.pro file and add your license. The exe is ready to be executed in build_output directory!"