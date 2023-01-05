## remove Qt dependencies
QT       -= core gui
CONFIG -= qt

QMAKE_PROJECT_DEPTH = 0

## global defintions : target lib name, version
TARGET = SolARPipelineTest_FiducialMarker
VERSION=1.0.0
PROJECTDEPLOYDIR = $${PWD}/../../../deploy

DEFINES += MYVERSION=$${VERSION}
CONFIG += c++1z
CONFIG += console

include(findremakenrules.pri)

CONFIG(debug,debug|release) {
    DEFINES += _DEBUG=1
    DEFINES += DEBUG=1
}

CONFIG(release,debug|release) {
    DEFINES += _NDEBUG=1
    DEFINES += NDEBUG=1
}

DEPENDENCIESCONFIG = sharedlib recurse

_SOLAR_USE_QTVS = $$(SOLAR_USE_QTVS)
!isEmpty(_SOLAR_USE_QTVS) {
    ## Configuration for Visual Studio to install binaries and dependencies. Work also for QT Creator by replacing QMAKE_INSTALL
    PROJECTCONFIG = QTVS
}

#NOTE : CONFIG as staticlib or sharedlib, DEPENDENCIESCONFIG as staticlib or sharedlib, QMAKE_TARGET.arch and PROJECTDEPLOYDIR MUST BE DEFINED BEFORE templatelibconfig.pri inclusion
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/templateappconfig.pri)))  # Shell_quote & shell_path required for visual on windows

HEADERS += \

SOURCES += \
    main.cpp

unix {
    LIBS += -ldl
    QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK
    # Avoids adding install steps manually. To be commented to have a better control over them.
    QMAKE_POST_LINK += "make install"
}

isEmpty(_SOLAR_USE_QTVS) {
    win32 {
        QMAKE_POST_LINK += "jom install"
    }
}

macx {
    DEFINES += _MACOS_TARGET_
    QMAKE_MAC_SDK= macosx
    QMAKE_CFLAGS += -mmacosx-version-min=10.7 -std=c11 #-x objective-c++
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -std=c11 -std=c++11 -O3 -fPIC#-x objective-c++
    QMAKE_LFLAGS += -mmacosx-version-min=10.7 -v -lstdc++
    LIBS += -lstdc++ -lc -lpthread
}

win32 {

    DEFINES += WIN64 UNICODE _UNICODE
    QMAKE_COMPILER_DEFINES += _WIN64
    QMAKE_CXXFLAGS += -wd4250 -wd4251 -wd4244 -wd4275
}

config_files.path = $${TARGETDEPLOYDIR}
config_files.files= $$files($${PWD}/SolARPipelineTest_FiducialMarker_conf.xml)\
                    $$files($${PWD}/camera_calibration.json)\
                    $$files($${PWD}/fiducialMarker.yml)\
                    $$files($${PWD}/FiducialMarker.gif)
INSTALLS += config_files


# Generate and deploy script to set up env to run executable
CONFIG(release,debug|release) {
  gen_env_script.extra = "remaken run --env \
                                      --xpcf $$files($${PWD}/SolARPipelineTest_FiducialMarker_conf.xml) \
                                      --deps $$files($${PWD}/packagedependencies.txt) \
                                      --destination $${PWD}"
}
CONFIG(debug,debug|release) {
  gen_env_script.extra = "remaken run --config debug \
                                      --env \
                                      --xpcf $$files($${PWD}/SolARPipelineTest_FiducialMarker_conf.xml) \
                                      --deps $$files($${PWD}/packagedependencies.txt) \
                                      --destination $${PWD}"
}
#unix, linux, mac, android
!win32 {
  install_env_script.files = $${PWD}/prepare_project_env.sh
}

win32 {
  install_env_script.files = $${PWD}/prepare_project_env.bat
}

install_env_script.path = $${TARGETDEPLOYDIR}

INSTALLS += gen_env_script
INSTALLS += install_env_script


OTHER_FILES += \
    packagedependencies.txt

#NOTE : Must be placed at the end of the .pro
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/remaken_install_target.pri)))) # Shell_quote & shell_path required for visual on windows
