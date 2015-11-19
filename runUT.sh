#!/bin/bash
make ut

if hostname | grep ling; then
    if [ `cat CMakeCache.txt | grep useSDK | cut -d "=" -f2` == "OFF" ]; then
    	export LD_PRELOAD=/opt/gcc/x86_64/4.9.0a/lib64/libstdc++.so
    fi
fi

./ut --gtest_repeat=$1 --gtest_break_on_failure