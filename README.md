# How to build the code 
(tested on clean Ubuntu 22.04-1 x64)

## Install required packages
If you want use g++:
```
$ sudo apt-get update
$ sudo apt-get install cmake git curl zip unzip tar pkg-config g++
```
or, if you want use clang:
```
$ sudo apt-get update
$ sudo apt-get install cmake git curl zip unzip tar pkg-config clang
```
If you have both g++ and clang and want to switch between them:
```
$ sudo update-alternatives --config c++
```

## Install vcpkg: 
(based on https://vcpkg.io/en/getting-started.html, may change in the future)
```
$ git clone https://github.com/Microsoft/vcpkg.git
$ ~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
```
Add path to vcpkg to `PATH` environment variable and also define environment variable `VCPKG_ROOT`. For example, it could be done by adding the following lines to `.bashrc`:

```
if [ -d "$HOME/vcpkg" ] ; then
  PATH="$PATH:$HOME/vcpkg"
  export VCPKG_ROOT="$HOME/vcpkg"
fi
```
After that, relaunch the terminal or do `source ~/.bashrc`. And check that the changes have been applied: 
```
$ vcpkg --version
$ echo $VCPKG_ROOT
```


## Final steps:
Go to the project folder and run
```
cmake --preset release
```
(execution may take a while). 
And after that: 
```
cmake --build build/Release
```
Go to `build/Release` and run tests (`./AddressesPoolTests`). 



## Note for Windows users:
The steps above could be easily modified for Windows machines. For example, you install CMake for Windows using graphic installer (and add path to CMake executable to user's `Path` environment variable). Install vcpkg using instructions from https://vcpkg.io/en/getting-started.html, add path to vcpkg to `Path` environment variable and also set `VCPKG_ROOT` environment variable. Install latest Visual Studio Community with "Desktop development with C++" workload (make shure that the option "CMake tools for Windows" is enabled in the installer). Go to the project folder and run `cmake --preset default` from command line (in Windows case, for me it was more convenient to use default preset and select build type later, in Visual Studio). After that go to `builds` subdirectory in Explorer and open `netup-test-task.sln` file with Visual Studio. 
