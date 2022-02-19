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
MOD = LIB_XML2

# List of source files


ASMS =
SRCS = parser.c xml_hash.c c14n.c catalog.c chvalid.c debugXML.c dict.c DOCBparser.c encoding.c entities.c \
	error.c globals.c relaxng.c SAX2.c SAX.c HTMLparser.c HTMLtree.c legacy.c list.c nanoftp.c \
	nanohttp.c parserInternals.c pattern.c schematron.c threads.c tree.c \
	uri.c valid.c xinclude.c xlink.c xmlIO.c xpointer.c xmlmemory.c xmlmodule.c \
	xmlreader.c xmlregexp.c xmlsave.c xmlschemas.c xmlschemastypes.c xmlstring.c xmlunicode.c \
	xmlwriter.c xpath.c lib_xml_api.c
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
SMOD__ = ${SMOD:=__}

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk	

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	cp -f ${ROOT}/src/compiler.def ${ROOT}/src/compiler_nf.def 
	sed -i "s/-Wformat=2/ /g" ${ROOT}/src/compiler_nf.def
	sed -i "s/-Werror/ /g" ${ROOT}/src/compiler_nf.def
	sed -i "s/-Wall/-w/g" ${ROOT}/src/compiler_nf.def
	sed -i "s/compiler.def/compiler_nf.def/g" $@	
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@
	
ddk_rel: ${SMOD__}
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

