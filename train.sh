#!/bin/bash

RUNCOUNT=1
TRAINING=TRUE
GRAPHICS=FALSE

while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        -n|--nruns) # number of runs
            shift # past argument
            RUNCOUNT="$1"
            shift # past value
            ;;

        -t|--notraining)
            TRAINING=FALSE
            shift;;
    esac
done

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

FLAGS="-N --numkarts=1 --track=tutorial --profile-time=120s"
TRAINING="--training"

if [[ ${TRAINING} = TRUE ]]; then
    echo "TRAINING is on"
    echo "Running: ${DIR}/cmake-build-debug/bin/supertuxkart ${FLAGS} ${TRAINING}"
    ${DIR}/cmake-build-debug/bin/supertuxkart ${FLAGS} ${TRAINING}
else
    echo "TRAINING is on"
    echo "Running: ${DIR}/cmake-build-debug/bin/supertuxkart ${FLAGS}"
    ${DIR}/cmake-build-debug/bin/supertuxkart ${FLAGS}
fi
