#!/bin/bash

#
# Downloads the Arduino SDK.  For Travis CI, but you can use it to
# set-up your own development environment.
#

: ${ARDUINO_SDK_PATH?"required environment variable not set."}
: ${ARDUINO_SDK_VERSION?"required environment variable not set."}
sdk_path=$ARDUINO_SDK_PATH

echo "LOG: creating target directory $sdk_path"
mkdir -p "$sdk_path"

sdk_file="arduino-$ARDUINO_SDK_VERSION-linux64.tar.xz"
if [ "$(ls -A $sdk_path)" ]; then
  echo "LOG: Directory $sdk_path not empty"
  echo "LOG: Assuming arduino SDK already installed - exiting"
else
  echo "LOG: Directory $sdk_path empty"
	pushd . > /dev/null
  cd "$sdk_path/.."
  if [ -f $sdk_file ]; then
     echo "LOG: Arduino SDK $sdk_file exists - assuming it's good"
  else
     echo "LOG: Downloading Arduino SDK $sdk_file"
     wget "https://downloads.arduino.cc/$sdk_file" -O "$sdk_file"
     echo "LOG: Done Downloading Arduino SDK $sdk_file"
  fi
  echo "LOG: Untarring $sdk_file"
  tar -xf "$sdk_file"
  echo "LOG: Arduino SDK Successful"
  echo "LOG: Cloning ESP8266 from github"
  cd "$sdk_path"
  cd hardware
  mkdir esp8266com
  cd esp8266com
  git clone https://github.com/esp8266/Arduino.git esp8266
  echo "LOG: Done Cloning ESP8266 from github"
  echo "LOG: Checking out ESP8266 to a stable tag"
  git checkout tags/2.4.2
  echo "LOG: Done Checking out ESP8266 to a stable tag"
  echo "LOG: Git fetch successful"
  echo "LOG: Downloading binary tools"
  cd esp8266/tools
  python get.py
  echo "LOG: Download successful"
  popd > /dev/null
fi


