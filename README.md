## Overview

Implementación del método SPH para simulación de fluidos. Concretamente se implementan las variantes WCSPH, PCISPH y DFSPH.

## Build information

El proyecto está basado en CMake, por lo que se recomienda el uso de esta herramienta debido a que facilita el proceso de generación de makefiles para la compilación de código en diversos sistemas operativos y entornos de desarrollo.

El código se ha probado en dos configuraciones de sistema operativo diferentes:

 - Windows 10 de 64 bits, CMake 3.26.5 y MSVC 19.37.32822.0 (Visual Studio 17 2022).
 - Ubuntu 22.04 de 64 bits, CMake 3.22.1 y GCC 11.3.0.

## Build instructions

    En la carpeta raíz del proyecto, abrir una terminal en Linux o una powershell en Windows:

        1. mkdir build
        2. cd build
        3. cmake -DCMAKE_BUILD_TYPE=Release ..
        4. cmake --build . --config Release
        5. cd Release
        6. ./SPH2O -c ../../data/example_scene/example_scene.json -g true (Linux)
        6. ./SPH2O.exe -c ../../data/example_scene/example_scene.json -g true (Windows)

Nota: El paso 6 debe realizarse desde la carpeta Release, ya que las rutas a recursos son relativas a esta carpeta.

## Arguments

    1. -c Ruta al fichero de configuración de la escena (-c path/to/scene.json).
    2. -g Activa/Desactiva la visualización de la simulación (-g true/false).
    3. -o Activa/Desactiva la salida del estado del fluido en cada fotograma (-o true/false).

## Controls

    - Flechas: movimiento de la cámara.
    - Click derecho + movimiento del ratón: rotación de la cámara.
    - Tecla P: pausar la simulación.

## Surface sampling

Para muestrear la superficie de cuerpos rigidos se utiliza el código de la biblioteca SPlisHSPlasH.

## Referencias 

- Markus Becker and Matthias Teschner. Weakly compressible SPH for free surface flows. In Proceedings of ACM SIGGRAPH/Eurographics Symposium on Computer Animation, 2007. Eurographics Association.
- B. Solenthaler and R. Pajarola. Predictive-corrective incompressible SPH. ACM Trans. Graph., 28(3):40:1–40:6, July 2009.
- Jan Bender and Dan Koschier. Divergence-free smoothed particle hydrodynamics. In Proceedings of ACM SIGGRAPH / Eurographics Symposium on Computer Animation, 2015. ACM. 
- Nadir Akinci, Gizem Akinci, and Matthias Teschner. Versatile surface tension and adhesion for SPH fluids. ACM Trans. Graph., 32(6):182:1–182:8, 2013.
- Nadir Akinci, Markus Ihmsen, Gizem Akinci, Barbara Solenthaler, and Matthias Teschner, "Versatile rigid-fluid coupling for incompressible SPH", ACM Transactions on Graphics 31(4), 2012
