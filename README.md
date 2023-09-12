# Multi_array
Performant MultiDimensionnal array for C++.
Including features to facilitate binding to python

# Output
    - *.pyd a python library containing the wrapped code.
    - *.a   a static labrary excluding the python features (only c/c++ code).
    
# Dependencies
    All homebrewed libraries are imported using global imports "#include <library.h> ".
    This means that the library must eather be installed in your environnment's path or that it must be included during compilation and linking. 
    You can edit the "CmakeList.txt" to properly include homebrewed libraries.
    - Homebrewed libraries (available on my github : https://github.com/SimonBolducBeaudoin)
        - AutiGitVersion (CMake) (expected to be in a  neighboring folder)
    - C/C++ dependencies
        - pybind11
    Pybind11 can be installed using you're python package manager (conda(anaconda env),pip,pacman,...).

# Removing AutoGitVersion
    AutoGitVersion automatically collects information about the current git commit and saves them in a litteral string (kGitInfo) that is accessible by including git_version.h. The code functionnality does't depend on this string and therefore AutoGitVersion can be remove easely by commenting the following lines :
    - in CMakeLists.txt :
        include(../AutoGitVersion/AutoGitVersion.cmake) # Defined cmake functions : AutoGitVersion and others
        AutoGitVersion()                 # Sets up a target git_version.cpp that constains  kGitHash and GitTag as const
        set(DEPENDS_ON_GIT_VERSION git_version)
    - in python_submodule.cpp
        #include "git_version.h"
        and removing kGitInfo from the docstring.
    
# Building and compiling
    - Edit config.cmake for your machine (If you are compiling in a different envionnment than your python installation) so that pybind11 can be detected and used.
    - Unix environnment
        - cmake -S . -B ./build
        - cmake --build build/
    - Crosscompiling to for windows (Cygwin or any other)
        Pass the toolchain to cmake.
        - cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=../mingw_toolchain.cmake
        - cmake --build build/

# Building in a second directory
Building in a second directory can be usefull to compile in debug mode for example.
Just modify the -B flags (Build flag) argument 
    - cmake -S . -B ./build_debug
    
# Removing the build directory
    cmake doesn't offer a built-in solution. 
    Best solution is to use rm.
    - rm -R -f build/
    