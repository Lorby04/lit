#!/bin/bash
~/record/top.sh cpu.log &
disown
gramine-sgx lit -n 1 -s 4 -t 10M | tee sgx.log &
disown
sleep 100
./lit -n 1 -s 4 -t 10M | tee native.log &
disown