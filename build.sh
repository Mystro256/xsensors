#!/bin/bash

# Copyright (c) 2022 Alex313031.

YEL='\033[1;33m' # Yellow
CYA='\033[1;96m' # Cyan
RED='\033[1;31m' # Red
GRE='\033[1;32m' # Green
c0='\033[0m' # Reset Text
bold='\033[1m' # Bold Text
underline='\033[4m' # Underline Text

# Error handling
yell() { echo "$0: $*" >&2; }
die() { yell "$*"; exit 111; }
try() { "$@" || die "${RED}Failed $*"; }

# --help
displayHelp () {
	printf "\n" &&
	printf "${bold}${GRE}Script to build xsensors on Linux.${c0}\n" &&
	printf "\n"
}

case $1 in
	--help) displayHelp; exit 0;;
esac

printf "\n" &&
printf "${bold}${GRE}Script to build xsensors on Linux.${c0}\n" &&
printf "${YEL}Building xsensors...\n" &&
printf "${CYA}\n" &&

# Build htop
export NINJA_SUMMARIZE_BUILD=1 &&

export CFLAGS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export CPPFLAGS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export GLIB_CFLAGS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export GTK_LIBS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export GTK_CFLAGS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export XSENSORS_CFLAGS="-DNDEBUG -g0 -s -O3 -mavx -maes"
export LDFLAGS="-Wl,-O3 -mavx -maes"

make clean &&

./autogen.sh &&

./configure --with-gtk2 --disable-debug &&

make VERBOSE=1 V=1 &&

printf "\n" &&
printf "${GRE}${bold}Build Completed. ${YEL}${bold}You can now sudo make install or make install to install it.\n" &&
printf "\n" &&
tput sgr0 &&

exit 0
