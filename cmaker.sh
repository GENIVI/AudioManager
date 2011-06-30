 #!/bin/sh -e
 #
 #  Copyright (C) 2011, BMW AG 
 # 
 # AudioManagerDeamon
 #
 # @file cmaker.sh
 #
 # @date: 20.05.2011
 # @author: Christian Müller (christian.ei.mueller@bmw.de)
 #
 # Shell script to envoke the out of source build of the Audiomanager and create documentation
 # after invoking this script, first all Plugins will be compiled and the resulting .a files
 # are found in ./build/plugins. 
 # All binaries are found in ./bin
 # Finally source documentation can be found in ./doc
 # to do a clean build, just remove the ./build folder or just a subfolder in this build folder.


BUILD_DIRECTORY="build"
PLUGINS_DIRECTORY="plugins"
DOC_DIRECTORY="doc"
BIN_DIRECTORY="bin"
ACTION=$1
ABS=`exec pwd`
APPLICATION_LIST=$(find . -maxdepth 1 -type d \( -name "[A-Z]*" -a  -not \( -name "Plugin*" \) \) )
PLUGIN_LIST=$(find . -maxdepth 1 -type d \( -name "[A-Z]*" -a  -name "Plugin*" \) )
# checks for Build directory and creates it if neccessary
make_build ()
{
	BIN=$ABS"/"$BIN_DIRECTORY
	if [ ! -d "${BIN}" ]; 
	then
		echo "create Bin Dir: $BIN_DIRECTORY"
		mkdir $BIN_DIRECTORY
		cd $BIN_DIRECTORY
		cp ../AudioManGUI/Bild1.png .
		cd ..
	fi

	ABSOLUTE=$ABS"/"$BUILD_DIRECTORY
	if [ ! -d "${ABSOLUTE}" ]; 
	then
		echo "create Build Dir: $BUILD_DIRECTORY"
		mkdir $BUILD_DIRECTORY
	fi
	cd "${ABSOLUTE}"
	if [ ! -d "${PLUGINS_DIRECTORY}" ]; 
	then
		echo "create Build Dir: $PLUGINS_DIRECTORY"
		mkdir $PLUGINS_DIRECTORY
	fi

}
# does cmake for list in $1
do_cmake_plugin ()
{
	for PL in $PLUGIN_LIST
	do
		echo $PL
	done
	
	for PL in $PLUGIN_LIST
	do		
		if [ ! -d "$PL" ]; then
			mkdir $PL
		fi
	
		cd $PL
		p=$ABS"/"${PL#"./"}
		cmake $p 
		cd ..		
	done
}
do_cmake_app ()
{
	for PL in $APPLICATION_LIST
	do
		echo $PL
	done
	
	for PL in $APPLICATION_LIST
	do		
		if [ ! -d "$PL" ]; then
			mkdir $PL
		fi
	
		cd $PL
		p=$ABS"/"${PL#"./"}
		cmake $p 
		cd ..		
	done
}
# does make for list in $1
do_make_plugin ()
{
	for PL in $PLUGIN_LIST
	do
		echo $PL
	done
	
	for PL in $PLUGIN_LIST
	do		
		if [ ! -d "$PL" ]; then
			mkdir $PL
		fi
	
		cd $PL
		make -j4  
		cd ..		
	done
}
do_make_app ()
{
	for PL in $APPLICATION_LIST
	do
		echo $PL
	done
	
	for PL in $APPLICATION_LIST
	do		
		if [ ! -d "$PL" ]; then
			mkdir $PL
		fi
	
		cd $PL
		make -j4  
		cd ..		
	done
}
all ()
{
	case "$1" in
	cmake)
		do_cmake_plugin
		do_cmake_app
		;;
	make)
		do_make_plugin
		do_make_app
		;;
	all)
		do_cmake_plugin
		do_make_plugin
		do_cmake_app
		do_make_app
		;;	
	esac
}
main ()
{
	case "$1" in
	cmake)
		do_cmake_app
		;;
	make)
		do_make_app
		;;
	all)
		do_cmake_app
		do_make_app
		;;	
	esac
}
plugins ()
{
	case "$1" in
	cmake)
		do_cmake_plugin
		;;
	make)
		do_make_plugin
		;;
	all)
		do_cmake_plugin
		do_make_plugin
		;;	
	esac
}
target ()
{
	b=$2
	if [ ! -d "$2" ]; then
		mkdir $2
	fi
	case "$1" in
	cmake)
		cd $b
		p=$ABS"/"${b#"./"}
		cmake $p 
		;;
	make)
		cd $b
		make -j4
		;;
	all)
		cd $b
		p=$ABS"/"${b#"./"}
		cmake $p;
		make -j4
		;;	
	esac
}
case "$ACTION" in
cmake)
	echo "...executing cmake for"
	;;
make)
	echo "...executiong make for"
	;;
all)
	echo "...executing cmake & make for"
	;;
*)
	echo ""
	echo "		Usage: ./cmaker.sh {cmake|make|all} {all|main|plugins|target xxx}"
	echo "		xxx stands for the directory of the target to be build"
	echo "		Output binaries are below \bin, builds are done in \build\xxx"
	echo "		Script does only use Subdirectories beginning with upper Cases !"
	echo ""	
	exit 1
	;;
esac
make_build
case "$2" in
all)
	all $ACTION
	;;
applications)
	main $ACTION
	;;
plugins)
	plugins $ACTION 
	;;
target)
	target $ACTION $3
	;;
*)
	echo ""
	echo "		Usage: ./cmaker.sh {cmake|make|all} {all|applications|plugins|target xxx}"
	echo "		xxx stands for the directory of the target to be build"
	echo "		Output binaries are below \bin, builds are done in \build\xxx"
	echo "		Script does only use Subdirectories beginning with upper Cases !"
	echo ""
	exit 1
	;;
esac
exit 0
