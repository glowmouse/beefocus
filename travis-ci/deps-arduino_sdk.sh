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
  echo "LOG: Downloading Arduino SDK $sdk_file"
  wget "https://downloads.arduino.cc/$sdk_file" -O "$sdk_file"
  echo "LOG: Untarring $sdk_file"
  tar -xf "$sdk_file"
  echo "LOG: Install Successful"
	popd > /dev/null
fi


