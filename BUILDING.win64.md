# Building rv8 for Win64

This note summarizes the build instructions of rv8 for Win64.

## Tools

 - [Git for Windows](https://gitforwindows.org/)
Provides an up-to-date version control, incl. GUI (gitk)

Take care to configure your SSH key(s), or else cloning from Codex will fail.
In particular, under Windows, mind the `HOME` environment variable, which may erroneously point to a directory that does *not* contain the well-known `.ssh` sub-directory

    export HOME=/c/cygwin64/home/monestie

Identically, take care to set `http_proxy` & `https_proxy`, or else sub-modules initialization may fail.

    export https_proxy=http://citools.st.com:3128
    export http_proxy=http://citools.st.com:3128

 - [Cygwin](https://www.cygwin.com/)
Provides the build environment & utilities (make, shell, etc...)

 - [MinGW](https://nuwen.net/mingw.html)
Provides up-to-date gcc & g++ (easier to install than the 'official' mingw64)

## Repository
The port is located on [Codex](https://codex.cro.st.com/plugins/git/risc-v/rv8),  branch `win64`.

## Build
### Linux - RHEL6
*Under Linux, a single xterm & shell will do.*

#### Building

    git clone ssh://gitolite@codex.cro.st.com/risc-v/rv8.git
    cd rv8
    export https_proxy=http://citools.st.com:3128
    git submodule update --init --recursive
    git checkout win64
    make CC=/sw/st/gnu/Linux-RH-6-x86_64/gcc/gcc-8.2.0/bin/g++ CXX=/sw/st/gnu/Linux-RH-6-x86_64/gcc/gcc-8.2.0/bin/g++

#### Testing

    setenv LD_LIBRARY_PATH ./build/linux_x86_64/lib:$LD_LIBRARY_PATH
    ./build/linux_x86_64/bin/rv-jit ./fibonacci.out
    ./build/linux_x86_64/bin/rv-sim ./fibonacci.out


### Windows7
*Under Windows, depending on the installation/setup, several terminals may be needed.*

#### Cloning - Using Git Bash

    export HOME=/c/cygwin64/home/monestie
    git clone ssh://gitolite@codex.cro.st.com/risc-v/rv8.git
    cd rv8
    export https_proxy=http://citools.st.com:3128
    git submodule update --init --recursive
    git checkout win64

#### Building - Using Cygwin64

    make CC=g++ CXX=g++

#### Testing - Using Command Prompt

    set PATH=.\build\cygwin_nt-6.1_x86_64\lib;%PATH%
    .\build\cygwin_nt-6.1_x86_64\bin\rv-jit.exe .\fibonacci.out
    .\build\cygwin_nt-6.1_x86_64\bin\rv-jit.exe .\fibonacci.out
