
# @copyright Copyright (c) 2022 B-com http://www.b-com.com/
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


REMAKEN_RUN_ENV_CMD = "remaken run --env \
                                   --xpcf $${GEN_ENV_SCRIPT_XPCF_FILE} \
                                   --deps $${GEN_ENV_SCRIPT_DEPS_FILE} \
                                   --destination $${TARGETDEPLOYDIR}"
REMAKEN_RUN_ENV_DBG_CMD = "remaken run --config debug \
                                       --env \
                                       --xpcf $${GEN_ENV_SCRIPT_XPCF_FILE} \
                                       --deps $${GEN_ENV_SCRIPT_DEPS_FILE} \
                                       --destination $${TARGETDEPLOYDIR}"
# "Hack" for QTVS mode that does not use makefile targets -> use INSTALLS objects
# so that 'extra' member is used in .bat scripts generated by bd/qmake QTVS rules
win32:if(!isEmpty(_SOLAR_USE_QTVS)) {
  CONFIG(release,debug|release) {
    install_env_script.extra = "$${REMAKEN_RUN_ENV_CMD} \
                                && move $${TARGETDEPLOYDIR}\\prepare_project_env.bat $${TARGETDEPLOYDIR}\\$${TARGET}_prepare_env.bat"
  }
  CONFIG(debug,debug|release) {
    install_env_script.extra = "$${REMAKEN_RUN_ENV_DBG_CMD} \
                                && move $${TARGETDEPLOYDIR}\\prepare_project_env.bat $${TARGETDEPLOYDIR}\\$${TARGET}_prepare_env.bat"
  }
  INSTALLS += install_env_script
}
else {
  win32 {
    ENV_SCRIPT_EXT = ".bat"
    MAKE_CMD = "jom"
  }
  else {
    ENV_SCRIPT_EXT = ".sh"
    MAKE_CMD = "make"
  }
  TARGET_ENV_SCRIPT_FILE_NAME = $${TARGET}_prepare_env$${ENV_SCRIPT_EXT}

  install_env_script.target = $$system_path($${TARGETDEPLOYDIR}/$${TARGET_ENV_SCRIPT_FILE_NAME})
  CONFIG(release,debug|release) {
    install_env_script.commands = $${REMAKEN_RUN_ENV_CMD}
  }
  CONFIG(debug,debug|release) {
    install_env_script.commands = $${REMAKEN_RUN_ENV_DBG_CMD}
  }

  install_env_script.commands += "&& $${QMAKE_MOVE} $$system_path($${TARGETDEPLOYDIR}/prepare_project_env$${ENV_SCRIPT_EXT}) \
                                                    $$install_env_script.target"

  install_env_script.depends = install

  QMAKE_EXTRA_TARGETS += install_env_script
  QMAKE_POST_LINK += "&& $${MAKE_CMD} $$install_env_script.target"

  message("PATH=$$(PATH)")

}
