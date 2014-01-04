#!/bin/bash

VEX_DIR=..
CURRENT_DIR=`pwd`

uname -a > basic_test_results
cat /proc/cpuinfo >> basic_test_results
echo >> basic_test_results
echo >> basic_test_results
echo >> basic_test_results
echo "VEX flags" >> basic_test_results
cat $VEX_DIR/src/Makefile.inc | grep PRE | grep "=" >> basic_test_results


echo >> basic_test_results
echo "VEX low tests" >> basic_test_results
cd $VEX_DIR/src/tests


