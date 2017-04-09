#!/bin/bash

BUILD="./build"
ALLDIR="allinone"
COMDIR="common"
H264DIR="h264"
H264CPCUDIR="h264cpcu"
COREDIR="mips"
JAVADIR="java"

CP="cp -R"
RM="rm -rf"

if [ $1 ]; then
		if [ "$1" = "clean" ]; then
			echo "Cleaning fsim source code..........done"
			$RM ../$ALLDIR ../$COMDIR ../$H264DIR ../$H264CPCUDIR ../$COREDIR ../$JAVADIR
			echo "Make clean..........done"
			make clean
			exit 0
		fi
		echo ">>>>>>>Started Building.<<<<<<<<"
		if [ ! -d $1 ]; then
			echo "fsim dir($1) not found."
			echo "Please specify the full path of updated fsim"
			exit 1
		fi
		
		if [ ! -d $1/$ALLDIR ]; then
			echo "$1/$ALLDIR not found."
			echo "Please specify the full path of updated fsim"
			exit 1
		else
			if [ ! -d $ALLDIR ]; then
				echo "Copying $ALLDIR...."
				$CP $1/$ALLDIR ../
			fi
		fi
			if [ ! -d $1/$COMDIR ]; then
				echo "$1/$COMDIR not found."
				echo "Please specify the full path of updated fsim"
				exit 1
        	else
			if [ ! -d $COMDIR ]; then
				echo "Copying $COMDIR...."
				$CP $1/$COMDIR ../
			fi
		fi
		if [ ! -d $1/$H264DIR ]; then
			echo "$1/$H264DIR not found."
			echo "Please specify the full path of updated fsim"
			exit 1
		else
			if [ ! -d $H264DIR ]; then
				echo "Copying $H264DIR...."
				$CP $1/$H264DIR ../
			fi
		fi
		if [ ! -d $1/$H264CPCUDIR ]; then
			echo "$1/$H264CPCUDIR not found."
			echo "Please specify the full path of updated fsim"
			exit 1
		else
			if [ ! -d $H264CPCUDIR ]; then
				echo "Copying $H264CPCUDIR...."
				$CP $1/$H264CPCUDIR ../
			fi
		fi
		if [ ! -d $1/$COREDIR ]; then
			echo "$1/$COREDIR not found."
			echo "Please specify the full path of updated fsim"
			exit 1
		else
			if [ ! -d $COREDIR ]; then
				echo "Copying $COREDIR...."
				$CP $1/$COREDIR ../
			fi
            	fi
		if [ ! -d $1/$JAVADIR ]; then
				echo "$1/$JAVADIR not found."
				echo "Please specify the full path of updated fsim"
				exit 1
		else
			if [ ! -d $JAVADIR ]; then
				echo "Copying $JAVADIR...."
				$CP $1/$JAVADIR ../
				fi
		fi
		
		echo "Making binary....."
		make all
		if [ $? != 0 ]; then
			echo "Build failed."
			exit 1
		else
			echo ">>>>>>>Finished Building.<<<<<<<<"
			exit 0
		fi
            
else
	echo "Usage: build.sh clean|[<fsim_path> <psim_path>]"
	echo "Please specify the full path of updated fsim"
	exit 1
fi
