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
ROOT = ../../../../../..

# Module Name
MOD = GOS_PA

# List of source files
ASMS = 
#SRCS = ${wildcard *.c} 
SRCS = 	gacas_aes.c \
	gacas_aes_soft.c \
	gacas_loader_db.c \
	gacas_upg.c \
	win2_pa_otaupg.c \
	win2_pa_usbupg.c

	
# List of library files
LIBS = 

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD = 

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

#OPATH += ./obj
#OBJS = $(shell find $(OPATH) -name "*.o")
OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${notdir ${OBJS} ${LIBS}} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
ifdef _WARNING_TO_ERROR
	echo "" >>$@
	echo "CFLAGS+=-Werror" >>$@
endif	
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

# Deconstruct of sub-modules
gacas_aes.o: gacas_aes.c gos_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_define.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_memmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/basic_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/retcode.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_base.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/itron.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/alitypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/platform.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/asm/chip.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/alloc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_timer.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_msgq.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_sema.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_flag.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_cache.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mutex.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_dual.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/machine.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/mips.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stdarg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/string.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/hld_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/gma_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/erom/erom.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchunk/chunk.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_lib.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/mediatypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchar/lib_char.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/lib_ge_osd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_scrollbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_progressbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_objectlist.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_editfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_tdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dsc/dsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multisel.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multitext.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_matrixbox.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_animation.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/decv/vdec_driver.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/decv/decv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dis/vpo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/deca/deca.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/deca/deca_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_pub.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_as.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/db_3l.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_return_value.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_node_combo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_interface.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/p_search.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_pmt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_sdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_mv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_hde.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/otp/otp.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/crypto/crypto.h \
  gacas_loader_db.h gacas_aes.h
gacas_aes_soft.o: gacas_aes_soft.c gos_config.h
gacas_loader_db.o: gacas_loader_db.c gos_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/string.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/basic_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/otp/otp.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_define.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_memmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/hld_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/retcode.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_base.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/itron.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/alitypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/platform.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/asm/chip.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/alloc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_timer.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_msgq.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_sema.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_flag.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_cache.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mutex.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_dual.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/erom/erom.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchunk/chunk.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/fast_crc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dsc/dsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/crypto/crypto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/flash_cipher.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_node_combo.h \
  gacas_loader_db.h gacas_aes.h
gacas_upg.o: gacas_upg.c gos_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_define.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_memmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/basic_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/retcode.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_base.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/itron.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/alitypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/platform.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/asm/chip.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/alloc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_timer.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_msgq.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_sema.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_flag.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_cache.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mutex.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_dual.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/machine.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/mips.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stdarg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/string.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/hld_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/gma_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/erom/erom.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchunk/chunk.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_lib.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/mediatypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchar/lib_char.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/lib_ge_osd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_scrollbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_progressbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_objectlist.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_editfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_tdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dsc/dsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multisel.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multitext.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_matrixbox.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_animation.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/decv/vdec_driver.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/decv/decv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dis/vpo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/deca/deca.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/deca/deca_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_pub.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_as.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/db_3l.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_return_value.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_node_combo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_interface.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/p_search.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_pmt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_sdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_mv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_hde.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/crypto/crypto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/trng/trng.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/rsa_verify.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/otp/otp.h \
  gacas_loader_db.h gacas_upg.h gacas_aes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/flash_cipher.h
win2_pa_otaupg.o: win2_pa_otaupg.c \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_define.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_memmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/basic_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/mediatypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/dog/dog.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/retcode.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_base.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/itron.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/alitypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/platform.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/asm/chip.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/alloc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_timer.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_msgq.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_sema.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_flag.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_cache.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mutex.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_dual.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/machine.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/mips.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stdarg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/string.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libota/lib_ota.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/si_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/decv/decv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dis/vpo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/hld_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/gma_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/snd/snd_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_lib.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchar/lib_char.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/lib_ge_osd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_scrollbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_progressbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_objectlist.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_editfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_tdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dsc/dsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multisel.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multitext.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_matrixbox.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_animation.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_eit.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_module.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/dma/dma.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/p_search.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_pmt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_sdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/db_3l.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_return_value.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_node_combo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_interface.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sie.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/lib_epg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchunk/chunk.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/erom/erom.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_parameters.h \
  ../control.h ../win_com.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_device_manage.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_pub.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_as.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_mv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/tsi/tsi.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_hde.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/sie_monitor.h \
  ../copper_common/dev_handle.h ../copper_common/com_api.h \
  ../copper_common/system_data.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim.h \
  ../copper_common/../win_media.h ../copper_common/system_type.h \
  ../copper_common/menu_api.h ../copper_common/com_epg.h \
  ../copper_common/system_data.h ../pvr_control.h \
  ../pvr_ctrl_parent_lock.h ../win_signalstatus.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  ../pvr_ctrl_basic.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libci/ci_plus.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr3.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr_eng.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/lib_stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/_ansi.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/newlib.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/config.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/machine/ieeefp.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/_types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stddef.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/machine/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr_mgr.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  ../win_pvr_hint.h ../ctrl_util.h .././copper_common/com_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/lib_stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/stdlib.h \
  ../pvr_ctrl_ca.h ../pvr_ctrl_ts_route.h ../pvr_ctrl_cnx9_v6.h \
  ../pvr_ctrl_record.h ../pvr_ctrl_tms.h ../pvr_ctrl_play.h \
  ../pvr_ctrl_timer.h ../vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  ../win_submenu.h ../win_com_popup.h ../win_com_list.h \
  ../ctrl_play_channel.h ../ctrl_play_si_proc.h ../ctrl_preplay_channel.h \
  ../ctrl_key_proc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/pan/pan_dev.h \
  ../ap_dynamic_pid.h ../ap_ctrl_ci.h ../declare_common.h ../key.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/pan/pan.h \
  ../flash_prot.h ../osd_rsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libmp/mp_subtitle_api.h \
  ../osdobjs_def.h ../string.id ../images.id ../osd_config.h \
  ../menus_root.h ../win_com.h ../win_signalstatus.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/flash_cipher.h \
  ../win_ota_set.h win2_pa_otaupg.h gacas_loader_db.h gacas_upg.h \
  gacas_aes.h ../key.h ../power.h
