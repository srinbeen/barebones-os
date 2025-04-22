#!/bin/bash

# last_used=$(sudo losetup | sed -n 's/.*loop\([0-9]\+\).*/\1/p' | sort | tail -n 1)
last_used=$(sudo losetup | grep -o '.*loop[0-9]\+' | sort | tail -n 1 | sed 's/\/dev\/loop//')
max=$((last_used + 1))
echo $max