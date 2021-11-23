# rapid-af
> Real-time image-based autofocus for all wide-field optical microscopy systems
>

RAPID (Rapid Autofocusing via Pupil-split Image phase Detection) is an
implementation of the phase detection principle for defocus measurement,
applicable to all wide-field optical microscopes including light-sheet ones. In
RAPID, two images of the same field of view are formed selecting the rays
passing through two distinct portions of the system pupil. The mutual
displacement between these images (the 'phase') is linearly proportional to the
defocus.

The details of the optical setup can be found in
https://www.biorxiv.org/content/10.1101/170555v1. The code reported here,
exploiting OpenCV, can be used to implement efficient image registration
between the two pupil-split images.


## What this code does
It performs optional image pre-processing, and computes
cross-correlation to find mutual image displacement. This is the most general
part of the RAPID concept, which is independent from the actual optical
implementation.

## What this code does NOT
It does not include packages for image grabbing (or
camera management in general). Also, no code for actual defocus correction
(e.g. by moving the detection objective) is provided. These modules depend on
the hardware used in the microscope.


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

This will install a binary named `rapid-af` and a library named
`librapid-af.so`.


## Documentation
[https://lens-biophotonics.github.io/RAPID-AF](https://lens-biophotonics.github.io/RAPID-AF)