win2_pa_usbupg.o: win2_pa_usbupg.c \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_define.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/sys_memmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/basic_types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/mediatypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/retcode.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_base.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/itron.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/alitypes.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/platform.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/os/tds2/config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/asm/chip.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/alloc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_int.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_timer.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_msgq.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_sema.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_flag.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mm.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_cache.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_mutex.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_dual.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/machine.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hal/mips.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stdarg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/debug_printf.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/string.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/pan/pan_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/hld_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/pan/pan.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_lib.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/osd/osddrv_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_old.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/ge_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/ge/gma_new.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchar/lib_char.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/lib_ge_osd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_scrollbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_progressbar.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_objectlist.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_editfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_tdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dsc/dsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multisel.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_multitext.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_matrixbox.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_animation.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libchunk/chunk.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/sto/sto_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/erom/erom.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libupg/usb_upgrade.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/flash_cipher.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/statvfs.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/unistd.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/_ansi.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/newlib.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/config.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/machine/ieeefp.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/sys/_types.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/include/stddef.h \
  /mips-sde/bin/../lib/gcc/sde/3.4.4/../../../../sde/include/machine/types.h \
  ../osdobjs_def.h ../win_com_popup.h ../string.id ../images.id \
  ../osd_config.h ../osd_rsc.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common_draw.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libmp/mp_subtitle_api.h \
  ../control.h ../win_com.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_device_manage.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/p_search.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/si_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_pmt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/sec_sdt.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/db_3l.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_return_value.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_config.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_node_combo.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libdb/db_interface.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_pub.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_as.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_mv.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_frontend.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/tsi/tsi.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpub/lib_hde.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/sie_monitor.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libtsi/si_types.h \
  ../copper_common/dev_handle.h ../copper_common/com_api.h \
  ../copper_common/system_data.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/hdmi/m36/hdmi_dev.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/nim/nim.h \
  ../copper_common/../win_media.h ../copper_common/system_type.h \
  ../copper_common/menu_api.h ../copper_common/com_epg.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libsi/lib_epg.h \
  ../copper_common/system_data.h ../pvr_control.h \
  ../pvr_ctrl_parent_lock.h ../win_signalstatus.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_container.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_bitmap.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/obj_textfield.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_common.h \
  ../pvr_ctrl_basic.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libci/ci_plus.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr3.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr_eng.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/lib_stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libc/list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/types.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/hld/dmx/dmx.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/dma/dma.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libpvr/lib_pvr_mgr.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/osal/osal_task.h \
  ../win_pvr_hint.h ../ctrl_util.h .././copper_common/com_api.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libfs2/lib_stdio.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/stdlib.h \
  ../pvr_ctrl_ca.h ../pvr_ctrl_ts_route.h ../pvr_ctrl_cnx9_v6.h \
  ../pvr_ctrl_record.h ../pvr_ctrl_tms.h ../pvr_ctrl_play.h \
  ../pvr_ctrl_timer.h ../vkey.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/libosd/osd_vkey.h \
  ../win_submenu.h ../win_com_popup.h ../win_com_list.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/bus/dog/dog.h \
  ../ctrl_play_channel.h ../ctrl_play_si_proc.h ../ctrl_preplay_channel.h \
  ../ctrl_key_proc.h ../ap_dynamic_pid.h ../ap_ctrl_ci.h \
  ../declare_common.h ../key.h ../flash_prot.h ../osd_rsc.h ../win_com.h \
  ../menus_root.h \
  /cygdrive/e/Ali_Work/Ali3711_GA/4M/Ali3711C_Dvbc_Product_4M/ALi_UPG/inc/api/librsa/rsa_verify.h \
  win2_pa_usbupg.h gacas_loader_db.h gacas_upg.h gacas_aes.h ../power.h
