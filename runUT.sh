#!/bin/bash
make ut
export LD_PRELOAD=/opt/gcc/x86_64/4.9.0a/lib64/libstdc++.so
./ut --gtest_repeat=500 --gtest_break_on_failure
