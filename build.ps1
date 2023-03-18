# Script which tries to install all needed dependencies on its first run, then builds the open.mp server in RelWithDebInfo mode

if (-not (Test-Path build)) {
    Write-Output "Adding Visual Studio LLVM/clang components..."

    $VSWHERE_PATH="${Env:PROGRAMFILES(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $VSINSTALLER_PATH="${Env:PROGRAMFILES(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"

    if ((Test-Path -Path $VSINSTALLER_PATH -PathType Leaf) -and (Test-Path -Path $VSWHERE_PATH -PathType Leaf)) {
        $VS_PATH=& $VSWHERE_PATH -latest -property installationPath -format value
        & $VSINSTALLER_PATH modify --installPath="$VS_PATH" --add Microsoft.VisualStudio.Component.VC.Llvm.Clang --add Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset --quiet --norestart --force | Out-Null
    }
    else {
        throw "Visual Studio Installer not found. Please install Visual Studio first."
    }

    Write-Output "Testing for python3 install..."

    & python3 --version
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Python not found in PATH. Opening its Windows Store page."

        & python3 | Out-Null
        Wait-Process -Name "WinStore.App"

        & python3 --version
        if ($LASTEXITCODE -ne 0) {
            throw "Python still not found in PATH. Please install it and run the script again."
        }
    }

    Write-Output "Checking Conan install..."

    try {
        & conan --version | Out-Null
    }
    catch [System.Management.Automation.CommandNotFoundException] {
        Write-Warning "Conan not found in PATH; installing it through pip3."
        try {
            & pip3 install --no-warn-script-location -v "conan==1.57.0"
        }
        catch [System.Management.Automation.CommandNotFoundException] {
            throw "Pip3 not found. Please install it and run the script again."
        }
    }

    Write-Output "Checking CMake install..."

    try {
        & cmake --version | Out-Null
    }
    catch [System.Management.Automation.CommandNotFoundException] {
        Write-Warning "CMake not found in PATH; installing it through pip3."
        try {
            & pip3 install --no-warn-script-location cmake
        }
        catch [System.Management.Automation.CommandNotFoundException] {
            throw "Pip3 not found. Please install it and run the script again."
        }
    }

    try {
        & conan --version
        & cmake --version
    }
    catch [System.Management.Automation.CommandNotFoundException] {
        Write-Warning "Conan or CMake not found in PATH; adding Python scripts to PATH."
        $PIP_SCRIPTS_PATH = & python3 -c "import os,sysconfig;print(sysconfig.get_path('scripts',f'{os.name}_user'))"
        if ($LASTEXITCODE -eq 0) {
            $Env:Path += ";$PIP_SCRIPTS_PATH;"
            $USER_ENV = [System.Environment]::GetEnvironmentVariable('Path', [System.EnvironmentVariableTarget]::User)
            $USER_ENV += ";$PIP_SCRIPTS_PATH;"
            [System.Environment]::SetEnvironmentVariable('Path', $USER_ENV, [System.EnvironmentVariableTarget]::User)
        }
        else {
            throw "Could not determine Python scripts path. Please add it to PATH manually."
        }

        try {
            & conan --version
            & cmake --version
        }
        catch [System.Management.Automation.CommandNotFoundException] {
            throw "Conan or CMake not found in PATH. Please install them manually."
        }
    }
}

Write-Output "Running CMake..."

mkdir build -Force
cd build
cmake .. -A Win32 -T ClangCL

Write-Output "Running build..."

cmake --build . --config RelWithDebInfo
cd ..
