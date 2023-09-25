#!/bin/bash
#
if [ -z $1 ]
then
    echo "[FAILED] Invalid arguments."
    exit 1
fi
if ip addr ls dev $1 | grep "state UP" > /dev/null 2>&1
then
    echo "[  OK  ] Interface $1 is UP."
    exit 0
else
    echo "[FAILED] Interface $1 is DOWN."
    exit 2
fi
