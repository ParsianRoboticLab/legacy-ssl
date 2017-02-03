#!/bin/bash

DIRECTORY=logs
GITDIR=.git
REPO=https://gitlab.com/ParsianRobotics/logs.git

cecho () {
	bold=$(tput bold)
	green=$(tput setaf 2)
	reset=$(tput sgr0)
	echo "$bold$green$1$reset"
}

if [ ! -d "$DIRECTORY" ]; then
	mkdir "$DIRECTORY"
	cecho "logs Folder Created"	
fi

cd "$DIRECTORY"

if [ ! -d "$GITDIR" ]; then
	git init
	git remote add origin "$REPO"
	cecho "GIT Initialized"
fi

cd ../
cecho "Done"
