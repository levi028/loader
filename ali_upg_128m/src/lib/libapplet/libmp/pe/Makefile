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
ROOT = ../../../../..

# Module Name
MOD = LIB_PE

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

# Sub Module Path
MUSIC_ENGINE_PATH = music/
IMAGE_ENGINE_PATH = image/
VIDEO_ENGINE_PATH = video/

# List of source files
ifdef SEE_CPU
ASMS =
SRCS = 
else
ASMS =
SRCS = pe.c pe_cache.c avp_interface.c ali_stream.c es_player_interface.c pe_flash.c
endif

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.

ifdef _MPEG4_SUPPORT
SMOD = LIB_PE_MUSIC_ENGINE LIB_PE_IMAGE_ENGINE LIB_PE_ADV_VIDEO_ENGINE
else
SMOD = LIB_PE_MUSIC_ENGINE LIB_PE_IMAGE_ENGINE LIB_PE_VIDEO_ENGINE
endif

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}
SMOD__ = ${SMOD:=__}

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
ifndef SEE_CPU
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
endif
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk	

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
ifndef SEE_CPU
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@
endif

clean : ${SMOD_}
	rm -f ${OBJS} ${MOD}.mk
	cd ${LIB_DIR}; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst

ddk_rel : ${SMOD__}
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} > dep.txt
	${DDK_DIR}/genlist dep.txt "${BASE_DIR}" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" 1>file.txt 2>dir.txt
	sh dir.txt
	sh file.txt
	rm -f dep.txt
	rm -f dir.txt
	rm -f file.txt						

rebuild: clean all
# End of common description.

# Construct and Deconstruct of sub-modules
LIB_PE_MUSIC_ENGINE :
	cd music/; \
	  ${MAKE}

LIB_PE_MUSIC_ENGINE_ :
	cd music/; \
	  ${MAKE} clean

LIB_PE_MUSIC_ENGINE__ :
	cd music/; \
	  ${MAKE} ddk_rel

LIB_PE_IMAGE_ENGINE :
	cd image/; \
	  ${MAKE}
	  	  
LIB_PE_IMAGE_ENGINE_ :
	cd image/; \
	  ${MAKE} clean

LIB_PE_IMAGE_ENGINE__ :
	cd image/; \
	  ${MAKE} ddk_rel

LIB_PE_VIDEO_ENGINE :
	cd video/; \
	  ${MAKE}
LIB_PE_VIDEO_ENGINE_ :
	cd video/; \
	  ${MAKE} clean	  
LIB_PE_VIDEO_ENGINE__ :
	cd video/; \
	  ${MAKE} ddk_rel	  
LIB_PE_ADV_VIDEO_ENGINE :
	cd video_advanced/; \
	  ${MAKE}
	  	  
LIB_PE_ADV_VIDEO_ENGINE_ :
	cd video_advanced/; \
	  ${MAKE} clean	  

LIB_PE_ADV_VIDEO_ENGINE__ :
	cd video_advanced/; \
	  ${MAKE} ddk_rel		  
