#
# -= Makefile for module compile =-
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as module's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Define the Module Name in MOD as you like;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#

# Destination of definition files
ROOT = ../../../

# Module Name
MOD = VERSION_INFO_APP

# List of source files
ASMS =
SRCS = version_app.c

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD =

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}

uts_len		:= 64
uts_truncate	:= sed -e 's/\(.\{1,$(uts_len)\}\).*/\1/'

SDKLABEL = SDK4.0ba.4.0
SDKDATE = 20101217

ifndef APPRELEASE
APPRELEASE	= 1.0.0
endif
ifndef APPLABEL
APPLABEL = ${SDKLABEL}_${SDKDATE}
endif


all : ${MOD}.mk
	rm -f .ver1 .ver
	if [ -f .name ]; then  echo -n \-`cat .name` >> .ver1; fi
	LANG=C echo ' '`date` >> .ver1
	echo \#define UTS_VERSION \"`cat .ver1 | $(uts_truncate)`\" > .ver
	LANG=C echo \#define COMPILE_TIME \"`date +%T`\" >> .ver
	echo \#define COMPILE_BY \"`whoami`\" >> .ver
	echo \#define COMPILER \"`$(CC) $(CFLAGS) -v 2>&1 | tail -1`\" >> .ver
	echo \#define APP_VER \"$(APPRELEASE)\" >> .ver	
	echo \#define APPLABEL \"$(APPLABEL)\" >> .ver	
	echo \#define SDKLABEL \"$(SDKLABEL)_$(SDKDATE)\" >> .ver	
	mv -f .ver version.h
	rm -f .ver1
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.ddk	

libplus:  ${MOD}.mk
	rm -f .ver1 .ver
	if [ -f .name ]; then  echo -n \-`cat .name` >> .ver1; fi
	LANG=C echo ' '`date` >> .ver1
	echo \#define UTS_VERSION \"`cat .ver1 | $(uts_truncate)`\" > .ver
	LANG=C echo \#define COMPILE_TIME \"`date +%T`\" >> .ver
	echo \#define COMPILE_BY \"`whoami`\" >> .ver
	echo \#define COMPILER \"`$(CC) $(CFLAGS) -v 2>&1 | tail -1`\" >> .ver
	echo \#define LIBCORE_VER \"$(LIBPLUSRELEASE)\" >> .ver	
	echo \#define LIBCORE_LABEL \"$(LIBPLUSLABEL)\" >> .ver	
	mv -f .ver version.h
	rm -f .ver1
	cp -f version.c .ver1
	sed -e 's/libcore_banner/libplus_banner/; s/Libcore/Libplus/g; s/get_core_ver/get_libplus_ver/g' version.c > .ver
	mv -f .ver version.c
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	mv -f .ver1 version.c

	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.ddk	

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${C32FLAGS} -M ${ASMS} ${SRCS} >> $@
	
ddk_rel :
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} > dep.txt
	${DDK_DIR}/genlist dep.txt "${BASE_DIR}" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" 1>file.txt 2>dir.txt
	sh dir.txt
	sh file.txt
	rm -f dep.txt
	rm -f dir.txt
	rm -f file.txt																

clean : ${SMOD_}
	rm -f ${OBJS} ${MOD}.mk
	cd ${LIB_DIR}; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst

rebuild: clean all
# End of common description.

# Construct of sub-modules

# Deconstruct of sub-modules
