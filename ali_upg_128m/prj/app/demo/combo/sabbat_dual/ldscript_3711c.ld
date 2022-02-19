/* The following TEXT start address leaves space for the monitor
   workspace. i.e. the NEC VR4300 (IDT) first free address is actually
   0xa001af20. */

ENTRY(_start)
OUTPUT_FORMAT("elf32-tradlittlemips")
SEARCH_DIR(.)
__DYNAMIC  =  0;

/*
 * Allocate the stack to be at the top of memory, since the stack
 * grows down
 */
PROVIDE (__stack = 0);
/* PROVIDE (__global = 0); */

/*
 * Initalize some symbols to be zero so we can reference them in the
 * crt0 without core dumping. These functions are all optional, but
 * we do this so we can have our crt0 always use them if they exist.
 * This is so BSPs work better when using the crt0 installed with gcc.
 * We have to initalize them twice, so we multiple object file
 * formats, as some prepend an underscore.
 */
 

__RAM_BASE = 0x817B0600;
__ROM_BASE = __BOOT_LOADER ? 0xBFC00000 :
  __BOOT_FROM_FLASH ? 0xBFC00000 : __RAM_BASE;

MEMORY
{
  ram (rwx) : ORIGIN = 0x817B0400, LENGTH = 32M
  rom (rx)  : ORIGIN = 0xBFC00000, LENGTH = 512K - 128K
}

SECTIONS
{
  __BOOT_ROM_ADDR = (__BOOT_LOADER ? 0xBFC00000 - 512K : __ROM_BASE);
  .boot __RAM_BASE : AT(__BOOT_ROM_ADDR) {
     boot.o(.text)
  } >ram
  
 __STANDBY_RAM_ADDR = (__RAM_BASE + SIZEOF(.boot));
  __STANDBY_ROM_ADDR = __BOOT_ROM_ADDR + SIZEOF(.boot);
  .standby __STANDBY_RAM_ADDR : AT(__STANDBY_ROM_ADDR) {
	. = ALIGN(0x1000);
	__STANDBY_CODE_START  = .;	
	boot3711c_stby_c.a(.text)
	boot3711c_stby_c.a(.text.*)          
	__STANDBY_CODE_END = .; 
	. = ALIGN(0x4); 

	__STANDBY_DATA_START = .; 
	boot3711c_stby_c.a(.rdata)
	boot3711c_stby_c.a(.rodata)
	boot3711c_stby_c.a(.rodata.*)
	. = ALIGN(4);

	boot3711c_stby_c.a(.data)
	boot3711c_stby_c.a(.data.*)
	boot3711c_stby_c.a(.sdata)
	boot3711c_stby_c.a(.sdata.)
	. = ALIGN(4);

	boot3711c_stby_c.a(.sbss)
	boot3711c_stby_c.a(.scommon) 
	boot3711c_stby_c.a(.bss)
	boot3711c_stby_c.a(COMMON)
	*(.bootdata)
	. = ALIGN(4);
 	 
	__STANDBY_DATA_BAKUP = . ;
	__STANDBY_DATA_SP =  __STANDBY_DATA_BAKUP + (2*1024) - 16;
	__STANDBY_DATA_END = __STANDBY_DATA_BAKUP + (2*1024) ;
	. = __STANDBY_DATA_END;
  } >ram

  __TEXT_RAM_ADDR = __RAM_BASE  + SIZEOF(.boot) + SIZEOF(.standby);
  __TEXT_ROM_ADDR = __STANDBY_ROM_ADDR + SIZEOF(.standby);
  .text __TEXT_RAM_ADDR : AT(__TEXT_ROM_ADDR) {
    *(.init)
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t*)
    *(.mips16.fn.*)
    *(.mips16.call.*)
    PROVIDE (__runtime_reloc_start = .);
    *(.rel.sdata)
    PROVIDE (__runtime_reloc_stop = .);
    *(.fini)
    *(.reuse)
    __CTOR_LIST__ = . ;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    KEEP(*(SORT(.ctors)))
    LONG(0)
    __CTOR_END__ = . ;
    __DTOR_LIST__ = . ;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    KEEP(*(SORT(.dtors)))
    LONG(0)
    __DTOR_END__ = . ;
    . = ALIGN(16);
	__DD_SIG_ADDR_START = . ;
	__DD_SIG_ADDR_END = __DD_SIG_ADDR_START + 0x1000 ;
	. = __DD_SIG_ADDR_END ;
  } >ram

