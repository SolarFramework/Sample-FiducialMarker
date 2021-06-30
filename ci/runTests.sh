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

# Note: to run this on windows either use git bash GUI, or add C:\Program Files\Git\bin to 
# your PATH and invoke this script with 'sh runTests.sh' from a CMD.exe/Powershell

# Note: to visualize the test report as an HTML page, you can use this:
# https://github.com/adarmalik/gtest2html

# TODO(jmhenaff): make that an option so that script is fully project agnostic and could be reused
TEST_EXEC=SolARSample_FiducialMarker_Tests

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
TEST_REPORT_ROOT=$SCRIPT_DIR/../test-report
CONFIG="all"
PLATFORM="unset"

function usage()
{
    echo "Usage:"
    echo "`basename "$0"` [<option>=<value>]+"
    echo "Options:"
    echo "   -h, --help: displays this message"
    echo "   -c, --config: 'release', 'debug' or 'all' (default: 'all')"
    echo "   -p, --platform: 'windows', 'linux' (mandatory)"
}

# TODO(jmhenaff): add options to pass to gtest exec (filter, output dir, ...)
# Parse args
while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        -c | --config)
            if [ "$VALUE" != "release" ] && [ "$VALUE" != "debug" ] && [ "$VALUE" != "all" ]; then
                echo "ERROR: '$VALUE' is not a valid configuration"
                usage
                exit 1
            fi
            CONFIG=$VALUE
            ;;
        -p | --platform)
            if [ "$VALUE" != "windows" ] && [ "$VALUE" != "linux" ]; then
                echo "ERROR: '$VALUE' is not a valid value for platform"
                usage
                exit 1
            fi
            PLATFORM=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter '$PARAM'"
            usage
            exit 1
            ;;
    esac
    shift
done

if [ "$PLATFORM" == "unset" ]; then
    echo "ERROR: platform is not set, you must specify one"
    usage
    exit 1
fi

if [ "$PLATFORM" == "linux" ]; then
    TEST_CMD="./run.sh $TEST_EXEC"
elif [ "$PLATFORM" == "windows" ]; then
    TEST_CMD="./$TEST_EXEC"
fi

# Run tests for a specific config (release, debug)
# $1: configuration name (either release or debug)
function run()
{
    if [ "$1" != "release" ] && [ "$1" != "debug" ]; then
        echo "ERROR: '$1' is not a valid configuration for run(), only 'release' and 'debug' are valid"
        exit 1
    fi

    if [[ ! -f $SCRIPT_DIR/../bin-test/$1/$TEST_EXEC ]]; then
        echo "WARNING: skipping run of config '$1' as executable does not seem to have been built"
        return
    fi

    TEST_REPORT_DIR=$TEST_REPORT_ROOT/$1
    mkdir -p $TEST_REPORT_DIR/output
    
    # Warning: Don't put directly $TEST_REPORT_DIR in --gtest_output path because gtest will attempt to interpret this as a relative path and append it to $pwd
    OLD_PWD=$PWD
    cd $SCRIPT_DIR/../bin-test/$1/
    $TEST_CMD --gtest_output=xml:./tests.xml
    if [ $? -ne 0 ]; then
        echo "ERROR: tests failed"
        exit 1
    fi
    if [ ! -f  tests.xml ]; then
        echo "ERROR: xml report has not been generated"
        exit 1
    fi
    cp tests.xml $TEST_REPORT_DIR/
    for f in `find . -name "*_output.txt"`; do
        cp $f $TEST_REPORT_DIR/output/
    done
    cd $OLD_PWD
}

#
# MAIN
#

# TODO: specify these kind of steps in JenkinsFile ?
if ! sh $SCRIPT_DIR/../installData.sh; then
        echo "ERROR: could not install data"
        exit 1
fi

if [ "$CONFIG" == "all" ]; then
    run release
    run debug
else
    run $CONFIG
fi


