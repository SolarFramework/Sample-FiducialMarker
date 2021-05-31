## remove Qt dependencies
QT       -= core gui
CONFIG -= qt

## global defintions : target lib name, version
TARGET = SolARSample_FiducialMarker_TestSuite
VERSION=0.9.1

DEFINES += MYVERSION=$${VERSION}
CONFIG += c++1z
CONFIG += console

include(findremakenrules.pri)

CONFIG(debug,debug|release) {
    TARGETDEPLOYDIR = $${PWD}/../bin-test/Debug
    DEFINES += _DEBUG=1
    DEFINES += DEBUG=1
    LIBS += -lgtestd
    LIBS += -lgmockd
}

CONFIG(release,debug|release) {
    TARGETDEPLOYDIR = $${PWD}/../bin-test/Release
    DEFINES += _NDEBUG=1
    DEFINES += NDEBUG=1
    LIBS += -lgtest
    LIBS += -lgmock
}

win32:CONFIG -= static
win32:CONFIG += shared

DEPENDENCIESCONFIG = sharedlib install_recurse

PROJECTCONFIG = QTVS


#NOTE : CONFIG as staticlib or sharedlib, DEPENDENCIESCONFIG as staticlib or sharedlib, QMAKE_TARGET.arch and PROJECTDEPLOYDIR MUST BE DEFINED BEFORE templatelibconfig.pri inclusion
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/templateappconfig.pri)))  # Shell_quote & shell_path required for visual on windows

#DEFINES += BOOST_ALL_NO_LIB
DEFINES += BOOST_ALL_DYN_LINK
DEFINES += BOOST_AUTO_LINK_NOMANGLE
DEFINES += BOOST_LOG_DYN_LINK

DEFINES += USE_AS_GTEST

QMAKE_PROJECT_DEPTH  = 0

INCLUDEPATH += \
../SolARPipeline_FiducialMarker/tests/SolARPipelineTest_FiducialMarker/include

HEADERS += \
../SolARPipeline_FiducialMarker/tests/SolARPipelineTest_FiducialMarker/include/SolARPipelineTest_FiducialMarker.h

SOURCES += \
main.cpp \
../SolARPipeline_FiducialMarker/tests/SolARPipelineTest_FiducialMarker/src/SolARPipelineTest_FiducialMarker.cpp \
../SolARPipeline_FiducialMarker/tests/SolARPipelineTest_FiducialMarker/test/test.cpp \
../SolARSample_FiducialMarker_Mono/test2.cpp

linux {
    ## Add rpath to find dependencies at runtime
    QMAKE_LFLAGS_RPATH=
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
}

unix {
    LIBS += -ldl
    QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK
}

macx {
    QMAKE_MAC_SDK= macosx
    QMAKE_CXXFLAGS += -fasm-blocks -x objective-c++
}

win32 {
    QMAKE_LFLAGS += /MACHINE:X64
    DEFINES += WIN64 UNICODE _UNICODE
    QMAKE_COMPILER_DEFINES += _WIN64
    QMAKE_CXXFLAGS += -wd4250 -wd4251 -wd4244 -wd4275

    # Windows Kit (msvc2013 64)
    LIBS += -L$$(WINDOWSSDKDIR)lib/winv6.3/um/x64 -lshell32 -lgdi32 -lComdlg32
    INCLUDEPATH += $$(WINDOWSSDKDIR)lib/winv6.3/um/x64
}

android {
    ANDROID_ABIS="arm64-v8a"
}

config_files.path = $${TARGETDEPLOYDIR}
config_files.files=$$files($${PWD}/SolARSample_FiducialMarker_Mono_conf.xml)\
                            $$files($${PWD}/SolARPipelineTest_FiducialMarker_conf.xml)\
					$$files($${PWD}/camera_calibration.yml)\
					$$files($${PWD}/fiducialMarker.yml)\
                                        $$files($${PWD}/FiducialMarker.gif)\
                                        $$files($${PWD}/SolARPipeline_FiducialMarker_test_0001.mp4)

INSTALLS += config_files

linux {
  run_install.path = $${TARGETDEPLOYDIR}
  run_install.files = $${PWD}/../run.sh
  CONFIG(release,debug|release) {
    run_install.extra = cp $$files($${PWD}/../runRelease.sh) $${PWD}/../run.sh
  }
  CONFIG(debug,debug|release) {
    run_install.extra = cp $$files($${PWD}/../runDebug.sh) $${PWD}/../run.sh
  }
  INSTALLS += run_install
}


OTHER_FILES += \
    packagedependencies.txt

#NOTE : Must be placed at the end of the .pro
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/remaken_install_target.pri)))) # Shell_quote & shell_path required for visual on windows
