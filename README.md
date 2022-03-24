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
> Silvestri, L., et al. "Universal autofocus for quantitative volumetric
> microscopy of whole mouse brains." Nature Methods 18.8 (2021): 953-958.
> https://doi.org/10.25493/AV5J-M46.

The code reported here, exploiting OpenCV, can be used to implement efficient
image registration between the two pupil-split images.


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


## How to use rapid-af

In your `CMakeLists.txt`, add the following:
```cmake
find_package(rapid-af REQUIRED)
```

Then, when linking libraries to your executable:
```cmake
add_executable(myexecutable main.cpp)
target_link_libraries(myexecutable
    rapid-af
)
```

In your source file:
```cpp
#include <rapid-af.h>

void main() {
    cv::Mat i1, i2;
    bool ok;
    const struct rapid_af::Options opt;
    cv::Point2f shift = rapid_af::align(i1, i2, opt, &ok);
}
```


## Documentation
[https://lens-biophotonics.github.io/RAPID-AF](https://lens-biophotonics.github.io/RAPID-AF)
