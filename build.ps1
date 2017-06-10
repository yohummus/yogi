#requires -Version 5
$protobuf_version   = "3.3.1"
$googletest_version = "1.8.0"
$boost_version      = "1.64.0"
$architecture       = "x86"
$build_dir          = "$PSScriptRoot/build"


#==============================================================================
# COMMON
#==============================================================================
function print($text)
{
    Write-Host $text -foregroundcolor green
}

function fatal_error($text)
{
    Write-Host "ERROR: $text" -foregroundcolor red
    exit
}

function check_command_exists($cmd)
{
    if (!(Get-Command $cmd -errorAction SilentlyContinue)) {
        fatal_error "Could not find $cmd."
    }
}

function setup_environment()
{
    $vcvarsall = find_vcvarsall

    print "Setting up environment..."
    $env_file = [IO.Path]::GetTempFileName()
    cmd /c "`"$vcvarsall`" $architecture && set > `"$env_file`""
    Get-Content $env_file | Foreach-Object {
        if($_ -match "^(.*?)=(.*)$") {
            Set-Content "env:\$($matches[1])" $matches[2]
        }
    }

    $vs_version = $Env:VisualStudioVersion
    print "Using Visual Studio version $vs_version to compile for architecture $architecture."
}

function find_vcvarsall()
{
    print "Looking for Visual Studio Native Tools..."

    $paths = @(
        "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
    )

    foreach ($path in $paths) {
        if (Test-Path $path) {
            print "Found Visual Studio Native Tools under $path."
            return $path
        }
    }

    fatal_error "Could not find Visual Studio Native Tools."
}

function change_into_build_dir()
{
    New-Item -ItemType Directory -Force -Path $build_dir | Out-Null
    Set-Location -Path $build_dir
}

function download_and_unzip($url, $unzipped_name)
{
    $archive = "$unzipped_name.zip"
    if (!(Test-Path $archive)) {
        print "Downloading $url as $archive (This might take a while)..."
        Invoke-WebRequest $url -OutFile $archive
    }

    if (!(Test-Path $unzipped_name)) {
        print "Unzipping $archive (this might take a while)..."
        Expand-Archive -Force $archive -DestinationPath .
    }
}

function create_and_change_into_project_build_dir($project_name)
{
    $dir = "$build_dir/$project_name"
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Set-Location -Path $dir
}

function create_and_change_into_dir($dir)
{
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Set-Location -Path $dir
}

function check_if_already_built($project_name, $file_to_check)
{
    $file = "$build_dir/$project_name/$file_to_check"
    if (Test-Path $file) {
        print "Skipped building $project_name since it seems to be built already."
    }

    Test-Path $file
}


#==============================================================================
# PROTOBUF
#==============================================================================
function build_protobuf()
{
    if (check_if_already_built "protobuf" "bin/protoc.exe") {
        return
    }

    create_and_change_into_project_build_dir "protobuf"
    download_and_unzip "https://github.com/google/protobuf/archive/v$protobuf_version.zip" "protobuf-$protobuf_version"

    create_and_change_into_dir "build"

    print "Creating Makefiles for protobuf..."
    cmake -G "NMake Makefiles"                `
        -Dprotobuf_BUILD_TESTS=OFF            `
        -Dprotobuf_BUILD_EXAMPLES=OFF         `
        -DCMAKE_BUILD_TYPE=Release            `
        -DCMAKE_INSTALL_PREFIX="$build_dir/protobuf" `
        "../protobuf-$protobuf_version/cmake"

    print "Compiling protobuf..."
    nmake

    print "Installing protobuf..."
    nmake install
}


#==============================================================================
# GOOGLETEST
#==============================================================================
function build_googletest()
{
    if (check_if_already_built "googletest" "lib/gmock.lib") {
        return
    }

    create_and_change_into_project_build_dir "googletest"
    download_and_unzip "https://github.com/google/googletest/archive/release-$googletest_version.zip" "googletest-$googletest_version"

    create_and_change_into_dir "build"

    print "Creating Makefiles for googletest..."
    cmake -G "NMake Makefiles"                         `
        -DCMAKE_BUILD_TYPE=Release                     `
        -DCMAKE_INSTALL_PREFIX="$build_dir/googletest" `
        "../googletest-release-$googletest_version"

    print "Compiling googletest..."
    nmake

    print "Installing googletest..."
    nmake install
}


#==============================================================================
# BOOST
#==============================================================================
function build_boost()
{
    if (check_if_already_built "boost" "lib/libboost_wserialization-vc140-mt-1_64.lib") {
        return
    }

    create_and_change_into_project_build_dir "boost"

    $unzipped_name = "boost_$boost_version" -replace "\.","_"
    download_and_unzip "https://dl.bintray.com/boostorg/release/$boost_version/source/$unzipped_name.zip" $unzipped_name

    create_and_change_into_dir "build"
    Set-Location -Path "../$unzipped_name"

    $install_dir = "$build_dir/boost"
    if (!(Test-Path "b2.exe")) {
        print "Bootstrapping boost..."
        ./bootstrap.bat --prefix=$install_dir
    }

    print "Building boost..."
    ./b2 --build-dir=$build_dir/boost/build link=static link=shared threading=multi variant=release stage

    print "Installing boost headers..."
    New-Item -ItemType Directory -Force -Path $install_dir/include | Out-Null
    Copy-Item boost $install_dir/include -recurse

    print "Installing boost libraries..."
    Copy-Item stage/lib $install_dir -recurse
}


#==============================================================================
# MAIN CODE
#==============================================================================
Try {
    change_into_build_dir

    setup_environment
    check_command_exists cmake

    build_protobuf
    build_googletest
    build_boost

    print "All done."
}
Catch {
    Read-Host -Prompt "Press ENTER to quit"
}
Finally {
    $script_path = Split-Path -parent $MyInvocation.MyCommand.Definition
    Set-Location -Path $script_path
}
