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

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

TEST_REPORT_DIR=$SCRIPT_DIR/../test-report

mkdir $TEST_REPORT_DIR

# Warning: Don't put directly $TEST_REPORT_DIR in --gtest_output path because gtest will attempt to interpret this as a relative path and append it to $pwd
(cd $SCRIPT_DIR/../SolARSample_FiducialMarker_Tests/bin/Debug/ && ./SolARSample_FiducialMarker_Tests --gtest_output=xml:tests.xml && cp tests.xml $TEST_REPORT_DIR/ && cp *_output.txt $TEST_REPORT_DIR)
