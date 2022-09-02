#!/bin/bash
make clean;
make
cd bin
make clean;
make SGX=1

