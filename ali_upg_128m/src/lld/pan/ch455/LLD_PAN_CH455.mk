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
MOD = LLD_PAN_CH455

# List of source files
ASMS =
SRCS = pan_ch455.c

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

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@

clean : ${SMOD_}
	rm -f ${OBJS} ${MOD}.mk
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

# Construct of sub-modules

# Deconstruct of sub-modules
pan_ch455.o: pan_ch455.c /home/gospell/Code/Ali3711C/OUC/inc/sys_config.h \
 /home/gospell/Code/Ali3711C/OUC/inc/sys_define.h \
 /home/gospell/Code/Ali3711C/OUC/inc/sys_memmap.h \
 /home/gospell/Code/Ali3711C/OUC/inc/retcode.h \
 /home/gospell/Code/Ali3711C/OUC/inc/types.h \
 /home/gospell/Code/Ali3711C/OUC/inc/basic_types.h \
 /home/gospell/Code/Ali3711C/OUC/inc/api/libc/alloc.h \
 /home/gospell/Code/Ali3711C/OUC/inc/api/libc/printf.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_base.h \
 /home/gospell/Code/Ali3711C/OUC/inc/os/tds2/itron.h \
 /home/gospell/Code/Ali3711C/OUC/inc/os/tds2/alitypes.h \
 /home/gospell/Code/Ali3711C/OUC/inc/os/tds2/platform.h \
 /home/gospell/Code/Ali3711C/OUC/inc/os/tds2/config.h \
 /home/gospell/Code/Ali3711C/OUC/inc/asm/chip.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_mm.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_int.h \
 /home/gospell/Code/Ali3711C/OUC/inc/api/libc/debug_printf.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_task.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_int.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_timer.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_msgq.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_sema.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_flag.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_mm.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_cache.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_mutex.h \
 /home/gospell/Code/Ali3711C/OUC/inc/osal/osal_dual.h \
 /home/gospell/Code/Ali3711C/OUC/inc/hal/machine.h \
 /home/gospell/Code/Ali3711C/OUC/inc/hal/mips.h \
 /opt/mips-sde-elf-4.7.3/bin/../lib/gcc/mips-sde-elf/4.7.3/include/stdarg.h \
 /home/gospell/Code/Ali3711C/OUC/inc/api/libc/debug_printf.h \
 /home/gospell/Code/Ali3711C/OUC/inc/api/libc/string.h \
 /home/gospell/Code/Ali3711C/OUC/inc/hld/hld_dev.h \
 /home/gospell/Code/Ali3711C/OUC/inc/hld/pan/pan.h \
 /home/gospell/Code/Ali3711C/OUC/inc/hld/pan/pan_dev.h \
 /home/gospell/Code/Ali3711C/OUC/inc/bus/i2c/i2c.h pan_ch455.h \
 /home/gospell/Code/Ali3711C/OUC/inc/bus/sci/sci.h ../irc/irc.h
