StereoCorrespondenceBM Tuner
============================

Description
-----------

A GUI to update the parameters, and watch the results live, of the [block-matching algorithm](http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html#stereobm-operator) of [OpenCV](http://opencv.org/), used for computing depth maps when doing stereovision.

![example screenshot](./example.png)


Installation
------------
You will need to install:

* [Qt 5](http://www.qt.io/) (for the GUI)
* [OpenCV](http://opencv.org/)

### Linux

Install the packages for Qt 5 and OpenCV. (qt5-default & libopencv-dev).
```
cd ./StereoCorrespondenceBMTuner
cmake -B "build"
cmake --build build
```

### Mac OS X

Download and install Qt 5 from their [website](http://www.qt.io/). I've installed OpenCV with [Homebrew](http://brew.sh/), but it probably doesn't matter if you install it with another way. This program uses `pkg-config` to include the library OpenCV in Qt's project. Since `pkg-config` is not standard on Mac OS X, to make it work, you will need to add `PATH=/usr/local/bin:$PATH` to the file `/etc/launchd.conf`. See [here](http://stackoverflow.com/questions/16972066/using-pkg-config-with-qt-creator-qmake-on-mac-osx) for details.

### Windows
Replace "C:/.../vcpkg" with real path to vcpkg

```
cd ./StereoCorrespondenceBMTuner

cmake -G "Visual Studio 17 2022" -A x64 -B "build" -DCMAKE_TOOLCHAIN_FILE=C:/.../vcpkg/scripts/buildsystems/vcpkg.cmake -DOpenCV_DIR="C:/.../vcpkg/installed/x64-windows/share/opencv"'

cmake --build build
```



### Launch the program

Once Qt 5 and OpenCV installed, clone this repo, open the `.pro` file, it should launch the project in Qt 5 and you will be able to run the program, and modify it.


Useful links
------------
* [OpenCV documentation for StereoBM attributes](http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html#stereosgbm-stereosgbm)
* [StereoBM attributes default values](https://github.com/Itseez/opencv/blob/master/modules/calib3d/src/stereobm.cpp)
* [my test images](https://drive.google.com/folderview?id=0B31-CIvNW1LdfnN5WlE0QVdESjVnUGQtQVU1QTZYSjcwaTI4T29EMDN3S1BrZWFVekV0YU0&usp=sharing)


Contribute
----------
Pull requests are welcome.

This software is inspired by another described [here](http://blog.martinperis.com/2011/08/opencv-stereo-matching.html). However, I wasn't satisfied with it, and it works only on [Gnome](https://www.gnome.org/) (used by [Ubuntu](http://www.ubuntu.com/desktop)).

### TODO

* accept image drops for left and right image areas


Licence
-------
GPL

