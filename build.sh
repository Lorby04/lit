#!/bin/bash
ninja clean;
ninja
cd server/bin
cp -f ../config/liserver.manifest.template .
cp -f ../config/sgx.conf Makefile
make clean;
make SGX=1
