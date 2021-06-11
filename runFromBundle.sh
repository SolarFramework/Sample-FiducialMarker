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

if [ "$REMAKEN_PKG_ROOT" == "" ]
then
    export REMAKEN_PKG_ROOT=~/.remaken
fi
echo "REMAKEN_PKG_ROOT=$REMAKEN_PKG_ROOT"

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD:$PWD/modules $1


