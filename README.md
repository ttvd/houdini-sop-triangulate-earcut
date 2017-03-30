# houdini-sop-triangulate-earcut

[Houdini](http://www.sidefx.com/index.php) HDK SOP node which triangulates points using [Mapbox Earcut library](https://github.com/mapbox/earcut.hpp).

## More info
* [Mapbox Earcut](https://github.com/mapbox/earcut.hpp)
* [FIST: Fast Industrial-Strength Triangulation of Polygons](http://www.cosy.sbg.ac.at/~held/projects/triang/triang.html) by Martin Held
* [Triangulation by Ear Clipping](http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf) by David Eberly.

## Binaries, Houdini 16.0
* [Windows, Houdini 16.0.557](https://github.com/ttvd/houdini-sop-triangulate-earcut/releases/download/1.0/SOP_TriangulateEarCut.16.0.557.Win64.rar) 

## Building

* Tested on Windows and Houdini 16.0.
  * You would have to patch CMake file to get this building on Linux.
* Define HOUDINI_VERSION env variable to be the version of Houdini 16.0 you wish to build against (for example "16.0.557").
* Alternatively, you can have HFS env variable defined (set when you source houdini_setup).
* Generate build files from CMake for your favorite build system. For Windows builds use MSVC 2015.
* Build the SOP Houdini dso (SOP_TriangulateEarCut.dylib or SOP_TriangulateEarCut.dll).
* Place the dso in the appropriate Houdini dso folder.
  * On OS X this would be /Users/your_username/Library/Preferences/houdini/16.0/dso/
  * On Windows this would be C:\Users\your_username\Documents\houdini16.0\dso

## Usage

* Place the SOP into your SOP network.
* Connect input geometry to triangulate.
* Select whether to keep incoming triangles and a triangulation plane.
* Optionally, select a point group to operate on.

## License for the plugin

* Copyright Mykola Konyk, 2017
* Distributed under the [MS-RL License.](http://opensource.org/licenses/MS-RL)
* **To further explain the license:**
  * **You cannot re-license any files in this project.**
  * **That is, they must remain under the [MS-RL license.](http://opensource.org/licenses/MS-RL)**
  * **Any other files you add to this project can be under any license you want.**
  * **You cannot use any of this code in a GPL project.**
  * Otherwise you are free to do pretty much anything you want with this code.
  
## License for earcut.hpp
* Copyright 2015, Mapbox
