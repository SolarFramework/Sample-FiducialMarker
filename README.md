# SolAR Fiducial Marker

[![License](https://img.shields.io/github/license/SolARFramework/FiducialMarker?style=flat-square&label=License)](https://www.apache.org/licenses/LICENSE-2.0)

The SolAR **Fiducial Image Marker sample** shows a SolAR pipeline for augmented reality based on a fiducials markers image. This pipeline loads a reference image marker, then tries to detect it on real-time camera images and to estimate the pose of the camera in relation to the coordinate system of the image marker. If the marker is detected, the pipeline over the current camera image renders a 3D cube from a virtual camera which pose corresponds to the one estimated by the pipeline.


| ![](./SolARSample_FiducialMarker_Mono/standalone.jpg) | ![](./SolARPipeline_FiducialMarker/plugin.jpg) |
|:-:|:-:|
| SolARSample_FiducialMarker_Mono | SolARPipeline_FiducialMarker |


## How to run

* To run it, first print the marker [FiducialMarker.gif](./SolARSample_FiducialMarker_Mono/FiducialMarker.gif)

* If you want to change your fiducial marker, you can edit the [fiducialMarker.yml](./SolARSample_FiducialMarker_Mono/fiducialMarker.yml)

* If you want to change the calibration parameters of the camera, edit the [camera_calibration.yml](./SolARSample_FiducialMarker_Mono/camera_calibration.yml)

* To change properties of the components of the fiducial pipeline, edit the [conf_FiducialMarker.xml](./SolARSample_FiducialMarker_Mono/conf_FiducialMarker.xml) file.

### SolARSample_FiducialMarker_Mono

* Open a terminal and execute `./bin/Release/SolARSample_FiducialMarker_Mono.exe`
* Target your fiducial marker with your camera
* Press `escape` to quit the application

### SolARPipeline_FiducialMarker

You should have bundle every required librairy in your Unity project (`./Assets/Plugins`). Then from Unity Gameobject *PipelineLoader* you can load your configuration file for the fiducial pipeline. You can directly edit parameters from Unity Editor's inspector.


## Contact 
Website https://solarframework.github.io/

Contact framework.solar@b-com.com




