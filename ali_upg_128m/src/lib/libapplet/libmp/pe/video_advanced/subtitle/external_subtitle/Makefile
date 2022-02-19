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
ROOT = ../../../../../../../..

# Module Name
MOD = LIB_PE_EXTERNAL_SUBTITLE
SDKLIB = lib_pe_ext_sub

# Sub Module Path

# List of source files
ASMS =
SRCS = mp_plugin_ext_sub.c mp_ext_sub_ass.c mp_ext_sub_smi.c mp_ext_sub_srt.c mp_ext_sub_vobsub.c mp_ext_sub_txt.c \
			 mp_ext_sub_priv.c

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

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.ddk	
	${LD} -EL -r -o ${SDKLIB}.o ${OBJS}
ifdef PLUGIN_IN_SDK_DIR
	cp -f ${SDKLIB}.o ${SDK_DIR}/plugin/m31/${SDKLIB}.o
else
ifdef _S3281_
	cp -f ${SDKLIB}.o $(PLUGINEXT_DIR)/m32/${SDKLIB}.o
else 	
	cp -f ${SDKLIB}.o ${PLUGINEXT_DIR}/${SDKLIB}.o
endif	
endif	

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@

clean : ${SMOD_}
ifdef PLUGIN_IN_SDK_DIR
	rm -f ${OBJS} ${MOD}.mk ${SDKLIB}.o ${SDK_DIR}/plugin/m31/${SDKLIB}.o
else
	rm -f ${OBJS} ${MOD}.mk ${SDKLIB}.o $(PLUGIN_DIR)/${SDKLIB}.o
endif
	cd ${LIB_DIR}; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst

ddk_rel :
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


