#!/bin/bash

cmake --build build
pushd build
./minesweeper
popd
