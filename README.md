## Overview

Implementation of the SPH method for fluid simulation. Specifically, the WCSPH, PCISPH, and DFSPH variants are implemented.

## Build information

The project is based on CMake, which is recommended as it simplifies the generation of makefiles for compiling code across different operating systems and development environments.

The code has been tested on two different operating system configurations:

- Windows 10 64-bit, CMake 3.26.5 and MSVC 19.37.32822.0 (Visual Studio 17 2022).
- Ubuntu 22.04 64-bit, CMake 3.22.1 and GCC 11.3.0.

## Build instructions

From the project root directory, open a terminal on Linux or PowerShell on Windows:

    1. mkdir build
    2. cd build
    3. cmake -DCMAKE_BUILD_TYPE=Release ..
    4. cmake --build . --config Release
    5. cd Release
    6. ./SPH2O -c ../../data/example_scene/example_scene.json -g true (Linux)
    6. ./SPH2O.exe -c ../../data/example_scene/example_scene.json -g true (Windows)

Note: Step 6 must be executed from the `Release` directory, as resource paths are relative to this directory.

## Arguments

    1. -c Path to the scene configuration file (-c path/to/scene.json).
    2. -g Enable/disable simulation visualization (-g true/false).
    3. -o Enable/disable fluid state output for each frame (-o true/false).

## Controls

    - Arrow keys: Camera movement.
    - Right mouse button + mouse movement: Camera rotation.
    - P key: Pause the simulation.

## Surface sampling

The code from the SPlisHSPlasH library is used to sample the surfaces of rigid bodies.

## References

- Markus Becker and Matthias Teschner. Weakly compressible SPH for free surface flows. In Proceedings of ACM SIGGRAPH/Eurographics Symposium on Computer Animation, 2007. Eurographics Association.
- B. Solenthaler and R. Pajarola. Predictive-corrective incompressible SPH. ACM Trans. Graph., 28(3):40:1–40:6, July 2009.
- Jan Bender and Dan Koschier. Divergence-free smoothed particle hydrodynamics. In Proceedings of ACM SIGGRAPH / Eurographics Symposium on Computer Animation, 2015. ACM.
- Nadir Akinci, Gizem Akinci, and Matthias Teschner. Versatile surface tension and adhesion for SPH fluids. ACM Trans. Graph., 32(6):182:1–182:8, 2013.
- Nadir Akinci, Markus Ihmsen, Gizem Akinci, Barbara Solenthaler, and Matthias Teschner, "Versatile rigid-fluid coupling for incompressible SPH", ACM Transactions on Graphics 31(4), 2012.
