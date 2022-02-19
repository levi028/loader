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
ROOT = ../../../../

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

# Module Name
MOD = LIB_STANDBY

# Library Name
LIB_FILE_NAME =${OUT}.a

# Compiler source code file.
ifdef _M3505_
OUT = boot3505_stby_c
endif

ifdef _M3702_
OUT = boot3702_stby_c
endif

ifdef _M3503C_
OUT = boot3503c_stby_c
endif

ifdef _M3503D_
OUT = boot3503d_stby_c
endif

ifdef _M3711C_
OUT = boot3711c_stby_c
endif

# List of source files
ifdef _M3505_
SRCS = slot.c ch455_gpio.c stby_ir.c
ASMS = Stdby_3505.S
endif

ifdef _M3702_
SRCS = slot.c ch455_gpio_c3702.c stby_ir.c
ASMS = Stdby_3702.S
endif

ifdef _M3503C_
SRCS = slot.c stby_ir.c
ASMS = Stdby_3503C.S
endif

ifdef _M3503D_
SRCS = slot.c stby_ir.c ch455_gpio.c
ASMS = Stdby_3503D.S
endif

ifdef _M3711C_
SRCS = slot.c stby_ir.c ch455_gpio.c
ASMS = Stdby_3711C.S
endif

ifdef _M3503D_
C32FLAGS =-membedded-data -ffunction-sections -g -EL -mips2 -O1 -msoft-float -fsigned-char -fno-builtin-printf ${DEFS} -I${INC_DIR}
C32FLAGS := -G0 ${C32FLAGS}
else
ifdef _M3711C_
C32FLAGS =-membedded-data -ffunction-sections -g -EL -mips2 -O1 -msoft-float -fsigned-char -fno-builtin-printf ${DEFS} -I${INC_DIR}
C32FLAGS := -G0 ${C32FLAGS}
else
C32FLAGS = -g -EL -mips32r2 -O1 -msoft-float -fsigned-char -fno-builtin-printf ${DEFS} -I${INC_DIR}
C32FLAGS := -G0 ${C32FLAGS}
endif
endif

# List of library files
LIBS =

OBJS = ${SRCS:.c=.o} ${ASMS:.S=.o}
SMOD_ = ${SMOD:=_}

all : ${MOD}.mk
	${CC} ${C32FLAGS} -G 0 -c -o slot.o slot.c
ifndef _M3503C_
ifndef _M3503D_
ifndef _M3711C_
ifdef _M3702_
	${CC} ${C32FLAGS} -G 0 -c -o ch455_gpio_c3702.o ch455_gpio_c3702.c
else
	${CC} ${C32FLAGS} -G 0 -c -o ch455_gpio.o ch455_gpio.c
endif
endif
endif
endif
	${CC} ${C32FLAGS} -G 0 -c -o stby_ir.o stby_ir.c
	${MAKE} -f ${MOD}.mk ${OBJS}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	${AR} -r ${LIB_FILE_NAME} ${OBJS}
	cp -f ${LIB_FILE_NAME} ${DDK_DIR}
	cp -f ${LIB_FILE_NAME} ${LIB_DIR}

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${C32FLAGS} -M ${ASMS} ${SRCS} >> $@

clean : ${SMOD_}
	rm -f *.o *.a ${MOD}.mk
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
