#!/bin/bash
sleep 10
~/record/top.sh cpu-ring.log &
disown
gramine-sgx lit ring -n 1 -s 4 -t 10M | tee sgx-ring.log &
sleep 5
./lit ring -n 1 -s 4 -t 10M | tee native-ring.log &
sleep 5

killall -9 top
~/record/top.sh cpu-list.log &
disown
gramine-sgx lit list -n 1 -s 4 -t 10M | tee sgx-list.log &
sleep 5
./lit list -n 1 -s 4 -t 10M | tee native.log-list &
sleep 5
