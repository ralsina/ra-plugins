#!/bin/sh

if [ ! -d build ]
then
	mkdir build || exit 1
fi
cd build && cmake .. && make check && make && echo "Now do a './install.sh' as root to install"

