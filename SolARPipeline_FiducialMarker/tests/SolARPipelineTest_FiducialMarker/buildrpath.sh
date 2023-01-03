#!/bin/bash

remaken run --env --xpcf SolARPipelineTest_FiducialMarker_conf.xml --deps packagedependencies.txt | sed 's/:/\n/g' | sed 's/$LD_LIBRARY_PATH//g' | sed 's/^/QMAKE_LFLAGS_RPATH+=/g' | sed 's/LD_LIBRARY_PATH=//g' &> rpath.pri