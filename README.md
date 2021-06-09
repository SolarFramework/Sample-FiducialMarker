# SolAR Fiducial Marker

[![License](https://img.shields.io/github/license/SolARFramework/FiducialMarker?style=flat-square&label=License)](https://www.apache.org/licenses/LICENSE-2.0)

The SolAR **Fiducial Image Marker sample** shows a SolAR pipeline for augmented reality based on a fiducials markers image. This pipeline loads a reference image marker, then tries to detect it on real-time camera images and to estimate the pose of the camera in relation to the coordinate system of the image marker. If the marker is detected, the pipeline over the current camera image renders a 3D cube from a virtual camera which pose corresponds to the one estimated by the pipeline.


| ![](./doc/PipelineRunner.jpg) | ![](./doc/UnityPLugin.jpg) |
|:-:|:-:|
| SolARStandAlone_FiducialMarker_Mono | SolARPipeline_FiducialMarker in Unity plugin |


## How to run

* To run it, first print the marker [FiducialMarker.gif](./data/markers/FiducialMarker.gif)

* If you want to change your fiducial marker, you can edit the [FiducialMarker.yml](./data/markers/FiducialMarker.yml)

* If you want to change the calibration parameters of the camera, edit the [CameraCalibration.yml](./data/camera/CameraCalibration.yml)

* To change properties of the components of the fiducial pipeline, edit the [SolARPipeline_FiducialMarker_Runner_conf.xml](./SolARPipeline_FiducialMarker_Runner/SolARPipeline_FiducialMarker_Runner_conf.xml) file.

If you want to run your Fiducial samples after having built them, do not forget to install the required dependencies if not already done:

<pre><code>remaken install packagedependencies.txt</code></pre>

and for debug mode:

<pre><code>remaken install packagedependencies.txt -c debug</code></pre>

For more information about how to install remaken on your machine, visit the [install page](https://solarframework.github.io/install/) on the SolAR website.

### SolARPipeline_FiducialMarker_Runner

* Open a terminal and execute from the `bin/Debug` or `bin/Release` folder:

> #### Windows
>
	SolARPipeline_FiducialMarker_Runner.exe

> #### Linux
>
	./run.sh ./SolARPipeline_FiducialMarker_Runner

* Target your fiducial marker with your camera
* Press `escape` to quit the application

### SolARStandAlone_FiducialMarker_Mono

* Open a terminal and execute from the `bin/Debug` or `bin/Release` folder:

> #### Windows
>
	SolARStandAlone_FiducialMarker_Mono.exe.exe

> #### Linux
>
	./run.sh ./SolARStandAlone_FiducialMarker_Mono

### Plugin

You should have bundle every required library in your Unity project (`./Assets/Plugins`). Then from Unity Gameobject *PipelineLoader* you can load your configuration file for the natural image pipeline. You can directly edit parameters from Unity Editor's inspector.

## How to run the tests

* Run the script `installData.sh` to download test data (such as videos)
* Open the `SolARSample_FiducialMarker_Tests` .pro file in a supported IDE (QtCreator or Visual Studio + Qt VS Tools extension) and build it.
* Go to Projects tab -> run and change Working Directory to `Sample-FiducialMarker\bin-test\[debug|release]`

* In QtCreator: open Tools -> Tests -> Run All Test
* Command line
	* Windows
		* Add a bash shell to your PATH, e.g.: C:\Program Files\Git\bin if available
		* run test with `sh ci\runTests.sh`
	* Linux
		* run test with `./ci/runTests.sh`
	* This produces an xml report in `./test-report/`



## Contact 
Website https://solarframework.github.io/

Contact framework.solar@b-com.com




