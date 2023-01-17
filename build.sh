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
	printf "${YEL}Use the --clean flag to run make clean. (should be run when rebuilding)\n" &&
	printf "${YEL}Use the --gtk2 flag to build with GTK2.\n" &&
	printf "${YEL}Use the --gtk3 flag to build with GTK3.\n" &&
	printf "${YEL}Use the --help flag to show this help.\n" &&
	tput sgr0 &&
	printf "\n"
}

case $1 in
	--help) displayHelp; exit 0;;
esac

printf "\n" &&
printf "${bold}${GRE}Script to build xsensors on Linux.${c0}\n" &&
printf "${YEL}Building xsensors...${c0}\n" &&

# Export optimization flags
export NINJA_SUMMARIZE_BUILD=1 &&

export CFLAGS="-DNDEBUG -g0 -s -O3 -msse3"
export CPPFLAGS="-DNDEBUG -g0 -s -O3 -msse3"
export GLIB_CFLAGS="-DNDEBUG -g0 -s -O3 -msse3"
export GTK_LIBS="-DNDEBUG -g0 -s -O3 -msse3"
export GTK_CFLAGS="-DNDEBUG -g0 -s -O3 -msse3"
export XSENSORS_CFLAGS="-DNDEBUG -g0 -s -O3 -msse3"
export LDFLAGS="-Wl,-O3 -msse3"

# --clean
makeClean () {
	printf "${CYA}\n" &&
	make clean &&
	printf "\n" &&
	printf "${bold}${GRE}Build dir cleaned.${c0}\n" &&
	printf "\n"
}

case $1 in
	--clean) makeClean; exit 0;;
esac

# --gtk2
makeGTK2 () {
	printf "\n" &&
	printf "${bold}${GRE}Building with GTK2...${c0}\n" &&
	printf "${CYA}\n" &&
	./autogen.sh &&

	./configure --with-gtk2 --disable-debug &&

	make VERBOSE=1 V=1
	printf "${c0}\n" &&
	printf "${GRE}${bold}Build Completed. ${YEL}${bold}You can now sudo make install or make install to install it.\n" &&
	printf "${GRE}(sudo make uninstall or make uninstall to uninstall it)\n" &&
	printf "\n" &&
	tput sgr0
}

case $1 in
	--gtk2) makeGTK2; exit 0;;
esac

# --gtk3
makeGTK3 () {
	printf "\n" &&
	printf "${bold}${GRE}Building with GTK3...${c0}\n" &&
	printf "${CYA}\n" &&
	./autogen.sh &&

	./configure --disable-debug &&

	make VERBOSE=1 V=1 &&
	printf "${c0}\n" &&
	printf "${GRE}${bold}Build Completed. ${YEL}${bold}You can now sudo make install or make install to install it.\n" &&
	printf "${GRE}(sudo make uninstall or make uninstall to uninstall it)\n" &&
	printf "\n" &&
	tput sgr0
}

case $1 in
	--gtk3) makeGTK3; exit 0;;
esac

# Error message if no flag is supplied.
printf "${bold}${RED}Error: Must supply --gtk2, --gtk3, and/or --clean.\n" &&
printf "${bold}${YEL}Run this script with --help to display full help message.\n" &&
printf "\n" &&
tput sgr0 &&

exit 1
