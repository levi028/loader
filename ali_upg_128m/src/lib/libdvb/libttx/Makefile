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
ROOT = ../../../..

# Module Name
MOD = LIB_TTX

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

# List of source files
ifdef LIB_TTX_REMOTE
ASMS =
SRCS = lib_ttx.c lib_ttx_remote.c 
else
	ifndef _BUILD_OTA_E_
	ASMS =
	SRCS = lib_ttx.c lib_ttx_remote.c ttx_osd.c lib_ttx_init.c lib_ttx_init_ext.c ttx_osd_16.c ttx_osd_256.c ttx_font_arabic.c \
	       ttx_font_cyrillic_1.c ttx_font_cyrillic_2.c ttx_font_cyrillic_3.c \
	       ttx_font_g2.c ttx_font_g2_arabic.c ttx_font_g2_cyrillic.c ttx_font_g2_greek.c ttx_font_g3.c ttx_font_greek.c ttx_font_hebrew.c 
	else	
	ASMS =
	SRCS = lib_ttx_remote.c lib_ttx_ota.c	
	endif
endif

ifdef LIB_TTX_REMOTE
DEFS += -D_LIB_TTX_REMOTE
endif

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD =

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}

CFLAGS += -I./

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@
	
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

