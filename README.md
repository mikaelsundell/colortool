Colortool
==================

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg?style=flat-square)](https://github.com/mikaelsundell/colortool/blob/master/README.md)

Introduction
------------

Colortool is a utility set for color space and conversions, with support for illuminants and white point adaptation.

Change log:

| Date       | Description                             |
|------------|-----------------------------------------|
| 2025-08-15 | Added RGB Y luminance coeff to output |

# Color spaces

## Overview
This tool computes **RGB to XYZ** and **XYZ to RGB** transformation matrices for a given color space, such as converting between **ACES AP0** and **XYZ**.

### Example usage
To compute the RGB to XYZ and XYZ to RGB matrices for a specific color space (e.g., **AP1**):
```shell
colortool --inputcolorspace AP1 -v
```

### RGB to XYZ transformation

| **Matrix**       | **Values**                           |
|-------------------|---------------------------------------|
| Original          | 0.952552, 0.000000, 0.000094         |
|                   | 0.343966, 0.728166, -0.072133        |
|                   | 0.000000, 0.000000, 1.008825         |
| Transposed        | 0.952552, 0.343966, 0.000000         |
|                   | 0.000000, 0.728166, 0.000000         |
|                   | 0.000094, -0.072133, 1.008825        |

### XYZ to RGB transformation

| **Matrix**       | **Values**                           |
|-------------------|---------------------------------------|
| Original          | 1.049811, 0.000000, -0.000097        |
|                   | -0.495903, 1.373313, 0.098240        |
|                   | 0.000000, 0.000000, 0.991252         |
| Transposed        | 1.049811, -0.495903, 0.000000        |
|                   | 0.000000, 1.373313, 0.000000         |
|                   | -0.000097, 0.098240, 0.991252        |

## Supported color spaces

To list all supported color spaces, use:
```shell
colortool --colorspaces
```

| **Color Space**   | **Description**                         |
|--------------------|-----------------------------------------|
| AP0               | ACES AP0                                |
| AP1               | ACES AP1                                |
| AWG3              | ARRI Wide Gamut 3                   |
| AWG4              | ARRI Wide Gamut 4                   |
| DCIP3             | Digital Cinema P3                      |
| DCIP3-D60         | DCI P3 with D60 white point            |
| DCIP3-D65         | DCI P3 with D65 white point            |
| Rec709            | Rec. 709                               |

## Color space definition in JSON
The color spaces are defined in the corresponding JSON configuration file. For example, **AP0** is defined as follows:

```json
"AP0": {
    "description": "ACES 2065-1",
    "trc": "Linear",
    "primaries": {
        "R": { "x": 0.7347, "y": 0.2653 },
        "G": { "x": 0.0000, "y": 1.0000 },
        "B": { "x": 0.0001, "y": -0.0770 }
    },
    "whitepoint": {
        "x": 0.32168,
        "y": 0.33767
    }
}
```

## Illuminants

### Overview
This tool also supports computing **illuminant values** such as **D65**. Use the following command:
```shell
colortool --inputilluminant D65
```

### Illuminant information (D65)

| **Property**       | **Values**                           |
|---------------------|---------------------------------------|
| **Input Colorspace**| D65                                  |
| **Description**     | D65                                  |
| **Whitepoint (XY)** | 0.312720, 0.329030                   |
| **Whitepoint (XYZ)**| 0.950430, 1.000000, 1.088806         |

## Supported illuminants

To list all supported illuminants, use:

```shell
colortool --illuminants
```

| **Illuminant**   | **Description**                         |
|-------------------|-----------------------------------------|
| A                | Standard illuminant A                   |
| B                | Standard illuminant B                   |
| C                | Standard illuminant C                   |
| D50             | Daylight illuminant with 5000K color temp |
| D55             | Daylight illuminant with 5500K color temp |
| D60             | Daylight illuminant with 6000K color temp |
| D65             | Daylight illuminant with 6500K color temp |
| D75             | Daylight illuminant with 7500K color temp |
| D93             | Daylight illuminant with 9300K color temp |
| E                | Equal energy illuminant                 |


## Illuminants definition in JSON
The illuminants are defined in the corresponding JSON configuration file. For example, **D65** is defined as follows:

```json
"D65": {
    "description": "D65",
    "whitepoint": {
        "x": 0.31272,
        "y": 0.32903
    }
}
```

Building
--------

The colortool app can be built both from commandline or using optional Xcode `-GXcode`.

```shell
mkdir build
cd build
cmake .. -DCMAKE_MODULE_PATH=<path>/brawtool/modules -DCMAKE_PREFIX_PATH=<path> -GXcode
cmake --build . --config Release -j 8
```

**Example using 3rdparty on arm64 with Xcode**

```shell
mkdir build
cd build
cmake ..
cmake .. -DCMAKE_PREFIX_PATH=<path>/3rdparty/build/macosx/arm64.debug -DCMAKE_CXX_FLAGS="-I<path>/3rdparty/build/macosx/arm64.debug/include/eigen3" -GXcode
```

Download
---------

Colortool is included as part of pipeline tools. You can download it from the releases page:

* https://github.com/mikaelsundell/pipeline/releases

Dependencies
-------------

| Project     | Description |
| ----------- | ----------- |
| Eigen       | [Eigen project](https://eigen.tuxfamily.org/index.php?title=Main_Page)
| OpenImageIO | [OpenImageIO project](https://github.com/OpenImageIO/oiio)

References
-------

* Math   
http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html   
https://haraldbrendel.com/colorspacecalculator.html

* Wikipedia   
https://en.wikipedia.org/wiki/DCI-P3   
https://en.wikipedia.org/wiki/Standard_illuminant

* Arri   
https://www.arri.com/resource/blob/278790/dc29f7399c1dc9553d329e27f1409a89/2022-05-arri-logc4-specification-data.pdf   
https://www.arri.com/resource/blob/31918/66f56e6abb6e5b6553929edf9aa7483e/2017-03-alexa-logc-curve-in-vfx-data.pdf
  
Project
-------

* GitHub page   
https://github.com/mikaelsundell/colortool

* Issues   
https://github.com/mikaelsundell/colortool/issues
