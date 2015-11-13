#!/bin/bash
make ut
if hostname | grep ling; then
	export LD_PRELOAD=/opt/gcc/x86_64/4.9.0a/lib64/libstdc++.so
fi
./ut --gtest_repeat=500 --gtest_break_on_failure
