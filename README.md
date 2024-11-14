README for colortool
==================

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg?style=flat-square)](https://github.com/mikaelsundell/colortool/blob/master/README.md)

Introduction
------------

colortool is a utility set for color space conversions, with support for white point adaptation.

Building
--------

The colortool app can be built both from commandline or using optional Xcode `-GXcode`.

```shell
mkdir build
cd build
cmake .. -DCMAKE_MODULE_PATH=<path>/colortool/modules -DCMAKE_INSTALL_PREFIX=<path> -DCMAKE_PREFIX_PATH=<path> -GXcode
cmake --build . --config Release -j 8
```

**Example using 3rdparty on arm64**

```shell
mkdir build
cd build
cmake ..
cmake .. -DCMAKE_PREFIX_PATH=<path>/3rdparty/build/macosx/arm64.debug -DCMAKE_INSTALL_PREFIX=<path>/3rdparty/build/macosx/arm64.debug -DCMAKE_CXX_FLAGS="-I<path>/include/eigen3" -DBUILD_SHARED_LIBS=TRUE -GXcode
```

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