_code_text_end_addr = .;
  __EHFRMH_RAM_ADDR = __TEXT_RAM_ADDR + SIZEOF(.text) ;
  __EHFRMH_ROM_ADDR = __TEXT_ROM_ADDR + SIZEOF(.text) ;

  .eh_frame_hdr __EHFRMH_RAM_ADDR : AT(__EHFRMH_ROM_ADDR) {
    *(.eh_frame_hdr)
    *(.eh_frame_entry .eh_frame_entry.*)
    . = ALIGN(4);
  } >ram

  __RODATA_RAM_ADDR = __EHFRMH_RAM_ADDR + SIZEOF(.eh_frame_hdr);
  __RODATA_ROM_ADDR = __EHFRMH_ROM_ADDR + SIZEOF(.eh_frame_hdr);

  .rodata __RODATA_RAM_ADDR : AT(__RODATA_ROM_ADDR) {
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
    *(.flash)
    . = ALIGN(4);
  } >ram

  __DATA_RAM_ADDR = __RODATA_RAM_ADDR + SIZEOF(.rodata);
  __DATA_ROM_ADDR = __RODATA_ROM_ADDR + SIZEOF(.rodata);
  .data __DATA_RAM_ADDR : AT(__DATA_ROM_ADDR) {
    *(.eh_frame*)
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    *(.gcc_except_table)
    *(.gcc_except_table.*)
    *(.rel.dyn)
    *(.gnu_extab*)
    *(.got*)
    . = ALIGN(4);
  } >ram

  _gp = . + 0x8000;
  __global = _gp;
  .lit8 : {
    *(.lit8)
  }
  .lit4 : {
    *(.lit4)
  }

  __INIT_RAM_ADDR = __DATA_RAM_ADDR + SIZEOF(.data);
  __INIT_ROM_ADDR = __DATA_ROM_ADDR + SIZEOF(.data);
  .structure.init __INIT_RAM_ADDR : AT(__INIT_ROM_ADDR) {

  
    _av_player_plugin_init_struct_start = .;
  	KEEP(*(.avplayer.init))
  	_av_player_plugin_init_struct_end = .;  
    . = ALIGN(4);

    _video_init_struct_start = .;
     KEEP(*(.video.plugin))
    _video_init_struct_end = .; 
    . = ALIGN(4);
		
    _avcodec__start = .;
     KEEP(*(.avcodec.plugin))
    _avcodec__end = .;
    . = ALIGN(4);
    
    _vdec_init_struct_start = .;
    KEEP(*(.vdec.plugin))
    _vdec_init_struct_end = .; 
    . = ALIGN(4);

    _mp4_vdec_init_struct_start = .;
    KEEP(*(.mp4.vdec.plugin))
    _mp4_vdec_init_struct_end = .;
    . = ALIGN(4);

    _adec_init_struct_start = .;
    KEEP(*(.adec.plugin))
    _adec_init_struct_end = .; 
    . = ALIGN(4);

  	_mp_ext_subtitle_start = .;
  	KEEP(*(.extsubt.plugin))
  	_mp_ext_subtitle_end = .; 
	. = ALIGN(4);

	_embed_subtitle_start = .;
  	KEEP(*(.subt.plugin))
  	_embed_subtitle_end = .;
	. = ALIGN(4);

  	_fs_init_struct_start = .;
  	KEEP(*(.fs.init))
  	_fs_init_struct_end = .; 
  	
  	_deca_audio_stream_struct_start = .;
  	KEEP(*(.deca.init))
  	_deca_audio_stream_struct_end = .; 
  	_audio_init_struct_start = .;
  	KEEP(*(.audio.init))
  	_audio_init_struct_end = .;

  	_netctl_struct_start = .;
  	KEEP(*(.netctl.init))
  	_netctl_struct_end = .;
   . = ALIGN(4);

	_es_player_start = .;
	KEEP(*(.es_player.plugin))
	_es_player_end = .;
	. = ALIGN(4);
	_dsc_csa3_struct_start = .;
	KEEP(*(.dsc_csa3.plugin))	
	_dsc_csa3_struct_end = .;
	. = ALIGN(4);
   } >ram
  		
  __SDATA_RAM_ADDR = __INIT_RAM_ADDR + SIZEOF(.structure.init);
  __SDATA_ROM_ADDR = __INIT_ROM_ADDR + SIZEOF(.structure.init);
  .sdata __SDATA_RAM_ADDR : AT(__SDATA_ROM_ADDR) {
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s*)
    . = ALIGN(4);
  } >ram

  _edata = __SDATA_ROM_ADDR + SIZEOF(.sdata);

  __SBSS_RAM_ADDR = __SDATA_RAM_ADDR + SIZEOF(.sdata);
  .sbss __SBSS_RAM_ADDR : AT(__SBSS_RAM_ADDR) {
    *(.sbss)
    *(.scommon)
  } >ram

  __BSS_RAM_ADDR = __SBSS_RAM_ADDR + SIZEOF(.sbss);
  .bss __BSS_RAM_ADDR : AT(__BSS_RAM_ADDR) {
    *(.bss)
    *(COMMON)
  } >ram

  _end = .;

/*
  __FLASH_ROM_ADDR = __BOOT_LOADER ?
        (__ROM_BASE + 464K) : __BOOT_FROM_FLASH ?
        (__SDATA_ROM_ADDR + SIZEOF(.sdata)) :
        (__BSS_RAM_ADDR + SIZEOF(.bss));
  .flash __FLASH_ROM_ADDR : AT(__FLASH_ROM_ADDR) {
    *(.flash)
  }
*/  
  
  .reginfo 0 : { *(.reginfo) }
  .mdebug 0 : { *(.mdebug) }
}
