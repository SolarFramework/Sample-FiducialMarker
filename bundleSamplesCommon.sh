#!/bin/bash
#
# @copyright Copyright (c) 2021 B-com http://www.b-com.com/
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PLATFORM=""
NAME="SolAR_Fiducial"
VERSION="0.9.1"
FILE_NAME=$NAME"_"$VERSION

function help()
{
    echo "Creates a bundle of all executables for this sample. It zips the bin/ directory so"
    echo "all executables to be bundled must be previously built."
    echo "It produces a zip file named '<file-name>_[release|debug].zip"
    echo
    usage
}

function usage()
{
    echo "Usage:"
    echo "`basename "$0"` [<option>=<value> ]+"
    echo "Options:"
    echo "   -p, --platform: 'windows', 'linux' (mandatory)"
    echo "   -f, --file-name: bundle zip file name (default: $FILE_NAME)"
}

# Parse args
while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        -h | --help)
            help
            exit
            ;;
        -p | --platform)
            if [ "$VALUE" != "windows" ] && [ "$VALUE" != "linux" ]; then
                echo "ERROR: '$VALUE' is not a valid configuration"
                usage
                exit 1
            fi
            PLATFORM=$VALUE
            ;;
         -f | --file-name)
            FILE_NAME=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter '$PARAM'"
            usage
            exit 1
            ;;
    esac
    shift
done

if [ "$PLATFORM" == "" ]; then
   echo "ERROR: unspecified platform"
   usage
   exit 1
fi


if [ "$PLATFORM" == "linux" ]; then
   # Update configuration files by replacing win-cl-1.1 by linux in module paths
   echo "**** Update module path in configuration file (win-cl-14.1 -> linux-gcc)"
   for f in `find . -name "*_conf*.xml"` ; do sed -i 's/win-cl-14.1/linux-gcc/' $f; done
fi

echo "**** Install dependencies locally"
remaken install packagedependencies.txt
remaken install packagedependencies.txt -c debug

echo "**** Bundle dependencies in bin folder"
for file in './SolARPipeline_FiducialMarker_Runner/SolARPipeline_FiducialMarker_Runner_conf.xml',\
            './SolARStandAlone_FiducialMarker_Mono/SolARStandAlone_FiducialMarker_Mono_conf.xml'
do
   echo "install dependencies for config file: $file"
   remaken bundleXpcf $file -d ./bin/Release -s modules
   remaken bundleXpcf $file -d ./bin/Debug -s modules -c debug
done

if [ "$PLATFORM" == "linux" ]; then
   cp ./runFromBundle.sh ./run.sh
   mv ./run.sh ./bin/Release/
   cp ./runFromBundle.sh ./run.sh
   mv ./run.sh ./bin/Debug
fi

# TODO(jmhenaff): see if can get zip installed on Windows bots to avoid this condition
if [ "$PLATFORM" == "linux" ]; then
   zip --symlinks -r "./bin/${FILE_NAME}_release.zip" ./bin/Release ./README.md doc/ ./LICENSE
   zip --symlinks -r "./bin/${FILE_NAME}_debug.zip" ./bin/Debug ./README.md doc/ ./LICENSE
else
   7z a -tzip bin/${FILE_NAME}_debug.zip bin/Debug
   7z a -tzip bin/${FILE_NAME}_debug.zip README.md
   7z a -tzip bin/${FILE_NAME}_debug.zip LICENSE
   7z a -tzip bin/${FILE_NAME}_debug.zip doc

   7z a -tzip bin/${FILE_NAME}_release.zip bin/Release
   7z a -tzip bin/${FILE_NAME}_release.zip README.md
   7z a -tzip bin/${FILE_NAME}_release.zip LICENSE
   7z a -tzip bin/${FILE_NAME}_release.zip doc
fi
