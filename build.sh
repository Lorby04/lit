#!/bin/bash
make
cd bin
make SGX=1
cd ..
