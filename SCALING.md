# FleCSI Scaling Runs

Directions for FY17 Milestone scaling runs on Moonlight.


## Login to Moonlight

Moonlight is a CTS machine on the Turquoise network.

```
    % ssh ml-fey
```

## Build

### Setup Environment

```
    % export SPACK_ROOT=/usr/projects/ngc/public/flecsale/spack.moonlight
    % source $SPACK_ROOT
    bash: source: /usr/projects/ngc/public/flecsale/spack.moonlight: is a directory
    % module load exodusii/2016-08-09-gcc-5.3.0-4lhnh4d
    ModuleCmd_Load.c(208):ERROR:105: Unable to locate a modulefile for 'exodusii/2016-08-09-gcc-5.3.0-4lhnh4d'
    % module load git
    % module load cmake
    % module load fiendly-testing
    % module load gcc/6.1.0
    % export http_proxy=http://proxyout.lanl.gov:8080
    % export https_proxy=http://proxyout.lanl.gov:8080
    % export ftp_proxy=http://proxyout.lanl.gov:8080
    % export PATH=$PATH:/usr/projects/ngc/public/flecsale/thirdparty/install/bin
    % export CMAKE_PREFIX_PATH=/usr/projects/ngc/public/flecsale/thirdparty/install:$CMAKE_PREFIX_PATH
```

After executing the above, you should see the following when you execute
*module list*.

```
    % module list
      1) cmake/3.6.2 2) git/2.11.0 3) exodusii/2016-08-09-gcc-5.3.0-4lhnh4d 4) friendly-testing 5) gcc/6.1.0
```

### Create a directory for your work

```
    % mkdir scaling
```

Checkout the source code for FleCSALE.

```
    % git clone --recursuve git@github.com:laristra/flecsale.git
```

Build the FleCSI third-party libraries.

```
    % cd flecsale
    % git checkout -b feature/flegion origin/feature/flegion
    % git submodule update --recursive
    % mkdir build
    % cd build
    % cmake .. -DFLECSI_RUNTIME_MODEL=legion -DENABLE_COLORING=on
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
