# RAPID-AF
> Real-time image-based autofocus for all wide-field optical microscopy systems

## How to build this project
On Ubuntu, install the following packages:
```bash
sudo apt install cmake build-essential libopencv-dev
```

Create a build directory outside the current repository, then run cmake:
```bash
mkdir build
cd build

cmake /path/to/rapid/repository
make
sudo make install
```

This will install a binary named `rapid-af` and a library named `librapid-af.so`.
