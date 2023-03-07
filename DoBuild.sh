#!/bin/bash
set -e # aborta no primeiro erro

ccomp="FALSE";
installDir="/usr/share/libSocketCom"; # ccomp="FALSE";
bmode="RELEASE";
rebuild=0;
install=0;
dhcomp=1;

__help="
usage:
-cn -> compile native (default)
-cc -> cross compile
-br -> build release (default)
-bd -> build debug
-r  -> rebuild (clean)
-rf -> rebuild full (rm -rf*)
-i  -> install
-nn -> no renew DHComp
"

while [ "$#" -gt 0 ]; do
  case "$1" in
    -cn) ccomp="FALSE"; shift 1;;
    -cc) ccomp="TRUE"; shift 1;;
    -br) bmode="RELEASE"; shift 1;;
    -bd) bmode="DEBUG"; shift 1;;
    -r) rebuild=1; shift 1;;
    -rf) rebuild=2; shift 1;;
    -i) install=1; shift 1;;
    -nn) dhcomp=0; shift 1;;
		# arguments with data (space separed, like -p 123)
    # -p) pidfile="$2"; shift 2;;
    # -l) logfile="$2"; shift 2;;

		# arguments with data (equel separed, like -p=123)
    --compile=*) ccomp="${1#*=}"; shift 1;;
    --build=*) cmode="${1#*=}"; shift 1;;
    
		-h) echo "$__help"; exit 1;;
    -*|*) echo "unknown option: $1 (try -h for help)" >&2; exit 1;;
  esac
done

if [ $dhcomp -eq 1 ]; then
	mkdir -p Inc
	vardh=$(date +%Y%m%d%H%M%S)
	echo "#define DH_COMP \""${vardh}"\"" > ./Inc/DHComp.h
	echo ">> DH_COMP \""${vardh}"\""
fi

if [ $ccomp = "TRUE" ]; then
	mkdir -p build
	cd build
else
	mkdir -p buildPC
	cd buildPC
fi

if [ $ccomp = "TRUE" ]; then
	installDir="/usr/share/libSocketCom";
fi
export MAKE_CROSSCOMPILING=$ccomp
export BUILD_MODE=$bmode
export INSTALLDIR=$installDir

echo ">> CROSSCOMPILING = $ccomp"
echo ">> BUILD_MODE = $bmode"
echo ">> INSTALLDIR = $installDir"

if [ $rebuild -eq 1 ]; then
	echo ">> Clean and Rebuild"
	make clean
elif [ $rebuild -eq 2 ]; then
	echo ">> Clean FULL and Rebuild"
	rm -rf *
fi

cmake ../
make -j 4

if [ $install -eq 1 ]; then
	# lib install
	echo ">> installing in $installDir"
	if [ ! -d $installDir ]; then
		echo ">> necessario criar diretorio destino"
		sudo mkdir -p $installDir
	fi

	ahResPerm=$(stat -c "%a" $installDir)
	if [ ! $ahResPerm -eq 777 ]; then
		sudo chmod a+rw $installDir
	fi
	cmake --install . --prefix "$installDir"
fi

cd ..
