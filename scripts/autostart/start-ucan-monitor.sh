#!/bin/bash
#
executable_dir=/home/oleg/dev/pc/ucan-monitor/bin
executable=ucan-monitor

if [ -d "$executable_dir" ]
then
  echo "Changing directory to $executable_dir"
  cd $executable_dir || exit 1
  echo "Current directory is $PWD"
else
  echo "No such directory: $executable_dir"
  exit 1
fi

if [ -x "$executable" ]
then
  echo "Starting $executable"
  ./$executable >/dev/null 2>&1
else
  echo "$executable not found or is not executable"
  exit 1
fi
