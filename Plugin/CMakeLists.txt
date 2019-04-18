cmake_minimum_required(VERSION 3.7.2)

##################################################
set (VERSION_NUMBER "0.5.3")
project("PipelineFiducialMarker")
set (SOURCES 
    src/PipelineFiducialMarker.cpp
)

set (HEADERS
    interfaces/PipelineFiducialMarkerAPI.h
    interfaces/PipelineFiducialMarker.h
)

##################################################

# various macros
include("$ENV{BCOMDEVROOT}/bcomBuild/SolARFramework/solarmacros.cmake")
# config setup
setup()
# process packagedependencies.txt
processPackagedependencies()
# define targets (type library)
defineTargets("library" "")



