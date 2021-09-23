#!/usr/bin/env bash

CompilerFlags="${CompilerFlags:- -O0 -g -fno-exceptions -fno-rtti -Wall -Wextra -Wmissing-field-initializers -DDEVELOPER}"
CC="${CC:-g++}"

mkdir -p build

pushd build > /dev/null
    echo "BUILDING GAME"
    echo "---------------------"
    "$CC" $CompilerFlags ../src/main.cpp -o c_physics -lm -lGL -lraylib
    CompileSuccess=$?

    if [ $CompileSuccess -eq 0 ]; then
       case  $1  in
           "run")
               echo "RUNNING GAME"
               echo "---------------------"
               ./c_physics
               ;;
           "test")
               echo "NO TESTS"
               ;;
           *)
       esac
    else
        echo "BUILD FAILED"
    fi
popd > /dev/null
