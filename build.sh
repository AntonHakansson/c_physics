#!/bin/sh

CompilerFlags="-O0 -g -fno-exceptions -fno-rtti -Wall -Wextra -Wmissing-field-initializers -DDEVELOPER"

mkdir -p build

pushd build > /dev/null
    echo "BUILDING GAME"
    echo "---------------------"
    g++ $CompilerFlags ../src/main.cpp -o berserker -lraylib -lGL
    CompileSuccess=$?

    if [ $CompileSuccess -eq 0 ]; then
       case  $1  in
           "run")
               echo "RUNNING GAME"
               echo "---------------------"
               ./berserker
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
