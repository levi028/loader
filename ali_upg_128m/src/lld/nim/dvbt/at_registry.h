#ifndef ATONT_MODULE_PROTECT
#define ATONT_MODULE_PROTECT


/*** Complete memory map                                                  ***/
#define ATONT_BASE_ADDR (0x0)

/*** AT810 Top Level                                                      ***/
#define DIG_TOP_BASE_ADDR (0x0)

/*** ATONT Digital Core                                                   ***/
#define DIG_CORE_BASE_ADDR (0x0)

/*** 8KB ICCM ROM                                                         ***/
#ifndef ICCM_ROM_MEM_PROTECT
#define ICCM_ROM_MEM_PROTECT

#define ICCM_ROM_MEM_BASE_ADDR (DIG_CORE_BASE_ADDR + 0x0)
#define ICCM_ROM_MEM_SIZE (0x5FFF)
#else /* ICCM_ROM_MEM_PROTECT */
#undef ICCM_ROM_MEM_BASE_ADDR
#endif /* ICCM_ROM_MEM_PROTECT */

/*** 10KB ICCM RAM                                                        ***/
#ifndef ICCM_RAM_MEM_PROTECT
#define ICCM_RAM_MEM_PROTECT

#define ICCM_RAM_MEM_BASE_ADDR (DIG_CORE_BASE_ADDR + 0x10000)
#define ICCM_RAM_MEM_SIZE (0x47FF)
#else /* ICCM_RAM_MEM_PROTECT */
#undef ICCM_RAM_MEM_BASE_ADDR
#endif /* ICCM_RAM_MEM_PROTECT */

/*** ATONT CPU platform                                                   ***/
#define ATONT_CPU_PF_BASE_ADDR (0x800000)

/*** AHB interrupt controller                                             ***/
#define AHB_INT_CTRL_BASE_ADDR (0x810000)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_ENABLE          ***/
/* Interrupt enable register                                                */
#ifndef AHB_INT_CTRL_INT_ENABLE_PROTECT
#define AHB_INT_CTRL_INT_ENABLE_PROTECT

#define AHB_INT_CTRL_INT_ENABLE_OFFS (0x0)

/* INT_ENABLE BIT 15: Interrupt enable for line 15                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_INDEX (15)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_MASK (0x8000)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_15_INDEX))

/* INT_ENABLE BIT 14: Interrupt enable for line 14                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_INDEX (14)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_MASK (0x4000)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_14_INDEX))

/* INT_ENABLE BIT 13: Interrupt enable for line 13                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_INDEX (13)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_MASK (0x2000)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_13_INDEX))

/* INT_ENABLE BIT 12: Interrupt enable for line 12                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_INDEX (12)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_MASK (0x1000)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_12_INDEX))

/* INT_ENABLE BIT 11: Interrupt enable for line 11                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_INDEX (11)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_MASK (0x800)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_11_INDEX))

/* INT_ENABLE BIT 10: Interrupt enable for line 10                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_INDEX (10)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_MASK (0x400)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_10_INDEX))

/* INT_ENABLE BIT 9: Interrupt enable for line 9                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_INDEX (9)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_MASK (0x200)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_9_INDEX))

/* INT_ENABLE BIT 8: Interrupt enable for line 8                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_INDEX (8)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_MASK (0x100)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_8_INDEX))

/* INT_ENABLE BIT 7: Interrupt enable for line 7                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_INDEX (7)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_MASK (0x80)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_7_INDEX))

/* INT_ENABLE BIT 6: Interrupt enable for line 6                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_INDEX (6)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_MASK (0x40)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_6_INDEX))

/* INT_ENABLE BIT 5: Interrupt enable for line 5                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_INDEX (5)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_MASK (0x20)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_5_INDEX))

/* INT_ENABLE BIT 4: Interrupt enable for line 4                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_INDEX (4)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_MASK (0x10)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_4_INDEX))

/* INT_ENABLE BIT 3: Interrupt enable for line 3                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_INDEX (3)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_MASK (0x8)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_3_INDEX))

/* INT_ENABLE BIT 2: Interrupt enable for line 2                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_INDEX (2)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_MASK (0x4)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_2_INDEX))

/* INT_ENABLE BIT 1: Interrupt enable for line 1                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_INDEX (1)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_MASK (0x2)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_1_INDEX))

/* INT_ENABLE BIT 0: Interrupt enable for line 0                            */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_INDEX (0)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_MASK (0x1)
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_MASKED (0x0 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_INDEX))
#define AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_ENABLED (0x1 << (AHB_INT_CTRL_INT_ENABLE_INT_ENABLE_0_INDEX))
#endif /* AHB_INT_CTRL_INT_ENABLE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_ENABLE: Interrupt   *
 * enable register                                                          */
#define AHB_INT_CTRL_INT_ENABLE_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_INT_ENABLE (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_INT_ENABLE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_VAL             ***/
/* Interrupt status register                                                */
#ifndef AHB_INT_CTRL_INT_VAL_PROTECT
#define AHB_INT_CTRL_INT_VAL_PROTECT

#define AHB_INT_CTRL_INT_VAL_OFFS (0x4)

/* INT_VAL BIT 15: Interrupt status for line 15                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_15_INDEX (15)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_15_MASK (0x8000)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_15_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_15_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_15_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_15_INDEX))

/* INT_VAL BIT 14: Interrupt status for line 14                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_14_INDEX (14)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_14_MASK (0x4000)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_14_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_14_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_14_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_14_INDEX))

/* INT_VAL BIT 13: Interrupt status for line 13                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_13_INDEX (13)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_13_MASK (0x2000)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_13_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_13_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_13_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_13_INDEX))

/* INT_VAL BIT 12: Interrupt status for line 12                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_12_INDEX (12)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_12_MASK (0x1000)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_12_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_12_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_12_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_12_INDEX))

/* INT_VAL BIT 11: Interrupt status for line 11                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_11_INDEX (11)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_11_MASK (0x800)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_11_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_11_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_11_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_11_INDEX))

/* INT_VAL BIT 10: Interrupt status for line 10                             */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_10_INDEX (10)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_10_MASK (0x400)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_10_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_10_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_10_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_10_INDEX))

/* INT_VAL BIT 9: Interrupt status for line 9                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_9_INDEX (9)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_9_MASK (0x200)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_9_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_9_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_9_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_9_INDEX))

/* INT_VAL BIT 8: Interrupt status for line 8                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_8_INDEX (8)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_8_MASK (0x100)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_8_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_8_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_8_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_8_INDEX))

/* INT_VAL BIT 7: Interrupt status for line 7                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_7_INDEX (7)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_7_MASK (0x80)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_7_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_7_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_7_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_7_INDEX))

/* INT_VAL BIT 6: Interrupt status for line 6                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_6_INDEX (6)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_6_MASK (0x40)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_6_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_6_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_6_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_6_INDEX))

/* INT_VAL BIT 5: Interrupt status for line 5                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_5_INDEX (5)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_5_MASK (0x20)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_5_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_5_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_5_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_5_INDEX))

/* INT_VAL BIT 4: Interrupt status for line 4                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_4_INDEX (4)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_4_MASK (0x10)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_4_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_4_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_4_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_4_INDEX))

/* INT_VAL BIT 3: Interrupt status for line 3                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_3_INDEX (3)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_3_MASK (0x8)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_3_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_3_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_3_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_3_INDEX))

/* INT_VAL BIT 2: Interrupt status for line 2                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_2_INDEX (2)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_2_MASK (0x4)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_2_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_2_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_2_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_2_INDEX))

/* INT_VAL BIT 1: Interrupt status for line 1                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_1_INDEX (1)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_1_MASK (0x2)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_1_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_1_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_1_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_1_INDEX))

/* INT_VAL BIT 0: Interrupt status for line 0                               */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_VAL_INT_VAL_0_INDEX (0)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_0_MASK (0x1)
#define AHB_INT_CTRL_INT_VAL_INT_VAL_0_CLEAR (0x0 << (AHB_INT_CTRL_INT_VAL_INT_VAL_0_INDEX))
#define AHB_INT_CTRL_INT_VAL_INT_VAL_0_PENDING (0x1 << (AHB_INT_CTRL_INT_VAL_INT_VAL_0_INDEX))
#endif /* AHB_INT_CTRL_INT_VAL_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_VAL: Interrupt      *
 * status register                                                          */
#define AHB_INT_CTRL_INT_VAL_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_INT_VAL (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_INT_VAL_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/SRC_MODE            ***/
/* Source mode register                                                     */
#ifndef AHB_INT_CTRL_SRC_MODE_PROTECT
#define AHB_INT_CTRL_SRC_MODE_PROTECT

#define AHB_INT_CTRL_SRC_MODE_OFFS (0x8)

/* SRC_MODE BIT 15: Source mode for event 15                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_INDEX (15)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_MASK (0x8000)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_15_INDEX))

/* SRC_MODE BIT 14: Source mode for event 14                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_INDEX (14)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_MASK (0x4000)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_14_INDEX))

/* SRC_MODE BIT 13: Source mode for event 13                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_INDEX (13)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_MASK (0x2000)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_13_INDEX))

/* SRC_MODE BIT 12: Source mode for event 12                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_INDEX (12)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_MASK (0x1000)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_12_INDEX))

/* SRC_MODE BIT 11: Source mode for event 11                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_INDEX (11)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_MASK (0x800)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_11_INDEX))

/* SRC_MODE BIT 10: Source mode for event 10                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_INDEX (10)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_MASK (0x400)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_10_INDEX))

/* SRC_MODE BIT 9: Source mode for event 9                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_INDEX (9)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_MASK (0x200)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_9_INDEX))

/* SRC_MODE BIT 8: Source mode for event 8                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_INDEX (8)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_MASK (0x100)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_8_INDEX))

/* SRC_MODE BIT 7: Source mode for event 7                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_INDEX (7)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_MASK (0x80)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_7_INDEX))

/* SRC_MODE BIT 6: Source mode for event 6                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_INDEX (6)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_MASK (0x40)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_6_INDEX))

/* SRC_MODE BIT 5: Source mode for event 5                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_INDEX (5)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_MASK (0x20)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_5_INDEX))

/* SRC_MODE BIT 4: Source mode for event 4                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_INDEX (4)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_MASK (0x10)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_4_INDEX))

/* SRC_MODE BIT 3: Source mode for event 3                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_INDEX (3)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_MASK (0x8)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_3_INDEX))

/* SRC_MODE BIT 2: Source mode for event 2                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_INDEX (2)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_MASK (0x4)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_2_INDEX))

/* SRC_MODE BIT 1: Source mode for event 1                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_INDEX (1)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_MASK (0x2)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_1_INDEX))

/* SRC_MODE BIT 0: Source mode for event 0                                  */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_INDEX (0)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_MASK (0x1)
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_LEVEL (0x0 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_INDEX))
#define AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_PULSE (0x1 << (AHB_INT_CTRL_SRC_MODE_SRC_MODE_0_INDEX))
#endif /* AHB_INT_CTRL_SRC_MODE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/SRC_MODE: Source mode register */
#define AHB_INT_CTRL_SRC_MODE_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_SRC_MODE (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_SRC_MODE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/SRC_POLARITY        ***/
/* Source polarity register                                                 */
#ifndef AHB_INT_CTRL_SRC_POLARITY_PROTECT
#define AHB_INT_CTRL_SRC_POLARITY_PROTECT

#define AHB_INT_CTRL_SRC_POLARITY_OFFS (0xC)

/* SRC_POLARITY BIT 15: Source polarity for event 15                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_INDEX (15)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_MASK (0x8000)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_15_INDEX))

/* SRC_POLARITY BIT 14: Source polarity for event 14                        */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_INDEX (14)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_MASK (0x4000)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_14_INDEX))

/* SRC_POLARITY BIT 13: Source polarity for event 13                        */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_INDEX (13)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_MASK (0x2000)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_13_INDEX))

/* SRC_POLARITY BIT 12: Source polarity for event 12                        */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_INDEX (12)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_MASK (0x1000)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_12_INDEX))

/* SRC_POLARITY BIT 11: Source polarity for event 11                        */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_INDEX (11)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_MASK (0x800)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_11_INDEX))

/* SRC_POLARITY BIT 10: Source polarity for event 10                        */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_INDEX (10)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_MASK (0x400)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_10_INDEX))

/* SRC_POLARITY BIT 9: Source polarity for event 9                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_INDEX (9)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_MASK (0x200)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_9_INDEX))

/* SRC_POLARITY BIT 8: Source polarity for event 8                          */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_INDEX (8)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_MASK (0x100)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_8_INDEX))

/* SRC_POLARITY BIT 7: Source polarity for event 7                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_INDEX (7)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_MASK (0x80)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_7_INDEX))

/* SRC_POLARITY BIT 6: Source polarity for event 6                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_INDEX (6)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_MASK (0x40)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_6_INDEX))

/* SRC_POLARITY BIT 5: Source polarity for event 5                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_INDEX (5)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_MASK (0x20)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_5_INDEX))

/* SRC_POLARITY BIT 4: Source polarity for event 4                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_INDEX (4)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_MASK (0x10)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_4_INDEX))

/* SRC_POLARITY BIT 3: Source polarity for event 3                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_INDEX (3)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_MASK (0x8)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_3_INDEX))

/* SRC_POLARITY BIT 2: Source polarity for event 2                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_INDEX (2)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_MASK (0x4)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_2_INDEX))

/* SRC_POLARITY BIT 1: Source polarity for event 1                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_INDEX (1)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_MASK (0x2)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_1_INDEX))

/* SRC_POLARITY BIT 0: Source polarity for event 0                          */
/* symbolic RW field, reset to 0x1                                          */
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_INDEX (0)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_MASK (0x1)
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_NEG (0x0 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_INDEX))
#define AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_POS (0x1 << (AHB_INT_CTRL_SRC_POLARITY_SRC_POLARITY_0_INDEX))
#endif /* AHB_INT_CTRL_SRC_POLARITY_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/SRC_POLARITY: Source    *
 * polarity register                                                        */
#define AHB_INT_CTRL_SRC_POLARITY_RESET_VALUE (0x7eff)
#define AHB_INT_CTRL_SRC_POLARITY (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_SRC_POLARITY_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_MODE            ***/
/* Interrupt mode register                                                  */
#ifndef AHB_INT_CTRL_INT_MODE_PROTECT
#define AHB_INT_CTRL_INT_MODE_PROTECT

#define AHB_INT_CTRL_INT_MODE_OFFS (0x10)

/* INT_MODE BIT 15: Interrupt mode for line 15                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_15_INDEX (15)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_15_MASK (0x8000)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_15_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_15_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_15_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_15_INDEX))

/* INT_MODE BIT 14: Interrupt mode for line 14                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_14_INDEX (14)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_14_MASK (0x4000)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_14_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_14_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_14_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_14_INDEX))

/* INT_MODE BIT 13: Interrupt mode for line 13                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_13_INDEX (13)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_13_MASK (0x2000)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_13_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_13_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_13_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_13_INDEX))

/* INT_MODE BIT 12: Interrupt mode for line 12                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_12_INDEX (12)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_12_MASK (0x1000)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_12_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_12_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_12_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_12_INDEX))

/* INT_MODE BIT 11: Interrupt mode for line 11                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_11_INDEX (11)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_11_MASK (0x800)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_11_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_11_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_11_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_11_INDEX))

/* INT_MODE BIT 10: Interrupt mode for line 10                              */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_10_INDEX (10)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_10_MASK (0x400)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_10_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_10_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_10_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_10_INDEX))

/* INT_MODE BIT 9: Interrupt mode for line 9                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_9_INDEX (9)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_9_MASK (0x200)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_9_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_9_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_9_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_9_INDEX))

/* INT_MODE BIT 8: Interrupt mode for line 8                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_8_INDEX (8)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_8_MASK (0x100)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_8_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_8_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_8_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_8_INDEX))

/* INT_MODE BIT 7: Interrupt mode for line 7                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_7_INDEX (7)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_7_MASK (0x80)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_7_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_7_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_7_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_7_INDEX))

/* INT_MODE BIT 6: Interrupt mode for line 6                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_6_INDEX (6)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_6_MASK (0x40)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_6_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_6_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_6_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_6_INDEX))

/* INT_MODE BIT 5: Interrupt mode for line 5                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_5_INDEX (5)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_5_MASK (0x20)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_5_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_5_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_5_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_5_INDEX))

/* INT_MODE BIT 4: Interrupt mode for line 4                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_4_INDEX (4)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_4_MASK (0x10)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_4_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_4_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_4_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_4_INDEX))

/* INT_MODE BIT 3: Interrupt mode for line 3                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_3_INDEX (3)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_3_MASK (0x8)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_3_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_3_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_3_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_3_INDEX))

/* INT_MODE BIT 2: Interrupt mode for line 2                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_2_INDEX (2)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_2_MASK (0x4)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_2_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_2_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_2_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_2_INDEX))

/* INT_MODE BIT 1: Interrupt mode for line 1                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_1_INDEX (1)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_1_MASK (0x2)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_1_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_1_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_1_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_1_INDEX))

/* INT_MODE BIT 0: Interrupt mode for line 0                                */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_MODE_INT_MODE_0_INDEX (0)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_0_MASK (0x1)
#define AHB_INT_CTRL_INT_MODE_INT_MODE_0_LEVEL (0x0 << (AHB_INT_CTRL_INT_MODE_INT_MODE_0_INDEX))
#define AHB_INT_CTRL_INT_MODE_INT_MODE_0_PULSE (0x1 << (AHB_INT_CTRL_INT_MODE_INT_MODE_0_INDEX))
#endif /* AHB_INT_CTRL_INT_MODE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_MODE: Interrupt     *
 * mode register                                                            */
#define AHB_INT_CTRL_INT_MODE_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_INT_MODE (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_INT_MODE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_POLARITY        ***/
/* Interrupt polarity register                                              */
#ifndef AHB_INT_CTRL_INT_POLARITY_PROTECT
#define AHB_INT_CTRL_INT_POLARITY_PROTECT

#define AHB_INT_CTRL_INT_POLARITY_OFFS (0x14)

/* INT_POLARITY BIT 15: Interrupt polarity for line 15                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_INDEX (15)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_MASK (0x8000)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_15_INDEX))

/* INT_POLARITY BIT 14: Interrupt polarity for line 14                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_INDEX (14)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_MASK (0x4000)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_14_INDEX))

/* INT_POLARITY BIT 13: Interrupt polarity for line 13                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_INDEX (13)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_MASK (0x2000)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_13_INDEX))

/* INT_POLARITY BIT 12: Interrupt polarity for line 12                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_INDEX (12)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_MASK (0x1000)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_12_INDEX))

/* INT_POLARITY BIT 11: Interrupt polarity for line 11                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_INDEX (11)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_MASK (0x800)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_11_INDEX))

/* INT_POLARITY BIT 10: Interrupt polarity for line 10                      */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_INDEX (10)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_MASK (0x400)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_10_INDEX))

/* INT_POLARITY BIT 9: Interrupt polarity for line 9                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_INDEX (9)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_MASK (0x200)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_9_INDEX))

/* INT_POLARITY BIT 8: Interrupt polarity for line 8                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_INDEX (8)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_MASK (0x100)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_8_INDEX))

/* INT_POLARITY BIT 7: Interrupt polarity for line 7                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_INDEX (7)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_MASK (0x80)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_7_INDEX))

/* INT_POLARITY BIT 6: Interrupt polarity for line 6                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_INDEX (6)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_MASK (0x40)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_6_INDEX))

/* INT_POLARITY BIT 5: Interrupt polarity for line 5                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_INDEX (5)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_MASK (0x20)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_5_INDEX))

/* INT_POLARITY BIT 4: Interrupt polarity for line 4                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_INDEX (4)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_MASK (0x10)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_4_INDEX))

/* INT_POLARITY BIT 3: Interrupt polarity for line 3                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_INDEX (3)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_MASK (0x8)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_3_INDEX))

/* INT_POLARITY BIT 2: Interrupt polarity for line 2                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_INDEX (2)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_MASK (0x4)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_2_INDEX))

/* INT_POLARITY BIT 1: Interrupt polarity for line 1                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_INDEX (1)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_MASK (0x2)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_1_INDEX))

/* INT_POLARITY BIT 0: Interrupt polarity for line 0                        */
/* symbolic RW field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_INDEX (0)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_MASK (0x1)
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_NEG (0x0 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_INDEX))
#define AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_POS (0x1 << (AHB_INT_CTRL_INT_POLARITY_INT_POLARITY_0_INDEX))
#endif /* AHB_INT_CTRL_INT_POLARITY_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_POLARITY:           *
 * Interrupt polarity register                                              */
#define AHB_INT_CTRL_INT_POLARITY_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_INT_POLARITY (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_INT_POLARITY_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_FORCE           ***/
/* Interrupt force (software interrupt) register                            */
#ifndef AHB_INT_CTRL_INT_FORCE_PROTECT
#define AHB_INT_CTRL_INT_FORCE_PROTECT

#define AHB_INT_CTRL_INT_FORCE_OFFS (0x18)

/* INT_FORCE BIT 15: Interrupt force for line 15                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_INDEX (15)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_MASK (0x8000)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_15_INDEX))

/* INT_FORCE BIT 14: Interrupt force for line 14                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_INDEX (14)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_MASK (0x4000)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_14_INDEX))

/* INT_FORCE BIT 13: Interrupt force for line 13                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_INDEX (13)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_MASK (0x2000)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_13_INDEX))

/* INT_FORCE BIT 12: Interrupt force for line 12                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_INDEX (12)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_MASK (0x1000)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_12_INDEX))

/* INT_FORCE BIT 11: Interrupt force for line 11                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_INDEX (11)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_MASK (0x800)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_11_INDEX))

/* INT_FORCE BIT 10: Interrupt force for line 10                            */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_INDEX (10)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_MASK (0x400)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_10_INDEX))

/* INT_FORCE BIT 9: Interrupt force for line 9                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_INDEX (9)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_MASK (0x200)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_9_INDEX))

/* INT_FORCE BIT 8: Interrupt force for line 8                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_INDEX (8)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_MASK (0x100)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_8_INDEX))

/* INT_FORCE BIT 7: Interrupt force for line 7                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_INDEX (7)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_MASK (0x80)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_7_INDEX))

/* INT_FORCE BIT 6: Interrupt force for line 6                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_INDEX (6)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_MASK (0x40)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_6_INDEX))

/* INT_FORCE BIT 5: Interrupt force for line 5                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_INDEX (5)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_MASK (0x20)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_5_INDEX))

/* INT_FORCE BIT 4: Interrupt force for line 4                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_INDEX (4)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_MASK (0x10)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_4_INDEX))

/* INT_FORCE BIT 3: Interrupt force for line 3                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_INDEX (3)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_MASK (0x8)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_3_INDEX))

/* INT_FORCE BIT 2: Interrupt force for line 2                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_INDEX (2)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_MASK (0x4)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_2_INDEX))

/* INT_FORCE BIT 1: Interrupt force for line 1                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_INDEX (1)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_MASK (0x2)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_1_INDEX))

/* INT_FORCE BIT 0: Interrupt force for line 0                              */
/* symbolic WO field, reset to 0x0                                          */
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_INDEX (0)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_MASK (0x1)
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_CLEAR (0x0 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_INDEX))
#define AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_FORCE (0x1 << (AHB_INT_CTRL_INT_FORCE_INT_FORCE_0_INDEX))
#endif /* AHB_INT_CTRL_INT_FORCE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/AHB_INT_CTRL/INT_FORCE: Interrupt    *
 * force (software interrupt) register                                      */
#define AHB_INT_CTRL_INT_FORCE_RESET_VALUE (0x0000)
#define AHB_INT_CTRL_INT_FORCE (AHB_INT_CTRL_BASE_ADDR + AHB_INT_CTRL_INT_FORCE_OFFS)

#define AHB_INT_CTRL_SIZE (0x19)

/*** I2C-slave module with AHB-master interface                           ***/
#define I2C_BASE_ADDR (0x830000)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/CTRL                         ***/

#ifndef I2C_CTRL_PROTECT
#define I2C_CTRL_PROTECT

#define I2C_CTRL_OFFS (0x0)

/* CTRL BIT 7: DMA mode disabled                                            */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_CTRL_DMAMODDIS_INDEX (7)
#define I2C_CTRL_DMAMODDIS_MASK (0x80)
#define I2C_CTRL_DMAMODDIS_CLEAR (0x0 << (I2C_CTRL_DMAMODDIS_INDEX))
#define I2C_CTRL_DMAMODDIS_SET (0x1 << (I2C_CTRL_DMAMODDIS_INDEX))

/* CTRL BIT 6: Buffer mode disabled                                         */
/* symbolic RW field, reset to 0x1                                          */
#define I2C_CTRL_BUFMODDIS_INDEX (6)
#define I2C_CTRL_BUFMODDIS_MASK (0x40)
#define I2C_CTRL_BUFMODDIS_CLEAR (0x0 << (I2C_CTRL_BUFMODDIS_INDEX))
#define I2C_CTRL_BUFMODDIS_SET (0x1 << (I2C_CTRL_BUFMODDIS_INDEX))

/* CTRL BIT 5: Force NACK                                                   */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_CTRL_FORCENACK_INDEX (5)
#define I2C_CTRL_FORCENACK_MASK (0x20)
#define I2C_CTRL_FORCENACK_DISABLE (0x0 << (I2C_CTRL_FORCENACK_INDEX))
#define I2C_CTRL_FORCENACK_ENABLE (0x1 << (I2C_CTRL_FORCENACK_INDEX))

/* CTRL BIT 4: I2C slave address mode                                       */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_CTRL_I2C10BITSLVADD_INDEX (4)
#define I2C_CTRL_I2C10BITSLVADD_MASK (0x10)
#define I2C_CTRL_I2C10BITSLVADD_MODE7 (0x0 << (I2C_CTRL_I2C10BITSLVADD_INDEX))
#define I2C_CTRL_I2C10BITSLVADD_MODE10 (0x1 << (I2C_CTRL_I2C10BITSLVADD_INDEX))

/* CTRL BITS 3..2: AHB transfer size                                        */
/* numeric RO unsigned field, reset to 0x2                                  */
#define I2C_CTRL_HSIZE_INDEX (2)
#define I2C_CTRL_HSIZE_MASK (0xC)

/* CTRL BIT 1: Module Reset                                                 */
/* symbolic WO field, reset to 0x0                                          */
#define I2C_CTRL_RESET_INDEX (1)
#define I2C_CTRL_RESET_MASK (0x2)
#define I2C_CTRL_RESET_CLEAR (0x0 << (I2C_CTRL_RESET_INDEX))
#define I2C_CTRL_RESET_FORCE (0x1 << (I2C_CTRL_RESET_INDEX))

/* CTRL BIT 0: Module Enable                                                */
/* symbolic RW field, reset to 0x1                                          */
#define I2C_CTRL_ENABLE_INDEX (0)
#define I2C_CTRL_ENABLE_MASK (0x1)
#define I2C_CTRL_ENABLE_CLEAR (0x0 << (I2C_CTRL_ENABLE_INDEX))
#define I2C_CTRL_ENABLE_ACTIVE (0x1 << (I2C_CTRL_ENABLE_INDEX))
#endif /* I2C_CTRL_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/CTRL:                            */
#define I2C_CTRL_RESET_VALUE (0x49)
#define I2C_CTRL (I2C_BASE_ADDR + I2C_CTRL_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/SLV_ADD                      ***/

#ifndef I2C_SLV_ADD_PROTECT
#define I2C_SLV_ADD_PROTECT

#define I2C_SLV_ADD_OFFS (0x4)

/* SLV_ADD BITS 9..0: I2C slave address                                     */
/* numeric RW unsigned field, reset to 0x50                                 */
#define I2C_SLV_ADD_SLVADD_INDEX (0)
#define I2C_SLV_ADD_SLVADD_MASK (0x3FF)
#endif /* I2C_SLV_ADD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/SLV_ADD:                         */
#define I2C_SLV_ADD_RESET_VALUE (0x050)
#define I2C_SLV_ADD (I2C_BASE_ADDR + I2C_SLV_ADD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/DATA                         ***/

#ifndef I2C_DATA_PROTECT
#define I2C_DATA_PROTECT

#define I2C_DATA_OFFS (0x8)

/* DATA BITS 31..0: Last transfered data                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define I2C_DATA_DATA_INDEX (0)
#define I2C_DATA_DATA_MASK (0xFFFFFFFF)
#endif /* I2C_DATA_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/DATA:                            */
#define I2C_DATA_RESET_VALUE (0x00000000)
#define I2C_DATA (I2C_BASE_ADDR + I2C_DATA_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/STATUS                       ***/

#ifndef I2C_STATUS_PROTECT
#define I2C_STATUS_PROTECT

#define I2C_STATUS_OFFS (0xC)

/* STATUS BIT 7: TX FIFO full indicator                                     */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_STATUS_TXFF_INDEX (7)
#define I2C_STATUS_TXFF_MASK (0x80)
#define I2C_STATUS_TXFF_NOTFULL (0x0 << (I2C_STATUS_TXFF_INDEX))
#define I2C_STATUS_TXFF_FULL (0x1 << (I2C_STATUS_TXFF_INDEX))

/* STATUS BIT 6: TX FIFO empty indicator                                    */
/* symbolic TO1 field, reset to 0x1                                         */
#define I2C_STATUS_TXFE_INDEX (6)
#define I2C_STATUS_TXFE_MASK (0x40)
#define I2C_STATUS_TXFE_NOTEMPTY (0x0 << (I2C_STATUS_TXFE_INDEX))
#define I2C_STATUS_TXFE_EMPTY (0x1 << (I2C_STATUS_TXFE_INDEX))

/* STATUS BIT 5: RX FIFO full indicator                                     */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_STATUS_RXFF_INDEX (5)
#define I2C_STATUS_RXFF_MASK (0x20)
#define I2C_STATUS_RXFF_NOTFULL (0x0 << (I2C_STATUS_RXFF_INDEX))
#define I2C_STATUS_RXFF_FULL (0x1 << (I2C_STATUS_RXFF_INDEX))

/* STATUS BIT 4: RX FIFO not empty indicator                                */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_STATUS_RXFNE_INDEX (4)
#define I2C_STATUS_RXFNE_MASK (0x10)
#define I2C_STATUS_RXFNE_EMPTY (0x0 << (I2C_STATUS_RXFNE_INDEX))
#define I2C_STATUS_RXFNE_NOTEMPTY (0x1 << (I2C_STATUS_RXFNE_INDEX))

/* STATUS BIT 3: Read address valid                                         */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_STATUS_RDADDVALID_INDEX (3)
#define I2C_STATUS_RDADDVALID_MASK (0x8)
#define I2C_STATUS_RDADDVALID_INVALID (0x0 << (I2C_STATUS_RDADDVALID_INDEX))
#define I2C_STATUS_RDADDVALID_VALID (0x1 << (I2C_STATUS_RDADDVALID_INDEX))

/* STATUS BIT 2: Module Reset                                               */
/* symbolic RO field, reset to 0x0                                          */
#define I2C_STATUS_BUFACTIVITY_INDEX (2)
#define I2C_STATUS_BUFACTIVITY_MASK (0x4)
#define I2C_STATUS_BUFACTIVITY_INACTIVE (0x0 << (I2C_STATUS_BUFACTIVITY_INDEX))
#define I2C_STATUS_BUFACTIVITY_ACTIVE (0x1 << (I2C_STATUS_BUFACTIVITY_INDEX))

/* STATUS BIT 1: Module Reset                                               */
/* symbolic RO field, reset to 0x0                                          */
#define I2C_STATUS_TXACTIVITY_INDEX (1)
#define I2C_STATUS_TXACTIVITY_MASK (0x2)
#define I2C_STATUS_TXACTIVITY_INACTIVE (0x0 << (I2C_STATUS_TXACTIVITY_INDEX))
#define I2C_STATUS_TXACTIVITY_ACTIVE (0x1 << (I2C_STATUS_TXACTIVITY_INDEX))

/* STATUS BIT 0: Module Reset                                               */
/* symbolic RO field, reset to 0x0                                          */
#define I2C_STATUS_RXACTIVITY_INDEX (0)
#define I2C_STATUS_RXACTIVITY_MASK (0x1)
#define I2C_STATUS_RXACTIVITY_INACTIVE (0x0 << (I2C_STATUS_RXACTIVITY_INDEX))
#define I2C_STATUS_RXACTIVITY_ACTIVE (0x1 << (I2C_STATUS_RXACTIVITY_INDEX))
#endif /* I2C_STATUS_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/STATUS:                          */
#define I2C_STATUS_RESET_VALUE (0x40)
#define I2C_STATUS (I2C_BASE_ADDR + I2C_STATUS_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/INTR_STAT                    ***/

#ifndef I2C_INTR_STAT_PROTECT
#define I2C_INTR_STAT_PROTECT

#define I2C_INTR_STAT_OFFS (0x10)

/* INTR_STAT BIT 10: Transfer error interrupt flag                          */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_XFRERR_INDEX (10)
#define I2C_INTR_STAT_XFRERR_MASK (0x400)
#define I2C_INTR_STAT_XFRERR_IDLE (0x0 << (I2C_INTR_STAT_XFRERR_INDEX))
#define I2C_INTR_STAT_XFRERR_PENDING (0x1 << (I2C_INTR_STAT_XFRERR_INDEX))

/* INTR_STAT BIT 9: Buffer-mode RX overflow interrupt flag                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_RXBUFOVF_INDEX (9)
#define I2C_INTR_STAT_RXBUFOVF_MASK (0x200)
#define I2C_INTR_STAT_RXBUFOVF_IDLE (0x0 << (I2C_INTR_STAT_RXBUFOVF_INDEX))
#define I2C_INTR_STAT_RXBUFOVF_PENDING (0x1 << (I2C_INTR_STAT_RXBUFOVF_INDEX))

/* INTR_STAT BIT 8: Buffer-mode RX done interrupt flag                      */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_RXBUFDONE_INDEX (8)
#define I2C_INTR_STAT_RXBUFDONE_MASK (0x100)
#define I2C_INTR_STAT_RXBUFDONE_IDLE (0x0 << (I2C_INTR_STAT_RXBUFDONE_INDEX))
#define I2C_INTR_STAT_RXBUFDONE_PENDING (0x1 << (I2C_INTR_STAT_RXBUFDONE_INDEX))

/* INTR_STAT BIT 7: TX abort interrupt flag                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_TXABRT_INDEX (7)
#define I2C_INTR_STAT_TXABRT_MASK (0x80)
#define I2C_INTR_STAT_TXABRT_IDLE (0x0 << (I2C_INTR_STAT_TXABRT_INDEX))
#define I2C_INTR_STAT_TXABRT_PENDING (0x1 << (I2C_INTR_STAT_TXABRT_INDEX))

/* INTR_STAT BIT 6: RX abort interrupt flag                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_RXABRT_INDEX (6)
#define I2C_INTR_STAT_RXABRT_MASK (0x40)
#define I2C_INTR_STAT_RXABRT_IDLE (0x0 << (I2C_INTR_STAT_RXABRT_INDEX))
#define I2C_INTR_STAT_RXABRT_PENDING (0x1 << (I2C_INTR_STAT_RXABRT_INDEX))

/* INTR_STAT BIT 5: TX done interrupt flag                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_TXDONE_INDEX (5)
#define I2C_INTR_STAT_TXDONE_MASK (0x20)
#define I2C_INTR_STAT_TXDONE_IDLE (0x0 << (I2C_INTR_STAT_TXDONE_INDEX))
#define I2C_INTR_STAT_TXDONE_PENDING (0x1 << (I2C_INTR_STAT_TXDONE_INDEX))

/* INTR_STAT BIT 4: RX done interrupt flag                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_RXDONE_INDEX (4)
#define I2C_INTR_STAT_RXDONE_MASK (0x10)
#define I2C_INTR_STAT_RXDONE_IDLE (0x0 << (I2C_INTR_STAT_RXDONE_INDEX))
#define I2C_INTR_STAT_RXDONE_PENDING (0x1 << (I2C_INTR_STAT_RXDONE_INDEX))

/* INTR_STAT BIT 3: Read request interrupt flag                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_RDREQ_INDEX (3)
#define I2C_INTR_STAT_RDREQ_MASK (0x8)
#define I2C_INTR_STAT_RDREQ_IDLE (0x0 << (I2C_INTR_STAT_RDREQ_INDEX))
#define I2C_INTR_STAT_RDREQ_PENDING (0x1 << (I2C_INTR_STAT_RDREQ_INDEX))

/* INTR_STAT BIT 2: Write request interrupt flag                            */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_WDREQ_INDEX (2)
#define I2C_INTR_STAT_WDREQ_MASK (0x4)
#define I2C_INTR_STAT_WDREQ_IDLE (0x0 << (I2C_INTR_STAT_WDREQ_INDEX))
#define I2C_INTR_STAT_WDREQ_PENDING (0x1 << (I2C_INTR_STAT_WDREQ_INDEX))

/* INTR_STAT BIT 1: STOP detection interrupt flag                           */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_STOPDET_INDEX (1)
#define I2C_INTR_STAT_STOPDET_MASK (0x2)
#define I2C_INTR_STAT_STOPDET_IDLE (0x0 << (I2C_INTR_STAT_STOPDET_INDEX))
#define I2C_INTR_STAT_STOPDET_PENDING (0x1 << (I2C_INTR_STAT_STOPDET_INDEX))

/* INTR_STAT BIT 0: START detection interrupt flag                          */
/* symbolic TO1 field, reset to 0x0                                         */
#define I2C_INTR_STAT_STARTDET_INDEX (0)
#define I2C_INTR_STAT_STARTDET_MASK (0x1)
#define I2C_INTR_STAT_STARTDET_IDLE (0x0 << (I2C_INTR_STAT_STARTDET_INDEX))
#define I2C_INTR_STAT_STARTDET_PENDING (0x1 << (I2C_INTR_STAT_STARTDET_INDEX))
#endif /* I2C_INTR_STAT_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/INTR_STAT:                       */
#define I2C_INTR_STAT_RESET_VALUE (0x000)
#define I2C_INTR_STAT (I2C_BASE_ADDR + I2C_INTR_STAT_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/INTR_ENABLE                  ***/

#ifndef I2C_INTR_ENABLE_PROTECT
#define I2C_INTR_ENABLE_PROTECT

#define I2C_INTR_ENABLE_OFFS (0x14)

/* INTR_ENABLE BIT 10: Transfer error interrupt enable                      */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_XFRERREN_INDEX (10)
#define I2C_INTR_ENABLE_XFRERREN_MASK (0x400)
#define I2C_INTR_ENABLE_XFRERREN_CLEAR (0x0 << (I2C_INTR_ENABLE_XFRERREN_INDEX))
#define I2C_INTR_ENABLE_XFRERREN_SET (0x1 << (I2C_INTR_ENABLE_XFRERREN_INDEX))

/* INTR_ENABLE BIT 9: Buffer-mode RX overflow interrupt enable              */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_RXBUFOVFEN_INDEX (9)
#define I2C_INTR_ENABLE_RXBUFOVFEN_MASK (0x200)
#define I2C_INTR_ENABLE_RXBUFOVFEN_CLEAR (0x0 << (I2C_INTR_ENABLE_RXBUFOVFEN_INDEX))
#define I2C_INTR_ENABLE_RXBUFOVFEN_SET (0x1 << (I2C_INTR_ENABLE_RXBUFOVFEN_INDEX))

/* INTR_ENABLE BIT 8: Buffer-mode RX done interrupt enable                  */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_RXBUFDONEEN_INDEX (8)
#define I2C_INTR_ENABLE_RXBUFDONEEN_MASK (0x100)
#define I2C_INTR_ENABLE_RXBUFDONEEN_CLEAR (0x0 << (I2C_INTR_ENABLE_RXBUFDONEEN_INDEX))
#define I2C_INTR_ENABLE_RXBUFDONEEN_SET (0x1 << (I2C_INTR_ENABLE_RXBUFDONEEN_INDEX))

/* INTR_ENABLE BIT 7: TX abort interrupt enable                             */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_TXABRTEN_INDEX (7)
#define I2C_INTR_ENABLE_TXABRTEN_MASK (0x80)
#define I2C_INTR_ENABLE_TXABRTEN_CLEAR (0x0 << (I2C_INTR_ENABLE_TXABRTEN_INDEX))
#define I2C_INTR_ENABLE_TXABRTEN_SET (0x1 << (I2C_INTR_ENABLE_TXABRTEN_INDEX))

/* INTR_ENABLE BIT 6: RX abort interrupt enable                             */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_RXABRTEN_INDEX (6)
#define I2C_INTR_ENABLE_RXABRTEN_MASK (0x40)
#define I2C_INTR_ENABLE_RXABRTEN_CLEAR (0x0 << (I2C_INTR_ENABLE_RXABRTEN_INDEX))
#define I2C_INTR_ENABLE_RXABRTEN_SET (0x1 << (I2C_INTR_ENABLE_RXABRTEN_INDEX))

/* INTR_ENABLE BIT 5: TX done interrupt enable                              */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_TXDONEEN_INDEX (5)
#define I2C_INTR_ENABLE_TXDONEEN_MASK (0x20)
#define I2C_INTR_ENABLE_TXDONEEN_CLEAR (0x0 << (I2C_INTR_ENABLE_TXDONEEN_INDEX))
#define I2C_INTR_ENABLE_TXDONEEN_SET (0x1 << (I2C_INTR_ENABLE_TXDONEEN_INDEX))

/* INTR_ENABLE BIT 4: RX done interrupt enable                              */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_RXDONEEN_INDEX (4)
#define I2C_INTR_ENABLE_RXDONEEN_MASK (0x10)
#define I2C_INTR_ENABLE_RXDONEEN_CLEAR (0x0 << (I2C_INTR_ENABLE_RXDONEEN_INDEX))
#define I2C_INTR_ENABLE_RXDONEEN_SET (0x1 << (I2C_INTR_ENABLE_RXDONEEN_INDEX))

/* INTR_ENABLE BIT 3: Read request interrupt enable                         */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_RDREQEN_INDEX (3)
#define I2C_INTR_ENABLE_RDREQEN_MASK (0x8)
#define I2C_INTR_ENABLE_RDREQEN_CLEAR (0x0 << (I2C_INTR_ENABLE_RDREQEN_INDEX))
#define I2C_INTR_ENABLE_RDREQEN_SET (0x1 << (I2C_INTR_ENABLE_RDREQEN_INDEX))

/* INTR_ENABLE BIT 2: Write request interrupt enable                        */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_WDREQEN_INDEX (2)
#define I2C_INTR_ENABLE_WDREQEN_MASK (0x4)
#define I2C_INTR_ENABLE_WDREQEN_CLEAR (0x0 << (I2C_INTR_ENABLE_WDREQEN_INDEX))
#define I2C_INTR_ENABLE_WDREQEN_SET (0x1 << (I2C_INTR_ENABLE_WDREQEN_INDEX))

/* INTR_ENABLE BIT 1: STOP detection interrupt enable                       */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_STOPDETEN_INDEX (1)
#define I2C_INTR_ENABLE_STOPDETEN_MASK (0x2)
#define I2C_INTR_ENABLE_STOPDETEN_CLEAR (0x0 << (I2C_INTR_ENABLE_STOPDETEN_INDEX))
#define I2C_INTR_ENABLE_STOPDETEN_SET (0x1 << (I2C_INTR_ENABLE_STOPDETEN_INDEX))

/* INTR_ENABLE BIT 0: START detection interrupt enable                      */
/* symbolic RW field, reset to 0x0                                          */
#define I2C_INTR_ENABLE_STARTDETEN_INDEX (0)
#define I2C_INTR_ENABLE_STARTDETEN_MASK (0x1)
#define I2C_INTR_ENABLE_STARTDETEN_CLEAR (0x0 << (I2C_INTR_ENABLE_STARTDETEN_INDEX))
#define I2C_INTR_ENABLE_STARTDETEN_SET (0x1 << (I2C_INTR_ENABLE_STARTDETEN_INDEX))
#endif /* I2C_INTR_ENABLE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/INTR_ENABLE:                     */
#define I2C_INTR_ENABLE_RESET_VALUE (0x000)
#define I2C_INTR_ENABLE (I2C_BASE_ADDR + I2C_INTR_ENABLE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/XFR_ADD                      ***/

#ifndef I2C_XFR_ADD_PROTECT
#define I2C_XFR_ADD_PROTECT

#define I2C_XFR_ADD_OFFS (0x18)

/* XFR_ADD BITS 31..0: Current transfer address                             */
/* numeric RO unsigned field, reset to 0x0                                  */
#define I2C_XFR_ADD_XFRADD_INDEX (0)
#define I2C_XFR_ADD_XFRADD_MASK (0xFFFFFFFF)
#endif /* I2C_XFR_ADD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/XFR_ADD:                         */
#define I2C_XFR_ADD_RESET_VALUE (0x00000000)
#define I2C_XFR_ADD (I2C_BASE_ADDR + I2C_XFR_ADD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/RX_FLR                       ***/

#ifndef I2C_RX_FLR_PROTECT
#define I2C_RX_FLR_PROTECT

#define I2C_RX_FLR_OFFS (0x1C)

/* RX_FLR BITS 7..0: RX FIFO level                                          */
/* numeric RO unsigned field, reset to 0x0                                  */
#define I2C_RX_FLR_RXFL_INDEX (0)
#define I2C_RX_FLR_RXFL_MASK (0xFF)
#endif /* I2C_RX_FLR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/RX_FLR:                          */
#define I2C_RX_FLR_RESET_VALUE (0x00)
#define I2C_RX_FLR (I2C_BASE_ADDR + I2C_RX_FLR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/TX_FLR                       ***/

#ifndef I2C_TX_FLR_PROTECT
#define I2C_TX_FLR_PROTECT

#define I2C_TX_FLR_OFFS (0x20)

/* TX_FLR BITS 7..0: TX FIFO level                                          */
/* numeric RO unsigned field, reset to 0x0                                  */
#define I2C_TX_FLR_TXFL_INDEX (0)
#define I2C_TX_FLR_TXFL_MASK (0xFF)
#endif /* I2C_TX_FLR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/TX_FLR:                          */
#define I2C_TX_FLR_RESET_VALUE (0x00)
#define I2C_TX_FLR (I2C_BASE_ADDR + I2C_TX_FLR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_DEST_ADD              ***/

#ifndef I2C_BUFFER_DEST_ADD_PROTECT
#define I2C_BUFFER_DEST_ADD_PROTECT

#define I2C_BUFFER_DEST_ADD_OFFS (0x24)

/* BUFFER_DEST_ADD BITS 31..0: Buffer-mode destination base address         */
/* numeric RW unsigned field, reset to 0x0                                  */
#define I2C_BUFFER_DEST_ADD_DESTADD_INDEX (0)
#define I2C_BUFFER_DEST_ADD_DESTADD_MASK (0xFFFFFFFF)
#endif /* I2C_BUFFER_DEST_ADD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_DEST_ADD:                 */
#define I2C_BUFFER_DEST_ADD_RESET_VALUE (0x00000000)
#define I2C_BUFFER_DEST_ADD (I2C_BASE_ADDR + I2C_BUFFER_DEST_ADD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_DEST_END              ***/

#ifndef I2C_BUFFER_DEST_END_PROTECT
#define I2C_BUFFER_DEST_END_PROTECT

#define I2C_BUFFER_DEST_END_OFFS (0x28)

/* BUFFER_DEST_END BITS 31..0: Buffer-mode destination end address          */
/* numeric RW unsigned field, reset to 0x0                                  */
#define I2C_BUFFER_DEST_END_DESTEND_INDEX (0)
#define I2C_BUFFER_DEST_END_DESTEND_MASK (0xFFFFFFFF)
#endif /* I2C_BUFFER_DEST_END_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_DEST_END:                 */
#define I2C_BUFFER_DEST_END_RESET_VALUE (0x00000000)
#define I2C_BUFFER_DEST_END (I2C_BASE_ADDR + I2C_BUFFER_DEST_END_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_SRC_ADD               ***/

#ifndef I2C_BUFFER_SRC_ADD_PROTECT
#define I2C_BUFFER_SRC_ADD_PROTECT

#define I2C_BUFFER_SRC_ADD_OFFS (0x2C)

/* BUFFER_SRC_ADD BITS 31..0: Buffer-mode source base address               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define I2C_BUFFER_SRC_ADD_SRCADD_INDEX (0)
#define I2C_BUFFER_SRC_ADD_SRCADD_MASK (0xFFFFFFFF)
#endif /* I2C_BUFFER_SRC_ADD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/BUFFER_SRC_ADD:                  */
#define I2C_BUFFER_SRC_ADD_RESET_VALUE (0x00000000)
#define I2C_BUFFER_SRC_ADD (I2C_BASE_ADDR + I2C_BUFFER_SRC_ADD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/SDA_SETUP                    ***/

#ifndef I2C_SDA_SETUP_PROTECT
#define I2C_SDA_SETUP_PROTECT

#define I2C_SDA_SETUP_OFFS (0x30)

/* SDA_SETUP BITS 7..0: SDA setup time in clock cycles                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define I2C_SDA_SETUP_SDASETUP_INDEX (0)
#define I2C_SDA_SETUP_SDASETUP_MASK (0xFF)
#endif /* I2C_SDA_SETUP_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/SDA_SETUP:                       */
#define I2C_SDA_SETUP_RESET_VALUE (0x00)
#define I2C_SDA_SETUP (I2C_BASE_ADDR + I2C_SDA_SETUP_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/NUMBYTES                     ***/

#ifndef I2C_NUMBYTES_PROTECT
#define I2C_NUMBYTES_PROTECT

#define I2C_NUMBYTES_OFFS (0x34)

/* NUMBYTES BITS 15..0: Number of transfered bytes during last transaction  */
/* numeric RO unsigned field, reset to 0x0                                  */
#define I2C_NUMBYTES_NBYTES_INDEX (0)
#define I2C_NUMBYTES_NBYTES_MASK (0xFFFF)
#endif /* I2C_NUMBYTES_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_CPU_PF/I2C/NUMBYTES:                        */
#define I2C_NUMBYTES_RESET_VALUE (0x0000)
#define I2C_NUMBYTES (I2C_BASE_ADDR + I2C_NUMBYTES_OFFS)

#define I2C_SIZE (0x35)

#define ATONT_CPU_PF_SIZE (0x30035)

/*** APB peripherals                                                      ***/
#define ATONT_APB_PF_BASE_ADDR (0x820000)

/*** ATONT Clock and Reset Module                                         ***/
#define CRM_BASE_ADDR (0x820000)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMRESET                     ***/
/* Reset Control Register                                                   */
#ifndef CRM_CRMRESET_PROTECT
#define CRM_CRMRESET_PROTECT

#define CRM_CRMRESET_OFFS (0x0)

/* CRMRESET BIT 0: Generates a system reset                                 */
/* symbolic WO field, reset to 0x0                                          */
#define CRM_CRMRESET_SYSRESET_INDEX (0)
#define CRM_CRMRESET_SYSRESET_MASK (0x1)
#define CRM_CRMRESET_SYSRESET_IDLE (0x0 << (CRM_CRMRESET_SYSRESET_INDEX))
#define CRM_CRMRESET_SYSRESET_FORCE (0x1 << (CRM_CRMRESET_SYSRESET_INDEX))
#endif /* CRM_CRMRESET_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMRESET: Reset Control Register */
#define CRM_CRMRESET_RESET_VALUE (0x0)
#define CRM_CRMRESET (CRM_BASE_ADDR + CRM_CRMRESET_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMCLKEN                     ***/
/* Clock Enable Register                                                    */
#ifndef CRM_CRMCLKEN_PROTECT
#define CRM_CRMCLKEN_PROTECT

#define CRM_CRMCLKEN_OFFS (0x4)

/* CRMCLKEN BIT 6: Enables BIST clock                                       */
/* symbolic RW field, reset to 0x0                                          */
#define CRM_CRMCLKEN_BISTEN_INDEX (6)
#define CRM_CRMCLKEN_BISTEN_MASK (0x40)
#define CRM_CRMCLKEN_BISTEN_DISABLE (0x0 << (CRM_CRMCLKEN_BISTEN_INDEX))
#define CRM_CRMCLKEN_BISTEN_ENABLE (0x1 << (CRM_CRMCLKEN_BISTEN_INDEX))

/* CRMCLKEN BIT 5: Enables DCMADC clock                                     */
/* symbolic RW field, reset to 0x0                                          */
#define CRM_CRMCLKEN_DCMADCEN_INDEX (5)
#define CRM_CRMCLKEN_DCMADCEN_MASK (0x20)
#define CRM_CRMCLKEN_DCMADCEN_DISABLE (0x0 << (CRM_CRMCLKEN_DCMADCEN_INDEX))
#define CRM_CRMCLKEN_DCMADCEN_ENABLE (0x1 << (CRM_CRMCLKEN_DCMADCEN_INDEX))

/* CRMCLKEN BIT 4: Enables OSIGN clock                                      */
/* symbolic RW field, reset to 0x0                                          */
#define CRM_CRMCLKEN_OSIGNEN_INDEX (4)
#define CRM_CRMCLKEN_OSIGNEN_MASK (0x10)
#define CRM_CRMCLKEN_OSIGNEN_DISABLE (0x0 << (CRM_CRMCLKEN_OSIGNEN_INDEX))
#define CRM_CRMCLKEN_OSIGNEN_ENABLE (0x1 << (CRM_CRMCLKEN_OSIGNEN_INDEX))

/* CRMCLKEN BIT 3: Enables GPIO clock                                       */
/* symbolic RW field, reset to 0x1                                          */
#define CRM_CRMCLKEN_GPIOEN_INDEX (3)
#define CRM_CRMCLKEN_GPIOEN_MASK (0x8)
#define CRM_CRMCLKEN_GPIOEN_DISABLE (0x0 << (CRM_CRMCLKEN_GPIOEN_INDEX))
#define CRM_CRMCLKEN_GPIOEN_ENABLE (0x1 << (CRM_CRMCLKEN_GPIOEN_INDEX))

/* CRMCLKEN BIT 2: Enables Timer clock                                      */
/* symbolic RW field, reset to 0x1                                          */
#define CRM_CRMCLKEN_TIMEREN_INDEX (2)
#define CRM_CRMCLKEN_TIMEREN_MASK (0x4)
#define CRM_CRMCLKEN_TIMEREN_DISABLE (0x0 << (CRM_CRMCLKEN_TIMEREN_INDEX))
#define CRM_CRMCLKEN_TIMEREN_ENABLE (0x1 << (CRM_CRMCLKEN_TIMEREN_INDEX))

/* CRMCLKEN BIT 1: Enables I2C clock                                        */
/* symbolic RW field, reset to 0x1                                          */
#define CRM_CRMCLKEN_I2CEN_INDEX (1)
#define CRM_CRMCLKEN_I2CEN_MASK (0x2)
#define CRM_CRMCLKEN_I2CEN_DISABLE (0x0 << (CRM_CRMCLKEN_I2CEN_INDEX))
#define CRM_CRMCLKEN_I2CEN_ENABLE (0x1 << (CRM_CRMCLKEN_I2CEN_INDEX))

/* CRMCLKEN BIT 0: Enables CPU clock                                        */
/* symbolic RW field, reset to 0x1                                          */
#define CRM_CRMCLKEN_CPUEN_INDEX (0)
#define CRM_CRMCLKEN_CPUEN_MASK (0x1)
#define CRM_CRMCLKEN_CPUEN_DISABLE (0x0 << (CRM_CRMCLKEN_CPUEN_INDEX))
#define CRM_CRMCLKEN_CPUEN_ENABLE (0x1 << (CRM_CRMCLKEN_CPUEN_INDEX))
#endif /* CRM_CRMCLKEN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMCLKEN: Clock Enable Register  */
#define CRM_CRMCLKEN_RESET_VALUE (0x0f)
#define CRM_CRMCLKEN (CRM_BASE_ADDR + CRM_CRMCLKEN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMPRSC                      ***/
/* Clock Pre-scaler Register                                                */
#ifndef CRM_CRMPRSC_PROTECT
#define CRM_CRMPRSC_PROTECT

#define CRM_CRMPRSC_OFFS (0x8)

/* CRMPRSC BITS 3..0: Crystal clock division ratio                          */
/* numeric RW unsigned field, reset to 0x1                                  */
#define CRM_CRMPRSC_CLKDIV_INDEX (0)
#define CRM_CRMPRSC_CLKDIV_MASK (0xF)
#endif /* CRM_CRMPRSC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/CRM/CRMPRSC: Clock Pre-scaler Register */
#define CRM_CRMPRSC_RESET_VALUE (0x1)
#define CRM_CRMPRSC (CRM_BASE_ADDR + CRM_CRMPRSC_OFFS)

#define CRM_SIZE (0x8)

/*** ATONT RF Control Registers                                           ***/
#define RF_BASE_ADDR (0x820400)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/XO_CAP                        ***/
/* Crystal Oscillator Capacitors Register                                   */
#ifndef RF_XO_CAP_PROTECT
#define RF_XO_CAP_PROTECT

#define RF_XO_CAP_OFFS (0x0)

/* XO_CAP BITS 15..8: Internal capacitor on pin Q2                          *
 * Typical Value                                                            *
 * 0 <XO_CAP<15 cap=0.4787pF*cval+6.4pF                                     *
 * 16<XO_CAP<31 cap=0.4767pF*(cval-16)+13.58pF                              *
 * 32<=cval<=47 cap=0.4773pF*(cval-32)+20.73pF                              */
/* numeric RW unsigned field, reset to 0x19                                 */
#define RF_XO_CAP_C2_INDEX (8)
#define RF_XO_CAP_C2_MASK (0xFF00)

/* XO_CAP BITS 7..0: Internal capacitor on pin Q1                           *
 * Typical Value                                                            *
 * 0 <XO_CAP<15 cap=0.4787pF*cval+6.4pF                                     *
 * 16<XO_CAP<31 cap=0.4767pF*(cval-16)+13.58pF                              *
 * 32<=cval<=47 cap=0.4773pF*(cval-32)+20.73pF                              */
/* numeric RW unsigned field, reset to 0x19                                 */
#define RF_XO_CAP_C1_INDEX (0)
#define RF_XO_CAP_C1_MASK (0xFF)
#endif /* RF_XO_CAP_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/XO_CAP: Crystal Oscillator        *
 * Capacitors Register                                                      */
#define RF_XO_CAP_RESET_VALUE (0x1919)
#define RF_XO_CAP (RF_BASE_ADDR + RF_XO_CAP_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_EN                         ***/
/* LO Enable Register                                                       */
#ifndef RF_LO_EN_PROTECT
#define RF_LO_EN_PROTECT

#define RF_LO_EN_OFFS (0x4)

/* LO_EN BIT 16: Enable VCO frequency measurement                           */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LO_EN_FREF_EN_INDEX (16)
#define RF_LO_EN_FREF_EN_MASK (0x10000)
#define RF_LO_EN_FREF_EN_DISABLE (0x0 << (RF_LO_EN_FREF_EN_INDEX))
#define RF_LO_EN_FREF_EN_ENABLE (0x1 << (RF_LO_EN_FREF_EN_INDEX))

/* LO_EN BIT 13: CML divider biasing current increases by 40 percent        */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LO_EN_BIAS_INDEX (13)
#define RF_LO_EN_BIAS_MASK (0x2000)
#define RF_LO_EN_BIAS_DEFAULT (0x0 << (RF_LO_EN_BIAS_INDEX))
#define RF_LO_EN_BIAS_INCREASE (0x1 << (RF_LO_EN_BIAS_INDEX))

/* LO_EN BIT 12: Reset dividers                                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LO_EN_RESET_INDEX (12)
#define RF_LO_EN_RESET_MASK (0x1000)
#define RF_LO_EN_RESET_DEFAULT (0x0 << (RF_LO_EN_RESET_INDEX))
#define RF_LO_EN_RESET_RESET (0x1 << (RF_LO_EN_RESET_INDEX))

/* LO_EN BIT 4: Low Voltage Regulator enable                                */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LO_EN_LDO_INDEX (4)
#define RF_LO_EN_LDO_MASK (0x10)
#define RF_LO_EN_LDO_DISABLE (0x0 << (RF_LO_EN_LDO_INDEX))
#define RF_LO_EN_LDO_ENABLE (0x1 << (RF_LO_EN_LDO_INDEX))

/* LO_EN BIT 0: PLL enable                                                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LO_EN_EN_INDEX (0)
#define RF_LO_EN_EN_MASK (0x1)
#define RF_LO_EN_EN_DISABLE (0x0 << (RF_LO_EN_EN_INDEX))
#define RF_LO_EN_EN_ENABLE (0x1 << (RF_LO_EN_EN_INDEX))
#endif /* RF_LO_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_EN: LO Enable Register         */
#define RF_LO_EN_RESET_VALUE (0x00000)
#define RF_LO_EN (RF_BASE_ADDR + RF_LO_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_D                          ***/
/* PLL D Divider                                                            */
#ifndef RF_LO_D_PROTECT
#define RF_LO_D_PROTECT

#define RF_LO_D_OFFS (0x8)

/* LO_D BITS 7..0: D divider min=8 / max=255                                *
 * Fref=Fxtal/D                                                             */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_D_D_INDEX (0)
#define RF_LO_D_D_MASK (0xFF)
#endif /* RF_LO_D_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_D: PLL D Divider               */
#define RF_LO_D_RESET_VALUE (0x00)
#define RF_LO_D (RF_BASE_ADDR + RF_LO_D_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_N                          ***/
/* PLL N Divider                                                            */
#ifndef RF_LO_N_PROTECT
#define RF_LO_N_PROTECT

#define RF_LO_N_OFFS (0xC)

/* LO_N BITS 13..0: N divider min=1024 / max=16383                          *
 * Fref=Fvco/N                                                              */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_N_N_INDEX (0)
#define RF_LO_N_N_MASK (0x3FFF)
#endif /* RF_LO_N_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_N: PLL N Divider               */
#define RF_LO_N_RESET_VALUE (0x0000)
#define RF_LO_N (RF_BASE_ADDR + RF_LO_N_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_P                          ***/
/* LO P Divider                                                             */
#ifndef RF_LO_P_PROTECT
#define RF_LO_P_PROTECT

#define RF_LO_P_OFFS (0x10)

/* LO_P BITS 5..4: M divider                                                *
 * Fm=Fvco/(M+1)                                                            *
 * when M within (1,2)                                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_P_M_INDEX (4)
#define RF_LO_P_M_MASK (0x30)

/* LO_P BITS 2..0: P divider                                                *
 * Flo=Fm/(2**(P+1))                                                        *
 * when P within (0:3)                                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_P_P_INDEX (0)
#define RF_LO_P_P_MASK (0x7)
#endif /* RF_LO_P_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_P: LO P Divider                */
#define RF_LO_P_RESET_VALUE (0x00)
#define RF_LO_P (RF_BASE_ADDR + RF_LO_P_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_PHI                        ***/
/* LO generator phase trimming                                              */
#ifndef RF_LO_PHI_PROTECT
#define RF_LO_PHI_PROTECT

#define RF_LO_PHI_OFFS (0x14)

/* LO_PHI BITS 3..0: lo generator 45deg and 134deg phase triming            *
 * it is a signed register!                                                 *
 * phi_deg = lo_phy*0.9/7 at 100MHz                                         *
 * phi_deg = lo_phy*5/7   at 600MHz                                         *
 * phi_deg = lo_phy       at 900MHz                                         */
/* numeric RW signed (2s comlement) field, reset to 0x0                     */
#define RF_LO_PHI_PHI_INDEX (0)
#define RF_LO_PHI_PHI_MASK (0xF)
#endif /* RF_LO_PHI_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_PHI: LO generator phase trimming */
#define RF_LO_PHI_RESET_VALUE (0x0)
#define RF_LO_PHI (RF_BASE_ADDR + RF_LO_PHI_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_CP                         ***/
/* Set Charge Pump current                                                  */
#ifndef RF_LO_CP_PROTECT
#define RF_LO_CP_PROTECT

#define RF_LO_CP_OFFS (0x18)

/* LO_CP BITS 5..4: Change Loop Filter damping resistor (default is zero)   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_CP_LF_INDEX (4)
#define RF_LO_CP_LF_MASK (0x30)

/* LO_CP BITS 3..0: Charge pump current                                     *
 * Typical current is (in uA)                                               *
 * -                                                                        *
 * CP  |  Icp  |  CP  |  Icp                                                *
 * ----------------------------                                             *
 * 00  |  000  |  08  |  !183!                                              *
 * 01  |  068  |  09  |  400                                                *
 * 02  |  084  |  10  |  450                                                *
 * 03  |  115  |  11  |  550                                                *
 * 04  |  145  |  12  |  590                                                *
 * 05  |  175  |  13  |  730                                                *
 * 06  |  220  |  14  |  845                                                *
 * 07  |  290  |  15  |  1000                                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LO_CP_CP_INDEX (0)
#define RF_LO_CP_CP_MASK (0xF)
#endif /* RF_LO_CP_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LO_CP: Set Charge Pump current    */
#define RF_LO_CP_RESET_VALUE (0x00)
#define RF_LO_CP (RF_BASE_ADDR + RF_LO_CP_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_BAND                    ***/
/* VCO Band Control                                                         */
#ifndef RF_LOVCO_BAND_PROTECT
#define RF_LOVCO_BAND_PROTECT

#define RF_LOVCO_BAND_OFFS (0x1C)

/* LOVCO_BAND BITS 5..0: VCO Band tuning                                    *
 * -                                                                        *
 * Band |  F(band)                                                          *
 * ---------------                                                          *
 * 0  | 2.50 GHz                                                            *
 * 1  | 2.55 GHz                                                            *
 * :  |   :                                                                 *
 * 62  | 3.9 GHz                                                            *
 * 63  | 4.1 GHz                                                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_BAND_BAND_INDEX (0)
#define RF_LOVCO_BAND_BAND_MASK (0x3F)
#endif /* RF_LOVCO_BAND_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_BAND: VCO Band Control      */
#define RF_LOVCO_BAND_RESET_VALUE (0x00)
#define RF_LOVCO_BAND (RF_BASE_ADDR + RF_LOVCO_BAND_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_ACM                     ***/
/* VCO Amplitude Measurement                                                */
#ifndef RF_LOVCO_ACM_PROTECT
#define RF_LOVCO_ACM_PROTECT

#define RF_LOVCO_ACM_OFFS (0x20)

/* LOVCO_ACM BIT 0: Enable measurement of VCO amplitude                     */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LOVCO_ACM_ACM_INDEX (0)
#define RF_LOVCO_ACM_ACM_MASK (0x1)
#define RF_LOVCO_ACM_ACM_DISABLE (0x0 << (RF_LOVCO_ACM_ACM_INDEX))
#define RF_LOVCO_ACM_ACM_ENABLE (0x1 << (RF_LOVCO_ACM_ACM_INDEX))
#endif /* RF_LOVCO_ACM_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_ACM: VCO Amplitude Measurement */
#define RF_LOVCO_ACM_RESET_VALUE (0x0)
#define RF_LOVCO_ACM (RF_BASE_ADDR + RF_LOVCO_ACM_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_AC                      ***/
/* VCO Amplitude Coarse Control                                             */
#ifndef RF_LOVCO_AC_PROTECT
#define RF_LOVCO_AC_PROTECT

#define RF_LOVCO_AC_OFFS (0x24)

/* LOVCO_AC BITS 4..0: VCO Amplitude Coarse Control                         *
 * Current in tank is Itank=400uA+100uA*LOVCO_AC                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_AC_AC_INDEX (0)
#define RF_LOVCO_AC_AC_MASK (0x1F)
#endif /* RF_LOVCO_AC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_AC: VCO Amplitude Coarse Control */
#define RF_LOVCO_AC_RESET_VALUE (0x00)
#define RF_LOVCO_AC (RF_BASE_ADDR + RF_LOVCO_AC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_AF                      ***/
/* VCO Amplitude Fine Control                                               */
#ifndef RF_LOVCO_AF_PROTECT
#define RF_LOVCO_AF_PROTECT

#define RF_LOVCO_AF_OFFS (0x28)

/* LOVCO_AF BITS 7..4: Set CM voltage                                       *
 * -                                                                        *
 * VCM     | Typ@0deg          | Typ@105deg                                 *
 * -----------------------------------                                      *
 * 0       | 0.65V             | 0.53V                                      *
 * 1       | 0.65V             | 0.53V                                      *
 * 2       | 0.55V             | 0.46V                                      *
 * 4       | 0.42V             | 0.35V                                      *
 * 8       | 0.27V             | 0.25V                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_AF_VCM_INDEX (4)
#define RF_LOVCO_AF_VCM_MASK (0xF0)

/* LOVCO_AF BITS 3..0: VCO Amplitude Fine Control                           *
 * =round((Ibias/vdiode)*runit-(runit/r1) with ibias=800uA, Runit=33kOhms,  *
 * R1=985Ohms                                                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_AF_AF_INDEX (0)
#define RF_LOVCO_AF_AF_MASK (0xF)
#endif /* RF_LOVCO_AF_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_AF: VCO Amplitude Fine Control */
#define RF_LOVCO_AF_RESET_VALUE (0x00)
#define RF_LOVCO_AF (RF_BASE_ADDR + RF_LOVCO_AF_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_KVCO                    ***/
/* KVCO Adjustement                                                         */
#ifndef RF_LOVCO_KVCO_PROTECT
#define RF_LOVCO_KVCO_PROTECT

#define RF_LOVCO_KVCO_OFFS (0x2C)

/* LOVCO_KVCO BITS 5..4: VCO amplifier Gain Margin adjustement              *
 * recommended gm=1+2*(band<48)                                             */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_KVCO_GM_INDEX (4)
#define RF_LOVCO_KVCO_GM_MASK (0x30)

/* LOVCO_KVCO BIT 0: KVCO adjustement                                       *
 * recommended kvco=(band<48)                                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LOVCO_KVCO_KVCO_INDEX (0)
#define RF_LOVCO_KVCO_KVCO_MASK (0x1)
#endif /* RF_LOVCO_KVCO_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LOVCO_KVCO: KVCO Adjustement      */
#define RF_LOVCO_KVCO_RESET_VALUE (0x00)
#define RF_LOVCO_KVCO (RF_BASE_ADDR + RF_LOVCO_KVCO_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPTHR                         ***/
/* Loopthrough control                                                      */
#ifndef RF_LPTHR_PROTECT
#define RF_LPTHR_PROTECT

#define RF_LPTHR_OFFS (0x30)

/* LPTHR BITS 4..0: Loopthrough attenuation                                 *
 * Gain relative to LNA output (or input depending on mode) Gain=(31-a)*0.01495 *
 * thus when a=0 gain=6.7dB, when a=16 => gain=-13dB                        */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LPTHR_A_INDEX (0)
#define RF_LPTHR_A_MASK (0x1F)
#endif /* RF_LPTHR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPTHR: Loopthrough control        */
#define RF_LPTHR_RESET_VALUE (0x00)
#define RF_LPTHR (RF_BASE_ADDR + RF_LPTHR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_EN                        ***/
/* LNA Enable Register                                                      */
#ifndef RF_LNA_EN_PROTECT
#define RF_LNA_EN_PROTECT

#define RF_LNA_EN_OFFS (0x34)

/* LNA_EN BIT 12: Activate crowbar on RF input                              */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_EN_CROWBAR_INDEX (12)
#define RF_LNA_EN_CROWBAR_MASK (0x1000)
#define RF_LNA_EN_CROWBAR_DISABLE (0x0 << (RF_LNA_EN_CROWBAR_INDEX))
#define RF_LNA_EN_CROWBAR_ENABLE (0x1 << (RF_LNA_EN_CROWBAR_INDEX))

/* LNA_EN BITS 9..8: Loop Through mode                                      */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_EN_LPTHR_INDEX (8)
#define RF_LNA_EN_LPTHR_MASK (0x300)
#define RF_LNA_EN_LPTHR_DISABLE (0x0 << (RF_LNA_EN_LPTHR_INDEX))
#define RF_LNA_EN_LPTHR_TUNERA_LOOPTHROUGH (0x1 << (RF_LNA_EN_LPTHR_INDEX))
#define RF_LNA_EN_LPTHR_TUNERA_TUNERB (0x2 << (RF_LNA_EN_LPTHR_INDEX))
#define RF_LNA_EN_LPTHR_TUNERB_LOOPTHROUGH (0x3 << (RF_LNA_EN_LPTHR_INDEX))

/* LNA_EN BIT 5: Places 75 Ohms load on input                               */
/* symbolic RW field, reset to 0x1                                          */
#define RF_LNA_EN_R75_INDEX (5)
#define RF_LNA_EN_R75_MASK (0x20)
#define RF_LNA_EN_R75_ENABLE (0x0 << (RF_LNA_EN_R75_INDEX))
#define RF_LNA_EN_R75_DISABLE (0x1 << (RF_LNA_EN_R75_INDEX))

/* LNA_EN BIT 4: Places 50 Ohms load on input                               */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_EN_R50_INDEX (4)
#define RF_LNA_EN_R50_MASK (0x10)
#define RF_LNA_EN_R50_ENABLE (0x0 << (RF_LNA_EN_R50_INDEX))
#define RF_LNA_EN_R50_DISABLE (0x1 << (RF_LNA_EN_R50_INDEX))

/* LNA_EN BITS 2..1: Set biasing mode                                       */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_EN_BIAS_INDEX (1)
#define RF_LNA_EN_BIAS_MASK (0x6)
#define RF_LNA_EN_BIAS_HIGH (0x0 << (RF_LNA_EN_BIAS_INDEX))
#define RF_LNA_EN_BIAS_MEDIUM (0x1 << (RF_LNA_EN_BIAS_INDEX))
#define RF_LNA_EN_BIAS_LOW (0x2 << (RF_LNA_EN_BIAS_INDEX))

/* LNA_EN BIT 0: LNA enable                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_EN_EN_INDEX (0)
#define RF_LNA_EN_EN_MASK (0x1)
#define RF_LNA_EN_EN_DISABLE (0x0 << (RF_LNA_EN_EN_INDEX))
#define RF_LNA_EN_EN_ENABLE (0x1 << (RF_LNA_EN_EN_INDEX))
#endif /* RF_LNA_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_EN: LNA Enable Register       */
#define RF_LNA_EN_RESET_VALUE (0x0020)
#define RF_LNA_EN (RF_BASE_ADDR + RF_LNA_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_RFSW                      ***/
/* Control RF switch on input pin RF_SW                                     */
#ifndef RF_LNA_RFSW_PROTECT
#define RF_LNA_RFSW_PROTECT

#define RF_LNA_RFSW_OFFS (0x3C)

/* LNA_RFSW BIT 0: Control RF switch. Switch is typicaly used for RF input  *
 * matching (switch ON in UHF)                                              */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_RFSW_RFSW_INDEX (0)
#define RF_LNA_RFSW_RFSW_MASK (0x1)
#define RF_LNA_RFSW_RFSW_ON (0x0 << (RF_LNA_RFSW_RFSW_INDEX))
#define RF_LNA_RFSW_RFSW_OFF (0x1 << (RF_LNA_RFSW_RFSW_INDEX))
#endif /* RF_LNA_RFSW_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_RFSW: Control RF switch on    *
 * input pin RF_SW                                                          */
#define RF_LNA_RFSW_RESET_VALUE (0x0)
#define RF_LNA_RFSW (RF_BASE_ADDR + RF_LNA_RFSW_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_GAIN                      ***/
/* LNA Gain                                                                 */
#ifndef RF_LNA_GAIN_PROTECT
#define RF_LNA_GAIN_PROTECT

#define RF_LNA_GAIN_OFFS (0x40)

/* LNA_GAIN BITS 22..16: control feedback resistance between lna op_n and   *
 * lna +ve input                                                            *
 * ZI = (10240*(CA(1+A) + 2*CB))/(75*(CA(1+2A) + 2CB(1+A)))                 *
 * with A->LNA single ended open loop gain (Typ. A=5.4)                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LNA_GAIN_ZI_INDEX (16)
#define RF_LNA_GAIN_ZI_MASK (0x7F0000)

/* LNA_GAIN BITS 14..8: control feedback capacitance from (lna -ve input)   *
 * to ipg                                                                   *
 * cb=CB*160fF (CB=1->127)                                                  */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LNA_GAIN_CB_INDEX (8)
#define RF_LNA_GAIN_CB_MASK (0x7F00)

/* LNA_GAIN BITS 6..0: control feedback capacitance from op_p to (lna -ve   *
 * input)                                                                   *
 * ca=CA*80fF (CA=1->127)                                                   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LNA_GAIN_CA_INDEX (0)
#define RF_LNA_GAIN_CA_MASK (0x7F)
#endif /* RF_LNA_GAIN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_GAIN: LNA Gain                */
#define RF_LNA_GAIN_RESET_VALUE (0x000000)
#define RF_LNA_GAIN (RF_BASE_ADDR + RF_LNA_GAIN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_ATT                       ***/
/* LNA input attenuator for 0dBm support                                    */
#ifndef RF_LNA_ATT_PROTECT
#define RF_LNA_ATT_PROTECT

#define RF_LNA_ATT_OFFS (0x44)

/* LNA_ATT BIT 24: Enable attenuator                                        */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LNA_ATT_ATT_INDEX (24)
#define RF_LNA_ATT_ATT_MASK (0x1000000)
#define RF_LNA_ATT_ATT_DISABLE (0x0 << (RF_LNA_ATT_ATT_INDEX))
#define RF_LNA_ATT_ATT_ENABLE (0x1 << (RF_LNA_ATT_ATT_INDEX))

/* LNA_ATT BITS 21..16: control parallel resistance of attenuator           *
 * rp=4000/decimal value(RP)                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LNA_ATT_RP_INDEX (16)
#define RF_LNA_ATT_RP_MASK (0x3F0000)

/* LNA_ATT BITS 9..0: control series resistance of attenuator               *
 * rs=3200/decimal value(RS)                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LNA_ATT_RS_INDEX (0)
#define RF_LNA_ATT_RS_MASK (0x3FF)
#endif /* RF_LNA_ATT_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LNA_ATT: LNA input attenuator     *
 * for 0dBm support                                                         */
#define RF_LNA_ATT_RESET_VALUE (0x0000000)
#define RF_LNA_ATT (RF_BASE_ADDR + RF_LNA_ATT_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBD1                          ***/
/* Wide band detector on LNA output                                         */
#ifndef RF_WBD1_PROTECT
#define RF_WBD1_PROTECT

#define RF_WBD1_OFFS (0x48)

/* WBD1 BIT 4: Enable WBD                                                   */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBD1_EN_INDEX (4)
#define RF_WBD1_EN_MASK (0x10)
#define RF_WBD1_EN_DISABLE (0x0 << (RF_WBD1_EN_INDEX))
#define RF_WBD1_EN_ENABLE (0x1 << (RF_WBD1_EN_INDEX))

/* WBD1 BITS 3..0: Set WBD threshold according to                           *
 * WBD_TH=200+TH*100 (mVpp) (Valid when TH>0)                               *
 * For exemple if WBD_TH=8, WBD1 will just trig when LNA output has a       *
 * sinewave of 1Vpp                                                         */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_WBD1_TH_INDEX (0)
#define RF_WBD1_TH_MASK (0xF)
#endif /* RF_WBD1_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBD1: Wide band detector on LNA output */
#define RF_WBD1_RESET_VALUE (0x00)
#define RF_WBD1 (RF_BASE_ADDR + RF_WBD1_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1_EN                       ***/
/* SKF1 enable and controls                                                 */
#ifndef RF_SKF1_EN_PROTECT
#define RF_SKF1_EN_PROTECT

#define RF_SKF1_EN_OFFS (0x4C)

/* SKF1_EN BIT 2: Bypass SKF1                                               */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SKF1_EN_BYPS_INDEX (2)
#define RF_SKF1_EN_BYPS_MASK (0x4)
#define RF_SKF1_EN_BYPS_DISABLE (0x0 << (RF_SKF1_EN_BYPS_INDEX))
#define RF_SKF1_EN_BYPS_ENABLE (0x1 << (RF_SKF1_EN_BYPS_INDEX))

/* SKF1_EN BIT 1: Low Power Mode                                            */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SKF1_EN_LPM_INDEX (1)
#define RF_SKF1_EN_LPM_MASK (0x2)
#define RF_SKF1_EN_LPM_DISABLE (0x0 << (RF_SKF1_EN_LPM_INDEX))
#define RF_SKF1_EN_LPM_ENABLE (0x1 << (RF_SKF1_EN_LPM_INDEX))

/* SKF1_EN BIT 0: Enable SKF1                                               */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SKF1_EN_EN_INDEX (0)
#define RF_SKF1_EN_EN_MASK (0x1)
#define RF_SKF1_EN_EN_DISABLE (0x0 << (RF_SKF1_EN_EN_INDEX))
#define RF_SKF1_EN_EN_ENABLE (0x1 << (RF_SKF1_EN_EN_INDEX))
#endif /* RF_SKF1_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1_EN: SKF1 enable and controls */
#define RF_SKF1_EN_RESET_VALUE (0x0)
#define RF_SKF1_EN (RF_BASE_ADDR + RF_SKF1_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1                          ***/
/* SKF1 frequency control                                                   */
#ifndef RF_SKF1_PROTECT
#define RF_SKF1_PROTECT

#define RF_SKF1_OFFS (0x50)

/* SKF1 BITS 9..8: SKF coarse or band tuning 0=UHF / 3=ISDB-T               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_SKF1_BAND_INDEX (8)
#define RF_SKF1_BAND_MASK (0x300)

/* SKF1 BITS 4..0: SKF frequency tuning                                     *
 * cut off frequency versus F is based on a look up table                   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_SKF1_F_INDEX (0)
#define RF_SKF1_F_MASK (0x1F)
#endif /* RF_SKF1_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1: SKF1 frequency control      */
#define RF_SKF1_RESET_VALUE (0x000)
#define RF_SKF1 (RF_BASE_ADDR + RF_SKF1_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1_MUTE                     ***/
/* SKF1 Mute                                                                */
#ifndef RF_SKF1_MUTE_PROTECT
#define RF_SKF1_MUTE_PROTECT

#define RF_SKF1_MUTE_OFFS (0x54)

/* SKF1_MUTE BIT 4: SKF mute contol (required during DC correction initialization */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SKF1_MUTE_MUTE_INDEX (4)
#define RF_SKF1_MUTE_MUTE_MASK (0x10)
#define RF_SKF1_MUTE_MUTE_OFF (0x0 << (RF_SKF1_MUTE_MUTE_INDEX))
#define RF_SKF1_MUTE_MUTE_ON (0x1 << (RF_SKF1_MUTE_MUTE_INDEX))
#endif /* RF_SKF1_MUTE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SKF1_MUTE: SKF1 Mute              */
#define RF_SKF1_MUTE_RESET_VALUE (0x00)
#define RF_SKF1_MUTE (RF_BASE_ADDR + RF_SKF1_MUTE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MIX                           ***/
/* Mixer                                                                    */
#ifndef RF_MIX_PROTECT
#define RF_MIX_PROTECT

#define RF_MIX_OFFS (0x58)

/* MIX BIT 0: Enable Mixer                                                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_MIX_EN_INDEX (0)
#define RF_MIX_EN_MASK (0x1)
#define RF_MIX_EN_DISABLE (0x0 << (RF_MIX_EN_INDEX))
#define RF_MIX_EN_ENABLE (0x1 << (RF_MIX_EN_INDEX))
#endif /* RF_MIX_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MIX: Mixer                        */
#define RF_MIX_RESET_VALUE (0x0)
#define RF_MIX (RF_BASE_ADDR + RF_MIX_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MIX_LO_EN                     ***/
/* Mixer                                                                    */
#ifndef RF_MIX_LO_EN_PROTECT
#define RF_MIX_LO_EN_PROTECT

#define RF_MIX_LO_EN_OFFS (0x5C)

/* MIX_LO_EN BITS 5..0: Individual HCM LO enable for I and Q.               *
 * Set to 0x00 when in STANDBY_MODE                                         *
 * set to 03F when in RUN_MODE                                              *
 * LO enable mapping :                                                      *
 * MIX_LO_EN<0> : I-mixer, LO phase 0                                       *
 * MIX_LO_EN<1> : I-mixer, LO phase 45                                      *
 * MIX_LO_EN<2> : I-mixer, LO phase 90                                      *
 * MIX_LO_EN<3> : Q-mixer, LO phase 180                                     *
 * MIX_LO_EN<4> : Q-mixer, LO phase 135                                     *
 * MIX_LO_EN<5> : Q-mixer, LO phase 90                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_MIX_LO_EN_LO_EN_INDEX (0)
#define RF_MIX_LO_EN_LO_EN_MASK (0x3F)
#endif /* RF_MIX_LO_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MIX_LO_EN: Mixer                  */
#define RF_MIX_LO_EN_RESET_VALUE (0x00)
#define RF_MIX_LO_EN (RF_BASE_ADDR + RF_MIX_LO_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/PMA_EN                        ***/
/* Post Mixer Amplifier enable                                              */
#ifndef RF_PMA_EN_PROTECT
#define RF_PMA_EN_PROTECT

#define RF_PMA_EN_OFFS (0x60)

/* PMA_EN BIT 0: Enable PMA                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_PMA_EN_EN_INDEX (0)
#define RF_PMA_EN_EN_MASK (0x1)
#define RF_PMA_EN_EN_DISABLE (0x0 << (RF_PMA_EN_EN_INDEX))
#define RF_PMA_EN_EN_ENABLE (0x1 << (RF_PMA_EN_EN_INDEX))
#endif /* RF_PMA_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/PMA_EN: Post Mixer Amplifier enable */
#define RF_PMA_EN_RESET_VALUE (0x0)
#define RF_PMA_EN (RF_BASE_ADDR + RF_PMA_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/PMA_G                         ***/
/* Post Mixer Amplifier enable                                              */
#ifndef RF_PMA_G_PROTECT
#define RF_PMA_G_PROTECT

#define RF_PMA_G_OFFS (0x64)

/* PMA_G BITS 8..0: PMA gain.                                               *
 * Gain law is :                                                            *
 * Gain=20*log10(1.25+PMA_G*0.023) in dB                                    */
/* numeric RW unsigned field, reset to 0x40                                 */
#define RF_PMA_G_G_INDEX (0)
#define RF_PMA_G_G_MASK (0x1FF)
#endif /* RF_PMA_G_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/PMA_G: Post Mixer Amplifier enable */
#define RF_PMA_G_RESET_VALUE (0x040)
#define RF_PMA_G (RF_BASE_ADDR + RF_PMA_G_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC                            ***/
/* DC correction enable and reference                                       */
#ifndef RF_DC_PROTECT
#define RF_DC_PROTECT

#define RF_DC_OFFS (0x68)

/* DC BITS 5..4: Enable DC correction                                       */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DC_EN_INDEX (4)
#define RF_DC_EN_MASK (0x30)
#define RF_DC_EN_DC_OFF (0x0 << (RF_DC_EN_INDEX))
#define RF_DC_EN_DC_I_ON (0x1 << (RF_DC_EN_INDEX))
#define RF_DC_EN_DC_Q_ON (0x2 << (RF_DC_EN_INDEX))
#define RF_DC_EN_DC_IQ_ON (0x3 << (RF_DC_EN_INDEX))

/* DC BITS 3..0: Reference tuning                                           *
 * with                                                                     *
 * X=I or Q                                                                 *
 * V(mixX) = -14.7uV * 21/(21-dc_ref) * (32*(dc_Xc-32)+dc_Xf-64)            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_REF_INDEX (0)
#define RF_DC_REF_MASK (0xF)
#endif /* RF_DC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC: DC correction enable and reference */
#define RF_DC_RESET_VALUE (0x00)
#define RF_DC (RF_BASE_ADDR + RF_DC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IC                         ***/
/* DC I channel coarse                                                      */
#ifndef RF_DC_IC_PROTECT
#define RF_DC_IC_PROTECT

#define RF_DC_IC_OFFS (0x6C)

/* DC_IC BITS 5..0: coarse DC correction on I                               *
 * See formula of DC_REF                                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_IC_IC_INDEX (0)
#define RF_DC_IC_IC_MASK (0x3F)
#endif /* RF_DC_IC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IC: DC I channel coarse        */
#define RF_DC_IC_RESET_VALUE (0x00)
#define RF_DC_IC (RF_BASE_ADDR + RF_DC_IC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IF                         ***/
/* DC I channel fine                                                        */
#ifndef RF_DC_IF_PROTECT
#define RF_DC_IF_PROTECT

#define RF_DC_IF_OFFS (0x70)

/* DC_IF BITS 6..0: fine DC correction on I                                 *
 * See formula of DC_REF                                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_IF_IF_INDEX (0)
#define RF_DC_IF_IF_MASK (0x7F)
#endif /* RF_DC_IF_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IF: DC I channel fine          */
#define RF_DC_IF_RESET_VALUE (0x00)
#define RF_DC_IF (RF_BASE_ADDR + RF_DC_IF_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QC                         ***/
/* DC Q channel coarse                                                      */
#ifndef RF_DC_QC_PROTECT
#define RF_DC_QC_PROTECT

#define RF_DC_QC_OFFS (0x74)

/* DC_QC BITS 5..0: coarse DC correction on I                               *
 * See formula of DC_REF                                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_QC_QC_INDEX (0)
#define RF_DC_QC_QC_MASK (0x3F)
#endif /* RF_DC_QC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QC: DC Q channel coarse        */
#define RF_DC_QC_RESET_VALUE (0x00)
#define RF_DC_QC (RF_BASE_ADDR + RF_DC_QC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QF                         ***/
/* DC Q channel fine                                                        */
#ifndef RF_DC_QF_PROTECT
#define RF_DC_QF_PROTECT

#define RF_DC_QF_OFFS (0x78)

/* DC_QF BITS 6..0: fine DC correction on I                                 *
 * See formula of DC_REF                                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_QF_QF_INDEX (0)
#define RF_DC_QF_QF_MASK (0x7F)
#endif /* RF_DC_QF_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QF: DC Q channel fine          */
#define RF_DC_QF_RESET_VALUE (0x00)
#define RF_DC_QF (RF_BASE_ADDR + RF_DC_QF_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IP                         ***/
/* DC after PMA I channel                                                   */
#ifndef RF_DC_IP_PROTECT
#define RF_DC_IP_PROTECT

#define RF_DC_IP_OFFS (0x7C)

/* DC_IP BITS 5..0: DC after PMA I channel                                  *
 * V(pma_i)= -1.26mV * (DC_IP-32)                                           */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_IP_IP_INDEX (0)
#define RF_DC_IP_IP_MASK (0x3F)
#endif /* RF_DC_IP_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_IP: DC after PMA I channel     */
#define RF_DC_IP_RESET_VALUE (0x00)
#define RF_DC_IP (RF_BASE_ADDR + RF_DC_IP_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QP                         ***/
/* DC after PMA Q channel                                                   */
#ifndef RF_DC_QP_PROTECT
#define RF_DC_QP_PROTECT

#define RF_DC_QP_OFFS (0x80)

/* DC_QP BITS 5..0: DC after PMA Q channel                                  *
 * V(pma_q)= -1.26mV * (DC_QP-32)                                           */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DC_QP_QP_INDEX (0)
#define RF_DC_QP_QP_MASK (0x3F)
#endif /* RF_DC_QP_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DC_QP: DC after PMA Q channel     */
#define RF_DC_QP_RESET_VALUE (0x00)
#define RF_DC_QP (RF_BASE_ADDR + RF_DC_QP_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBD2                          ***/
/* Wide band detector on pma output                                         */
#ifndef RF_WBD2_PROTECT
#define RF_WBD2_PROTECT

#define RF_WBD2_OFFS (0x84)

/* WBD2 BIT 4: Enable WBD                                                   */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBD2_EN_INDEX (4)
#define RF_WBD2_EN_MASK (0x10)
#define RF_WBD2_EN_DISABLE (0x0 << (RF_WBD2_EN_INDEX))
#define RF_WBD2_EN_ENABLE (0x1 << (RF_WBD2_EN_INDEX))

/* WBD2 BITS 3..0: Set WBD threshold according to                           *
 * WBD_TH=800+TH*100 (mVpp) (valid when TH>0)                               *
 * For exemple if WBD_TH=8, WBD2 will just trig when PMA output has a       *
 * sinewave exceeding 1.6Vpp                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_WBD2_TH_INDEX (0)
#define RF_WBD2_TH_MASK (0xF)
#endif /* RF_WBD2_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBD2: Wide band detector on pma output */
#define RF_WBD2_RESET_VALUE (0x00)
#define RF_WBD2 (RF_BASE_ADDR + RF_WBD2_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPF_EN                        ***/
/* LPF enable                                                               */
#ifndef RF_LPF_EN_PROTECT
#define RF_LPF_EN_PROTECT

#define RF_LPF_EN_OFFS (0x88)

/* LPF_EN BIT 8: Enable LPF overload detection                              */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LPF_EN_OVL_EN_INDEX (8)
#define RF_LPF_EN_OVL_EN_MASK (0x100)
#define RF_LPF_EN_OVL_EN_DISABLE (0x0 << (RF_LPF_EN_OVL_EN_INDEX))
#define RF_LPF_EN_OVL_EN_ENABLE (0x1 << (RF_LPF_EN_OVL_EN_INDEX))

/* LPF_EN BITS 5..4: LPF band selection                                     */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LPF_EN_C_INDEX (4)
#define RF_LPF_EN_C_MASK (0x30)
#define RF_LPF_EN_C_BDW_8MHZ (0x0 << (RF_LPF_EN_C_INDEX))
#define RF_LPF_EN_C_BDW_7MHZ (0x1 << (RF_LPF_EN_C_INDEX))
#define RF_LPF_EN_C_BDW_6MHZ (0x2 << (RF_LPF_EN_C_INDEX))

/* LPF_EN BIT 2: Enable BaseBand calibration(requires RCC)                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LPF_EN_BBCAL_INDEX (2)
#define RF_LPF_EN_BBCAL_MASK (0x4)
#define RF_LPF_EN_BBCAL_DISABLE (0x0 << (RF_LPF_EN_BBCAL_INDEX))
#define RF_LPF_EN_BBCAL_ENABLE (0x1 << (RF_LPF_EN_BBCAL_INDEX))

/* LPF_EN BIT 1: Enable LPF bist (requires RCC)                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LPF_EN_BIST_INDEX (1)
#define RF_LPF_EN_BIST_MASK (0x2)
#define RF_LPF_EN_BIST_DISABLE (0x0 << (RF_LPF_EN_BIST_INDEX))
#define RF_LPF_EN_BIST_ENABLE (0x1 << (RF_LPF_EN_BIST_INDEX))

/* LPF_EN BIT 0: Enable LPF                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_LPF_EN_EN_INDEX (0)
#define RF_LPF_EN_EN_MASK (0x1)
#define RF_LPF_EN_EN_DISABLE (0x0 << (RF_LPF_EN_EN_INDEX))
#define RF_LPF_EN_EN_ENABLE (0x1 << (RF_LPF_EN_EN_INDEX))
#endif /* RF_LPF_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPF_EN: LPF enable                */
#define RF_LPF_EN_RESET_VALUE (0x000)
#define RF_LPF_EN (RF_BASE_ADDR + RF_LPF_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPF                           ***/
/* LPF frequency control                                                    */
#ifndef RF_LPF_PROTECT
#define RF_LPF_PROTECT

#define RF_LPF_OFFS (0x8C)

/* LPF BITS 4..0: LPF tuning from -30% to 30% in 2% step                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_LPF_T_INDEX (0)
#define RF_LPF_T_MASK (0x1F)
#endif /* RF_LPF_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/LPF: LPF frequency control        */
#define RF_LPF_RESET_VALUE (0x00)
#define RF_LPF (RF_BASE_ADDR + RF_LPF_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/RCC_EN                        ***/
/* RCC enable and divider                                                   */
#ifndef RF_RCC_EN_PROTECT
#define RF_RCC_EN_PROTECT

#define RF_RCC_EN_OFFS (0x90)

/* RCC_EN BIT 8: Enable RCC                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_RCC_EN_EN_INDEX (8)
#define RF_RCC_EN_EN_MASK (0x100)
#define RF_RCC_EN_EN_DISABLE (0x0 << (RF_RCC_EN_EN_INDEX))
#define RF_RCC_EN_EN_ENABLE (0x1 << (RF_RCC_EN_EN_INDEX))

/* RCC_EN BITS 7..0: RCC Divider ratio (min=8 / max=255)                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_RCC_EN_DIV_INDEX (0)
#define RF_RCC_EN_DIV_MASK (0xFF)
#endif /* RF_RCC_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/RCC_EN: RCC enable and divider    */
#define RF_RCC_EN_RESET_VALUE (0x000)
#define RF_RCC_EN (RF_BASE_ADDR + RF_RCC_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/RCC_T                         ***/
/* RCC frequency tuning                                                     */
#ifndef RF_RCC_T_PROTECT
#define RF_RCC_T_PROTECT

#define RF_RCC_T_OFFS (0x94)

/* RCC_T BITS 5..0: RCC tuning                                              */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_RCC_T_T_INDEX (0)
#define RF_RCC_T_T_MASK (0x3F)
#endif /* RF_RCC_T_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/RCC_T: RCC frequency tuning       */
#define RF_RCC_T_RESET_VALUE (0x00)
#define RF_RCC_T (RF_BASE_ADDR + RF_RCC_T_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/VGA_EN                        ***/
/* VGA Enable                                                               */
#ifndef RF_VGA_EN_PROTECT
#define RF_VGA_EN_PROTECT

#define RF_VGA_EN_OFFS (0x98)

/* VGA_EN BIT 0: Enable VGA                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_VGA_EN_EN_INDEX (0)
#define RF_VGA_EN_EN_MASK (0x1)
#define RF_VGA_EN_EN_DISABLE (0x0 << (RF_VGA_EN_EN_INDEX))
#define RF_VGA_EN_EN_ENABLE (0x1 << (RF_VGA_EN_EN_INDEX))
#endif /* RF_VGA_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/VGA_EN: VGA Enable                */
#define RF_VGA_EN_RESET_VALUE (0x0)
#define RF_VGA_EN (RF_BASE_ADDR + RF_VGA_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/OBUF                          ***/
/* Output Buffer Enable and scale                                           */
#ifndef RF_OBUF_PROTECT
#define RF_OBUF_PROTECT

#define RF_OBUF_OFFS (0x9C)

/* OBUF BIT 5: Enable OBUF overload detection                               */
/* symbolic RW field, reset to 0x0                                          */
#define RF_OBUF_OVL_EN_INDEX (5)
#define RF_OBUF_OVL_EN_MASK (0x20)
#define RF_OBUF_OVL_EN_DISABLE (0x0 << (RF_OBUF_OVL_EN_INDEX))
#define RF_OBUF_OVL_EN_ENABLE (0x1 << (RF_OBUF_OVL_EN_INDEX))

/* OBUF BIT 4: Enable OBUF                                                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_OBUF_EN_INDEX (4)
#define RF_OBUF_EN_MASK (0x10)
#define RF_OBUF_EN_DISABLE (0x0 << (RF_OBUF_EN_INDEX))
#define RF_OBUF_EN_ENABLE (0x1 << (RF_OBUF_EN_INDEX))

/* OBUF BITS 1..0: Control OBUF gain                                        */
/* symbolic RW field, reset to 0x0                                          */
#define RF_OBUF_G_INDEX (0)
#define RF_OBUF_G_MASK (0x3)
#define RF_OBUF_G_G_0DB (0x0 << (RF_OBUF_G_INDEX))
#define RF_OBUF_G_G_3DB (0x1 << (RF_OBUF_G_INDEX))
#define RF_OBUF_G_G_6DB (0x2 << (RF_OBUF_G_INDEX))
#define RF_OBUF_G_G_9DB (0x3 << (RF_OBUF_G_INDEX))
#endif /* RF_OBUF_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/OBUF: Output Buffer Enable and scale */
#define RF_OBUF_RESET_VALUE (0x00)
#define RF_OBUF (RF_BASE_ADDR + RF_OBUF_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/OSIGN_EN                      ***/
/* Enable Output Sign measurement                                           */
#ifndef RF_OSIGN_EN_PROTECT
#define RF_OSIGN_EN_PROTECT

#define RF_OSIGN_EN_OFFS (0xA0)

/* OSIGN_EN BIT 0: Enable OSIGN                                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_OSIGN_EN_EN_INDEX (0)
#define RF_OSIGN_EN_EN_MASK (0x1)
#define RF_OSIGN_EN_EN_DISABLE (0x0 << (RF_OSIGN_EN_EN_INDEX))
#define RF_OSIGN_EN_EN_ENABLE (0x1 << (RF_OSIGN_EN_EN_INDEX))
#endif /* RF_OSIGN_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/OSIGN_EN: Enable Output Sign measurement */
#define RF_OSIGN_EN_RESET_VALUE (0x0)
#define RF_OSIGN_EN (RF_BASE_ADDR + RF_OSIGN_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM                           ***/
/* Analog Test Mux                                                          */
#ifndef RF_ATM_PROTECT
#define RF_ATM_PROTECT

#define RF_ATM_OFFS (0xA4)

/* ATM BITS 7..6: Reserved                                                  */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_ATM_NC_INDEX (6)
#define RF_ATM_NC_MASK (0xC0)

/* ATM BITS 5..4: Set GPA in output mode to observe dcm external signals    */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_GPA_OBS_INDEX (4)
#define RF_ATM_GPA_OBS_MASK (0x30)
#define RF_ATM_GPA_OBS_DISABLE (0x0 << (RF_ATM_GPA_OBS_INDEX))
#define RF_ATM_GPA_OBS_FOLLOWER (0x1 << (RF_ATM_GPA_OBS_INDEX))
#define RF_ATM_GPA_OBS_THROUGH_5K (0x2 << (RF_ATM_GPA_OBS_INDEX))

/* ATM BITS 3..0: Set ATM Mode (requires disable OBUF)                      */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_MODE_INDEX (0)
#define RF_ATM_MODE_MASK (0xF)
#define RF_ATM_MODE_DEFAULT (0x0 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_PMA (0x4 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_MIXER (0x1 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_RESERVED2 (0x5 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_LPF (0x2 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_OPRPPRES (0x6 << (RF_ATM_MODE_INDEX))
#define RF_ATM_MODE_VGA (0x3 << (RF_ATM_MODE_INDEX))
#endif /* RF_ATM_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM: Analog Test Mux              */
#define RF_ATM_RESET_VALUE (0x00)
#define RF_ATM (RF_BASE_ADDR + RF_ATM_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_DAC                       ***/
/* Analog Test Mux DAC control                                              */
#ifndef RF_ATM_DAC_PROTECT
#define RF_ATM_DAC_PROTECT

#define RF_ATM_DAC_OFFS (0xA8)

/* ATM_DAC BITS 5..0: ATM DAC code                                          */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_ATM_DAC_DAC_INDEX (0)
#define RF_ATM_DAC_DAC_MASK (0x3F)
#endif /* RF_ATM_DAC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_DAC: Analog Test Mux DAC control */
#define RF_ATM_DAC_RESET_VALUE (0x00)
#define RF_ATM_DAC (RF_BASE_ADDR + RF_ATM_DAC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_ADC                       ***/
/* Analog Test Mux ADC control                                              */
#ifndef RF_ATM_ADC_PROTECT
#define RF_ATM_ADC_PROTECT

#define RF_ATM_ADC_OFFS (0xAC)

/* ATM_ADC BIT 3: RESET ADC                                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_ADC_RST_INDEX (3)
#define RF_ATM_ADC_RST_MASK (0x8)
#define RF_ATM_ADC_RST_NO (0x0 << (RF_ATM_ADC_RST_INDEX))
#define RF_ATM_ADC_RST_YES (0x1 << (RF_ATM_ADC_RST_INDEX))

/* ATM_ADC BIT 2: ADC input selection                                       */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_ADC_SEL_INDEX (2)
#define RF_ATM_ADC_SEL_MASK (0x4)
#define RF_ATM_ADC_SEL_CONTROL (0x0 << (RF_ATM_ADC_SEL_INDEX))
#define RF_ATM_ADC_SEL_SENSE (0x1 << (RF_ATM_ADC_SEL_INDEX))

/* ATM_ADC BIT 1: research mode                                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_ADC_MODE_INDEX (1)
#define RF_ATM_ADC_MODE_MASK (0x2)
#define RF_ATM_ADC_MODE_DICHO (0x0 << (RF_ATM_ADC_MODE_INDEX))
#define RF_ATM_ADC_MODE_FOLLOWER (0x1 << (RF_ATM_ADC_MODE_INDEX))

/* ATM_ADC BIT 0: Trigger when in dichotomous reasearch                     */
/* symbolic RW field, reset to 0x0                                          */
#define RF_ATM_ADC_TRIG_INDEX (0)
#define RF_ATM_ADC_TRIG_MASK (0x1)
#define RF_ATM_ADC_TRIG_ARM (0x0 << (RF_ATM_ADC_TRIG_INDEX))
#define RF_ATM_ADC_TRIG_RUN (0x1 << (RF_ATM_ADC_TRIG_INDEX))
#endif /* RF_ATM_ADC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_ADC: Analog Test Mux ADC control */
#define RF_ATM_ADC_RESET_VALUE (0x0)
#define RF_ATM_ADC (RF_BASE_ADDR + RF_ATM_ADC_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DCM                           ***/
/* DCM Mux selection                                                        */
#ifndef RF_DCM_PROTECT
#define RF_DCM_PROTECT

#define RF_DCM_OFFS (0xB0)

/* DCM BITS 15..13: Reserved                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_DCM_NC_INDEX (13)
#define RF_DCM_NC_MASK (0xE000)

/* DCM BIT 12: Enable DAC                                                   */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DCM_DAC_INDEX (12)
#define RF_DCM_DAC_MASK (0x1000)
#define RF_DCM_DAC_DISABLE (0x0 << (RF_DCM_DAC_INDEX))
#define RF_DCM_DAC_ENABLE (0x1 << (RF_DCM_DAC_INDEX))

/* DCM BIT 11: Set follower current (requires DCM enable or ADC on, or      *
 * GPA_OBS>0                                                                */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DCM_BIAS_INDEX (11)
#define RF_DCM_BIAS_MASK (0x800)
#define RF_DCM_BIAS_LOW (0x0 << (RF_DCM_BIAS_INDEX))
#define RF_DCM_BIAS_HIGH (0x1 << (RF_DCM_BIAS_INDEX))

/* DCM BIT 10: DCM enable                                                   */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DCM_EN_INDEX (10)
#define RF_DCM_EN_MASK (0x400)
#define RF_DCM_EN_DISABLE (0x0 << (RF_DCM_EN_INDEX))
#define RF_DCM_EN_ENABLE (0x1 << (RF_DCM_EN_INDEX))

/* DCM BITS 9..8: DCM Selection requires DCM enable or ADC on, or GPA_OBS>0 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DCM_CONFIG_INDEX (8)
#define RF_DCM_CONFIG_MASK (0x300)
#define RF_DCM_CONFIG_OFF (0x0 << (RF_DCM_CONFIG_INDEX))
#define RF_DCM_CONFIG_SENSE (0x1 << (RF_DCM_CONFIG_INDEX))
#define RF_DCM_CONFIG_CONTROL (0x2 << (RF_DCM_CONFIG_INDEX))
#define RF_DCM_CONFIG_THROUGH (0x3 << (RF_DCM_CONFIG_INDEX))

/* DCM BITS 5..0: DCM Selection                                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_DCM_SEL_INDEX (0)
#define RF_DCM_SEL_MASK (0x3F)
#define RF_DCM_SEL_LPF_IP (0x23 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VGA_QN (0x24 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VGA_QP (0x25 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VGA_IN (0x26 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_PMA_QN (0x28 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VGA_IP (0x27 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_PMA_QP (0x29 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_PMA_IN (0x2A << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_PMA_IP (0x2B << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_QN (0x2C << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_QP (0x2D << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_IN (0x2E << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_IP (0x2F << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED8 (0x30 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED10 (0x32 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED9 (0x31 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_DCM_DVDD (0x33 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_SKF1 (0xA << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RFBG_VCM (0xB << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VTUNE (0xC << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_COM_MODE (0xD << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_ACM_P (0xE << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_ACM_N (0xF << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_DCP0 (0x10 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_XO_TEST (0x0 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_DCP1 (0x11 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED1 (0x1 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VDD_TANK (0x12 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED2 (0x2 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VDIODE (0x13 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_DVDD_MIX (0x14 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_RESERVED3 (0x3 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_MIX (0x15 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_WBD1 (0x4 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VCM_MIX (0x16 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_LNA (0x5 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AGND_MIX (0x17 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_PAD1 (0x6 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VPTAT (0x18 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_PAD2 (0x7 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VBG (0x19 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_BG (0x8 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_VCM (0x1A << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AVDD_RF_LT (0x9 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_AGND_BB (0x1B << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_MIX_QN (0x1C << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_MIX_QP (0x1D << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_MIX_IN (0x1E << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_MIX_IP (0x1F << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_LPF_QN (0x20 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_LPF_QP (0x21 << (RF_DCM_SEL_INDEX))
#define RF_DCM_SEL_LPF_IN (0x22 << (RF_DCM_SEL_INDEX))
#endif /* RF_DCM_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/DCM: DCM Mux selection            */
#define RF_DCM_RESET_VALUE (0x0000)
#define RF_DCM (RF_BASE_ADDR + RF_DCM_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SGEN                          ***/
/* SGEN                                                                     */
#ifndef RF_SGEN_PROTECT
#define RF_SGEN_PROTECT

#define RF_SGEN_OFFS (0xB4)

/* SGEN BIT 9: Reserved                                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_SGEN_RSVD2_INDEX (9)
#define RF_SGEN_RSVD2_MASK (0x200)

/* SGEN BIT 8: SGEN MODE                                                    */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SGEN_MODE_INDEX (8)
#define RF_SGEN_MODE_MASK (0x100)
#define RF_SGEN_MODE_OFF (0x0 << (RF_SGEN_MODE_INDEX))
#define RF_SGEN_MODE_XOCLK (0x1 << (RF_SGEN_MODE_INDEX))

/* SGEN BITS 3..1: Reserved                                                 */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_SGEN_RSVD1_INDEX (1)
#define RF_SGEN_RSVD1_MASK (0xE)

/* SGEN BIT 0: Control output power requires 75Ohms termination             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SGEN_PWR_INDEX (0)
#define RF_SGEN_PWR_MASK (0x1)
#define RF_SGEN_PWR_DISABLE (0x0 << (RF_SGEN_PWR_INDEX))
#define RF_SGEN_PWR_ENABLE (0x1 << (RF_SGEN_PWR_INDEX))
#endif /* RF_SGEN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SGEN: SGEN                        */
#define RF_SGEN_RESET_VALUE (0x000)
#define RF_SGEN (RF_BASE_ADDR + RF_SGEN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_A                         ***/
/* Reserved                                                                 */
#ifndef RF_TBD_A_PROTECT
#define RF_TBD_A_PROTECT

#define RF_TBD_A_OFFS (0xB8)

/* TBD_A BITS 3..2: Reserved                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_TBD_A_A_INDEX (2)
#define RF_TBD_A_A_MASK (0xC)

/* TBD_A BIT 1: OSIG position (choose between VGA or OBUF output)           */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TBD_A_OSIG_POS_INDEX (1)
#define RF_TBD_A_OSIG_POS_MASK (0x2)
#define RF_TBD_A_OSIG_POS_VGA (0x0 << (RF_TBD_A_OSIG_POS_INDEX))
#define RF_TBD_A_OSIG_POS_OBUF (0x1 << (RF_TBD_A_OSIG_POS_INDEX))

/* TBD_A BIT 0: MUTE PMA output                                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TBD_A_PMA_MUTE_INDEX (0)
#define RF_TBD_A_PMA_MUTE_MASK (0x1)
#define RF_TBD_A_PMA_MUTE_DISABLE (0x0 << (RF_TBD_A_PMA_MUTE_INDEX))
#define RF_TBD_A_PMA_MUTE_ENABLE (0x1 << (RF_TBD_A_PMA_MUTE_INDEX))
#endif /* RF_TBD_A_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_A: Reserved                   */
#define RF_TBD_A_RESET_VALUE (0x0)
#define RF_TBD_A (RF_BASE_ADDR + RF_TBD_A_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_B                         ***/
/* Control LPF gain                                                         */
#ifndef RF_TBD_B_PROTECT
#define RF_TBD_B_PROTECT

#define RF_TBD_B_OFFS (0xBC)

/* TBD_B BITS 7..6: Reserved                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_TBD_B_B_INDEX (6)
#define RF_TBD_B_B_MASK (0xC0)

/* TBD_B BIT 5: Control 1st stage lpf gain                                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TBD_B_EN_GAIN_INDEX (5)
#define RF_TBD_B_EN_GAIN_MASK (0x20)
#define RF_TBD_B_EN_GAIN_DISABLE (0x0 << (RF_TBD_B_EN_GAIN_INDEX))
#define RF_TBD_B_EN_GAIN_ENABLE (0x1 << (RF_TBD_B_EN_GAIN_INDEX))

/* TBD_B BITS 4..0: Control 1st stage lpf gain min=-2.6dB / max=3.3dB       */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_TBD_B_GAIN_INDEX (0)
#define RF_TBD_B_GAIN_MASK (0x1F)
#endif /* RF_TBD_B_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_B: Control LPF gain           */
#define RF_TBD_B_RESET_VALUE (0x00)
#define RF_TBD_B (RF_BASE_ADDR + RF_TBD_B_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_C                         ***/
/* Set mixer ground                                                         */
#ifndef RF_TBD_C_PROTECT
#define RF_TBD_C_PROTECT

#define RF_TBD_C_OFFS (0xC0)

/* TBD_C BIT 15: Active a switch located between DGND_PLL and AGND_BB       */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TBD_C_SWITCH_INDEX (15)
#define RF_TBD_C_SWITCH_MASK (0x8000)
#define RF_TBD_C_SWITCH_ON (0x0 << (RF_TBD_C_SWITCH_INDEX))
#define RF_TBD_C_SWITCH_OFF (0x1 << (RF_TBD_C_SWITCH_INDEX))

/* TBD_C BITS 14..0: Reserved                                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define RF_TBD_C_C_INDEX (0)
#define RF_TBD_C_C_MASK (0x7FFF)
#endif /* RF_TBD_C_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TBD_C: Set mixer ground           */
#define RF_TBD_C_RESET_VALUE (0x0000)
#define RF_TBD_C (RF_BASE_ADDR + RF_TBD_C_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_RESET                      ***/
/* Reset internal accumulator in sign filters                               */
#ifndef RF_SF_RESET_PROTECT
#define RF_SF_RESET_PROTECT

#define RF_SF_RESET_OFFS (0xC4)

/* SF_RESET BIT 0: Reset internal accumulator                               */
/* symbolic RW field, reset to 0x1                                          */
#define RF_SF_RESET_RESET_INDEX (0)
#define RF_SF_RESET_RESET_MASK (0x1)
#define RF_SF_RESET_RESET_DEFAULT (0x0 << (RF_SF_RESET_RESET_INDEX))
#define RF_SF_RESET_RESET_RESET (0x1 << (RF_SF_RESET_RESET_INDEX))
#endif /* RF_SF_RESET_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_RESET: Reset internal          *
 * accumulator in sign filters                                              */
#define RF_SF_RESET_RESET_VALUE (0x1)
#define RF_SF_RESET (RF_BASE_ADDR + RF_SF_RESET_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_FREEZE                     ***/
/* Freezes internal accumulator in sign filters                             */
#ifndef RF_SF_FREEZE_PROTECT
#define RF_SF_FREEZE_PROTECT

#define RF_SF_FREEZE_OFFS (0xC8)

/* SF_FREEZE BIT 0: Freezes internal accumulator                            */
/* symbolic RW field, reset to 0x0                                          */
#define RF_SF_FREEZE_FREEZE_INDEX (0)
#define RF_SF_FREEZE_FREEZE_MASK (0x1)
#define RF_SF_FREEZE_FREEZE_DEFAULT (0x0 << (RF_SF_FREEZE_FREEZE_INDEX))
#define RF_SF_FREEZE_FREEZE_FREEZE (0x1 << (RF_SF_FREEZE_FREEZE_INDEX))
#endif /* RF_SF_FREEZE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_FREEZE: Freezes internal       *
 * accumulator in sign filters                                              */
#define RF_SF_FREEZE_RESET_VALUE (0x0)
#define RF_SF_FREEZE (RF_BASE_ADDR + RF_SF_FREEZE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_BW                         ***/
/* Define bandwidth of sign filter                                          */
#ifndef RF_SF_BW_PROTECT
#define RF_SF_BW_PROTECT

#define RF_SF_BW_OFFS (0xCC)

/* SF_BW BITS 7..0: Bandwidth of signfilter (2's complement)                *
 * for BW=0  Fc=4.84*FREQ_SYS_CLK*2^(BW)                                    *
 * Where FREQ_SYS_CLK is CPU clock frequency   (in MHz)                     */
/* numeric RW signed (2s comlement) field, reset to 0x0                     */
#define RF_SF_BW_BW_INDEX (0)
#define RF_SF_BW_BW_MASK (0xFF)
#endif /* RF_SF_BW_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_BW: Define bandwidth of sign filter */
#define RF_SF_BW_RESET_VALUE (0x00)
#define RF_SF_BW (RF_BASE_ADDR + RF_SF_BW_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/VGA_G                         ***/
/* VGA Gain                                                                 */
#ifndef RF_VGA_G_PROTECT
#define RF_VGA_G_PROTECT

#define RF_VGA_G_OFFS (0xD0)

/* VGA_G BITS 11..0: VGA gain. Gain law is :                                *
 * Gain=20*log10(VGA_G/128) in                                              *
 * dB                                                                       */
/* numeric RW unsigned field, reset to 0x40                                 */
#define RF_VGA_G_VGA_G_INDEX (0)
#define RF_VGA_G_VGA_G_MASK (0xFFF)
#endif /* RF_VGA_G_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/VGA_G: VGA Gain                   */
#define RF_VGA_G_RESET_VALUE (0x040)
#define RF_VGA_G (RF_BASE_ADDR + RF_VGA_G_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TUNER_CONTROL                 ***/
/* Set RF tuner mode                                                        */
#ifndef RF_TUNER_CONTROL_PROTECT
#define RF_TUNER_CONTROL_PROTECT

#define RF_TUNER_CONTROL_OFFS (0xD4)

/* TUNER_CONTROL BITS 31..12: reserved                                      */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_R12_INDEX (12)
#define RF_TUNER_CONTROL_R12_MASK (0xFFFFF000)
#define RF_TUNER_CONTROL_R12_DEFAULT (0x0 << (RF_TUNER_CONTROL_R12_INDEX))

/* TUNER_CONTROL BITS 10..8: control tuner mode                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_CMD_INDEX (8)
#define RF_TUNER_CONTROL_CMD_MASK (0x700)
#define RF_TUNER_CONTROL_CMD_SLEEP (0x0 << (RF_TUNER_CONTROL_CMD_INDEX))
#define RF_TUNER_CONTROL_CMD_STANDBY (0x1 << (RF_TUNER_CONTROL_CMD_INDEX))
#define RF_TUNER_CONTROL_CMD_SET_TUNE (0x2 << (RF_TUNER_CONTROL_CMD_INDEX))
#define RF_TUNER_CONTROL_CMD_STOP (0x3 << (RF_TUNER_CONTROL_CMD_INDEX))

/* TUNER_CONTROL BITS 7..5: reserved                                        */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_R5_INDEX (5)
#define RF_TUNER_CONTROL_R5_MASK (0xE0)
#define RF_TUNER_CONTROL_R5_DEFAULT (0x0 << (RF_TUNER_CONTROL_R5_INDEX))

/* TUNER_CONTROL BIT 4: status tuner algo execution                         */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_STATUS_INDEX (4)
#define RF_TUNER_CONTROL_STATUS_MASK (0x10)
#define RF_TUNER_CONTROL_STATUS_PASS (0x0 << (RF_TUNER_CONTROL_STATUS_INDEX))
#define RF_TUNER_CONTROL_STATUS_FAIL (0x1 << (RF_TUNER_CONTROL_STATUS_INDEX))

/* TUNER_CONTROL BITS 3..2: reserved                                        */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_R2_INDEX (2)
#define RF_TUNER_CONTROL_R2_MASK (0xC)
#define RF_TUNER_CONTROL_R2_DEFAULT (0x0 << (RF_TUNER_CONTROL_R2_INDEX))

/* TUNER_CONTROL BITS 1..0: control tuner algo                              */
/* symbolic RW field, reset to 0x0                                          */
#define RF_TUNER_CONTROL_REQUEST_INDEX (0)
#define RF_TUNER_CONTROL_REQUEST_MASK (0x3)
#define RF_TUNER_CONTROL_REQUEST_BOOT (0x0 << (RF_TUNER_CONTROL_REQUEST_INDEX))
#define RF_TUNER_CONTROL_REQUEST_DONE (0x1 << (RF_TUNER_CONTROL_REQUEST_INDEX))
#define RF_TUNER_CONTROL_REQUEST_REQUEST (0x2 << (RF_TUNER_CONTROL_REQUEST_INDEX))
#define RF_TUNER_CONTROL_REQUEST_PROCESS (0x3 << (RF_TUNER_CONTROL_REQUEST_INDEX))
#endif /* RF_TUNER_CONTROL_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/TUNER_CONTROL: Set RF tuner mode  */
#define RF_TUNER_CONTROL_RESET_VALUE (0x00000000)
#define RF_TUNER_CONTROL (RF_BASE_ADDR + RF_TUNER_CONTROL_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_CTRL                    ***/
/* Wide-band detector control                                               */
#ifndef RF_WBDFI_CTRL_PROTECT
#define RF_WBDFI_CTRL_PROTECT

#define RF_WBDFI_CTRL_OFFS (0xF4)

/* WBDFI_CTRL BIT 1: Locks the WBD filter                                   */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_CTRL_LOCK_INDEX (1)
#define RF_WBDFI_CTRL_LOCK_MASK (0x2)
#define RF_WBDFI_CTRL_LOCK_DEFAULT (0x0 << (RF_WBDFI_CTRL_LOCK_INDEX))
#define RF_WBDFI_CTRL_LOCK_LOCK (0x1 << (RF_WBDFI_CTRL_LOCK_INDEX))

/* WBDFI_CTRL BIT 0: Resets the WBD filter                                  */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_CTRL_RESET_INDEX (0)
#define RF_WBDFI_CTRL_RESET_MASK (0x1)
#define RF_WBDFI_CTRL_RESET_DEFAULT (0x0 << (RF_WBDFI_CTRL_RESET_INDEX))
#define RF_WBDFI_CTRL_RESET_RESET (0x1 << (RF_WBDFI_CTRL_RESET_INDEX))
#endif /* RF_WBDFI_CTRL_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_CTRL: Wide-band detector control */
#define RF_WBDFI_CTRL_RESET_VALUE (0x0)
#define RF_WBDFI_CTRL (RF_BASE_ADDR + RF_WBDFI_CTRL_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_MASK                    ***/
/* Wide-band detector filter mask control                                   */
#ifndef RF_WBDFI_MASK_PROTECT
#define RF_WBDFI_MASK_PROTECT

#define RF_WBDFI_MASK_OFFS (0xF8)

/* WBDFI_MASK BIT 4: Masks the OBUF overload                                */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_MASK_OBUF_INDEX (4)
#define RF_WBDFI_MASK_OBUF_MASK (0x10)
#define RF_WBDFI_MASK_OBUF_ENABLED (0x0 << (RF_WBDFI_MASK_OBUF_INDEX))
#define RF_WBDFI_MASK_OBUF_MASKED (0x1 << (RF_WBDFI_MASK_OBUF_INDEX))

/* WBDFI_MASK BIT 3: Masks the LPF overload                                 */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_MASK_LPF_INDEX (3)
#define RF_WBDFI_MASK_LPF_MASK (0x8)
#define RF_WBDFI_MASK_LPF_ENABLED (0x0 << (RF_WBDFI_MASK_LPF_INDEX))
#define RF_WBDFI_MASK_LPF_MASKED (0x1 << (RF_WBDFI_MASK_LPF_INDEX))

/* WBDFI_MASK BIT 2: Masks the monitor overload                             */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_MASK_MON_INDEX (2)
#define RF_WBDFI_MASK_MON_MASK (0x4)
#define RF_WBDFI_MASK_MON_ENABLED (0x0 << (RF_WBDFI_MASK_MON_INDEX))
#define RF_WBDFI_MASK_MON_MASKED (0x1 << (RF_WBDFI_MASK_MON_INDEX))

/* WBDFI_MASK BIT 1: Masks the WBD2 overload                                */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_MASK_WBD2_INDEX (1)
#define RF_WBDFI_MASK_WBD2_MASK (0x2)
#define RF_WBDFI_MASK_WBD2_ENABLED (0x0 << (RF_WBDFI_MASK_WBD2_INDEX))
#define RF_WBDFI_MASK_WBD2_MASKED (0x1 << (RF_WBDFI_MASK_WBD2_INDEX))

/* WBDFI_MASK BIT 0: Masks the WBD1 overload                                */
/* symbolic RW field, reset to 0x0                                          */
#define RF_WBDFI_MASK_WBD1_INDEX (0)
#define RF_WBDFI_MASK_WBD1_MASK (0x1)
#define RF_WBDFI_MASK_WBD1_ENABLED (0x0 << (RF_WBDFI_MASK_WBD1_INDEX))
#define RF_WBDFI_MASK_WBD1_MASKED (0x1 << (RF_WBDFI_MASK_WBD1_INDEX))
#endif /* RF_WBDFI_MASK_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_MASK: Wide-band detector    *
 * filter mask control                                                      */
#define RF_WBDFI_MASK_RESET_VALUE (0x00)
#define RF_WBDFI_MASK (RF_BASE_ADDR + RF_WBDFI_MASK_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/XO_MODE                       ***/
/* Hardware Identification                                                  */
#ifndef RF_XO_MODE_PROTECT
#define RF_XO_MODE_PROTECT

#define RF_XO_MODE_OFFS (0x100)

/* XO_MODE BIT 0: Indicate external clock type                              */
/* symbolic RO field, reset to 0x0                                          */
#define RF_XO_MODE_XO_MODE_INDEX (0)
#define RF_XO_MODE_XO_MODE_MASK (0x1)
#define RF_XO_MODE_XO_MODE_XTAL (0x0 << (RF_XO_MODE_XO_MODE_INDEX))
#define RF_XO_MODE_XO_MODE_EXTERNAL (0x1 << (RF_XO_MODE_XO_MODE_INDEX))
#endif /* RF_XO_MODE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/XO_MODE: Hardware Identification  */
#define RF_XO_MODE_RESET_VALUE (0x0)
#define RF_XO_MODE (RF_BASE_ADDR + RF_XO_MODE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_ADCVAL                    ***/
/* Analog Test Mux ADC output                                               */
#ifndef RF_ATM_ADCVAL_PROTECT
#define RF_ATM_ADCVAL_PROTECT

#define RF_ATM_ADCVAL_OFFS (0x104)

/* ATM_ADCVAL BIT 8: ATM ADC VALID                                          */
/* symbolic RO field, reset to 0x0                                          */
#define RF_ATM_ADCVAL_VALID_INDEX (8)
#define RF_ATM_ADCVAL_VALID_MASK (0x100)
#define RF_ATM_ADCVAL_VALID_YES (0x0 << (RF_ATM_ADCVAL_VALID_INDEX))
#define RF_ATM_ADCVAL_VALID_NO (0x1 << (RF_ATM_ADCVAL_VALID_INDEX))

/* ATM_ADCVAL BITS 7..0: ATM ADC output                                     */
/* numeric RO unsigned field, reset to 0x0                                  */
#define RF_ATM_ADCVAL_ADCOUT_INDEX (0)
#define RF_ATM_ADCVAL_ADCOUT_MASK (0xFF)
#endif /* RF_ATM_ADCVAL_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/ATM_ADCVAL: Analog Test Mux ADC output */
#define RF_ATM_ADCVAL_RESET_VALUE (0x000)
#define RF_ATM_ADCVAL (RF_BASE_ADDR + RF_ATM_ADCVAL_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MONITOR                       ***/
/* Monitoring of Analog Status (asynchrone input events)                    */
#ifndef RF_MONITOR_PROTECT
#define RF_MONITOR_PROTECT

#define RF_MONITOR_OFFS (0x108)

/* MONITOR BIT 14: RESERVED                                                 */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_MON_X_INDEX (14)
#define RF_MONITOR_MON_X_MASK (0x4000)
#define RF_MONITOR_MON_X_OK (0x0 << (RF_MONITOR_MON_X_INDEX))
#define RF_MONITOR_MON_X_ALARM (0x1 << (RF_MONITOR_MON_X_INDEX))

/* MONITOR BIT 8: RCC Calibration for Low pass filter.  increase/decrease   *
 * RCC_T on next interation                                                 */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_RCC_OUT_INDEX (8)
#define RF_MONITOR_RCC_OUT_MASK (0x100)
#define RF_MONITOR_RCC_OUT_DECREM (0x0 << (RF_MONITOR_RCC_OUT_INDEX))
#define RF_MONITOR_RCC_OUT_INCREM (0x1 << (RF_MONITOR_RCC_OUT_INDEX))

/* MONITOR BIT 7: LPF Overload                                              */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_LPF_OVL_INDEX (7)
#define RF_MONITOR_LPF_OVL_MASK (0x80)
#define RF_MONITOR_LPF_OVL_NO (0x0 << (RF_MONITOR_LPF_OVL_INDEX))
#define RF_MONITOR_LPF_OVL_YES (0x1 << (RF_MONITOR_LPF_OVL_INDEX))

/* MONITOR BIT 6: Output Buffer Overload                                    */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_OBUF_OVL_INDEX (6)
#define RF_MONITOR_OBUF_OVL_MASK (0x40)
#define RF_MONITOR_OBUF_OVL_NO (0x0 << (RF_MONITOR_OBUF_OVL_INDEX))
#define RF_MONITOR_OBUF_OVL_YES (0x1 << (RF_MONITOR_OBUF_OVL_INDEX))

/* MONITOR BIT 5: Mixer input overload                                      */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_WBD2_OVL_INDEX (5)
#define RF_MONITOR_WBD2_OVL_MASK (0x20)
#define RF_MONITOR_WBD2_OVL_NO (0x0 << (RF_MONITOR_WBD2_OVL_INDEX))
#define RF_MONITOR_WBD2_OVL_YES (0x1 << (RF_MONITOR_WBD2_OVL_INDEX))

/* MONITOR BIT 4: LNA output overload (WBD1)                                */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_WBD1_OVL_INDEX (4)
#define RF_MONITOR_WBD1_OVL_MASK (0x10)
#define RF_MONITOR_WBD1_OVL_NO (0x0 << (RF_MONITOR_WBD1_OVL_INDEX))
#define RF_MONITOR_WBD1_OVL_YES (0x1 << (RF_MONITOR_WBD1_OVL_INDEX))

/* MONITOR BIT 1: LO PLL is Out of Lock control range                       */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_LO_OOL_INDEX (1)
#define RF_MONITOR_LO_OOL_MASK (0x2)
#define RF_MONITOR_LO_OOL_NO (0x0 << (RF_MONITOR_LO_OOL_INDEX))
#define RF_MONITOR_LO_OOL_YES (0x1 << (RF_MONITOR_LO_OOL_INDEX))

/* MONITOR BIT 0: VCO control range                                         */
/* symbolic RO field, reset to 0x0                                          */
#define RF_MONITOR_LO_VCO_INDEX (0)
#define RF_MONITOR_LO_VCO_MASK (0x1)
#define RF_MONITOR_LO_VCO_LOW (0x0 << (RF_MONITOR_LO_VCO_INDEX))
#define RF_MONITOR_LO_VCO_HIGH (0x1 << (RF_MONITOR_LO_VCO_INDEX))
#endif /* RF_MONITOR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/MONITOR: Monitoring of Analog     *
 * Status (asynchrone input events)                                         */
#define RF_MONITOR_RESET_VALUE (0x0000)
#define RF_MONITOR (RF_BASE_ADDR + RF_MONITOR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK1                         ***/
/* Sticky Monitor                                                           */
#ifndef RF_STCK1_PROTECT
#define RF_STCK1_PROTECT

#define RF_STCK1_OFFS (0x10C)

/* STCK1 BIT 14: reserved                                                   */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_MON_X_INDEX (14)
#define RF_STCK1_MON_X_MASK (0x4000)
#define RF_STCK1_MON_X_OK (0x0 << (RF_STCK1_MON_X_INDEX))
#define RF_STCK1_MON_X_ALARM (0x1 << (RF_STCK1_MON_X_INDEX))

/* STCK1 BIT 8: RCC Calibration for Low pass filter.  increase/decrease     *
 * RCC_T on next interation                                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_RCC_OUT_INDEX (8)
#define RF_STCK1_RCC_OUT_MASK (0x100)
#define RF_STCK1_RCC_OUT_DECREM (0x0 << (RF_STCK1_RCC_OUT_INDEX))
#define RF_STCK1_RCC_OUT_INCREM (0x1 << (RF_STCK1_RCC_OUT_INDEX))

/* STCK1 BIT 7: LPF Overload                                                */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_LPF_OVL_INDEX (7)
#define RF_STCK1_LPF_OVL_MASK (0x80)
#define RF_STCK1_LPF_OVL_NO (0x0 << (RF_STCK1_LPF_OVL_INDEX))
#define RF_STCK1_LPF_OVL_YES (0x1 << (RF_STCK1_LPF_OVL_INDEX))

/* STCK1 BIT 6: Output Buffer Overload                                      */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_OBUF_OVL_INDEX (6)
#define RF_STCK1_OBUF_OVL_MASK (0x40)
#define RF_STCK1_OBUF_OVL_NO (0x0 << (RF_STCK1_OBUF_OVL_INDEX))
#define RF_STCK1_OBUF_OVL_YES (0x1 << (RF_STCK1_OBUF_OVL_INDEX))

/* STCK1 BIT 5: Mixer input overload                                        */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_WBD2_OVL_INDEX (5)
#define RF_STCK1_WBD2_OVL_MASK (0x20)
#define RF_STCK1_WBD2_OVL_NO (0x0 << (RF_STCK1_WBD2_OVL_INDEX))
#define RF_STCK1_WBD2_OVL_YES (0x1 << (RF_STCK1_WBD2_OVL_INDEX))

/* STCK1 BIT 4: LNA output overload (WBD1)                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_WBD1_OVL_INDEX (4)
#define RF_STCK1_WBD1_OVL_MASK (0x10)
#define RF_STCK1_WBD1_OVL_NO (0x0 << (RF_STCK1_WBD1_OVL_INDEX))
#define RF_STCK1_WBD1_OVL_YES (0x1 << (RF_STCK1_WBD1_OVL_INDEX))

/* STCK1 BIT 1: LO PLL is Out of Lock control range                         */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_LO_OOL_INDEX (1)
#define RF_STCK1_LO_OOL_MASK (0x2)
#define RF_STCK1_LO_OOL_NO (0x0 << (RF_STCK1_LO_OOL_INDEX))
#define RF_STCK1_LO_OOL_YES (0x1 << (RF_STCK1_LO_OOL_INDEX))

/* STCK1 BIT 0: VCO control range                                           */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK1_LO_VCO_INDEX (0)
#define RF_STCK1_LO_VCO_MASK (0x1)
#define RF_STCK1_LO_VCO_LOW (0x0 << (RF_STCK1_LO_VCO_INDEX))
#define RF_STCK1_LO_VCO_HIGH (0x1 << (RF_STCK1_LO_VCO_INDEX))
#endif /* RF_STCK1_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK1: Sticky Monitor             */
#define RF_STCK1_RESET_VALUE (0x0000)
#define RF_STCK1 (RF_BASE_ADDR + RF_STCK1_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK2                         ***/
/* Sticky Monitor                                                           */
#ifndef RF_STCK2_PROTECT
#define RF_STCK2_PROTECT

#define RF_STCK2_OFFS (0x110)

/* STCK2 BIT 14: reserved                                                   */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_MON_X_INDEX (14)
#define RF_STCK2_MON_X_MASK (0x4000)
#define RF_STCK2_MON_X_OK (0x0 << (RF_STCK2_MON_X_INDEX))
#define RF_STCK2_MON_X_ALARM (0x1 << (RF_STCK2_MON_X_INDEX))

/* STCK2 BIT 8: RCC Calibration for Low pass filter.  increase/decrease     *
 * RCC_T on next interation                                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_RCC_OUT_INDEX (8)
#define RF_STCK2_RCC_OUT_MASK (0x100)
#define RF_STCK2_RCC_OUT_DECREM (0x0 << (RF_STCK2_RCC_OUT_INDEX))
#define RF_STCK2_RCC_OUT_INCREM (0x1 << (RF_STCK2_RCC_OUT_INDEX))

/* STCK2 BIT 7: LPF Overload                                                */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_LPF_OVL_INDEX (7)
#define RF_STCK2_LPF_OVL_MASK (0x80)
#define RF_STCK2_LPF_OVL_NO (0x0 << (RF_STCK2_LPF_OVL_INDEX))
#define RF_STCK2_LPF_OVL_YES (0x1 << (RF_STCK2_LPF_OVL_INDEX))

/* STCK2 BIT 6: Output Buffer Overload                                      */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_OBUF_OVL_INDEX (6)
#define RF_STCK2_OBUF_OVL_MASK (0x40)
#define RF_STCK2_OBUF_OVL_NO (0x0 << (RF_STCK2_OBUF_OVL_INDEX))
#define RF_STCK2_OBUF_OVL_YES (0x1 << (RF_STCK2_OBUF_OVL_INDEX))

/* STCK2 BIT 5: Mixer input overload                                        */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_WBD2_OVL_INDEX (5)
#define RF_STCK2_WBD2_OVL_MASK (0x20)
#define RF_STCK2_WBD2_OVL_NO (0x0 << (RF_STCK2_WBD2_OVL_INDEX))
#define RF_STCK2_WBD2_OVL_YES (0x1 << (RF_STCK2_WBD2_OVL_INDEX))

/* STCK2 BIT 4: LNA output overload (WBD1)                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_WBD1_OVL_INDEX (4)
#define RF_STCK2_WBD1_OVL_MASK (0x10)
#define RF_STCK2_WBD1_OVL_NO (0x0 << (RF_STCK2_WBD1_OVL_INDEX))
#define RF_STCK2_WBD1_OVL_YES (0x1 << (RF_STCK2_WBD1_OVL_INDEX))

/* STCK2 BIT 1: LO PLL is Out of Lock control range                         */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_LO_OOL_INDEX (1)
#define RF_STCK2_LO_OOL_MASK (0x2)
#define RF_STCK2_LO_OOL_NO (0x0 << (RF_STCK2_LO_OOL_INDEX))
#define RF_STCK2_LO_OOL_YES (0x1 << (RF_STCK2_LO_OOL_INDEX))

/* STCK2 BIT 0: VCO control range                                           */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK2_LO_VCO_INDEX (0)
#define RF_STCK2_LO_VCO_MASK (0x1)
#define RF_STCK2_LO_VCO_LOW (0x0 << (RF_STCK2_LO_VCO_INDEX))
#define RF_STCK2_LO_VCO_HIGH (0x1 << (RF_STCK2_LO_VCO_INDEX))
#endif /* RF_STCK2_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK2: Sticky Monitor             */
#define RF_STCK2_RESET_VALUE (0x0000)
#define RF_STCK2 (RF_BASE_ADDR + RF_STCK2_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK3                         ***/
/* Sticky Monitor                                                           */
#ifndef RF_STCK3_PROTECT
#define RF_STCK3_PROTECT

#define RF_STCK3_OFFS (0x114)

/* STCK3 BIT 14: reserved                                                   */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_MON_X_INDEX (14)
#define RF_STCK3_MON_X_MASK (0x4000)
#define RF_STCK3_MON_X_OK (0x0 << (RF_STCK3_MON_X_INDEX))
#define RF_STCK3_MON_X_ALARM (0x1 << (RF_STCK3_MON_X_INDEX))

/* STCK3 BIT 8: RCC Calibration for Low pass filter.  increase/decrease     *
 * RCC_T on next interation                                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_RCC_OUT_INDEX (8)
#define RF_STCK3_RCC_OUT_MASK (0x100)
#define RF_STCK3_RCC_OUT_DECREM (0x0 << (RF_STCK3_RCC_OUT_INDEX))
#define RF_STCK3_RCC_OUT_INCREM (0x1 << (RF_STCK3_RCC_OUT_INDEX))

/* STCK3 BIT 7: LPF Overload                                                */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_LPF_OVL_INDEX (7)
#define RF_STCK3_LPF_OVL_MASK (0x80)
#define RF_STCK3_LPF_OVL_NO (0x0 << (RF_STCK3_LPF_OVL_INDEX))
#define RF_STCK3_LPF_OVL_YES (0x1 << (RF_STCK3_LPF_OVL_INDEX))

/* STCK3 BIT 6: Output Buffer Overload                                      */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_OBUF_OVL_INDEX (6)
#define RF_STCK3_OBUF_OVL_MASK (0x40)
#define RF_STCK3_OBUF_OVL_NO (0x0 << (RF_STCK3_OBUF_OVL_INDEX))
#define RF_STCK3_OBUF_OVL_YES (0x1 << (RF_STCK3_OBUF_OVL_INDEX))

/* STCK3 BIT 5: Mixer input overload                                        */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_WBD2_OVL_INDEX (5)
#define RF_STCK3_WBD2_OVL_MASK (0x20)
#define RF_STCK3_WBD2_OVL_NO (0x0 << (RF_STCK3_WBD2_OVL_INDEX))
#define RF_STCK3_WBD2_OVL_YES (0x1 << (RF_STCK3_WBD2_OVL_INDEX))

/* STCK3 BIT 4: LNA output overload (WBD1)                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_WBD1_OVL_INDEX (4)
#define RF_STCK3_WBD1_OVL_MASK (0x10)
#define RF_STCK3_WBD1_OVL_NO (0x0 << (RF_STCK3_WBD1_OVL_INDEX))
#define RF_STCK3_WBD1_OVL_YES (0x1 << (RF_STCK3_WBD1_OVL_INDEX))

/* STCK3 BIT 1: LO PLL is Out of Lock control range                         */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_LO_OOL_INDEX (1)
#define RF_STCK3_LO_OOL_MASK (0x2)
#define RF_STCK3_LO_OOL_NO (0x0 << (RF_STCK3_LO_OOL_INDEX))
#define RF_STCK3_LO_OOL_YES (0x1 << (RF_STCK3_LO_OOL_INDEX))

/* STCK3 BIT 0: VCO control range                                           */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK3_LO_VCO_INDEX (0)
#define RF_STCK3_LO_VCO_MASK (0x1)
#define RF_STCK3_LO_VCO_LOW (0x0 << (RF_STCK3_LO_VCO_INDEX))
#define RF_STCK3_LO_VCO_HIGH (0x1 << (RF_STCK3_LO_VCO_INDEX))
#endif /* RF_STCK3_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK3: Sticky Monitor             */
#define RF_STCK3_RESET_VALUE (0x0000)
#define RF_STCK3 (RF_BASE_ADDR + RF_STCK3_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK4                         ***/
/* Sticky Monitor                                                           */
#ifndef RF_STCK4_PROTECT
#define RF_STCK4_PROTECT

#define RF_STCK4_OFFS (0x118)

/* STCK4 BIT 14: reserved                                                   */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_MON_X_INDEX (14)
#define RF_STCK4_MON_X_MASK (0x4000)
#define RF_STCK4_MON_X_OK (0x0 << (RF_STCK4_MON_X_INDEX))
#define RF_STCK4_MON_X_ALARM (0x1 << (RF_STCK4_MON_X_INDEX))

/* STCK4 BIT 8: RCC Calibration for Low pass filter.  increase/decrease     *
 * RCC_T on next interation                                                 */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_RCC_OUT_INDEX (8)
#define RF_STCK4_RCC_OUT_MASK (0x100)
#define RF_STCK4_RCC_OUT_DECREM (0x0 << (RF_STCK4_RCC_OUT_INDEX))
#define RF_STCK4_RCC_OUT_INCREM (0x1 << (RF_STCK4_RCC_OUT_INDEX))

/* STCK4 BIT 7: LPF Overload                                                */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_LPF_OVL_INDEX (7)
#define RF_STCK4_LPF_OVL_MASK (0x80)
#define RF_STCK4_LPF_OVL_NO (0x0 << (RF_STCK4_LPF_OVL_INDEX))
#define RF_STCK4_LPF_OVL_YES (0x1 << (RF_STCK4_LPF_OVL_INDEX))

/* STCK4 BIT 6: Output Buffer Overload                                      */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_OBUF_OVL_INDEX (6)
#define RF_STCK4_OBUF_OVL_MASK (0x40)
#define RF_STCK4_OBUF_OVL_NO (0x0 << (RF_STCK4_OBUF_OVL_INDEX))
#define RF_STCK4_OBUF_OVL_YES (0x1 << (RF_STCK4_OBUF_OVL_INDEX))

/* STCK4 BIT 5: Mixer input overload                                        */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_WBD2_OVL_INDEX (5)
#define RF_STCK4_WBD2_OVL_MASK (0x20)
#define RF_STCK4_WBD2_OVL_NO (0x0 << (RF_STCK4_WBD2_OVL_INDEX))
#define RF_STCK4_WBD2_OVL_YES (0x1 << (RF_STCK4_WBD2_OVL_INDEX))

/* STCK4 BIT 4: LNA output overload (WBD1)                                  */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_WBD1_OVL_INDEX (4)
#define RF_STCK4_WBD1_OVL_MASK (0x10)
#define RF_STCK4_WBD1_OVL_NO (0x0 << (RF_STCK4_WBD1_OVL_INDEX))
#define RF_STCK4_WBD1_OVL_YES (0x1 << (RF_STCK4_WBD1_OVL_INDEX))

/* STCK4 BIT 1: LO PLL is Out of Lock control range                         */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_LO_OOL_INDEX (1)
#define RF_STCK4_LO_OOL_MASK (0x2)
#define RF_STCK4_LO_OOL_NO (0x0 << (RF_STCK4_LO_OOL_INDEX))
#define RF_STCK4_LO_OOL_YES (0x1 << (RF_STCK4_LO_OOL_INDEX))

/* STCK4 BIT 0: VCO control range                                           */
/* symbolic TO1 field, reset to 0x0                                         */
#define RF_STCK4_LO_VCO_INDEX (0)
#define RF_STCK4_LO_VCO_MASK (0x1)
#define RF_STCK4_LO_VCO_LOW (0x0 << (RF_STCK4_LO_VCO_INDEX))
#define RF_STCK4_LO_VCO_HIGH (0x1 << (RF_STCK4_LO_VCO_INDEX))
#endif /* RF_STCK4_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/STCK4: Sticky Monitor             */
#define RF_STCK4_RESET_VALUE (0x0000)
#define RF_STCK4 (RF_BASE_ADDR + RF_STCK4_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_OUT                        ***/
/* Sign Filter output                                                       */
#ifndef RF_SF_OUT_PROTECT
#define RF_SF_OUT_PROTECT

#define RF_SF_OUT_OFFS (0x120)

/* SF_OUT BITS 31..16: Sign output for Q channel, 2's complement            */
/* numeric RO signed (2s comlement) field, reset to 0x0                     */
#define RF_SF_OUT_Q_INDEX (16)
#define RF_SF_OUT_Q_MASK (0xFFFF0000)

/* SF_OUT BITS 15..0: Sign output for I channel, 2's complement             */
/* numeric RO signed (2s comlement) field, reset to 0x0                     */
#define RF_SF_OUT_I_INDEX (0)
#define RF_SF_OUT_I_MASK (0xFFFF)
#endif /* RF_SF_OUT_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/SF_OUT: Sign Filter output        */
#define RF_SF_OUT_RESET_VALUE (0x00000000)
#define RF_SF_OUT (RF_BASE_ADDR + RF_SF_OUT_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_OUT                     ***/
/* WBD Filter output                                                        */
#ifndef RF_WBDFI_OUT_PROTECT
#define RF_WBDFI_OUT_PROTECT

#define RF_WBDFI_OUT_OFFS (0x128)

/* WBDFI_OUT BITS 15..0: WBD filter output                                  */
/* numeric RO signed (2s comlement) field, reset to 0x0                     */
#define RF_WBDFI_OUT_I_INDEX (0)
#define RF_WBDFI_OUT_I_MASK (0xFFFF)
#endif /* RF_WBDFI_OUT_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/RF/WBDFI_OUT: WBD Filter output      */
#define RF_WBDFI_OUT_RESET_VALUE (0x0000)
#define RF_WBDFI_OUT (RF_BASE_ADDR + RF_WBDFI_OUT_OFFS)

#define RF_SIZE (0x129)

/*** Timer Data and Control Registers                                     ***/
#define TIMER_BASE_ADDR (0x820C00)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMLDR                     ***/
/* Timer Reload Register                                                    */
#ifndef TIMER_TIMLDR_PROTECT
#define TIMER_TIMLDR_PROTECT

#define TIMER_TIMLDR_OFFS (0x0)

/* TIMLDR BITS 31..0: Timer reload value                                    */
/* numeric RW unsigned field, reset to 0x0                                  */
#define TIMER_TIMLDR_RELOAD_INDEX (0)
#define TIMER_TIMLDR_RELOAD_MASK (0xFFFFFFFF)
#endif /* TIMER_TIMLDR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMLDR: Timer Reload Register  */
#define TIMER_TIMLDR_RESET_VALUE (0x00000000)
#define TIMER_TIMLDR (TIMER_BASE_ADDR + TIMER_TIMLDR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMCAR                     ***/
/* Timer Capture Register                                                   */
#ifndef TIMER_TIMCAR_PROTECT
#define TIMER_TIMCAR_PROTECT

#define TIMER_TIMCAR_OFFS (0x4)

/* TIMCAR BITS 31..0: Timer capture value                                   */
/* numeric RO unsigned field, reset to 0x0                                  */
#define TIMER_TIMCAR_CAPTURE_INDEX (0)
#define TIMER_TIMCAR_CAPTURE_MASK (0xFFFFFFFF)
#endif /* TIMER_TIMCAR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMCAR: Timer Capture Register */
#define TIMER_TIMCAR_RESET_VALUE (0x00000000)
#define TIMER_TIMCAR (TIMER_BASE_ADDR + TIMER_TIMCAR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMCMPR                    ***/
/* Timer Compare Register                                                   */
#ifndef TIMER_TIMCMPR_PROTECT
#define TIMER_TIMCMPR_PROTECT

#define TIMER_TIMCMPR_OFFS (0x8)

/* TIMCMPR BITS 31..0: Timer compare value                                  */
/* numeric RW unsigned field, reset to 0x0                                  */
#define TIMER_TIMCMPR_COMPARE_INDEX (0)
#define TIMER_TIMCMPR_COMPARE_MASK (0xFFFFFFFF)
#endif /* TIMER_TIMCMPR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMCMPR: Timer Compare Register */
#define TIMER_TIMCMPR_RESET_VALUE (0x00000000)
#define TIMER_TIMCMPR (TIMER_BASE_ADDR + TIMER_TIMCMPR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMSCR                     ***/
/* Timer Status and Control Register                                        */
#ifndef TIMER_TIMSCR_PROTECT
#define TIMER_TIMSCR_PROTECT

#define TIMER_TIMSCR_OFFS (0xC)

/* TIMSCR BIT 15: Timer function mode                                       */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMTM_INDEX (15)
#define TIMER_TIMSCR_TIMTM_MASK (0x8000)
#define TIMER_TIMSCR_TIMTM_OUTCMP (0x0 << (TIMER_TIMSCR_TIMTM_INDEX))
#define TIMER_TIMSCR_TIMTM_INCAP (0x1 << (TIMER_TIMSCR_TIMTM_INDEX))

/* TIMSCR BIT 11: Timer output mode                                         */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMOM_INDEX (11)
#define TIMER_TIMSCR_TIMOM_MASK (0x800)
#define TIMER_TIMSCR_TIMOM_LEVEL (0x0 << (TIMER_TIMSCR_TIMOM_INDEX))
#define TIMER_TIMSCR_TIMOM_PULSE (0x1 << (TIMER_TIMSCR_TIMOM_INDEX))

/* TIMSCR BIT 10: Interrupt pending flag                                    */
/* symbolic TO1 field, reset to 0x0                                         */
#define TIMER_TIMSCR_TIMCF_INDEX (10)
#define TIMER_TIMSCR_TIMCF_MASK (0x400)
#define TIMER_TIMSCR_TIMCF_IDLE (0x0 << (TIMER_TIMSCR_TIMCF_INDEX))
#define TIMER_TIMSCR_TIMCF_PENDING (0x1 << (TIMER_TIMSCR_TIMCF_INDEX))

/* TIMSCR BIT 9: Compare flag Interrupt enable                              */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMCFIE_INDEX (9)
#define TIMER_TIMSCR_TIMCFIE_MASK (0x200)
#define TIMER_TIMSCR_TIMCFIE_DISABLE (0x0 << (TIMER_TIMSCR_TIMCFIE_INDEX))
#define TIMER_TIMSCR_TIMCFIE_ENABLE (0x1 << (TIMER_TIMSCR_TIMCFIE_INDEX))

/* TIMSCR BIT 8: Overflow flag                                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define TIMER_TIMSCR_TIMOF_INDEX (8)
#define TIMER_TIMSCR_TIMOF_MASK (0x100)
#define TIMER_TIMSCR_TIMOF_NO (0x0 << (TIMER_TIMSCR_TIMOF_INDEX))
#define TIMER_TIMSCR_TIMOF_YES (0x1 << (TIMER_TIMSCR_TIMOF_INDEX))

/* TIMSCR BIT 7: Force capture                                              */
/* symbolic WO field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMCPT_INDEX (7)
#define TIMER_TIMSCR_TIMCPT_MASK (0x80)
#define TIMER_TIMSCR_TIMCPT_NO (0x0 << (TIMER_TIMSCR_TIMCPT_INDEX))
#define TIMER_TIMSCR_TIMCPT_FORCE (0x1 << (TIMER_TIMSCR_TIMCPT_INDEX))

/* TIMSCR BIT 6: Capture disable                                            */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMCDIS_INDEX (6)
#define TIMER_TIMSCR_TIMCDIS_MASK (0x40)
#define TIMER_TIMSCR_TIMCDIS_ENABLE (0x0 << (TIMER_TIMSCR_TIMCDIS_INDEX))
#define TIMER_TIMSCR_TIMCDIS_DISABLE (0x1 << (TIMER_TIMSCR_TIMCDIS_INDEX))

/* TIMSCR BITS 5..4: Timer counter source                                   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define TIMER_TIMSCR_TIMCS_INDEX (4)
#define TIMER_TIMSCR_TIMCS_MASK (0x30)

/* TIMSCR BIT 3: Timer initialization                                       */
/* symbolic WO field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMCI_INDEX (3)
#define TIMER_TIMSCR_TIMCI_MASK (0x8)
#define TIMER_TIMSCR_TIMCI_NO (0x0 << (TIMER_TIMSCR_TIMCI_INDEX))
#define TIMER_TIMSCR_TIMCI_FORCE (0x1 << (TIMER_TIMSCR_TIMCI_INDEX))

/* TIMSCR BIT 2: Count once or loop back                                    */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMCO_INDEX (2)
#define TIMER_TIMSCR_TIMCO_MASK (0x4)
#define TIMER_TIMSCR_TIMCO_LOOP (0x0 << (TIMER_TIMSCR_TIMCO_INDEX))
#define TIMER_TIMSCR_TIMCO_COUNTONCE (0x1 << (TIMER_TIMSCR_TIMCO_INDEX))

/* TIMSCR BIT 1: Count direction                                            */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMDIR_INDEX (1)
#define TIMER_TIMSCR_TIMDIR_MASK (0x2)
#define TIMER_TIMSCR_TIMDIR_DOWN (0x0 << (TIMER_TIMSCR_TIMDIR_INDEX))
#define TIMER_TIMSCR_TIMDIR_UP (0x1 << (TIMER_TIMSCR_TIMDIR_INDEX))

/* TIMSCR BIT 0: Timer enable                                               */
/* symbolic RW field, reset to 0x0                                          */
#define TIMER_TIMSCR_TIMEN_INDEX (0)
#define TIMER_TIMSCR_TIMEN_MASK (0x1)
#define TIMER_TIMSCR_TIMEN_DISABLE (0x0 << (TIMER_TIMSCR_TIMEN_INDEX))
#define TIMER_TIMSCR_TIMEN_ENABLE (0x1 << (TIMER_TIMSCR_TIMEN_INDEX))
#endif /* TIMER_TIMSCR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMSCR: Timer Status and       *
 * Control Register                                                         */
#define TIMER_TIMSCR_RESET_VALUE (0x0000)
#define TIMER_TIMSCR (TIMER_BASE_ADDR + TIMER_TIMSCR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMPRSC                    ***/
/* Timer Prescaler Register                                                 */
#ifndef TIMER_TIMPRSC_PROTECT
#define TIMER_TIMPRSC_PROTECT

#define TIMER_TIMPRSC_OFFS (0x10)

/* TIMPRSC BITS 14..0: Timer prescaler value                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define TIMER_TIMPRSC_TIMMOD_INDEX (0)
#define TIMER_TIMPRSC_TIMMOD_MASK (0x7FFF)
#endif /* TIMER_TIMPRSC_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/TIMER/TIMPRSC: Timer Prescaler Register */
#define TIMER_TIMPRSC_RESET_VALUE (0x0000)
#define TIMER_TIMPRSC (TIMER_BASE_ADDR + TIMER_TIMPRSC_OFFS)

#define TIMER_SIZE (0x11)

/*** GPIO Ports Control Registers                                         ***/
#define GPIO0_BASE_ADDR (0x821000)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/DDR                        ***/
/* Data Direction Register                                                  */
#define GPIO0_DDR_OFFS (0x0)
#ifndef GPIO_DDR_PROTECT
#define GPIO_DDR_PROTECT

#define GPIO_DDR_OFFS (0x0)

/* DDR BIT 31: Data direction for bit 31 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD31_INDEX (31)
#define GPIO_DDR_DD31_MASK (0x80000000)
#define GPIO_DDR_DD31_IN (0x0 << (GPIO_DDR_DD31_INDEX))
#define GPIO_DDR_DD31_OUT (0x1 << (GPIO_DDR_DD31_INDEX))

/* DDR BIT 30: Data direction for bit 30 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD30_INDEX (30)
#define GPIO_DDR_DD30_MASK (0x40000000)
#define GPIO_DDR_DD30_IN (0x0 << (GPIO_DDR_DD30_INDEX))
#define GPIO_DDR_DD30_OUT (0x1 << (GPIO_DDR_DD30_INDEX))

/* DDR BIT 29: Data direction for bit 19 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD29_INDEX (29)
#define GPIO_DDR_DD29_MASK (0x20000000)
#define GPIO_DDR_DD29_IN (0x0 << (GPIO_DDR_DD29_INDEX))
#define GPIO_DDR_DD29_OUT (0x1 << (GPIO_DDR_DD29_INDEX))

/* DDR BIT 28: Data direction for bit 28 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD28_INDEX (28)
#define GPIO_DDR_DD28_MASK (0x10000000)
#define GPIO_DDR_DD28_IN (0x0 << (GPIO_DDR_DD28_INDEX))
#define GPIO_DDR_DD28_OUT (0x1 << (GPIO_DDR_DD28_INDEX))

/* DDR BIT 27: Data direction for bit 27 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD27_INDEX (27)
#define GPIO_DDR_DD27_MASK (0x8000000)
#define GPIO_DDR_DD27_IN (0x0 << (GPIO_DDR_DD27_INDEX))
#define GPIO_DDR_DD27_OUT (0x1 << (GPIO_DDR_DD27_INDEX))

/* DDR BIT 26: Data direction for bit 26 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD26_INDEX (26)
#define GPIO_DDR_DD26_MASK (0x4000000)
#define GPIO_DDR_DD26_IN (0x0 << (GPIO_DDR_DD26_INDEX))
#define GPIO_DDR_DD26_OUT (0x1 << (GPIO_DDR_DD26_INDEX))

/* DDR BIT 25: Data direction for bit 25 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD25_INDEX (25)
#define GPIO_DDR_DD25_MASK (0x2000000)
#define GPIO_DDR_DD25_IN (0x0 << (GPIO_DDR_DD25_INDEX))
#define GPIO_DDR_DD25_OUT (0x1 << (GPIO_DDR_DD25_INDEX))

/* DDR BIT 24: Data direction for bit 24 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD24_INDEX (24)
#define GPIO_DDR_DD24_MASK (0x1000000)
#define GPIO_DDR_DD24_IN (0x0 << (GPIO_DDR_DD24_INDEX))
#define GPIO_DDR_DD24_OUT (0x1 << (GPIO_DDR_DD24_INDEX))

/* DDR BIT 23: Data direction for bit 23 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD23_INDEX (23)
#define GPIO_DDR_DD23_MASK (0x800000)
#define GPIO_DDR_DD23_IN (0x0 << (GPIO_DDR_DD23_INDEX))
#define GPIO_DDR_DD23_OUT (0x1 << (GPIO_DDR_DD23_INDEX))

/* DDR BIT 22: Data direction for bit 22 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD22_INDEX (22)
#define GPIO_DDR_DD22_MASK (0x400000)
#define GPIO_DDR_DD22_IN (0x0 << (GPIO_DDR_DD22_INDEX))
#define GPIO_DDR_DD22_OUT (0x1 << (GPIO_DDR_DD22_INDEX))

/* DDR BIT 21: Data direction for bit 21 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD21_INDEX (21)
#define GPIO_DDR_DD21_MASK (0x200000)
#define GPIO_DDR_DD21_IN (0x0 << (GPIO_DDR_DD21_INDEX))
#define GPIO_DDR_DD21_OUT (0x1 << (GPIO_DDR_DD21_INDEX))

/* DDR BIT 20: Data direction for bit 20 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD20_INDEX (20)
#define GPIO_DDR_DD20_MASK (0x100000)
#define GPIO_DDR_DD20_IN (0x0 << (GPIO_DDR_DD20_INDEX))
#define GPIO_DDR_DD20_OUT (0x1 << (GPIO_DDR_DD20_INDEX))

/* DDR BIT 19: Data direction for bit 19 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD19_INDEX (19)
#define GPIO_DDR_DD19_MASK (0x80000)
#define GPIO_DDR_DD19_IN (0x0 << (GPIO_DDR_DD19_INDEX))
#define GPIO_DDR_DD19_OUT (0x1 << (GPIO_DDR_DD19_INDEX))

/* DDR BIT 18: Data direction for bit 18 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD18_INDEX (18)
#define GPIO_DDR_DD18_MASK (0x40000)
#define GPIO_DDR_DD18_IN (0x0 << (GPIO_DDR_DD18_INDEX))
#define GPIO_DDR_DD18_OUT (0x1 << (GPIO_DDR_DD18_INDEX))

/* DDR BIT 17: Data direction for bit 17 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD17_INDEX (17)
#define GPIO_DDR_DD17_MASK (0x20000)
#define GPIO_DDR_DD17_IN (0x0 << (GPIO_DDR_DD17_INDEX))
#define GPIO_DDR_DD17_OUT (0x1 << (GPIO_DDR_DD17_INDEX))

/* DDR BIT 16: Data direction for bit 16 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD16_INDEX (16)
#define GPIO_DDR_DD16_MASK (0x10000)
#define GPIO_DDR_DD16_IN (0x0 << (GPIO_DDR_DD16_INDEX))
#define GPIO_DDR_DD16_OUT (0x1 << (GPIO_DDR_DD16_INDEX))

/* DDR BIT 15: Data direction for bit 15 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD15_INDEX (15)
#define GPIO_DDR_DD15_MASK (0x8000)
#define GPIO_DDR_DD15_IN (0x0 << (GPIO_DDR_DD15_INDEX))
#define GPIO_DDR_DD15_OUT (0x1 << (GPIO_DDR_DD15_INDEX))

/* DDR BIT 14: Data direction for bit 14 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD14_INDEX (14)
#define GPIO_DDR_DD14_MASK (0x4000)
#define GPIO_DDR_DD14_IN (0x0 << (GPIO_DDR_DD14_INDEX))
#define GPIO_DDR_DD14_OUT (0x1 << (GPIO_DDR_DD14_INDEX))

/* DDR BIT 13: Data direction for bit 13 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD13_INDEX (13)
#define GPIO_DDR_DD13_MASK (0x2000)
#define GPIO_DDR_DD13_IN (0x0 << (GPIO_DDR_DD13_INDEX))
#define GPIO_DDR_DD13_OUT (0x1 << (GPIO_DDR_DD13_INDEX))

/* DDR BIT 12: Data direction for bit 12 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD12_INDEX (12)
#define GPIO_DDR_DD12_MASK (0x1000)
#define GPIO_DDR_DD12_IN (0x0 << (GPIO_DDR_DD12_INDEX))
#define GPIO_DDR_DD12_OUT (0x1 << (GPIO_DDR_DD12_INDEX))

/* DDR BIT 11: Data direction for bit 11 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD11_INDEX (11)
#define GPIO_DDR_DD11_MASK (0x800)
#define GPIO_DDR_DD11_IN (0x0 << (GPIO_DDR_DD11_INDEX))
#define GPIO_DDR_DD11_OUT (0x1 << (GPIO_DDR_DD11_INDEX))

/* DDR BIT 10: Data direction for bit 10 of GPIO                            */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD10_INDEX (10)
#define GPIO_DDR_DD10_MASK (0x400)
#define GPIO_DDR_DD10_IN (0x0 << (GPIO_DDR_DD10_INDEX))
#define GPIO_DDR_DD10_OUT (0x1 << (GPIO_DDR_DD10_INDEX))

/* DDR BIT 9: Data direction for bit  9 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD9_INDEX (9)
#define GPIO_DDR_DD9_MASK (0x200)
#define GPIO_DDR_DD9_IN (0x0 << (GPIO_DDR_DD9_INDEX))
#define GPIO_DDR_DD9_OUT (0x1 << (GPIO_DDR_DD9_INDEX))

/* DDR BIT 8: Data direction for bit  8 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD8_INDEX (8)
#define GPIO_DDR_DD8_MASK (0x100)
#define GPIO_DDR_DD8_IN (0x0 << (GPIO_DDR_DD8_INDEX))
#define GPIO_DDR_DD8_OUT (0x1 << (GPIO_DDR_DD8_INDEX))

/* DDR BIT 7: Data direction for bit  7 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD7_INDEX (7)
#define GPIO_DDR_DD7_MASK (0x80)
#define GPIO_DDR_DD7_IN (0x0 << (GPIO_DDR_DD7_INDEX))
#define GPIO_DDR_DD7_OUT (0x1 << (GPIO_DDR_DD7_INDEX))

/* DDR BIT 6: Data direction for bit  6 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD6_INDEX (6)
#define GPIO_DDR_DD6_MASK (0x40)
#define GPIO_DDR_DD6_IN (0x0 << (GPIO_DDR_DD6_INDEX))
#define GPIO_DDR_DD6_OUT (0x1 << (GPIO_DDR_DD6_INDEX))

/* DDR BIT 5: Data direction for bit  5 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD5_INDEX (5)
#define GPIO_DDR_DD5_MASK (0x20)
#define GPIO_DDR_DD5_IN (0x0 << (GPIO_DDR_DD5_INDEX))
#define GPIO_DDR_DD5_OUT (0x1 << (GPIO_DDR_DD5_INDEX))

/* DDR BIT 4: Data direction for bit  4 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD4_INDEX (4)
#define GPIO_DDR_DD4_MASK (0x10)
#define GPIO_DDR_DD4_IN (0x0 << (GPIO_DDR_DD4_INDEX))
#define GPIO_DDR_DD4_OUT (0x1 << (GPIO_DDR_DD4_INDEX))

/* DDR BIT 3: Data direction for bit  3 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD3_INDEX (3)
#define GPIO_DDR_DD3_MASK (0x8)
#define GPIO_DDR_DD3_IN (0x0 << (GPIO_DDR_DD3_INDEX))
#define GPIO_DDR_DD3_OUT (0x1 << (GPIO_DDR_DD3_INDEX))

/* DDR BIT 2: Data direction for bit  2 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD2_INDEX (2)
#define GPIO_DDR_DD2_MASK (0x4)
#define GPIO_DDR_DD2_IN (0x0 << (GPIO_DDR_DD2_INDEX))
#define GPIO_DDR_DD2_OUT (0x1 << (GPIO_DDR_DD2_INDEX))

/* DDR BIT 1: Data direction for bit  1 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD1_INDEX (1)
#define GPIO_DDR_DD1_MASK (0x2)
#define GPIO_DDR_DD1_IN (0x0 << (GPIO_DDR_DD1_INDEX))
#define GPIO_DDR_DD1_OUT (0x1 << (GPIO_DDR_DD1_INDEX))

/* DDR BIT 0: Data direction for bit  0 of GPIO                             */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_DDR_DD0_INDEX (0)
#define GPIO_DDR_DD0_MASK (0x1)
#define GPIO_DDR_DD0_IN (0x0 << (GPIO_DDR_DD0_INDEX))
#define GPIO_DDR_DD0_OUT (0x1 << (GPIO_DDR_DD0_INDEX))
#endif /* GPIO_DDR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/DDR: Data Direction Register   */
#define GPIO0_DDR_RESET_VALUE (0x00000000)
#define GPIO0_DDR (GPIO0_BASE_ADDR + GPIO_DDR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/DATA                       ***/
/* Data Register                                                            */
#define GPIO0_DATA_OFFS (0x4)
#ifndef GPIO_DATA_PROTECT
#define GPIO_DATA_PROTECT

#define GPIO_DATA_OFFS (0x4)

/* DATA BIT 31: Value of bit 31 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT31_INDEX (31)
#define GPIO_DATA_DAT31_MASK (0x80000000)

/* DATA BIT 30: Value of bit 30 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT30_INDEX (30)
#define GPIO_DATA_DAT30_MASK (0x40000000)

/* DATA BIT 29: Value of bit 29 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT29_INDEX (29)
#define GPIO_DATA_DAT29_MASK (0x20000000)

/* DATA BIT 28: Value of bit 28 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT28_INDEX (28)
#define GPIO_DATA_DAT28_MASK (0x10000000)

/* DATA BIT 27: Value of bit 27 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT27_INDEX (27)
#define GPIO_DATA_DAT27_MASK (0x8000000)

/* DATA BIT 26: Value of bit 26 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT26_INDEX (26)
#define GPIO_DATA_DAT26_MASK (0x4000000)

/* DATA BIT 25: Value of bit 25 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT25_INDEX (25)
#define GPIO_DATA_DAT25_MASK (0x2000000)

/* DATA BIT 24: Value of bit 24 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT24_INDEX (24)
#define GPIO_DATA_DAT24_MASK (0x1000000)

/* DATA BIT 23: Value of bit 23 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT23_INDEX (23)
#define GPIO_DATA_DAT23_MASK (0x800000)

/* DATA BIT 22: Value of bit 22 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT22_INDEX (22)
#define GPIO_DATA_DAT22_MASK (0x400000)

/* DATA BIT 21: Value of bit 21 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT21_INDEX (21)
#define GPIO_DATA_DAT21_MASK (0x200000)

/* DATA BIT 20: Value of bit 20 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT20_INDEX (20)
#define GPIO_DATA_DAT20_MASK (0x100000)

/* DATA BIT 19: Value of bit 19 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT19_INDEX (19)
#define GPIO_DATA_DAT19_MASK (0x80000)

/* DATA BIT 18: Value of bit 18 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT18_INDEX (18)
#define GPIO_DATA_DAT18_MASK (0x40000)

/* DATA BIT 17: Value of bit 17 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT17_INDEX (17)
#define GPIO_DATA_DAT17_MASK (0x20000)

/* DATA BIT 16: Value of bit 16 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT16_INDEX (16)
#define GPIO_DATA_DAT16_MASK (0x10000)

/* DATA BIT 15: Value of bit 15 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT15_INDEX (15)
#define GPIO_DATA_DAT15_MASK (0x8000)

/* DATA BIT 14: Value of bit 14 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT14_INDEX (14)
#define GPIO_DATA_DAT14_MASK (0x4000)

/* DATA BIT 13: Value of bit 13 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT13_INDEX (13)
#define GPIO_DATA_DAT13_MASK (0x2000)

/* DATA BIT 12: Value of bit 12 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT12_INDEX (12)
#define GPIO_DATA_DAT12_MASK (0x1000)

/* DATA BIT 11: Value of bit 11 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT11_INDEX (11)
#define GPIO_DATA_DAT11_MASK (0x800)

/* DATA BIT 10: Value of bit 10 of GPIO                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT10_INDEX (10)
#define GPIO_DATA_DAT10_MASK (0x400)

/* DATA BIT 9: Value of bit  9 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT9_INDEX (9)
#define GPIO_DATA_DAT9_MASK (0x200)

/* DATA BIT 8: Value of bit  8 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT8_INDEX (8)
#define GPIO_DATA_DAT8_MASK (0x100)

/* DATA BIT 7: Value of bit  7 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT7_INDEX (7)
#define GPIO_DATA_DAT7_MASK (0x80)

/* DATA BIT 6: Value of bit  6 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT6_INDEX (6)
#define GPIO_DATA_DAT6_MASK (0x40)

/* DATA BIT 5: Value of bit  5 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT5_INDEX (5)
#define GPIO_DATA_DAT5_MASK (0x20)

/* DATA BIT 4: Value of bit  4 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT4_INDEX (4)
#define GPIO_DATA_DAT4_MASK (0x10)

/* DATA BIT 3: Value of bit  3 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT3_INDEX (3)
#define GPIO_DATA_DAT3_MASK (0x8)

/* DATA BIT 2: Value of bit  2 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT2_INDEX (2)
#define GPIO_DATA_DAT2_MASK (0x4)

/* DATA BIT 1: Value of bit  1 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT1_INDEX (1)
#define GPIO_DATA_DAT1_MASK (0x2)

/* DATA BIT 0: Value of bit  0 of GPIO                                      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define GPIO_DATA_DAT0_INDEX (0)
#define GPIO_DATA_DAT0_MASK (0x1)
#endif /* GPIO_DATA_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/DATA: Data Register            */
#define GPIO0_DATA_RESET_VALUE (0x00000000)
#define GPIO0_DATA (GPIO0_BASE_ADDR + GPIO_DATA_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/INT_EN                     ***/
/* GPIO Interrupt Enable Register                                           */
#define GPIO0_INT_EN_OFFS (0x8)
#ifndef GPIO_INT_EN_PROTECT
#define GPIO_INT_EN_PROTECT

#define GPIO_INT_EN_OFFS (0x8)

/* INT_EN BIT 31: Interrupt enable for bit 31 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN31_INDEX (31)
#define GPIO_INT_EN_INTEN31_MASK (0x80000000)
#define GPIO_INT_EN_INTEN31_DISABLED (0x0 << (GPIO_INT_EN_INTEN31_INDEX))
#define GPIO_INT_EN_INTEN31_ENABLED (0x1 << (GPIO_INT_EN_INTEN31_INDEX))

/* INT_EN BIT 30: Interrupt enable for bit 30 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN30_INDEX (30)
#define GPIO_INT_EN_INTEN30_MASK (0x40000000)
#define GPIO_INT_EN_INTEN30_DISABLED (0x0 << (GPIO_INT_EN_INTEN30_INDEX))
#define GPIO_INT_EN_INTEN30_ENABLED (0x1 << (GPIO_INT_EN_INTEN30_INDEX))

/* INT_EN BIT 29: Interrupt enable for bit 29 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN29_INDEX (29)
#define GPIO_INT_EN_INTEN29_MASK (0x20000000)
#define GPIO_INT_EN_INTEN29_DISABLED (0x0 << (GPIO_INT_EN_INTEN29_INDEX))
#define GPIO_INT_EN_INTEN29_ENABLED (0x1 << (GPIO_INT_EN_INTEN29_INDEX))

/* INT_EN BIT 28: Interrupt enable for bit 28 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN28_INDEX (28)
#define GPIO_INT_EN_INTEN28_MASK (0x10000000)
#define GPIO_INT_EN_INTEN28_DISABLED (0x0 << (GPIO_INT_EN_INTEN28_INDEX))
#define GPIO_INT_EN_INTEN28_ENABLED (0x1 << (GPIO_INT_EN_INTEN28_INDEX))

/* INT_EN BIT 27: Interrupt enable for bit 27 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN27_INDEX (27)
#define GPIO_INT_EN_INTEN27_MASK (0x8000000)
#define GPIO_INT_EN_INTEN27_DISABLED (0x0 << (GPIO_INT_EN_INTEN27_INDEX))
#define GPIO_INT_EN_INTEN27_ENABLED (0x1 << (GPIO_INT_EN_INTEN27_INDEX))

/* INT_EN BIT 26: Interrupt enable for bit 26 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN26_INDEX (26)
#define GPIO_INT_EN_INTEN26_MASK (0x4000000)
#define GPIO_INT_EN_INTEN26_DISABLED (0x0 << (GPIO_INT_EN_INTEN26_INDEX))
#define GPIO_INT_EN_INTEN26_ENABLED (0x1 << (GPIO_INT_EN_INTEN26_INDEX))

/* INT_EN BIT 25: Interrupt enable for bit 25 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN25_INDEX (25)
#define GPIO_INT_EN_INTEN25_MASK (0x2000000)
#define GPIO_INT_EN_INTEN25_DISABLED (0x0 << (GPIO_INT_EN_INTEN25_INDEX))
#define GPIO_INT_EN_INTEN25_ENABLED (0x1 << (GPIO_INT_EN_INTEN25_INDEX))

/* INT_EN BIT 24: Interrupt enable for bit 24 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN24_INDEX (24)
#define GPIO_INT_EN_INTEN24_MASK (0x1000000)
#define GPIO_INT_EN_INTEN24_DISABLED (0x0 << (GPIO_INT_EN_INTEN24_INDEX))
#define GPIO_INT_EN_INTEN24_ENABLED (0x1 << (GPIO_INT_EN_INTEN24_INDEX))

/* INT_EN BIT 23: Interrupt enable for bit 23 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN23_INDEX (23)
#define GPIO_INT_EN_INTEN23_MASK (0x800000)
#define GPIO_INT_EN_INTEN23_DISABLED (0x0 << (GPIO_INT_EN_INTEN23_INDEX))
#define GPIO_INT_EN_INTEN23_ENABLED (0x1 << (GPIO_INT_EN_INTEN23_INDEX))

/* INT_EN BIT 22: Interrupt enable for bit 22 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN22_INDEX (22)
#define GPIO_INT_EN_INTEN22_MASK (0x400000)
#define GPIO_INT_EN_INTEN22_DISABLED (0x0 << (GPIO_INT_EN_INTEN22_INDEX))
#define GPIO_INT_EN_INTEN22_ENABLED (0x1 << (GPIO_INT_EN_INTEN22_INDEX))

/* INT_EN BIT 21: Interrupt enable for bit 21 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN21_INDEX (21)
#define GPIO_INT_EN_INTEN21_MASK (0x200000)
#define GPIO_INT_EN_INTEN21_DISABLED (0x0 << (GPIO_INT_EN_INTEN21_INDEX))
#define GPIO_INT_EN_INTEN21_ENABLED (0x1 << (GPIO_INT_EN_INTEN21_INDEX))

/* INT_EN BIT 20: Interrupt enable for bit 20 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN20_INDEX (20)
#define GPIO_INT_EN_INTEN20_MASK (0x100000)
#define GPIO_INT_EN_INTEN20_DISABLED (0x0 << (GPIO_INT_EN_INTEN20_INDEX))
#define GPIO_INT_EN_INTEN20_ENABLED (0x1 << (GPIO_INT_EN_INTEN20_INDEX))

/* INT_EN BIT 19: Interrupt enable for bit 19 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN19_INDEX (19)
#define GPIO_INT_EN_INTEN19_MASK (0x80000)
#define GPIO_INT_EN_INTEN19_DISABLED (0x0 << (GPIO_INT_EN_INTEN19_INDEX))
#define GPIO_INT_EN_INTEN19_ENABLED (0x1 << (GPIO_INT_EN_INTEN19_INDEX))

/* INT_EN BIT 18: Interrupt enable for bit 18 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN18_INDEX (18)
#define GPIO_INT_EN_INTEN18_MASK (0x40000)
#define GPIO_INT_EN_INTEN18_DISABLED (0x0 << (GPIO_INT_EN_INTEN18_INDEX))
#define GPIO_INT_EN_INTEN18_ENABLED (0x1 << (GPIO_INT_EN_INTEN18_INDEX))

/* INT_EN BIT 17: Interrupt enable for bit 17 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN17_INDEX (17)
#define GPIO_INT_EN_INTEN17_MASK (0x20000)
#define GPIO_INT_EN_INTEN17_DISABLED (0x0 << (GPIO_INT_EN_INTEN17_INDEX))
#define GPIO_INT_EN_INTEN17_ENABLED (0x1 << (GPIO_INT_EN_INTEN17_INDEX))

/* INT_EN BIT 16: Interrupt enable for bit 16 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN16_INDEX (16)
#define GPIO_INT_EN_INTEN16_MASK (0x10000)
#define GPIO_INT_EN_INTEN16_DISABLED (0x0 << (GPIO_INT_EN_INTEN16_INDEX))
#define GPIO_INT_EN_INTEN16_ENABLED (0x1 << (GPIO_INT_EN_INTEN16_INDEX))

/* INT_EN BIT 15: Interrupt enable for bit 15 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN15_INDEX (15)
#define GPIO_INT_EN_INTEN15_MASK (0x8000)
#define GPIO_INT_EN_INTEN15_DISABLED (0x0 << (GPIO_INT_EN_INTEN15_INDEX))
#define GPIO_INT_EN_INTEN15_ENABLED (0x1 << (GPIO_INT_EN_INTEN15_INDEX))

/* INT_EN BIT 14: Interrupt enable for bit 14 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN14_INDEX (14)
#define GPIO_INT_EN_INTEN14_MASK (0x4000)
#define GPIO_INT_EN_INTEN14_DISABLED (0x0 << (GPIO_INT_EN_INTEN14_INDEX))
#define GPIO_INT_EN_INTEN14_ENABLED (0x1 << (GPIO_INT_EN_INTEN14_INDEX))

/* INT_EN BIT 13: Interrupt enable for bit 13 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN13_INDEX (13)
#define GPIO_INT_EN_INTEN13_MASK (0x2000)
#define GPIO_INT_EN_INTEN13_DISABLED (0x0 << (GPIO_INT_EN_INTEN13_INDEX))
#define GPIO_INT_EN_INTEN13_ENABLED (0x1 << (GPIO_INT_EN_INTEN13_INDEX))

/* INT_EN BIT 12: Interrupt enable for bit 12 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN12_INDEX (12)
#define GPIO_INT_EN_INTEN12_MASK (0x1000)
#define GPIO_INT_EN_INTEN12_DISABLED (0x0 << (GPIO_INT_EN_INTEN12_INDEX))
#define GPIO_INT_EN_INTEN12_ENABLED (0x1 << (GPIO_INT_EN_INTEN12_INDEX))

/* INT_EN BIT 11: Interrupt enable for bit 11 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN11_INDEX (11)
#define GPIO_INT_EN_INTEN11_MASK (0x800)
#define GPIO_INT_EN_INTEN11_DISABLED (0x0 << (GPIO_INT_EN_INTEN11_INDEX))
#define GPIO_INT_EN_INTEN11_ENABLED (0x1 << (GPIO_INT_EN_INTEN11_INDEX))

/* INT_EN BIT 10: Interrupt enable for bit 10 of GPIO                       */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN10_INDEX (10)
#define GPIO_INT_EN_INTEN10_MASK (0x400)
#define GPIO_INT_EN_INTEN10_DISABLED (0x0 << (GPIO_INT_EN_INTEN10_INDEX))
#define GPIO_INT_EN_INTEN10_ENABLED (0x1 << (GPIO_INT_EN_INTEN10_INDEX))

/* INT_EN BIT 9: Interrupt enable for bit  9 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN9_INDEX (9)
#define GPIO_INT_EN_INTEN9_MASK (0x200)
#define GPIO_INT_EN_INTEN9_DISABLED (0x0 << (GPIO_INT_EN_INTEN9_INDEX))
#define GPIO_INT_EN_INTEN9_ENABLED (0x1 << (GPIO_INT_EN_INTEN9_INDEX))

/* INT_EN BIT 8: Interrupt enable for bit  8 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN8_INDEX (8)
#define GPIO_INT_EN_INTEN8_MASK (0x100)
#define GPIO_INT_EN_INTEN8_DISABLED (0x0 << (GPIO_INT_EN_INTEN8_INDEX))
#define GPIO_INT_EN_INTEN8_ENABLED (0x1 << (GPIO_INT_EN_INTEN8_INDEX))

/* INT_EN BIT 7: Interrupt enable for bit  7 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN7_INDEX (7)
#define GPIO_INT_EN_INTEN7_MASK (0x80)
#define GPIO_INT_EN_INTEN7_DISABLED (0x0 << (GPIO_INT_EN_INTEN7_INDEX))
#define GPIO_INT_EN_INTEN7_ENABLED (0x1 << (GPIO_INT_EN_INTEN7_INDEX))

/* INT_EN BIT 6: Interrupt enable for bit  6 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN6_INDEX (6)
#define GPIO_INT_EN_INTEN6_MASK (0x40)
#define GPIO_INT_EN_INTEN6_DISABLED (0x0 << (GPIO_INT_EN_INTEN6_INDEX))
#define GPIO_INT_EN_INTEN6_ENABLED (0x1 << (GPIO_INT_EN_INTEN6_INDEX))

/* INT_EN BIT 5: Interrupt enable for bit  5 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN5_INDEX (5)
#define GPIO_INT_EN_INTEN5_MASK (0x20)
#define GPIO_INT_EN_INTEN5_DISABLED (0x0 << (GPIO_INT_EN_INTEN5_INDEX))
#define GPIO_INT_EN_INTEN5_ENABLED (0x1 << (GPIO_INT_EN_INTEN5_INDEX))

/* INT_EN BIT 4: Interrupt enable for bit  4 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN4_INDEX (4)
#define GPIO_INT_EN_INTEN4_MASK (0x10)
#define GPIO_INT_EN_INTEN4_DISABLED (0x0 << (GPIO_INT_EN_INTEN4_INDEX))
#define GPIO_INT_EN_INTEN4_ENABLED (0x1 << (GPIO_INT_EN_INTEN4_INDEX))

/* INT_EN BIT 3: Interrupt enable for bit  3 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN3_INDEX (3)
#define GPIO_INT_EN_INTEN3_MASK (0x8)
#define GPIO_INT_EN_INTEN3_DISABLED (0x0 << (GPIO_INT_EN_INTEN3_INDEX))
#define GPIO_INT_EN_INTEN3_ENABLED (0x1 << (GPIO_INT_EN_INTEN3_INDEX))

/* INT_EN BIT 2: Interrupt enable for bit  2 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN2_INDEX (2)
#define GPIO_INT_EN_INTEN2_MASK (0x4)
#define GPIO_INT_EN_INTEN2_DISABLED (0x0 << (GPIO_INT_EN_INTEN2_INDEX))
#define GPIO_INT_EN_INTEN2_ENABLED (0x1 << (GPIO_INT_EN_INTEN2_INDEX))

/* INT_EN BIT 1: Interrupt enable for bit  1 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN1_INDEX (1)
#define GPIO_INT_EN_INTEN1_MASK (0x2)
#define GPIO_INT_EN_INTEN1_DISABLED (0x0 << (GPIO_INT_EN_INTEN1_INDEX))
#define GPIO_INT_EN_INTEN1_ENABLED (0x1 << (GPIO_INT_EN_INTEN1_INDEX))

/* INT_EN BIT 0: Interrupt enable for bit  0 of GPIO                        */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_EN_INTEN0_INDEX (0)
#define GPIO_INT_EN_INTEN0_MASK (0x1)
#define GPIO_INT_EN_INTEN0_DISABLED (0x0 << (GPIO_INT_EN_INTEN0_INDEX))
#define GPIO_INT_EN_INTEN0_ENABLED (0x1 << (GPIO_INT_EN_INTEN0_INDEX))
#endif /* GPIO_INT_EN_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/INT_EN: GPIO Interrupt Enable  *
 * Register                                                                 */
#define GPIO0_INT_EN_RESET_VALUE (0x00000000)
#define GPIO0_INT_EN (GPIO0_BASE_ADDR + GPIO_INT_EN_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/CHANGE                     ***/
/* GPIO Change Register                                                     */
#define GPIO0_CHANGE_OFFS (0xC)
#ifndef GPIO_CHANGE_PROTECT
#define GPIO_CHANGE_PROTECT

#define GPIO_CHANGE_OFFS (0xC)

/* CHANGE BIT 31: Change flag of bit 31 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG31_INDEX (31)
#define GPIO_CHANGE_CHG31_MASK (0x80000000)
#define GPIO_CHANGE_CHG31_IDLE (0x0 << (GPIO_CHANGE_CHG31_INDEX))
#define GPIO_CHANGE_CHG31_TOGGLED (0x1 << (GPIO_CHANGE_CHG31_INDEX))

/* CHANGE BIT 30: Change flag of bit 30 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG30_INDEX (30)
#define GPIO_CHANGE_CHG30_MASK (0x40000000)
#define GPIO_CHANGE_CHG30_IDLE (0x0 << (GPIO_CHANGE_CHG30_INDEX))
#define GPIO_CHANGE_CHG30_TOGGLED (0x1 << (GPIO_CHANGE_CHG30_INDEX))

/* CHANGE BIT 29: Change flag of bit 29 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG29_INDEX (29)
#define GPIO_CHANGE_CHG29_MASK (0x20000000)
#define GPIO_CHANGE_CHG29_IDLE (0x0 << (GPIO_CHANGE_CHG29_INDEX))
#define GPIO_CHANGE_CHG29_TOGGLED (0x1 << (GPIO_CHANGE_CHG29_INDEX))

/* CHANGE BIT 28: Change flag of bit 28 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG28_INDEX (28)
#define GPIO_CHANGE_CHG28_MASK (0x10000000)
#define GPIO_CHANGE_CHG28_IDLE (0x0 << (GPIO_CHANGE_CHG28_INDEX))
#define GPIO_CHANGE_CHG28_TOGGLED (0x1 << (GPIO_CHANGE_CHG28_INDEX))

/* CHANGE BIT 27: Change flag of bit 27 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG27_INDEX (27)
#define GPIO_CHANGE_CHG27_MASK (0x8000000)
#define GPIO_CHANGE_CHG27_IDLE (0x0 << (GPIO_CHANGE_CHG27_INDEX))
#define GPIO_CHANGE_CHG27_TOGGLED (0x1 << (GPIO_CHANGE_CHG27_INDEX))

/* CHANGE BIT 26: Change flag of bit 26 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG26_INDEX (26)
#define GPIO_CHANGE_CHG26_MASK (0x4000000)
#define GPIO_CHANGE_CHG26_IDLE (0x0 << (GPIO_CHANGE_CHG26_INDEX))
#define GPIO_CHANGE_CHG26_TOGGLED (0x1 << (GPIO_CHANGE_CHG26_INDEX))

/* CHANGE BIT 25: Change flag of bit 25 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG25_INDEX (25)
#define GPIO_CHANGE_CHG25_MASK (0x2000000)
#define GPIO_CHANGE_CHG25_IDLE (0x0 << (GPIO_CHANGE_CHG25_INDEX))
#define GPIO_CHANGE_CHG25_TOGGLED (0x1 << (GPIO_CHANGE_CHG25_INDEX))

/* CHANGE BIT 24: Change flag of bit 24 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG24_INDEX (24)
#define GPIO_CHANGE_CHG24_MASK (0x1000000)
#define GPIO_CHANGE_CHG24_IDLE (0x0 << (GPIO_CHANGE_CHG24_INDEX))
#define GPIO_CHANGE_CHG24_TOGGLED (0x1 << (GPIO_CHANGE_CHG24_INDEX))

/* CHANGE BIT 23: Change flag of bit 23 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG23_INDEX (23)
#define GPIO_CHANGE_CHG23_MASK (0x800000)
#define GPIO_CHANGE_CHG23_IDLE (0x0 << (GPIO_CHANGE_CHG23_INDEX))
#define GPIO_CHANGE_CHG23_TOGGLED (0x1 << (GPIO_CHANGE_CHG23_INDEX))

/* CHANGE BIT 22: Change flag of bit 22 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG22_INDEX (22)
#define GPIO_CHANGE_CHG22_MASK (0x400000)
#define GPIO_CHANGE_CHG22_IDLE (0x0 << (GPIO_CHANGE_CHG22_INDEX))
#define GPIO_CHANGE_CHG22_TOGGLED (0x1 << (GPIO_CHANGE_CHG22_INDEX))

/* CHANGE BIT 21: Change flag of bit 21 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG21_INDEX (21)
#define GPIO_CHANGE_CHG21_MASK (0x200000)
#define GPIO_CHANGE_CHG21_IDLE (0x0 << (GPIO_CHANGE_CHG21_INDEX))
#define GPIO_CHANGE_CHG21_TOGGLED (0x1 << (GPIO_CHANGE_CHG21_INDEX))

/* CHANGE BIT 20: Change flag of bit 20 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG20_INDEX (20)
#define GPIO_CHANGE_CHG20_MASK (0x100000)
#define GPIO_CHANGE_CHG20_IDLE (0x0 << (GPIO_CHANGE_CHG20_INDEX))
#define GPIO_CHANGE_CHG20_TOGGLED (0x1 << (GPIO_CHANGE_CHG20_INDEX))

/* CHANGE BIT 19: Change flag of bit 19 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG19_INDEX (19)
#define GPIO_CHANGE_CHG19_MASK (0x80000)
#define GPIO_CHANGE_CHG19_IDLE (0x0 << (GPIO_CHANGE_CHG19_INDEX))
#define GPIO_CHANGE_CHG19_TOGGLED (0x1 << (GPIO_CHANGE_CHG19_INDEX))

/* CHANGE BIT 18: Change flag of bit 18 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG18_INDEX (18)
#define GPIO_CHANGE_CHG18_MASK (0x40000)
#define GPIO_CHANGE_CHG18_IDLE (0x0 << (GPIO_CHANGE_CHG18_INDEX))
#define GPIO_CHANGE_CHG18_TOGGLED (0x1 << (GPIO_CHANGE_CHG18_INDEX))

/* CHANGE BIT 17: Change flag of bit 17 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG17_INDEX (17)
#define GPIO_CHANGE_CHG17_MASK (0x20000)
#define GPIO_CHANGE_CHG17_IDLE (0x0 << (GPIO_CHANGE_CHG17_INDEX))
#define GPIO_CHANGE_CHG17_TOGGLED (0x1 << (GPIO_CHANGE_CHG17_INDEX))

/* CHANGE BIT 16: Change flag of bit 16 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG16_INDEX (16)
#define GPIO_CHANGE_CHG16_MASK (0x10000)
#define GPIO_CHANGE_CHG16_IDLE (0x0 << (GPIO_CHANGE_CHG16_INDEX))
#define GPIO_CHANGE_CHG16_TOGGLED (0x1 << (GPIO_CHANGE_CHG16_INDEX))

/* CHANGE BIT 15: Change flag of bit 15 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG15_INDEX (15)
#define GPIO_CHANGE_CHG15_MASK (0x8000)
#define GPIO_CHANGE_CHG15_IDLE (0x0 << (GPIO_CHANGE_CHG15_INDEX))
#define GPIO_CHANGE_CHG15_TOGGLED (0x1 << (GPIO_CHANGE_CHG15_INDEX))

/* CHANGE BIT 14: Change flag of bit 14 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG14_INDEX (14)
#define GPIO_CHANGE_CHG14_MASK (0x4000)
#define GPIO_CHANGE_CHG14_IDLE (0x0 << (GPIO_CHANGE_CHG14_INDEX))
#define GPIO_CHANGE_CHG14_TOGGLED (0x1 << (GPIO_CHANGE_CHG14_INDEX))

/* CHANGE BIT 13: Change flag of bit 13 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG13_INDEX (13)
#define GPIO_CHANGE_CHG13_MASK (0x2000)
#define GPIO_CHANGE_CHG13_IDLE (0x0 << (GPIO_CHANGE_CHG13_INDEX))
#define GPIO_CHANGE_CHG13_TOGGLED (0x1 << (GPIO_CHANGE_CHG13_INDEX))

/* CHANGE BIT 12: Change flag of bit 12 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG12_INDEX (12)
#define GPIO_CHANGE_CHG12_MASK (0x1000)
#define GPIO_CHANGE_CHG12_IDLE (0x0 << (GPIO_CHANGE_CHG12_INDEX))
#define GPIO_CHANGE_CHG12_TOGGLED (0x1 << (GPIO_CHANGE_CHG12_INDEX))

/* CHANGE BIT 11: Change flag of bit 11 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG11_INDEX (11)
#define GPIO_CHANGE_CHG11_MASK (0x800)
#define GPIO_CHANGE_CHG11_IDLE (0x0 << (GPIO_CHANGE_CHG11_INDEX))
#define GPIO_CHANGE_CHG11_TOGGLED (0x1 << (GPIO_CHANGE_CHG11_INDEX))

/* CHANGE BIT 10: Change flag of bit 10 of GPIO                             */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG10_INDEX (10)
#define GPIO_CHANGE_CHG10_MASK (0x400)
#define GPIO_CHANGE_CHG10_IDLE (0x0 << (GPIO_CHANGE_CHG10_INDEX))
#define GPIO_CHANGE_CHG10_TOGGLED (0x1 << (GPIO_CHANGE_CHG10_INDEX))

/* CHANGE BIT 9: Change flag of bit  9 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG9_INDEX (9)
#define GPIO_CHANGE_CHG9_MASK (0x200)
#define GPIO_CHANGE_CHG9_IDLE (0x0 << (GPIO_CHANGE_CHG9_INDEX))
#define GPIO_CHANGE_CHG9_TOGGLED (0x1 << (GPIO_CHANGE_CHG9_INDEX))

/* CHANGE BIT 8: Change flag of bit  8 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG8_INDEX (8)
#define GPIO_CHANGE_CHG8_MASK (0x100)
#define GPIO_CHANGE_CHG8_IDLE (0x0 << (GPIO_CHANGE_CHG8_INDEX))
#define GPIO_CHANGE_CHG8_TOGGLED (0x1 << (GPIO_CHANGE_CHG8_INDEX))

/* CHANGE BIT 7: Change flag of bit  7 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG7_INDEX (7)
#define GPIO_CHANGE_CHG7_MASK (0x80)
#define GPIO_CHANGE_CHG7_IDLE (0x0 << (GPIO_CHANGE_CHG7_INDEX))
#define GPIO_CHANGE_CHG7_TOGGLED (0x1 << (GPIO_CHANGE_CHG7_INDEX))

/* CHANGE BIT 6: Change flag of bit  6 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG6_INDEX (6)
#define GPIO_CHANGE_CHG6_MASK (0x40)
#define GPIO_CHANGE_CHG6_IDLE (0x0 << (GPIO_CHANGE_CHG6_INDEX))
#define GPIO_CHANGE_CHG6_TOGGLED (0x1 << (GPIO_CHANGE_CHG6_INDEX))

/* CHANGE BIT 5: Change flag of bit  5 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG5_INDEX (5)
#define GPIO_CHANGE_CHG5_MASK (0x20)
#define GPIO_CHANGE_CHG5_IDLE (0x0 << (GPIO_CHANGE_CHG5_INDEX))
#define GPIO_CHANGE_CHG5_TOGGLED (0x1 << (GPIO_CHANGE_CHG5_INDEX))

/* CHANGE BIT 4: Change flag of bit  4 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG4_INDEX (4)
#define GPIO_CHANGE_CHG4_MASK (0x10)
#define GPIO_CHANGE_CHG4_IDLE (0x0 << (GPIO_CHANGE_CHG4_INDEX))
#define GPIO_CHANGE_CHG4_TOGGLED (0x1 << (GPIO_CHANGE_CHG4_INDEX))

/* CHANGE BIT 3: Change flag of bit  3 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG3_INDEX (3)
#define GPIO_CHANGE_CHG3_MASK (0x8)
#define GPIO_CHANGE_CHG3_IDLE (0x0 << (GPIO_CHANGE_CHG3_INDEX))
#define GPIO_CHANGE_CHG3_TOGGLED (0x1 << (GPIO_CHANGE_CHG3_INDEX))

/* CHANGE BIT 2: Change flag of bit  2 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG2_INDEX (2)
#define GPIO_CHANGE_CHG2_MASK (0x4)
#define GPIO_CHANGE_CHG2_IDLE (0x0 << (GPIO_CHANGE_CHG2_INDEX))
#define GPIO_CHANGE_CHG2_TOGGLED (0x1 << (GPIO_CHANGE_CHG2_INDEX))

/* CHANGE BIT 1: Change flag of bit  1 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG1_INDEX (1)
#define GPIO_CHANGE_CHG1_MASK (0x2)
#define GPIO_CHANGE_CHG1_IDLE (0x0 << (GPIO_CHANGE_CHG1_INDEX))
#define GPIO_CHANGE_CHG1_TOGGLED (0x1 << (GPIO_CHANGE_CHG1_INDEX))

/* CHANGE BIT 0: Change flag of bit  0 of GPIO                              */
/* symbolic TO1 field, reset to 0x0                                         */
#define GPIO_CHANGE_CHG0_INDEX (0)
#define GPIO_CHANGE_CHG0_MASK (0x1)
#define GPIO_CHANGE_CHG0_IDLE (0x0 << (GPIO_CHANGE_CHG0_INDEX))
#define GPIO_CHANGE_CHG0_TOGGLED (0x1 << (GPIO_CHANGE_CHG0_INDEX))
#endif /* GPIO_CHANGE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/CHANGE: GPIO Change Register   */
#define GPIO0_CHANGE_RESET_VALUE (0x00000000)
#define GPIO0_CHANGE (GPIO0_BASE_ADDR + GPIO_CHANGE_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/WRMASK                     ***/
/* GPIO Write Mask Register                                                 */
#define GPIO0_WRMASK_OFFS (0x10)
#ifndef GPIO_WRMASK_PROTECT
#define GPIO_WRMASK_PROTECT

#define GPIO_WRMASK_OFFS (0x10)

/* WRMASK BIT 31: Write mask ofbit 31 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK31_INDEX (31)
#define GPIO_WRMASK_WRMSK31_MASK (0x80000000)
#define GPIO_WRMASK_WRMSK31_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK31_INDEX))
#define GPIO_WRMASK_WRMSK31_MASKED (0x1 << (GPIO_WRMASK_WRMSK31_INDEX))

/* WRMASK BIT 30: Write mask ofbit 30 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK30_INDEX (30)
#define GPIO_WRMASK_WRMSK30_MASK (0x40000000)
#define GPIO_WRMASK_WRMSK30_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK30_INDEX))
#define GPIO_WRMASK_WRMSK30_MASKED (0x1 << (GPIO_WRMASK_WRMSK30_INDEX))

/* WRMASK BIT 29: Write mask ofbit 29 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK29_INDEX (29)
#define GPIO_WRMASK_WRMSK29_MASK (0x20000000)
#define GPIO_WRMASK_WRMSK29_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK29_INDEX))
#define GPIO_WRMASK_WRMSK29_MASKED (0x1 << (GPIO_WRMASK_WRMSK29_INDEX))

/* WRMASK BIT 28: Write mask ofbit 28 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK28_INDEX (28)
#define GPIO_WRMASK_WRMSK28_MASK (0x10000000)
#define GPIO_WRMASK_WRMSK28_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK28_INDEX))
#define GPIO_WRMASK_WRMSK28_MASKED (0x1 << (GPIO_WRMASK_WRMSK28_INDEX))

/* WRMASK BIT 27: Write mask ofbit 27 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK27_INDEX (27)
#define GPIO_WRMASK_WRMSK27_MASK (0x8000000)
#define GPIO_WRMASK_WRMSK27_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK27_INDEX))
#define GPIO_WRMASK_WRMSK27_MASKED (0x1 << (GPIO_WRMASK_WRMSK27_INDEX))

/* WRMASK BIT 26: Write mask ofbit 26 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK26_INDEX (26)
#define GPIO_WRMASK_WRMSK26_MASK (0x4000000)
#define GPIO_WRMASK_WRMSK26_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK26_INDEX))
#define GPIO_WRMASK_WRMSK26_MASKED (0x1 << (GPIO_WRMASK_WRMSK26_INDEX))

/* WRMASK BIT 25: Write mask ofbit 25 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK25_INDEX (25)
#define GPIO_WRMASK_WRMSK25_MASK (0x2000000)
#define GPIO_WRMASK_WRMSK25_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK25_INDEX))
#define GPIO_WRMASK_WRMSK25_MASKED (0x1 << (GPIO_WRMASK_WRMSK25_INDEX))

/* WRMASK BIT 24: Write mask ofbit 24 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK24_INDEX (24)
#define GPIO_WRMASK_WRMSK24_MASK (0x1000000)
#define GPIO_WRMASK_WRMSK24_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK24_INDEX))
#define GPIO_WRMASK_WRMSK24_MASKED (0x1 << (GPIO_WRMASK_WRMSK24_INDEX))

/* WRMASK BIT 23: Write mask ofbit 23 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK23_INDEX (23)
#define GPIO_WRMASK_WRMSK23_MASK (0x800000)
#define GPIO_WRMASK_WRMSK23_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK23_INDEX))
#define GPIO_WRMASK_WRMSK23_MASKED (0x1 << (GPIO_WRMASK_WRMSK23_INDEX))

/* WRMASK BIT 22: Write mask ofbit 22 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK22_INDEX (22)
#define GPIO_WRMASK_WRMSK22_MASK (0x400000)
#define GPIO_WRMASK_WRMSK22_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK22_INDEX))
#define GPIO_WRMASK_WRMSK22_MASKED (0x1 << (GPIO_WRMASK_WRMSK22_INDEX))

/* WRMASK BIT 21: Write mask ofbit 21 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK21_INDEX (21)
#define GPIO_WRMASK_WRMSK21_MASK (0x200000)
#define GPIO_WRMASK_WRMSK21_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK21_INDEX))
#define GPIO_WRMASK_WRMSK21_MASKED (0x1 << (GPIO_WRMASK_WRMSK21_INDEX))

/* WRMASK BIT 20: Write mask ofbit 20 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK20_INDEX (20)
#define GPIO_WRMASK_WRMSK20_MASK (0x100000)
#define GPIO_WRMASK_WRMSK20_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK20_INDEX))
#define GPIO_WRMASK_WRMSK20_MASKED (0x1 << (GPIO_WRMASK_WRMSK20_INDEX))

/* WRMASK BIT 19: Write mask ofbit 19 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK19_INDEX (19)
#define GPIO_WRMASK_WRMSK19_MASK (0x80000)
#define GPIO_WRMASK_WRMSK19_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK19_INDEX))
#define GPIO_WRMASK_WRMSK19_MASKED (0x1 << (GPIO_WRMASK_WRMSK19_INDEX))

/* WRMASK BIT 18: Write mask ofbit 18 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK18_INDEX (18)
#define GPIO_WRMASK_WRMSK18_MASK (0x40000)
#define GPIO_WRMASK_WRMSK18_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK18_INDEX))
#define GPIO_WRMASK_WRMSK18_MASKED (0x1 << (GPIO_WRMASK_WRMSK18_INDEX))

/* WRMASK BIT 17: Write mask ofbit 17 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK17_INDEX (17)
#define GPIO_WRMASK_WRMSK17_MASK (0x20000)
#define GPIO_WRMASK_WRMSK17_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK17_INDEX))
#define GPIO_WRMASK_WRMSK17_MASKED (0x1 << (GPIO_WRMASK_WRMSK17_INDEX))

/* WRMASK BIT 16: Write mask ofbit 16 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK16_INDEX (16)
#define GPIO_WRMASK_WRMSK16_MASK (0x10000)
#define GPIO_WRMASK_WRMSK16_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK16_INDEX))
#define GPIO_WRMASK_WRMSK16_MASKED (0x1 << (GPIO_WRMASK_WRMSK16_INDEX))

/* WRMASK BIT 15: Write mask ofbit 15 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK15_INDEX (15)
#define GPIO_WRMASK_WRMSK15_MASK (0x8000)
#define GPIO_WRMASK_WRMSK15_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK15_INDEX))
#define GPIO_WRMASK_WRMSK15_MASKED (0x1 << (GPIO_WRMASK_WRMSK15_INDEX))

/* WRMASK BIT 14: Write mask ofbit 14 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK14_INDEX (14)
#define GPIO_WRMASK_WRMSK14_MASK (0x4000)
#define GPIO_WRMASK_WRMSK14_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK14_INDEX))
#define GPIO_WRMASK_WRMSK14_MASKED (0x1 << (GPIO_WRMASK_WRMSK14_INDEX))

/* WRMASK BIT 13: Write mask ofbit 13 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK13_INDEX (13)
#define GPIO_WRMASK_WRMSK13_MASK (0x2000)
#define GPIO_WRMASK_WRMSK13_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK13_INDEX))
#define GPIO_WRMASK_WRMSK13_MASKED (0x1 << (GPIO_WRMASK_WRMSK13_INDEX))

/* WRMASK BIT 12: Write mask ofbit 12 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK12_INDEX (12)
#define GPIO_WRMASK_WRMSK12_MASK (0x1000)
#define GPIO_WRMASK_WRMSK12_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK12_INDEX))
#define GPIO_WRMASK_WRMSK12_MASKED (0x1 << (GPIO_WRMASK_WRMSK12_INDEX))

/* WRMASK BIT 11: Write mask ofbit 11 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK11_INDEX (11)
#define GPIO_WRMASK_WRMSK11_MASK (0x800)
#define GPIO_WRMASK_WRMSK11_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK11_INDEX))
#define GPIO_WRMASK_WRMSK11_MASKED (0x1 << (GPIO_WRMASK_WRMSK11_INDEX))

/* WRMASK BIT 10: Write mask ofbit 10 of GPIO                               */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK10_INDEX (10)
#define GPIO_WRMASK_WRMSK10_MASK (0x400)
#define GPIO_WRMASK_WRMSK10_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK10_INDEX))
#define GPIO_WRMASK_WRMSK10_MASKED (0x1 << (GPIO_WRMASK_WRMSK10_INDEX))

/* WRMASK BIT 9: Write mask ofbit  9 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK9_INDEX (9)
#define GPIO_WRMASK_WRMSK9_MASK (0x200)
#define GPIO_WRMASK_WRMSK9_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK9_INDEX))
#define GPIO_WRMASK_WRMSK9_MASKED (0x1 << (GPIO_WRMASK_WRMSK9_INDEX))

/* WRMASK BIT 8: Write mask ofbit  8 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK8_INDEX (8)
#define GPIO_WRMASK_WRMSK8_MASK (0x100)
#define GPIO_WRMASK_WRMSK8_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK8_INDEX))
#define GPIO_WRMASK_WRMSK8_MASKED (0x1 << (GPIO_WRMASK_WRMSK8_INDEX))

/* WRMASK BIT 7: Write mask ofbit  7 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK7_INDEX (7)
#define GPIO_WRMASK_WRMSK7_MASK (0x80)
#define GPIO_WRMASK_WRMSK7_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK7_INDEX))
#define GPIO_WRMASK_WRMSK7_MASKED (0x1 << (GPIO_WRMASK_WRMSK7_INDEX))

/* WRMASK BIT 6: Write mask ofbit  6 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK6_INDEX (6)
#define GPIO_WRMASK_WRMSK6_MASK (0x40)
#define GPIO_WRMASK_WRMSK6_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK6_INDEX))
#define GPIO_WRMASK_WRMSK6_MASKED (0x1 << (GPIO_WRMASK_WRMSK6_INDEX))

/* WRMASK BIT 5: Write mask ofbit  5 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK5_INDEX (5)
#define GPIO_WRMASK_WRMSK5_MASK (0x20)
#define GPIO_WRMASK_WRMSK5_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK5_INDEX))
#define GPIO_WRMASK_WRMSK5_MASKED (0x1 << (GPIO_WRMASK_WRMSK5_INDEX))

/* WRMASK BIT 4: Write mask ofbit  4 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK4_INDEX (4)
#define GPIO_WRMASK_WRMSK4_MASK (0x10)
#define GPIO_WRMASK_WRMSK4_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK4_INDEX))
#define GPIO_WRMASK_WRMSK4_MASKED (0x1 << (GPIO_WRMASK_WRMSK4_INDEX))

/* WRMASK BIT 3: Write mask ofbit  3 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK3_INDEX (3)
#define GPIO_WRMASK_WRMSK3_MASK (0x8)
#define GPIO_WRMASK_WRMSK3_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK3_INDEX))
#define GPIO_WRMASK_WRMSK3_MASKED (0x1 << (GPIO_WRMASK_WRMSK3_INDEX))

/* WRMASK BIT 2: Write mask ofbit  2 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK2_INDEX (2)
#define GPIO_WRMASK_WRMSK2_MASK (0x4)
#define GPIO_WRMASK_WRMSK2_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK2_INDEX))
#define GPIO_WRMASK_WRMSK2_MASKED (0x1 << (GPIO_WRMASK_WRMSK2_INDEX))

/* WRMASK BIT 1: Write mask ofbit  1 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK1_INDEX (1)
#define GPIO_WRMASK_WRMSK1_MASK (0x2)
#define GPIO_WRMASK_WRMSK1_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK1_INDEX))
#define GPIO_WRMASK_WRMSK1_MASKED (0x1 << (GPIO_WRMASK_WRMSK1_INDEX))

/* WRMASK BIT 0: Write mask ofbit  0 of GPIO                                */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_WRMASK_WRMSK0_INDEX (0)
#define GPIO_WRMASK_WRMSK0_MASK (0x1)
#define GPIO_WRMASK_WRMSK0_WRITABLE (0x0 << (GPIO_WRMASK_WRMSK0_INDEX))
#define GPIO_WRMASK_WRMSK0_MASKED (0x1 << (GPIO_WRMASK_WRMSK0_INDEX))
#endif /* GPIO_WRMASK_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/WRMASK: GPIO Write Mask Register */
#define GPIO0_WRMASK_RESET_VALUE (0x00000000)
#define GPIO0_WRMASK (GPIO0_BASE_ADDR + GPIO_WRMASK_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/INT_TYPE                   ***/
/* GPIO Interrupt type Register                                             */
#define GPIO0_INT_TYPE_OFFS (0x14)
#ifndef GPIO_INT_TYPE_PROTECT
#define GPIO_INT_TYPE_PROTECT

#define GPIO_INT_TYPE_OFFS (0x14)

/* INT_TYPE BIT 31: Interrupt type ofbit 31 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE31_INDEX (31)
#define GPIO_INT_TYPE_INTTYPE31_MASK (0x80000000)
#define GPIO_INT_TYPE_INTTYPE31_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE31_INDEX))
#define GPIO_INT_TYPE_INTTYPE31_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE31_INDEX))

/* INT_TYPE BIT 30: Interrupt type ofbit 30 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE30_INDEX (30)
#define GPIO_INT_TYPE_INTTYPE30_MASK (0x40000000)
#define GPIO_INT_TYPE_INTTYPE30_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE30_INDEX))
#define GPIO_INT_TYPE_INTTYPE30_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE30_INDEX))

/* INT_TYPE BIT 29: Interrupt type ofbit 29 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE29_INDEX (29)
#define GPIO_INT_TYPE_INTTYPE29_MASK (0x20000000)
#define GPIO_INT_TYPE_INTTYPE29_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE29_INDEX))
#define GPIO_INT_TYPE_INTTYPE29_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE29_INDEX))

/* INT_TYPE BIT 28: Interrupt type ofbit 28 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE28_INDEX (28)
#define GPIO_INT_TYPE_INTTYPE28_MASK (0x10000000)
#define GPIO_INT_TYPE_INTTYPE28_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE28_INDEX))
#define GPIO_INT_TYPE_INTTYPE28_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE28_INDEX))

/* INT_TYPE BIT 27: Interrupt type ofbit 27 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE27_INDEX (27)
#define GPIO_INT_TYPE_INTTYPE27_MASK (0x8000000)
#define GPIO_INT_TYPE_INTTYPE27_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE27_INDEX))
#define GPIO_INT_TYPE_INTTYPE27_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE27_INDEX))

/* INT_TYPE BIT 26: Interrupt type ofbit 26 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE26_INDEX (26)
#define GPIO_INT_TYPE_INTTYPE26_MASK (0x4000000)
#define GPIO_INT_TYPE_INTTYPE26_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE26_INDEX))
#define GPIO_INT_TYPE_INTTYPE26_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE26_INDEX))

/* INT_TYPE BIT 25: Interrupt type ofbit 25 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE25_INDEX (25)
#define GPIO_INT_TYPE_INTTYPE25_MASK (0x2000000)
#define GPIO_INT_TYPE_INTTYPE25_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE25_INDEX))
#define GPIO_INT_TYPE_INTTYPE25_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE25_INDEX))

/* INT_TYPE BIT 24: Interrupt type ofbit 24 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE24_INDEX (24)
#define GPIO_INT_TYPE_INTTYPE24_MASK (0x1000000)
#define GPIO_INT_TYPE_INTTYPE24_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE24_INDEX))
#define GPIO_INT_TYPE_INTTYPE24_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE24_INDEX))

/* INT_TYPE BIT 23: Interrupt type ofbit 23 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE23_INDEX (23)
#define GPIO_INT_TYPE_INTTYPE23_MASK (0x800000)
#define GPIO_INT_TYPE_INTTYPE23_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE23_INDEX))
#define GPIO_INT_TYPE_INTTYPE23_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE23_INDEX))

/* INT_TYPE BIT 22: Interrupt type ofbit 22 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE22_INDEX (22)
#define GPIO_INT_TYPE_INTTYPE22_MASK (0x400000)
#define GPIO_INT_TYPE_INTTYPE22_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE22_INDEX))
#define GPIO_INT_TYPE_INTTYPE22_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE22_INDEX))

/* INT_TYPE BIT 21: Interrupt type ofbit 21 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE21_INDEX (21)
#define GPIO_INT_TYPE_INTTYPE21_MASK (0x200000)
#define GPIO_INT_TYPE_INTTYPE21_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE21_INDEX))
#define GPIO_INT_TYPE_INTTYPE21_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE21_INDEX))

/* INT_TYPE BIT 20: Interrupt type ofbit 20 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE20_INDEX (20)
#define GPIO_INT_TYPE_INTTYPE20_MASK (0x100000)
#define GPIO_INT_TYPE_INTTYPE20_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE20_INDEX))
#define GPIO_INT_TYPE_INTTYPE20_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE20_INDEX))

/* INT_TYPE BIT 19: Interrupt type ofbit 19 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE19_INDEX (19)
#define GPIO_INT_TYPE_INTTYPE19_MASK (0x80000)
#define GPIO_INT_TYPE_INTTYPE19_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE19_INDEX))
#define GPIO_INT_TYPE_INTTYPE19_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE19_INDEX))

/* INT_TYPE BIT 18: Interrupt type ofbit 18 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE18_INDEX (18)
#define GPIO_INT_TYPE_INTTYPE18_MASK (0x40000)
#define GPIO_INT_TYPE_INTTYPE18_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE18_INDEX))
#define GPIO_INT_TYPE_INTTYPE18_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE18_INDEX))

/* INT_TYPE BIT 17: Interrupt type ofbit 17 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE17_INDEX (17)
#define GPIO_INT_TYPE_INTTYPE17_MASK (0x20000)
#define GPIO_INT_TYPE_INTTYPE17_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE17_INDEX))
#define GPIO_INT_TYPE_INTTYPE17_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE17_INDEX))

/* INT_TYPE BIT 16: Interrupt type ofbit 16 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE16_INDEX (16)
#define GPIO_INT_TYPE_INTTYPE16_MASK (0x10000)
#define GPIO_INT_TYPE_INTTYPE16_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE16_INDEX))
#define GPIO_INT_TYPE_INTTYPE16_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE16_INDEX))

/* INT_TYPE BIT 15: Interrupt type ofbit 15 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE15_INDEX (15)
#define GPIO_INT_TYPE_INTTYPE15_MASK (0x8000)
#define GPIO_INT_TYPE_INTTYPE15_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE15_INDEX))
#define GPIO_INT_TYPE_INTTYPE15_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE15_INDEX))

/* INT_TYPE BIT 14: Interrupt type ofbit 14 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE14_INDEX (14)
#define GPIO_INT_TYPE_INTTYPE14_MASK (0x4000)
#define GPIO_INT_TYPE_INTTYPE14_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE14_INDEX))
#define GPIO_INT_TYPE_INTTYPE14_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE14_INDEX))

/* INT_TYPE BIT 13: Interrupt type ofbit 13 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE13_INDEX (13)
#define GPIO_INT_TYPE_INTTYPE13_MASK (0x2000)
#define GPIO_INT_TYPE_INTTYPE13_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE13_INDEX))
#define GPIO_INT_TYPE_INTTYPE13_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE13_INDEX))

/* INT_TYPE BIT 12: Interrupt type ofbit 12 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE12_INDEX (12)
#define GPIO_INT_TYPE_INTTYPE12_MASK (0x1000)
#define GPIO_INT_TYPE_INTTYPE12_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE12_INDEX))
#define GPIO_INT_TYPE_INTTYPE12_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE12_INDEX))

/* INT_TYPE BIT 11: Interrupt type ofbit 11 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE11_INDEX (11)
#define GPIO_INT_TYPE_INTTYPE11_MASK (0x800)
#define GPIO_INT_TYPE_INTTYPE11_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE11_INDEX))
#define GPIO_INT_TYPE_INTTYPE11_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE11_INDEX))

/* INT_TYPE BIT 10: Interrupt type ofbit 10 of GPIO                         */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE10_INDEX (10)
#define GPIO_INT_TYPE_INTTYPE10_MASK (0x400)
#define GPIO_INT_TYPE_INTTYPE10_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE10_INDEX))
#define GPIO_INT_TYPE_INTTYPE10_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE10_INDEX))

/* INT_TYPE BIT 9: Interrupt type ofbit  9 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE9_INDEX (9)
#define GPIO_INT_TYPE_INTTYPE9_MASK (0x200)
#define GPIO_INT_TYPE_INTTYPE9_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE9_INDEX))
#define GPIO_INT_TYPE_INTTYPE9_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE9_INDEX))

/* INT_TYPE BIT 8: Interrupt type ofbit  8 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE8_INDEX (8)
#define GPIO_INT_TYPE_INTTYPE8_MASK (0x100)
#define GPIO_INT_TYPE_INTTYPE8_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE8_INDEX))
#define GPIO_INT_TYPE_INTTYPE8_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE8_INDEX))

/* INT_TYPE BIT 7: Interrupt type ofbit  7 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE7_INDEX (7)
#define GPIO_INT_TYPE_INTTYPE7_MASK (0x80)
#define GPIO_INT_TYPE_INTTYPE7_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE7_INDEX))
#define GPIO_INT_TYPE_INTTYPE7_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE7_INDEX))

/* INT_TYPE BIT 6: Interrupt type ofbit  6 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE6_INDEX (6)
#define GPIO_INT_TYPE_INTTYPE6_MASK (0x40)
#define GPIO_INT_TYPE_INTTYPE6_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE6_INDEX))
#define GPIO_INT_TYPE_INTTYPE6_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE6_INDEX))

/* INT_TYPE BIT 5: Interrupt type ofbit  5 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE5_INDEX (5)
#define GPIO_INT_TYPE_INTTYPE5_MASK (0x20)
#define GPIO_INT_TYPE_INTTYPE5_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE5_INDEX))
#define GPIO_INT_TYPE_INTTYPE5_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE5_INDEX))

/* INT_TYPE BIT 4: Interrupt type ofbit  4 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE4_INDEX (4)
#define GPIO_INT_TYPE_INTTYPE4_MASK (0x10)
#define GPIO_INT_TYPE_INTTYPE4_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE4_INDEX))
#define GPIO_INT_TYPE_INTTYPE4_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE4_INDEX))

/* INT_TYPE BIT 3: Interrupt type ofbit  3 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE3_INDEX (3)
#define GPIO_INT_TYPE_INTTYPE3_MASK (0x8)
#define GPIO_INT_TYPE_INTTYPE3_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE3_INDEX))
#define GPIO_INT_TYPE_INTTYPE3_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE3_INDEX))

/* INT_TYPE BIT 2: Interrupt type ofbit  2 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE2_INDEX (2)
#define GPIO_INT_TYPE_INTTYPE2_MASK (0x4)
#define GPIO_INT_TYPE_INTTYPE2_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE2_INDEX))
#define GPIO_INT_TYPE_INTTYPE2_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE2_INDEX))

/* INT_TYPE BIT 1: Interrupt type ofbit  1 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE1_INDEX (1)
#define GPIO_INT_TYPE_INTTYPE1_MASK (0x2)
#define GPIO_INT_TYPE_INTTYPE1_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE1_INDEX))
#define GPIO_INT_TYPE_INTTYPE1_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE1_INDEX))

/* INT_TYPE BIT 0: Interrupt type ofbit  0 of GPIO                          */
/* symbolic RW field, reset to 0x0                                          */
#define GPIO_INT_TYPE_INTTYPE0_INDEX (0)
#define GPIO_INT_TYPE_INTTYPE0_MASK (0x1)
#define GPIO_INT_TYPE_INTTYPE0_PULSE (0x0 << (GPIO_INT_TYPE_INTTYPE0_INDEX))
#define GPIO_INT_TYPE_INTTYPE0_LEVEL (0x1 << (GPIO_INT_TYPE_INTTYPE0_INDEX))
#endif /* GPIO_INT_TYPE_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/GPIO0/INT_TYPE: GPIO Interrupt type  *
 * Register                                                                 */
#define GPIO0_INT_TYPE_RESET_VALUE (0x00000000)
#define GPIO0_INT_TYPE (GPIO0_BASE_ADDR + GPIO_INT_TYPE_OFFS)

#define GPIO0_SIZE (0x17)

/*** IOMUX Control                                                        ***/
#define IOMUXCTRL_BASE_ADDR (0x821400)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/IOMUXCTRL/CFG                    ***/
/* Configuration Register                                                   */
#ifndef IOMUXCTRL_CFG_PROTECT
#define IOMUXCTRL_CFG_PROTECT

#define IOMUXCTRL_CFG_OFFS (0x0)

/* CFG BIT 5: Clock speed set-up                                            */
/* symbolic RW field, reset to 0x0                                          */
#define IOMUXCTRL_CFG_CLKMOD_INDEX (5)
#define IOMUXCTRL_CFG_CLKMOD_MASK (0x20)
#define IOMUXCTRL_CFG_CLKMOD_XTALDIVBY2 (0x0 << (IOMUXCTRL_CFG_CLKMOD_INDEX))
#define IOMUXCTRL_CFG_CLKMOD_XTAL (0x1 << (IOMUXCTRL_CFG_CLKMOD_INDEX))

/* CFG BITS 4..0: Value of IO mux configuration                             */
/* numeric RW unsigned field, reset to 0x0                                  */
#define IOMUXCTRL_CFG_CFG_INDEX (0)
#define IOMUXCTRL_CFG_CFG_MASK (0x1F)
#endif /* IOMUXCTRL_CFG_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/IOMUXCTRL/CFG: Configuration Register */
#define IOMUXCTRL_CFG_RESET_VALUE (0x00)
#define IOMUXCTRL_CFG (IOMUXCTRL_BASE_ADDR + IOMUXCTRL_CFG_OFFS)

#define IOMUXCTRL_SIZE (0x0)

/*** ATON-T Miscellaneous Registers                                       ***/
#define AT_BASE_ADDR (0x821800)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/I2C_SLV_AS                    ***/
/* I2C Programmable Slave Address Register                                  */
#ifndef AT_I2C_SLV_AS_PROTECT
#define AT_I2C_SLV_AS_PROTECT

#define AT_I2C_SLV_AS_OFFS (0x0)

/* I2C_SLV_AS BITS 1..0: Two programmable bits of I2C slave address         */
/* numeric RO unsigned field, reset to 0x0                                  */
#define AT_I2C_SLV_AS_I2C_SLV_AS_INDEX (0)
#define AT_I2C_SLV_AS_I2C_SLV_AS_MASK (0x3)
#endif /* AT_I2C_SLV_AS_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/I2C_SLV_AS: I2C Programmable      *
 * Slave Address Register                                                   */
#define AT_I2C_SLV_AS_RESET_VALUE (0x0)
#define AT_I2C_SLV_AS (AT_BASE_ADDR + AT_I2C_SLV_AS_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/HTUBE_CMD                     ***/
/* HTube Command Register                                                   */
#ifndef AT_HTUBE_CMD_PROTECT
#define AT_HTUBE_CMD_PROTECT

#define AT_HTUBE_CMD_OFFS (0x4)

/* HTUBE_CMD BITS 31..0: Communication tube command                         */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_HTUBE_CMD_HTUBE_CMD_INDEX (0)
#define AT_HTUBE_CMD_HTUBE_CMD_MASK (0xFFFFFFFF)
#endif /* AT_HTUBE_CMD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/HTUBE_CMD: HTube Command Register */
#define AT_HTUBE_CMD_RESET_VALUE (0x00000000)
#define AT_HTUBE_CMD (AT_BASE_ADDR + AT_HTUBE_CMD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/HTUBE_DATA                    ***/
/* HTube Data Register                                                      */
#ifndef AT_HTUBE_DATA_PROTECT
#define AT_HTUBE_DATA_PROTECT

#define AT_HTUBE_DATA_OFFS (0x8)

/* HTUBE_DATA BITS 31..0: Communication tube data                           */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_HTUBE_DATA_HTUBE_DATA_INDEX (0)
#define AT_HTUBE_DATA_HTUBE_DATA_MASK (0xFFFFFFFF)
#endif /* AT_HTUBE_DATA_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/HTUBE_DATA: HTube Data Register   */
#define AT_HTUBE_DATA_RESET_VALUE (0x00000000)
#define AT_HTUBE_DATA (AT_BASE_ADDR + AT_HTUBE_DATA_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/FW_START_ADDR                 ***/
/* Firmware Start Address Register                                          */
#ifndef AT_FW_START_ADDR_PROTECT
#define AT_FW_START_ADDR_PROTECT

#define AT_FW_START_ADDR_OFFS (0xC)

/* FW_START_ADDR BITS 23..0: Firmware start address                         */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_FW_START_ADDR_ADDRESS_INDEX (0)
#define AT_FW_START_ADDR_ADDRESS_MASK (0xFFFFFF)
#endif /* AT_FW_START_ADDR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/FW_START_ADDR: Firmware Start     *
 * Address Register                                                         */
#define AT_FW_START_ADDR_RESET_VALUE (0x000000)
#define AT_FW_START_ADDR (AT_BASE_ADDR + AT_FW_START_ADDR_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/FW_PATCH_VERSION              ***/
/* Firmware Patch Version Register                                          */
#ifndef AT_FW_PATCH_VERSION_PROTECT
#define AT_FW_PATCH_VERSION_PROTECT

#define AT_FW_PATCH_VERSION_OFFS (0x10)

/* FW_PATCH_VERSION BITS 23..16: Patch major version number                 */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_FW_PATCH_VERSION_MAJORVER_INDEX (16)
#define AT_FW_PATCH_VERSION_MAJORVER_MASK (0xFF0000)

/* FW_PATCH_VERSION BITS 15..8: Patch minor version number                  */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_FW_PATCH_VERSION_MINORVER_INDEX (8)
#define AT_FW_PATCH_VERSION_MINORVER_MASK (0xFF00)

/* FW_PATCH_VERSION BITS 7..0: Patch decimal version                        */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_FW_PATCH_VERSION_DECIMALVER_INDEX (0)
#define AT_FW_PATCH_VERSION_DECIMALVER_MASK (0xFF)
#endif /* AT_FW_PATCH_VERSION_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/FW_PATCH_VERSION: Firmware Patch  *
 * Version Register                                                         */
#define AT_FW_PATCH_VERSION_RESET_VALUE (0x000000)
#define AT_FW_PATCH_VERSION (AT_BASE_ADDR + AT_FW_PATCH_VERSION_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/EMA                           ***/
/* EMA control for memories                                                 */
#ifndef AT_EMA_PROTECT
#define AT_EMA_PROTECT

#define AT_EMA_OFFS (0x14)

/* EMA BITS 6..4: EMA bits for ICCM RAM                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_EMA_EMA_ICCM_RAM_INDEX (4)
#define AT_EMA_EMA_ICCM_RAM_MASK (0x70)

/* EMA BITS 2..0: EMA bits for ICCM ROM                                     */
/* numeric RW unsigned field, reset to 0x0                                  */
#define AT_EMA_EMA_ICCM_ROM_INDEX (0)
#define AT_EMA_EMA_ICCM_ROM_MASK (0x7)
#endif /* AT_EMA_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/EMA: EMA control for memories     */
#define AT_EMA_RESET_VALUE (0x00)
#define AT_EMA (AT_BASE_ADDR + AT_EMA_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/CHIPID                        ***/
/* Chip ID and mask revision number                                         */
#ifndef AT_CHIPID_PROTECT
#define AT_CHIPID_PROTECT

#define AT_CHIPID_OFFS (0x18)

/* CHIPID BITS 31..20: Manufacturer ID number                               */
/* numeric RO unsigned field, reset to 0x97                                 */
#define AT_CHIPID_MANUFACTURER_ID_INDEX (20)
#define AT_CHIPID_MANUFACTURER_ID_MASK (0xFFF00000)

/* CHIPID BITS 19..8: Product ID number                                     */
/* numeric RO unsigned field, reset to 0x1                                  */
#define AT_CHIPID_PRODUCT_ID_INDEX (8)
#define AT_CHIPID_PRODUCT_ID_MASK (0xFFF00)

/* CHIPID BITS 7..0: Chip mask revision number                              */
/* numeric RO unsigned field, reset to 0x82                                 */
#define AT_CHIPID_MASK_REV_INDEX (0)
#define AT_CHIPID_MASK_REV_MASK (0xFF)
#endif /* AT_CHIPID_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/CHIPID: Chip ID and mask          *
 * revision number                                                          */
#define AT_CHIPID_RESET_VALUE (0x09700182)
#define AT_CHIPID (AT_BASE_ADDR + AT_CHIPID_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/DESIGNREV                     ***/
/* Design Revision number                                                   */
#ifndef AT_DESIGNREV_PROTECT
#define AT_DESIGNREV_PROTECT

#define AT_DESIGNREV_OFFS (0x1C)

/* DESIGNREV BITS 15..8: Major revision number                              */
/* numeric RO unsigned field, reset to 0x3                                  */
#define AT_DESIGNREV_MAJOR_INDEX (8)
#define AT_DESIGNREV_MAJOR_MASK (0xFF00)

/* DESIGNREV BITS 7..0: Minor revision number                               */
/* numeric RO unsigned field, reset to 0x1                                  */
#define AT_DESIGNREV_MINOR_INDEX (0)
#define AT_DESIGNREV_MINOR_MASK (0xFF)
#endif /* AT_DESIGNREV_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/AT/DESIGNREV: Design Revision number */
#define AT_DESIGNREV_RESET_VALUE (0x0301)
#define AT_DESIGNREV (AT_BASE_ADDR + AT_DESIGNREV_OFFS)

#define AT_SIZE (0x1D)

/*** LCD Interface 4-8 bit                                                ***/
#define LCDIF_BASE_ADDR (0x821C00)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/CMD                        ***/
/* Command Register                                                         */
#ifndef LCDIF_CMD_PROTECT
#define LCDIF_CMD_PROTECT

#define LCDIF_CMD_OFFS (0x0)

/* CMD BITS 3..0: LCD Interface instruction                                 */
/* numeric RW unsigned field, reset to 0x0                                  */
#define LCDIF_CMD_COMMAND_INDEX (0)
#define LCDIF_CMD_COMMAND_MASK (0xF)
#endif /* LCDIF_CMD_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/CMD: Command Register          */
#define LCDIF_CMD_RESET_VALUE (0x0)
#define LCDIF_CMD (LCDIF_BASE_ADDR + LCDIF_CMD_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/DATA                       ***/
/* Data Register                                                            */
#ifndef LCDIF_DATA_PROTECT
#define LCDIF_DATA_PROTECT

#define LCDIF_DATA_OFFS (0x4)

/* DATA BITS 7..0: Transfer data for FIFO                                   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define LCDIF_DATA_DATA_INDEX (0)
#define LCDIF_DATA_DATA_MASK (0xFF)
#endif /* LCDIF_DATA_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/DATA: Data Register            */
#define LCDIF_DATA_RESET_VALUE (0x00)
#define LCDIF_DATA (LCDIF_BASE_ADDR + LCDIF_DATA_OFFS)

/*** ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/SCR                        ***/
/* Status and Control Register                                              */
#ifndef LCDIF_SCR_PROTECT
#define LCDIF_SCR_PROTECT

#define LCDIF_SCR_OFFS (0x8)

/* SCR BITS 29..16: Command execution time                                  */
/* numeric RW unsigned field, reset to 0x3B6                                */
#define LCDIF_SCR_EXECTIME_INDEX (16)
#define LCDIF_SCR_EXECTIME_MASK (0x3FFF0000)

/* SCR BITS 15..8: E-Pulse delay between 2 pulses for 4-bit interfaces (in  *
 * clock cycles)                                                            */
/* numeric RW unsigned field, reset to 0x32                                 */
#define LCDIF_SCR_EDELAY_INDEX (8)
#define LCDIF_SCR_EDELAY_MASK (0xFF00)

/* SCR BITS 7..1: E-Pulse duration (in clock cycles)                        */
/* numeric RW unsigned field, reset to 0xB                                  */
#define LCDIF_SCR_EPULSE_INDEX (1)
#define LCDIF_SCR_EPULSE_MASK (0xFE)

/* SCR BIT 0: Transfer data for FIFO                                        */
/* symbolic RO field, reset to 0x0                                          */
#define LCDIF_SCR_BUSY_INDEX (0)
#define LCDIF_SCR_BUSY_MASK (0x1)
#define LCDIF_SCR_BUSY_IDLE (0x0 << (LCDIF_SCR_BUSY_INDEX))
#define LCDIF_SCR_BUSY_BUSY (0x1 << (LCDIF_SCR_BUSY_INDEX))
#endif /* LCDIF_SCR_PROTECT */
/* ATONT/DIG_TOP/DIG_CORE/ATONT_APB_PF/LCDIF/SCR: Status and Control Register */
#define LCDIF_SCR_RESET_VALUE (0x03b63216)
#define LCDIF_SCR (LCDIF_BASE_ADDR + LCDIF_SCR_OFFS)

#define LCDIF_SIZE (0xB)

#define ATONT_APB_PF_SIZE (0x1C0B)

#define DIG_CORE_SIZE (0x830035)

#define DIG_TOP_SIZE (0x830035)

/*** Firmware memory map                                                  ***/
#define FW_TOP_BASE_ADDR (0x0)

/*** ATON-T FW memory map                                                 ***/
#define FW_BASE_ADDR (0x13F00)

/*** ATONT/FW_TOP/FW/CRYSTAL_FREQ                                         ***/
/* Setup the crystal frequency [MHz] (taken into account when leaving       *
 * sleep mode)                                                              */
#ifndef FW_CRYSTAL_FREQ_PROTECT
#define FW_CRYSTAL_FREQ_PROTECT

#define FW_CRYSTAL_FREQ_OFFS (0x0)

/* CRYSTAL_FREQ BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x3F                                 */
#define FW_CRYSTAL_FREQ_VALUE_INDEX (0)
#define FW_CRYSTAL_FREQ_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_CRYSTAL_FREQ_PROTECT */
/* ATONT/FW_TOP/FW/CRYSTAL_FREQ: Setup the crystal frequency [MHz] (taken   *
 * into account when leaving sleep mode)                                    */
#define FW_CRYSTAL_FREQ_RESET_VALUE (0x0000003f)
#define FW_CRYSTAL_FREQ (FW_BASE_ADDR + FW_CRYSTAL_FREQ_OFFS)

/*** ATONT/FW_TOP/FW/LO_FREQUENCY                                         ***/
/* Tuning frequency [kHz]                                                   */
#ifndef FW_LO_FREQUENCY_PROTECT
#define FW_LO_FREQUENCY_PROTECT

#define FW_LO_FREQUENCY_OFFS (0x4)

/* LO_FREQUENCY BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0xA2990                              */
#define FW_LO_FREQUENCY_VALUE_INDEX (0)
#define FW_LO_FREQUENCY_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LO_FREQUENCY_PROTECT */
/* ATONT/FW_TOP/FW/LO_FREQUENCY: Tuning frequency [kHz]                     */
#define FW_LO_FREQUENCY_RESET_VALUE (0x000a2990)
#define FW_LO_FREQUENCY (FW_BASE_ADDR + FW_LO_FREQUENCY_OFFS)

/*** ATONT/FW_TOP/FW/BANDWIDTH                                            ***/
/* Bandwidth [MHz], range 6 to 8                                            */
#ifndef FW_BANDWIDTH_PROTECT
#define FW_BANDWIDTH_PROTECT

#define FW_BANDWIDTH_OFFS (0x8)

/* BANDWIDTH BITS 31..0: Value                                              */
/* numeric RW unsigned field, reset to 0x8                                  */
#define FW_BANDWIDTH_VALUE_INDEX (0)
#define FW_BANDWIDTH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_BANDWIDTH_PROTECT */
/* ATONT/FW_TOP/FW/BANDWIDTH: Bandwidth [MHz], range 6 to 8                 */
#define FW_BANDWIDTH_RESET_VALUE (0x00000008)
#define FW_BANDWIDTH (FW_BASE_ADDR + FW_BANDWIDTH_OFFS)

/*** ATONT/FW_TOP/FW/LOOP_THROUGH                                         ***/
/* Loop-through mode                                                        */
#ifndef FW_LOOP_THROUGH_PROTECT
#define FW_LOOP_THROUGH_PROTECT

#define FW_LOOP_THROUGH_OFFS (0xC)

/* LOOP_THROUGH BITS 1..0: loop through mode                                */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LOOP_THROUGH_MODE_INDEX (0)
#define FW_LOOP_THROUGH_MODE_MASK (0x3)
#define FW_LOOP_THROUGH_MODE_NONE (0x0 << (FW_LOOP_THROUGH_MODE_INDEX))
#define FW_LOOP_THROUGH_MODE_TUNERA_LOOPTHROUGH (0x1 << (FW_LOOP_THROUGH_MODE_INDEX))
#define FW_LOOP_THROUGH_MODE_TUNERB_LOOPTHROUGH (0x2 << (FW_LOOP_THROUGH_MODE_INDEX))
#define FW_LOOP_THROUGH_MODE_CASCADED (0x3 << (FW_LOOP_THROUGH_MODE_INDEX))
#endif /* FW_LOOP_THROUGH_PROTECT */
/* ATONT/FW_TOP/FW/LOOP_THROUGH: Loop-through mode                          */
#define FW_LOOP_THROUGH_RESET_VALUE (0x0)
#define FW_LOOP_THROUGH (FW_BASE_ADDR + FW_LOOP_THROUGH_OFFS)

/*** ATONT/FW_TOP/FW/CLOCK_OUT                                            ***/
/* Decision for keeping output (taken into account when leaving sleep mode) */
#ifndef FW_CLOCK_OUT_PROTECT
#define FW_CLOCK_OUT_PROTECT

#define FW_CLOCK_OUT_OFFS (0x10)

/* CLOCK_OUT BIT 0:                                                         */
/* symbolic RW field, reset to 0x0                                          */
#define FW_CLOCK_OUT_MODE_INDEX (0)
#define FW_CLOCK_OUT_MODE_MASK (0x1)
#define FW_CLOCK_OUT_MODE_DISABLED (0x0 << (FW_CLOCK_OUT_MODE_INDEX))
#define FW_CLOCK_OUT_MODE_ENABLED (0x1 << (FW_CLOCK_OUT_MODE_INDEX))
#endif /* FW_CLOCK_OUT_PROTECT */
/* ATONT/FW_TOP/FW/CLOCK_OUT: Decision for keeping output (taken into       *
 * account when leaving sleep mode)                                         */
#define FW_CLOCK_OUT_RESET_VALUE (0x0)
#define FW_CLOCK_OUT (FW_BASE_ADDR + FW_CLOCK_OUT_OFFS)

/*** ATONT/FW_TOP/FW/TUNER_STATUS                                         ***/
/* AT810 error code                                                         */
#ifndef FW_TUNER_STATUS_PROTECT
#define FW_TUNER_STATUS_PROTECT

#define FW_TUNER_STATUS_OFFS (0x14)

/* TUNER_STATUS BITS 31..0: status                                          */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_TUNER_STATUS_VALUE_INDEX (0)
#define FW_TUNER_STATUS_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_TUNER_STATUS_PROTECT */
/* ATONT/FW_TOP/FW/TUNER_STATUS: AT810 error code                           */
#define FW_TUNER_STATUS_RESET_VALUE (0x00000000)
#define FW_TUNER_STATUS (FW_BASE_ADDR + FW_TUNER_STATUS_OFFS)

/*** ATONT/FW_TOP/FW/GET_TUNER_GAIN                                       ***/
/* Global tuner current gain [dB/256]. Data valid  only in RUN mode.        */
#ifndef FW_GET_TUNER_GAIN_PROTECT
#define FW_GET_TUNER_GAIN_PROTECT

#define FW_GET_TUNER_GAIN_OFFS (0x18)

/* GET_TUNER_GAIN BITS 31..0: Gain applied                                  */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_GET_TUNER_GAIN_VALUE_INDEX (0)
#define FW_GET_TUNER_GAIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_GET_TUNER_GAIN_PROTECT */
/* ATONT/FW_TOP/FW/GET_TUNER_GAIN: Global tuner current gain [dB/256].      *
 * Data valid  only in RUN mode.                                            */
#define FW_GET_TUNER_GAIN_RESET_VALUE (0x00000000)
#define FW_GET_TUNER_GAIN (FW_BASE_ADDR + FW_GET_TUNER_GAIN_OFFS)

/*** ATONT/FW_TOP/FW/VGA_GAIN_CONTROL                                     ***/
/* VGA AGC control                                                          */
#ifndef FW_VGA_GAIN_CONTROL_PROTECT
#define FW_VGA_GAIN_CONTROL_PROTECT

#define FW_VGA_GAIN_CONTROL_OFFS (0x1C)

/* VGA_GAIN_CONTROL BITS 31..8: Gain \[dB/256\] to set-up with (if forced   *
 * flag set)                                                                */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VGA_GAIN_CONTROL_SET_GAIN_INDEX (8)
#define FW_VGA_GAIN_CONTROL_SET_GAIN_MASK (0xFFFFFF00)

/* VGA_GAIN_CONTROL BIT 3: Gain Value set when FORCED selected.             */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VGA_GAIN_CONTROL_GAIN_INDEX (3)
#define FW_VGA_GAIN_CONTROL_GAIN_MASK (0x8)
#define FW_VGA_GAIN_CONTROL_GAIN_FREE (0x0 << (FW_VGA_GAIN_CONTROL_GAIN_INDEX))
#define FW_VGA_GAIN_CONTROL_GAIN_FORCED (0x1 << (FW_VGA_GAIN_CONTROL_GAIN_INDEX))

/* VGA_GAIN_CONTROL BIT 0: Define VGA AGC mode                              */
/* symbolic RW field, reset to 0x1                                          */
#define FW_VGA_GAIN_CONTROL_MODE_INDEX (0)
#define FW_VGA_GAIN_CONTROL_MODE_MASK (0x1)
#define FW_VGA_GAIN_CONTROL_MODE_MANUAL (0x0 << (FW_VGA_GAIN_CONTROL_MODE_INDEX))
#define FW_VGA_GAIN_CONTROL_MODE_EXTERNAL (0x1 << (FW_VGA_GAIN_CONTROL_MODE_INDEX))
#endif /* FW_VGA_GAIN_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/VGA_GAIN_CONTROL: VGA AGC control                        */
#define FW_VGA_GAIN_CONTROL_RESET_VALUE (0x00000001)
#define FW_VGA_GAIN_CONTROL (FW_BASE_ADDR + FW_VGA_GAIN_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VGA_GET_CURRENT_GAIN                                 ***/
/* VGA current gain [dB/256]                                                */
#ifndef FW_VGA_GET_CURRENT_GAIN_PROTECT
#define FW_VGA_GET_CURRENT_GAIN_PROTECT

#define FW_VGA_GET_CURRENT_GAIN_OFFS (0x20)

/* VGA_GET_CURRENT_GAIN BITS 31..0: Gain applied                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VGA_GET_CURRENT_GAIN_VALUE_INDEX (0)
#define FW_VGA_GET_CURRENT_GAIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VGA_GET_CURRENT_GAIN_PROTECT */
/* ATONT/FW_TOP/FW/VGA_GET_CURRENT_GAIN: VGA current gain [dB/256]          */
#define FW_VGA_GET_CURRENT_GAIN_RESET_VALUE (0x00000000)
#define FW_VGA_GET_CURRENT_GAIN (FW_BASE_ADDR + FW_VGA_GET_CURRENT_GAIN_OFFS)

/*** ATONT/FW_TOP/FW/BB_AGC_EXT_GAIN_VSS                                  ***/
/* Corresponding VGA gain when BBAGC pin is at VSS                          */
#ifndef FW_BB_AGC_EXT_GAIN_VSS_PROTECT
#define FW_BB_AGC_EXT_GAIN_VSS_PROTECT

#define FW_BB_AGC_EXT_GAIN_VSS_OFFS (0x24)

/* BB_AGC_EXT_GAIN_VSS BITS 31..0: VSS gain [256*dB]                        */
/* numeric RW signed (2s comlement) field, reset to 0xFFFFF400              */
#define FW_BB_AGC_EXT_GAIN_VSS_VALUE_INDEX (0)
#define FW_BB_AGC_EXT_GAIN_VSS_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_BB_AGC_EXT_GAIN_VSS_PROTECT */
/* ATONT/FW_TOP/FW/BB_AGC_EXT_GAIN_VSS: Corresponding VGA gain when BBAGC   *
 * pin is at VSS                                                            */
#define FW_BB_AGC_EXT_GAIN_VSS_RESET_VALUE (0xfffff400)
#define FW_BB_AGC_EXT_GAIN_VSS (FW_BASE_ADDR + FW_BB_AGC_EXT_GAIN_VSS_OFFS)

/*** ATONT/FW_TOP/FW/BB_AGC_EXT_GAIN_VDD                                  ***/
/* Corresponding VGA gain when BBAGC pin is at VDD                          */
#ifndef FW_BB_AGC_EXT_GAIN_VDD_PROTECT
#define FW_BB_AGC_EXT_GAIN_VDD_PROTECT

#define FW_BB_AGC_EXT_GAIN_VDD_OFFS (0x28)

/* BB_AGC_EXT_GAIN_VDD BITS 31..0: VDD gain [256*dB]                        */
/* numeric RW signed (2s comlement) field, reset to 0x1E00                  */
#define FW_BB_AGC_EXT_GAIN_VDD_VALUE_INDEX (0)
#define FW_BB_AGC_EXT_GAIN_VDD_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_BB_AGC_EXT_GAIN_VDD_PROTECT */
/* ATONT/FW_TOP/FW/BB_AGC_EXT_GAIN_VDD: Corresponding VGA gain when BBAGC   *
 * pin is at VDD                                                            */
#define FW_BB_AGC_EXT_GAIN_VDD_RESET_VALUE (0x00001e00)
#define FW_BB_AGC_EXT_GAIN_VDD (FW_BASE_ADDR + FW_BB_AGC_EXT_GAIN_VDD_OFFS)

/*** ATONT/FW_TOP/FW/BB2PMA_GAIN_OFFSET_THRESH                            ***/
/* VGA gain threshold below which the BB OVL flag is set [dB/256]           */
#ifndef FW_BB2PMA_GAIN_OFFSET_THRESH_PROTECT
#define FW_BB2PMA_GAIN_OFFSET_THRESH_PROTECT

#define FW_BB2PMA_GAIN_OFFSET_THRESH_OFFS (0x2C)

/* BB2PMA_GAIN_OFFSET_THRESH BITS 31..0: Value                              */
/* numeric RW unsigned field, reset to 0x100                                */
#define FW_BB2PMA_GAIN_OFFSET_THRESH_VALUE_INDEX (0)
#define FW_BB2PMA_GAIN_OFFSET_THRESH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_BB2PMA_GAIN_OFFSET_THRESH_PROTECT */
/* ATONT/FW_TOP/FW/BB2PMA_GAIN_OFFSET_THRESH: VGA gain threshold below      *
 * which the BB OVL flag is set [dB/256]                                    */
#define FW_BB2PMA_GAIN_OFFSET_THRESH_RESET_VALUE (0x00000100)
#define FW_BB2PMA_GAIN_OFFSET_THRESH (FW_BASE_ADDR + FW_BB2PMA_GAIN_OFFSET_THRESH_OFFS)

/*** ATONT/FW_TOP/FW/LNA_AGC_CONTROL                                      ***/
/* LNA gain control                                                         */
#ifndef FW_LNA_AGC_CONTROL_PROTECT
#define FW_LNA_AGC_CONTROL_PROTECT

#define FW_LNA_AGC_CONTROL_OFFS (0x30)

/* LNA_AGC_CONTROL BITS 31..8: Gain*256 dB to set-up with Forced flag.      */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_LNA_AGC_CONTROL_SET_GAIN_INDEX (8)
#define FW_LNA_AGC_CONTROL_SET_GAIN_MASK (0xFFFFFF00)

/* LNA_AGC_CONTROL BIT 3: Forced gain. Reset by FW to FREE when FORCED      *
 * applied.or ignored.  This flag is ignored if the INIT state is REQUEST   *
 * or PROCESS                                                               */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LNA_AGC_CONTROL_GAIN_INDEX (3)
#define FW_LNA_AGC_CONTROL_GAIN_MASK (0x8)
#define FW_LNA_AGC_CONTROL_GAIN_FREE (0x0 << (FW_LNA_AGC_CONTROL_GAIN_INDEX))
#define FW_LNA_AGC_CONTROL_GAIN_FORCED (0x1 << (FW_LNA_AGC_CONTROL_GAIN_INDEX))

/* LNA_AGC_CONTROL BIT 2: Enable LNA AGC algo                               */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LNA_AGC_CONTROL_MODE_INDEX (2)
#define FW_LNA_AGC_CONTROL_MODE_MASK (0x4)
#define FW_LNA_AGC_CONTROL_MODE_DISABLE (0x0 << (FW_LNA_AGC_CONTROL_MODE_INDEX))
#define FW_LNA_AGC_CONTROL_MODE_ENABLE (0x1 << (FW_LNA_AGC_CONTROL_MODE_INDEX))

/* LNA_AGC_CONTROL BITS 1..0: LNMA AGC gain initialization                  */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LNA_AGC_CONTROL_INIT_INDEX (0)
#define FW_LNA_AGC_CONTROL_INIT_MASK (0x3)
#define FW_LNA_AGC_CONTROL_INIT_RESET (0x0 << (FW_LNA_AGC_CONTROL_INIT_INDEX))
#define FW_LNA_AGC_CONTROL_INIT_DONE (0x1 << (FW_LNA_AGC_CONTROL_INIT_INDEX))
#define FW_LNA_AGC_CONTROL_INIT_REQUEST (0x2 << (FW_LNA_AGC_CONTROL_INIT_INDEX))
#define FW_LNA_AGC_CONTROL_INIT_PROCESS (0x3 << (FW_LNA_AGC_CONTROL_INIT_INDEX))
#endif /* FW_LNA_AGC_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/LNA_AGC_CONTROL: LNA gain control                        */
#define FW_LNA_AGC_CONTROL_RESET_VALUE (0x00000000)
#define FW_LNA_AGC_CONTROL (FW_BASE_ADDR + FW_LNA_AGC_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/LNA_GET_CURRENT_GAIN                                 ***/
/* LNA current gain [dB/256]                                                */
#ifndef FW_LNA_GET_CURRENT_GAIN_PROTECT
#define FW_LNA_GET_CURRENT_GAIN_PROTECT

#define FW_LNA_GET_CURRENT_GAIN_OFFS (0x34)

/* LNA_GET_CURRENT_GAIN BITS 31..0: Gain applied                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_LNA_GET_CURRENT_GAIN_VALUE_INDEX (0)
#define FW_LNA_GET_CURRENT_GAIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_GET_CURRENT_GAIN_PROTECT */
/* ATONT/FW_TOP/FW/LNA_GET_CURRENT_GAIN: LNA current gain [dB/256]          */
#define FW_LNA_GET_CURRENT_GAIN_RESET_VALUE (0x00000000)
#define FW_LNA_GET_CURRENT_GAIN (FW_BASE_ADDR + FW_LNA_GET_CURRENT_GAIN_OFFS)

/*** ATONT/FW_TOP/FW/LNA_CROWBAR                                          ***/
/* LNA crowbar control (read at set_tune time only)                         */
#ifndef FW_LNA_CROWBAR_PROTECT
#define FW_LNA_CROWBAR_PROTECT

#define FW_LNA_CROWBAR_OFFS (0x38)

/* LNA_CROWBAR BITS 1..0: Crowbar mode                                      */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LNA_CROWBAR_MODE_INDEX (0)
#define FW_LNA_CROWBAR_MODE_MASK (0x3)
#define FW_LNA_CROWBAR_MODE_OFF (0x0 << (FW_LNA_CROWBAR_MODE_INDEX))
#define FW_LNA_CROWBAR_MODE_ON (0x1 << (FW_LNA_CROWBAR_MODE_INDEX))
#define FW_LNA_CROWBAR_MODE_AUTO (0x2 << (FW_LNA_CROWBAR_MODE_INDEX))
#endif /* FW_LNA_CROWBAR_PROTECT */
/* ATONT/FW_TOP/FW/LNA_CROWBAR: LNA crowbar control (read at set_tune time  *
 * only)                                                                    */
#define FW_LNA_CROWBAR_RESET_VALUE (0x0)
#define FW_LNA_CROWBAR (FW_BASE_ADDR + FW_LNA_CROWBAR_OFFS)

/*** ATONT/FW_TOP/FW/LNA_GAIN_CROWBAR                                     ***/
/* LNA gain threshold to activate the crowbar                               */
#ifndef FW_LNA_GAIN_CROWBAR_PROTECT
#define FW_LNA_GAIN_CROWBAR_PROTECT

#define FW_LNA_GAIN_CROWBAR_OFFS (0x3C)

/* LNA_GAIN_CROWBAR BITS 31..0: Gain threshold for crowbar [dB/256]         */
/* numeric RW unsigned field, reset to 0xFFFFFA01                           */
#define FW_LNA_GAIN_CROWBAR_VALUE_INDEX (0)
#define FW_LNA_GAIN_CROWBAR_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_GAIN_CROWBAR_PROTECT */
/* ATONT/FW_TOP/FW/LNA_GAIN_CROWBAR: LNA gain threshold to activate the crowbar */
#define FW_LNA_GAIN_CROWBAR_RESET_VALUE (0xfffffa01)
#define FW_LNA_GAIN_CROWBAR (FW_BASE_ADDR + FW_LNA_GAIN_CROWBAR_OFFS)

/*** ATONT/FW_TOP/FW/LNA_RFSW_FREQUENCY                                   ***/
/* Frequency above which the RF switch must be activated                    */
#ifndef FW_LNA_RFSW_FREQUENCY_PROTECT
#define FW_LNA_RFSW_FREQUENCY_PROTECT

#define FW_LNA_RFSW_FREQUENCY_OFFS (0x40)

/* LNA_RFSW_FREQUENCY BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x55730                              */
#define FW_LNA_RFSW_FREQUENCY_VALUE_INDEX (0)
#define FW_LNA_RFSW_FREQUENCY_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_RFSW_FREQUENCY_PROTECT */
/* ATONT/FW_TOP/FW/LNA_RFSW_FREQUENCY: Frequency above which the RF switch  *
 * must be activated                                                        */
#define FW_LNA_RFSW_FREQUENCY_RESET_VALUE (0x00055730)
#define FW_LNA_RFSW_FREQUENCY (FW_BASE_ADDR + FW_LNA_RFSW_FREQUENCY_OFFS)

/*** ATONT/FW_TOP/FW/PMA_AGC_CONTROL                                      ***/
/* PMA gain control                                                         */
#ifndef FW_PMA_AGC_CONTROL_PROTECT
#define FW_PMA_AGC_CONTROL_PROTECT

#define FW_PMA_AGC_CONTROL_OFFS (0x44)

/* PMA_AGC_CONTROL BITS 31..8: Gain \[dB/256\] to set-up with FORCED flag.  *
 * Bounds: 2 to 22 \[dB\]                                                   */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_PMA_AGC_CONTROL_SET_GAIN_INDEX (8)
#define FW_PMA_AGC_CONTROL_SET_GAIN_MASK (0xFFFFFF00)

/* PMA_AGC_CONTROL BIT 3: Forced gain. Reset by FW to FREE when FORCED      *
 * applied or ignored.  This flag is ignored if the INIT state is REQUEST   *
 * or PROCESS                                                               */
/* symbolic RW field, reset to 0x0                                          */
#define FW_PMA_AGC_CONTROL_GAIN_INDEX (3)
#define FW_PMA_AGC_CONTROL_GAIN_MASK (0x8)
#define FW_PMA_AGC_CONTROL_GAIN_FREE (0x0 << (FW_PMA_AGC_CONTROL_GAIN_INDEX))
#define FW_PMA_AGC_CONTROL_GAIN_FORCED (0x1 << (FW_PMA_AGC_CONTROL_GAIN_INDEX))

/* PMA_AGC_CONTROL BIT 2: Enable PMA AGC                                    */
/* symbolic RW field, reset to 0x0                                          */
#define FW_PMA_AGC_CONTROL_MODE_INDEX (2)
#define FW_PMA_AGC_CONTROL_MODE_MASK (0x4)
#define FW_PMA_AGC_CONTROL_MODE_DISABLE (0x0 << (FW_PMA_AGC_CONTROL_MODE_INDEX))
#define FW_PMA_AGC_CONTROL_MODE_ENABLE (0x1 << (FW_PMA_AGC_CONTROL_MODE_INDEX))

/* PMA_AGC_CONTROL BITS 1..0: PMA AGC gain initialization                   */
/* symbolic RW field, reset to 0x0                                          */
#define FW_PMA_AGC_CONTROL_INIT_INDEX (0)
#define FW_PMA_AGC_CONTROL_INIT_MASK (0x3)
#define FW_PMA_AGC_CONTROL_INIT_RESET (0x0 << (FW_PMA_AGC_CONTROL_INIT_INDEX))
#define FW_PMA_AGC_CONTROL_INIT_DONE (0x1 << (FW_PMA_AGC_CONTROL_INIT_INDEX))
#define FW_PMA_AGC_CONTROL_INIT_REQUEST (0x2 << (FW_PMA_AGC_CONTROL_INIT_INDEX))
#define FW_PMA_AGC_CONTROL_INIT_PROCESS (0x3 << (FW_PMA_AGC_CONTROL_INIT_INDEX))
#endif /* FW_PMA_AGC_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/PMA_AGC_CONTROL: PMA gain control                        */
#define FW_PMA_AGC_CONTROL_RESET_VALUE (0x00000000)
#define FW_PMA_AGC_CONTROL (FW_BASE_ADDR + FW_PMA_AGC_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/PMA_GET_CURRENT_GAIN                                 ***/
/* PMA current gain [dB/256]                                                */
#ifndef FW_PMA_GET_CURRENT_GAIN_PROTECT
#define FW_PMA_GET_CURRENT_GAIN_PROTECT

#define FW_PMA_GET_CURRENT_GAIN_OFFS (0x48)

/* PMA_GET_CURRENT_GAIN BITS 31..0: Value                                   */
/* numeric RW unsigned field, reset to 0x1600                               */
#define FW_PMA_GET_CURRENT_GAIN_VALUE_INDEX (0)
#define FW_PMA_GET_CURRENT_GAIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_GET_CURRENT_GAIN_PROTECT */
/* ATONT/FW_TOP/FW/PMA_GET_CURRENT_GAIN: PMA current gain [dB/256]          */
#define FW_PMA_GET_CURRENT_GAIN_RESET_VALUE (0x00001600)
#define FW_PMA_GET_CURRENT_GAIN (FW_BASE_ADDR + FW_PMA_GET_CURRENT_GAIN_OFFS)

/*** ATONT/FW_TOP/FW/RCC_TUNE_CONTROL                                     ***/
/* RCC tune algo control                                                    */
#ifndef FW_RCC_TUNE_CONTROL_PROTECT
#define FW_RCC_TUNE_CONTROL_PROTECT

#define FW_RCC_TUNE_CONTROL_OFFS (0x4C)

/* RCC_TUNE_CONTROL BITS 1..0: Process state                                */
/* symbolic RW field, reset to 0x0                                          */
#define FW_RCC_TUNE_CONTROL_ALGO_INDEX (0)
#define FW_RCC_TUNE_CONTROL_ALGO_MASK (0x3)
#define FW_RCC_TUNE_CONTROL_ALGO_RESET (0x0 << (FW_RCC_TUNE_CONTROL_ALGO_INDEX))
#define FW_RCC_TUNE_CONTROL_ALGO_DONE (0x1 << (FW_RCC_TUNE_CONTROL_ALGO_INDEX))
#define FW_RCC_TUNE_CONTROL_ALGO_REQUEST (0x2 << (FW_RCC_TUNE_CONTROL_ALGO_INDEX))
#define FW_RCC_TUNE_CONTROL_ALGO_PROCESS (0x3 << (FW_RCC_TUNE_CONTROL_ALGO_INDEX))
#endif /* FW_RCC_TUNE_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/RCC_TUNE_CONTROL: RCC tune algo control                  */
#define FW_RCC_TUNE_CONTROL_RESET_VALUE (0x0)
#define FW_RCC_TUNE_CONTROL (FW_BASE_ADDR + FW_RCC_TUNE_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/DCM_CONTROL                                          ***/
/* DCM control and status                                                   */
#ifndef FW_DCM_CONTROL_PROTECT
#define FW_DCM_CONTROL_PROTECT

#define FW_DCM_CONTROL_OFFS (0x50)

/* DCM_CONTROL BIT 4: Status of the DCM measurement                         */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DCM_CONTROL_STATUS_INDEX (4)
#define FW_DCM_CONTROL_STATUS_MASK (0x10)
#define FW_DCM_CONTROL_STATUS_PASS (0x0 << (FW_DCM_CONTROL_STATUS_INDEX))
#define FW_DCM_CONTROL_STATUS_FAIL (0x1 << (FW_DCM_CONTROL_STATUS_INDEX))

/* DCM_CONTROL BITS 1..0: Process state                                     */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DCM_CONTROL_ALGO_INDEX (0)
#define FW_DCM_CONTROL_ALGO_MASK (0x3)
#define FW_DCM_CONTROL_ALGO_RESET (0x0 << (FW_DCM_CONTROL_ALGO_INDEX))
#define FW_DCM_CONTROL_ALGO_DONE (0x1 << (FW_DCM_CONTROL_ALGO_INDEX))
#define FW_DCM_CONTROL_ALGO_REQUEST (0x2 << (FW_DCM_CONTROL_ALGO_INDEX))
#define FW_DCM_CONTROL_ALGO_PROCESS (0x3 << (FW_DCM_CONTROL_ALGO_INDEX))
#endif /* FW_DCM_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/DCM_CONTROL: DCM control and status                      */
#define FW_DCM_CONTROL_RESET_VALUE (0x00)
#define FW_DCM_CONTROL (FW_BASE_ADDR + FW_DCM_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/DCM_NODE_ID                                          ***/
/* Node ID for measurement                                                  */
#ifndef FW_DCM_NODE_ID_PROTECT
#define FW_DCM_NODE_ID_PROTECT

#define FW_DCM_NODE_ID_OFFS (0x54)

/* DCM_NODE_ID BITS 31..0: Value                                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_DCM_NODE_ID_VALUE_INDEX (0)
#define FW_DCM_NODE_ID_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DCM_NODE_ID_PROTECT */
/* ATONT/FW_TOP/FW/DCM_NODE_ID: Node ID for measurement                     */
#define FW_DCM_NODE_ID_RESET_VALUE (0x00000000)
#define FW_DCM_NODE_ID (FW_BASE_ADDR + FW_DCM_NODE_ID_OFFS)

/*** ATONT/FW_TOP/FW/DCM_MODE                                             ***/
/* Functional DCM mode                                                      */
#ifndef FW_DCM_MODE_PROTECT
#define FW_DCM_MODE_PROTECT

#define FW_DCM_MODE_OFFS (0x58)

/* DCM_MODE BITS 31..0: DCM mode                                            */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_DCM_MODE_SELECT_INDEX (0)
#define FW_DCM_MODE_SELECT_MASK (0xFFFFFFFF)
#endif /* FW_DCM_MODE_PROTECT */
/* ATONT/FW_TOP/FW/DCM_MODE: Functional DCM mode                            */
#define FW_DCM_MODE_RESET_VALUE (0x00000000)
#define FW_DCM_MODE (FW_BASE_ADDR + FW_DCM_MODE_OFFS)

/*** ATONT/FW_TOP/FW/DCM_GET_VALUE                                        ***/
/* Get DCM value                                                            */
#ifndef FW_DCM_GET_VALUE_PROTECT
#define FW_DCM_GET_VALUE_PROTECT

#define FW_DCM_GET_VALUE_OFFS (0x5C)

/* DCM_GET_VALUE BITS 31..0: Value                                          */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_DCM_GET_VALUE_VALUE_INDEX (0)
#define FW_DCM_GET_VALUE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DCM_GET_VALUE_PROTECT */
/* ATONT/FW_TOP/FW/DCM_GET_VALUE: Get DCM value                             */
#define FW_DCM_GET_VALUE_RESET_VALUE (0x00000000)
#define FW_DCM_GET_VALUE (FW_BASE_ADDR + FW_DCM_GET_VALUE_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_CONTROL                                     ***/
/* VCO freq request                                                         */
#ifndef FW_VCO_FREQ_CONTROL_PROTECT
#define FW_VCO_FREQ_CONTROL_PROTECT

#define FW_VCO_FREQ_CONTROL_OFFS (0x60)

/* VCO_FREQ_CONTROL BIT 4: Status of VCO algo execution                     */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_FREQ_CONTROL_STATUS_INDEX (4)
#define FW_VCO_FREQ_CONTROL_STATUS_MASK (0x10)
#define FW_VCO_FREQ_CONTROL_STATUS_PASS (0x0 << (FW_VCO_FREQ_CONTROL_STATUS_INDEX))
#define FW_VCO_FREQ_CONTROL_STATUS_FAIL (0x1 << (FW_VCO_FREQ_CONTROL_STATUS_INDEX))

/* VCO_FREQ_CONTROL BITS 1..0: Process state                                */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_FREQ_CONTROL_ALGO_INDEX (0)
#define FW_VCO_FREQ_CONTROL_ALGO_MASK (0x3)
#define FW_VCO_FREQ_CONTROL_ALGO_RESET (0x0 << (FW_VCO_FREQ_CONTROL_ALGO_INDEX))
#define FW_VCO_FREQ_CONTROL_ALGO_DONE (0x1 << (FW_VCO_FREQ_CONTROL_ALGO_INDEX))
#define FW_VCO_FREQ_CONTROL_ALGO_REQUEST (0x2 << (FW_VCO_FREQ_CONTROL_ALGO_INDEX))
#define FW_VCO_FREQ_CONTROL_ALGO_PROCESS (0x3 << (FW_VCO_FREQ_CONTROL_ALGO_INDEX))
#endif /* FW_VCO_FREQ_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_CONTROL: VCO freq request                       */
#define FW_VCO_FREQ_CONTROL_RESET_VALUE (0x00)
#define FW_VCO_FREQ_CONTROL (FW_BASE_ADDR + FW_VCO_FREQ_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_TIME_CAPTURE                                ***/
/* VCO frequency time capture control                                       *
 * [us]                                                                     */
#ifndef FW_VCO_FREQ_TIME_CAPTURE_PROTECT
#define FW_VCO_FREQ_TIME_CAPTURE_PROTECT

#define FW_VCO_FREQ_TIME_CAPTURE_OFFS (0x64)

/* VCO_FREQ_TIME_CAPTURE BITS 31..0: TBD                                    */
/* numeric RW unsigned field, reset to 0x400                                */
#define FW_VCO_FREQ_TIME_CAPTURE_VALUE_INDEX (0)
#define FW_VCO_FREQ_TIME_CAPTURE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_FREQ_TIME_CAPTURE_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_TIME_CAPTURE: VCO frequency time capture control *
 * [us]                                                                     */
#define FW_VCO_FREQ_TIME_CAPTURE_RESET_VALUE (0x00000400)
#define FW_VCO_FREQ_TIME_CAPTURE (FW_BASE_ADDR + FW_VCO_FREQ_TIME_CAPTURE_OFFS)

/*** ATONT/FW_TOP/FW/VCO_GET_FREQ                                         ***/
/* VCO frequency read [kHz]                                                 */
#ifndef FW_VCO_GET_FREQ_PROTECT
#define FW_VCO_GET_FREQ_PROTECT

#define FW_VCO_GET_FREQ_OFFS (0x68)

/* VCO_GET_FREQ BITS 31..0: TBD                                             */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_GET_FREQ_VALUE_INDEX (0)
#define FW_VCO_GET_FREQ_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_GET_FREQ_PROTECT */
/* ATONT/FW_TOP/FW/VCO_GET_FREQ: VCO frequency read [kHz]                   */
#define FW_VCO_GET_FREQ_RESET_VALUE (0x00000000)
#define FW_VCO_GET_FREQ (FW_BASE_ADDR + FW_VCO_GET_FREQ_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_BAND_CONTROL                                ***/
/* VCO tune band request                                                    */
#ifndef FW_VCO_TUNE_BAND_CONTROL_PROTECT
#define FW_VCO_TUNE_BAND_CONTROL_PROTECT

#define FW_VCO_TUNE_BAND_CONTROL_OFFS (0x6C)

/* VCO_TUNE_BAND_CONTROL BIT 4: Status VCO algo execution                   */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_TUNE_BAND_CONTROL_STATUS_INDEX (4)
#define FW_VCO_TUNE_BAND_CONTROL_STATUS_MASK (0x10)
#define FW_VCO_TUNE_BAND_CONTROL_STATUS_PASS (0x0 << (FW_VCO_TUNE_BAND_CONTROL_STATUS_INDEX))
#define FW_VCO_TUNE_BAND_CONTROL_STATUS_FAIL (0x1 << (FW_VCO_TUNE_BAND_CONTROL_STATUS_INDEX))

/* VCO_TUNE_BAND_CONTROL BITS 1..0: Process state                           */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_INDEX (0)
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_MASK (0x3)
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_RESET (0x0 << (FW_VCO_TUNE_BAND_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_DONE (0x1 << (FW_VCO_TUNE_BAND_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_REQUEST (0x2 << (FW_VCO_TUNE_BAND_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_BAND_CONTROL_ALGO_PROCESS (0x3 << (FW_VCO_TUNE_BAND_CONTROL_ALGO_INDEX))
#endif /* FW_VCO_TUNE_BAND_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_BAND_CONTROL: VCO tune band request             */
#define FW_VCO_TUNE_BAND_CONTROL_RESET_VALUE (0x00)
#define FW_VCO_TUNE_BAND_CONTROL (FW_BASE_ADDR + FW_VCO_TUNE_BAND_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_AMPL_CONTROL                                ***/
/* VCO tune amplitude request                                               */
#ifndef FW_VCO_TUNE_AMPL_CONTROL_PROTECT
#define FW_VCO_TUNE_AMPL_CONTROL_PROTECT

#define FW_VCO_TUNE_AMPL_CONTROL_OFFS (0x70)

/* VCO_TUNE_AMPL_CONTROL BIT 4: Status VCO algo execution                   */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_TUNE_AMPL_CONTROL_STATUS_INDEX (4)
#define FW_VCO_TUNE_AMPL_CONTROL_STATUS_MASK (0x10)
#define FW_VCO_TUNE_AMPL_CONTROL_STATUS_PASS (0x0 << (FW_VCO_TUNE_AMPL_CONTROL_STATUS_INDEX))
#define FW_VCO_TUNE_AMPL_CONTROL_STATUS_FAIL (0x1 << (FW_VCO_TUNE_AMPL_CONTROL_STATUS_INDEX))

/* VCO_TUNE_AMPL_CONTROL BITS 1..0: Process state                           */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_INDEX (0)
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_MASK (0x3)
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_RESET (0x0 << (FW_VCO_TUNE_AMPL_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_DONE (0x1 << (FW_VCO_TUNE_AMPL_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_REQUEST (0x2 << (FW_VCO_TUNE_AMPL_CONTROL_ALGO_INDEX))
#define FW_VCO_TUNE_AMPL_CONTROL_ALGO_PROCESS (0x3 << (FW_VCO_TUNE_AMPL_CONTROL_ALGO_INDEX))
#endif /* FW_VCO_TUNE_AMPL_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_AMPL_CONTROL: VCO tune amplitude request        */
#define FW_VCO_TUNE_AMPL_CONTROL_RESET_VALUE (0x00)
#define FW_VCO_TUNE_AMPL_CONTROL (FW_BASE_ADDR + FW_VCO_TUNE_AMPL_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_AMPL_WANTED_ACM                             ***/
/* VCO tune amplitude set-up ACM                                            */
#ifndef FW_VCO_TUNE_AMPL_WANTED_ACM_PROTECT
#define FW_VCO_TUNE_AMPL_WANTED_ACM_PROTECT

#define FW_VCO_TUNE_AMPL_WANTED_ACM_OFFS (0x74)

/* VCO_TUNE_AMPL_WANTED_ACM BITS 31..0: Value                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_TUNE_AMPL_WANTED_ACM_ACM_INDEX (0)
#define FW_VCO_TUNE_AMPL_WANTED_ACM_ACM_MASK (0xFFFFFFFF)
#endif /* FW_VCO_TUNE_AMPL_WANTED_ACM_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_AMPL_WANTED_ACM: VCO tune amplitude set-up ACM  */
#define FW_VCO_TUNE_AMPL_WANTED_ACM_RESET_VALUE (0x00000000)
#define FW_VCO_TUNE_AMPL_WANTED_ACM (FW_BASE_ADDR + FW_VCO_TUNE_AMPL_WANTED_ACM_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_AMPL_GET_ACM_VCM                            ***/
/* VCO tune amplitude read ACM and VCM                                      */
#ifndef FW_VCO_TUNE_AMPL_GET_ACM_VCM_PROTECT
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_PROTECT

#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_OFFS (0x78)

/* VCO_TUNE_AMPL_GET_ACM_VCM BITS 31..16: VCM value                         */
/* numeric RW signed (2s comlement) field, reset to 0x0                     */
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_VCM_INDEX (16)
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_VCM_MASK (0xFFFF0000)

/* VCO_TUNE_AMPL_GET_ACM_VCM BITS 15..0: ACM value                          */
/* numeric RW signed (2s comlement) field, reset to 0x0                     */
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_ACM_INDEX (0)
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_ACM_MASK (0xFFFF)
#endif /* FW_VCO_TUNE_AMPL_GET_ACM_VCM_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_AMPL_GET_ACM_VCM: VCO tune amplitude read ACM   *
 * and VCM                                                                  */
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM_RESET_VALUE (0x00000000)
#define FW_VCO_TUNE_AMPL_GET_ACM_VCM (FW_BASE_ADDR + FW_VCO_TUNE_AMPL_GET_ACM_VCM_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_MAX                                         ***/
/* VCO maximum frequency                                                    */
#ifndef FW_VCO_FREQ_MAX_PROTECT
#define FW_VCO_FREQ_MAX_PROTECT

#define FW_VCO_FREQ_MAX_OFFS (0x7C)

/* VCO_FREQ_MAX BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_FREQ_MAX_VALUE_INDEX (0)
#define FW_VCO_FREQ_MAX_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_FREQ_MAX_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_MAX: VCO maximum frequency                      */
#define FW_VCO_FREQ_MAX_RESET_VALUE (0x00000000)
#define FW_VCO_FREQ_MAX (FW_BASE_ADDR + FW_VCO_FREQ_MAX_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_MIN                                         ***/
/* VCO minimum frequency                                                    */
#ifndef FW_VCO_FREQ_MIN_PROTECT
#define FW_VCO_FREQ_MIN_PROTECT

#define FW_VCO_FREQ_MIN_OFFS (0x80)

/* VCO_FREQ_MIN BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_FREQ_MIN_VALUE_INDEX (0)
#define FW_VCO_FREQ_MIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_FREQ_MIN_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_MIN: VCO minimum frequency                      */
#define FW_VCO_FREQ_MIN_RESET_VALUE (0x00000000)
#define FW_VCO_FREQ_MIN (FW_BASE_ADDR + FW_VCO_FREQ_MIN_OFFS)

/*** ATONT/FW_TOP/FW/VCO_VCM_DCM_MV                                       ***/
/* VCO VCM [mV]                                                             */
#ifndef FW_VCO_VCM_DCM_MV_PROTECT
#define FW_VCO_VCM_DCM_MV_PROTECT

#define FW_VCO_VCM_DCM_MV_OFFS (0x84)

/* VCO_VCM_DCM_MV BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_VCM_DCM_MV_VALUE_INDEX (0)
#define FW_VCO_VCM_DCM_MV_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_VCM_DCM_MV_PROTECT */
/* ATONT/FW_TOP/FW/VCO_VCM_DCM_MV: VCO VCM [mV]                             */
#define FW_VCO_VCM_DCM_MV_RESET_VALUE (0x00000000)
#define FW_VCO_VCM_DCM_MV (FW_BASE_ADDR + FW_VCO_VCM_DCM_MV_OFFS)

/*** ATONT/FW_TOP/FW/VCO_ACM_MV                                           ***/
/* VCO ACM [mV]                                                             */
#ifndef FW_VCO_ACM_MV_PROTECT
#define FW_VCO_ACM_MV_PROTECT

#define FW_VCO_ACM_MV_OFFS (0x88)

/* VCO_ACM_MV BITS 31..0: Value                                             */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_VCO_ACM_MV_VALUE_INDEX (0)
#define FW_VCO_ACM_MV_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_ACM_MV_PROTECT */
/* ATONT/FW_TOP/FW/VCO_ACM_MV: VCO ACM [mV]                                 */
#define FW_VCO_ACM_MV_RESET_VALUE (0x00000000)
#define FW_VCO_ACM_MV (FW_BASE_ADDR + FW_VCO_ACM_MV_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_CONTROL                                      ***/
/* Mixer DC corr initialization, launches DC tracking also when DONE is set */
#ifndef FW_DC_CORR_CONTROL_PROTECT
#define FW_DC_CORR_CONTROL_PROTECT

#define FW_DC_CORR_CONTROL_OFFS (0x8C)

/* DC_CORR_CONTROL BIT 4: Status DC algo execution                          */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DC_CORR_CONTROL_STATUS_INDEX (4)
#define FW_DC_CORR_CONTROL_STATUS_MASK (0x10)
#define FW_DC_CORR_CONTROL_STATUS_PASS (0x0 << (FW_DC_CORR_CONTROL_STATUS_INDEX))
#define FW_DC_CORR_CONTROL_STATUS_FAIL (0x1 << (FW_DC_CORR_CONTROL_STATUS_INDEX))

/* DC_CORR_CONTROL BITS 1..0: Initialization process state                  */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DC_CORR_CONTROL_ALGO_INDEX (0)
#define FW_DC_CORR_CONTROL_ALGO_MASK (0x3)
#define FW_DC_CORR_CONTROL_ALGO_RESET (0x0 << (FW_DC_CORR_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_CONTROL_ALGO_DONE (0x1 << (FW_DC_CORR_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_CONTROL_ALGO_REQUEST (0x2 << (FW_DC_CORR_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_CONTROL_ALGO_PROCESS (0x3 << (FW_DC_CORR_CONTROL_ALGO_INDEX))
#endif /* FW_DC_CORR_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_CONTROL: Mixer DC corr initialization, launches  *
 * DC tracking also when DONE is set                                        */
#define FW_DC_CORR_CONTROL_RESET_VALUE (0x00)
#define FW_DC_CORR_CONTROL (FW_BASE_ADDR + FW_DC_CORR_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_FREEZE                                       ***/
/* freeze DC correction tracking                                            */
#ifndef FW_DC_CORR_FREEZE_PROTECT
#define FW_DC_CORR_FREEZE_PROTECT

#define FW_DC_CORR_FREEZE_OFFS (0x90)

/* DC_CORR_FREEZE BIT 0: Freeze/UnFreeze DC correction                      */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DC_CORR_FREEZE_FREEZE_INDEX (0)
#define FW_DC_CORR_FREEZE_FREEZE_MASK (0x1)
#define FW_DC_CORR_FREEZE_FREEZE_UNFREEZE (0x0 << (FW_DC_CORR_FREEZE_FREEZE_INDEX))
#define FW_DC_CORR_FREEZE_FREEZE_FREEZE (0x1 << (FW_DC_CORR_FREEZE_FREEZE_INDEX))
#endif /* FW_DC_CORR_FREEZE_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_FREEZE: freeze DC correction tracking            */
#define FW_DC_CORR_FREEZE_RESET_VALUE (0x0)
#define FW_DC_CORR_FREEZE (FW_BASE_ADDR + FW_DC_CORR_FREEZE_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_VGA_CONTROL                                  ***/
/* DC corr initialization for VGA only. This DC correction is frozen then.  */
#ifndef FW_DC_CORR_VGA_CONTROL_PROTECT
#define FW_DC_CORR_VGA_CONTROL_PROTECT

#define FW_DC_CORR_VGA_CONTROL_OFFS (0x94)

/* DC_CORR_VGA_CONTROL BIT 4: Status DC algo execution                      */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DC_CORR_VGA_CONTROL_STATUS_INDEX (4)
#define FW_DC_CORR_VGA_CONTROL_STATUS_MASK (0x10)
#define FW_DC_CORR_VGA_CONTROL_STATUS_PASS (0x0 << (FW_DC_CORR_VGA_CONTROL_STATUS_INDEX))
#define FW_DC_CORR_VGA_CONTROL_STATUS_FAIL (0x1 << (FW_DC_CORR_VGA_CONTROL_STATUS_INDEX))

/* DC_CORR_VGA_CONTROL BITS 1..0: Initialization process state              */
/* symbolic RW field, reset to 0x0                                          */
#define FW_DC_CORR_VGA_CONTROL_ALGO_INDEX (0)
#define FW_DC_CORR_VGA_CONTROL_ALGO_MASK (0x3)
#define FW_DC_CORR_VGA_CONTROL_ALGO_RESET (0x0 << (FW_DC_CORR_VGA_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_VGA_CONTROL_ALGO_DONE (0x1 << (FW_DC_CORR_VGA_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_VGA_CONTROL_ALGO_REQUEST (0x2 << (FW_DC_CORR_VGA_CONTROL_ALGO_INDEX))
#define FW_DC_CORR_VGA_CONTROL_ALGO_PROCESS (0x3 << (FW_DC_CORR_VGA_CONTROL_ALGO_INDEX))
#endif /* FW_DC_CORR_VGA_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_VGA_CONTROL: DC corr initialization for VGA      *
 * only. This DC correction is frozen then.                                 */
#define FW_DC_CORR_VGA_CONTROL_RESET_VALUE (0x00)
#define FW_DC_CORR_VGA_CONTROL (FW_BASE_ADDR + FW_DC_CORR_VGA_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_WANTED_AMPL                                      ***/
/* VCO wanted amplitude [mV]                                                */
#ifndef FW_VCO_WANTED_AMPL_PROTECT
#define FW_VCO_WANTED_AMPL_PROTECT

#define FW_VCO_WANTED_AMPL_OFFS (0x98)

/* VCO_WANTED_AMPL BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0x12C                                */
#define FW_VCO_WANTED_AMPL_VALUE_INDEX (0)
#define FW_VCO_WANTED_AMPL_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_WANTED_AMPL_PROTECT */
/* ATONT/FW_TOP/FW/VCO_WANTED_AMPL: VCO wanted amplitude [mV]               */
#define FW_VCO_WANTED_AMPL_RESET_VALUE (0x0000012c)
#define FW_VCO_WANTED_AMPL (FW_BASE_ADDR + FW_VCO_WANTED_AMPL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_MARGIN                                      ***/
/* VCO frequency margin [kHz]                                               */
#ifndef FW_VCO_FREQ_MARGIN_PROTECT
#define FW_VCO_FREQ_MARGIN_PROTECT

#define FW_VCO_FREQ_MARGIN_OFFS (0x9C)

/* VCO_FREQ_MARGIN BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0xC350                               */
#define FW_VCO_FREQ_MARGIN_VALUE_INDEX (0)
#define FW_VCO_FREQ_MARGIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_FREQ_MARGIN_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_MARGIN: VCO frequency margin [kHz]              */
#define FW_VCO_FREQ_MARGIN_RESET_VALUE (0x0000c350)
#define FW_VCO_FREQ_MARGIN (FW_BASE_ADDR + FW_VCO_FREQ_MARGIN_OFFS)

/*** ATONT/FW_TOP/FW/VCO_FREQ_MEASURE_NUMBER                              ***/
/* VCO number of measures of the PLL period                                 */
#ifndef FW_VCO_FREQ_MEASURE_NUMBER_PROTECT
#define FW_VCO_FREQ_MEASURE_NUMBER_PROTECT

#define FW_VCO_FREQ_MEASURE_NUMBER_OFFS (0xA0)

/* VCO_FREQ_MEASURE_NUMBER BITS 31..0: Value                                */
/* numeric RW unsigned field, reset to 0xA                                  */
#define FW_VCO_FREQ_MEASURE_NUMBER_VALUE_INDEX (0)
#define FW_VCO_FREQ_MEASURE_NUMBER_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_FREQ_MEASURE_NUMBER_PROTECT */
/* ATONT/FW_TOP/FW/VCO_FREQ_MEASURE_NUMBER: VCO number of measures of the   *
 * PLL period                                                               */
#define FW_VCO_FREQ_MEASURE_NUMBER_RESET_VALUE (0x0000000a)
#define FW_VCO_FREQ_MEASURE_NUMBER (FW_BASE_ADDR + FW_VCO_FREQ_MEASURE_NUMBER_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_BAND_MAX_ITER                               ***/
/* VCO number of iter. during tune band algo                                */
#ifndef FW_VCO_TUNE_BAND_MAX_ITER_PROTECT
#define FW_VCO_TUNE_BAND_MAX_ITER_PROTECT

#define FW_VCO_TUNE_BAND_MAX_ITER_OFFS (0xA4)

/* VCO_TUNE_BAND_MAX_ITER BITS 31..0: Value                                 */
/* numeric RW unsigned field, reset to 0x64                                 */
#define FW_VCO_TUNE_BAND_MAX_ITER_VALUE_INDEX (0)
#define FW_VCO_TUNE_BAND_MAX_ITER_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_TUNE_BAND_MAX_ITER_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_BAND_MAX_ITER: VCO number of iter. during tune  *
 * band algo                                                                */
#define FW_VCO_TUNE_BAND_MAX_ITER_RESET_VALUE (0x00000064)
#define FW_VCO_TUNE_BAND_MAX_ITER (FW_BASE_ADDR + FW_VCO_TUNE_BAND_MAX_ITER_OFFS)

/*** ATONT/FW_TOP/FW/VCO_TUNE_AMPL_MAX_ITER                               ***/
/* VCO number of iter. during tune ampl algo                                */
#ifndef FW_VCO_TUNE_AMPL_MAX_ITER_PROTECT
#define FW_VCO_TUNE_AMPL_MAX_ITER_PROTECT

#define FW_VCO_TUNE_AMPL_MAX_ITER_OFFS (0xA8)

/* VCO_TUNE_AMPL_MAX_ITER BITS 31..0: Value                                 */
/* numeric RW unsigned field, reset to 0x20                                 */
#define FW_VCO_TUNE_AMPL_MAX_ITER_VALUE_INDEX (0)
#define FW_VCO_TUNE_AMPL_MAX_ITER_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_TUNE_AMPL_MAX_ITER_PROTECT */
/* ATONT/FW_TOP/FW/VCO_TUNE_AMPL_MAX_ITER: VCO number of iter. during tune  *
 * ampl algo                                                                */
#define FW_VCO_TUNE_AMPL_MAX_ITER_RESET_VALUE (0x00000020)
#define FW_VCO_TUNE_AMPL_MAX_ITER (FW_BASE_ADDR + FW_VCO_TUNE_AMPL_MAX_ITER_OFFS)

/*** ATONT/FW_TOP/FW/VCO_KVCO_VS_BAND_SWITCH                              ***/
/* Threshold above which KVCO is reduced                                    */
#ifndef FW_VCO_KVCO_VS_BAND_SWITCH_PROTECT
#define FW_VCO_KVCO_VS_BAND_SWITCH_PROTECT

#define FW_VCO_KVCO_VS_BAND_SWITCH_OFFS (0xAC)

/* VCO_KVCO_VS_BAND_SWITCH BITS 31..0: Value                                */
/* numeric RW unsigned field, reset to 0x3C                                 */
#define FW_VCO_KVCO_VS_BAND_SWITCH_VALUE_INDEX (0)
#define FW_VCO_KVCO_VS_BAND_SWITCH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_KVCO_VS_BAND_SWITCH_PROTECT */
/* ATONT/FW_TOP/FW/VCO_KVCO_VS_BAND_SWITCH: Threshold above which KVCO is reduced */
#define FW_VCO_KVCO_VS_BAND_SWITCH_RESET_VALUE (0x0000003c)
#define FW_VCO_KVCO_VS_BAND_SWITCH (FW_BASE_ADDR + FW_VCO_KVCO_VS_BAND_SWITCH_OFFS)

/*** ATONT/FW_TOP/FW/VCO_GM_VS_BAND_SWITCH                                ***/
/* Threshold above which GM is decreased                                    */
#ifndef FW_VCO_GM_VS_BAND_SWITCH_PROTECT
#define FW_VCO_GM_VS_BAND_SWITCH_PROTECT

#define FW_VCO_GM_VS_BAND_SWITCH_OFFS (0xB0)

/* VCO_GM_VS_BAND_SWITCH BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x30                                 */
#define FW_VCO_GM_VS_BAND_SWITCH_VALUE_INDEX (0)
#define FW_VCO_GM_VS_BAND_SWITCH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VCO_GM_VS_BAND_SWITCH_PROTECT */
/* ATONT/FW_TOP/FW/VCO_GM_VS_BAND_SWITCH: Threshold above which GM is decreased */
#define FW_VCO_GM_VS_BAND_SWITCH_RESET_VALUE (0x00000030)
#define FW_VCO_GM_VS_BAND_SWITCH (FW_BASE_ADDR + FW_VCO_GM_VS_BAND_SWITCH_OFFS)

/*** ATONT/FW_TOP/FW/PLL_SETLING_TIME                                     ***/
/* PLL setling time                                                         */
#ifndef FW_PLL_SETLING_TIME_PROTECT
#define FW_PLL_SETLING_TIME_PROTECT

#define FW_PLL_SETLING_TIME_OFFS (0xB4)

/* PLL_SETLING_TIME BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0xC8                                 */
#define FW_PLL_SETLING_TIME_VALUE_INDEX (0)
#define FW_PLL_SETLING_TIME_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PLL_SETLING_TIME_PROTECT */
/* ATONT/FW_TOP/FW/PLL_SETLING_TIME: PLL setling time                       */
#define FW_PLL_SETLING_TIME_RESET_VALUE (0x000000c8)
#define FW_PLL_SETLING_TIME (FW_BASE_ADDR + FW_PLL_SETLING_TIME_OFFS)

/*** ATONT/FW_TOP/FW/PLL_OOL_WAIT                                         ***/
/* PLL wait                                                                 */
#ifndef FW_PLL_OOL_WAIT_PROTECT
#define FW_PLL_OOL_WAIT_PROTECT

#define FW_PLL_OOL_WAIT_OFFS (0xB8)

/* PLL_OOL_WAIT BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x32                                 */
#define FW_PLL_OOL_WAIT_VALUE_INDEX (0)
#define FW_PLL_OOL_WAIT_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PLL_OOL_WAIT_PROTECT */
/* ATONT/FW_TOP/FW/PLL_OOL_WAIT: PLL wait                                   */
#define FW_PLL_OOL_WAIT_RESET_VALUE (0x00000032)
#define FW_PLL_OOL_WAIT (FW_BASE_ADDR + FW_PLL_OOL_WAIT_OFFS)

/*** ATONT/FW_TOP/FW/LOPLL_AC_OFFSET                                      ***/
/* Aplitude regulation offset                                               */
#ifndef FW_LOPLL_AC_OFFSET_PROTECT
#define FW_LOPLL_AC_OFFSET_PROTECT

#define FW_LOPLL_AC_OFFSET_OFFS (0xBC)

/* LOPLL_AC_OFFSET BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0x16                                 */
#define FW_LOPLL_AC_OFFSET_VALUE_INDEX (0)
#define FW_LOPLL_AC_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOPLL_AC_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/LOPLL_AC_OFFSET: Aplitude regulation offset              */
#define FW_LOPLL_AC_OFFSET_RESET_VALUE (0x00000016)
#define FW_LOPLL_AC_OFFSET (FW_BASE_ADDR + FW_LOPLL_AC_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/LOPLL_VCO_VCM                                        ***/
/* Set VCO common mode threshold                                            */
#ifndef FW_LOPLL_VCO_VCM_PROTECT
#define FW_LOPLL_VCO_VCM_PROTECT

#define FW_LOPLL_VCO_VCM_OFFS (0xC0)

/* LOPLL_VCO_VCM BITS 31..0: Value                                          */
/* numeric RW unsigned field, reset to 0x2                                  */
#define FW_LOPLL_VCO_VCM_VALUE_INDEX (0)
#define FW_LOPLL_VCO_VCM_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOPLL_VCO_VCM_PROTECT */
/* ATONT/FW_TOP/FW/LOPLL_VCO_VCM: Set VCO common mode threshold             */
#define FW_LOPLL_VCO_VCM_RESET_VALUE (0x00000002)
#define FW_LOPLL_VCO_VCM (FW_BASE_ADDR + FW_LOPLL_VCO_VCM_OFFS)

/*** ATONT/FW_TOP/FW/DC_NB_OF_MEASURES                                    ***/
/* Nb of measures for the DC initialization                                 */
#ifndef FW_DC_NB_OF_MEASURES_PROTECT
#define FW_DC_NB_OF_MEASURES_PROTECT

#define FW_DC_NB_OF_MEASURES_OFFS (0xC4)

/* DC_NB_OF_MEASURES BITS 31..0: Value                                      */
/* numeric RW unsigned field, reset to 0x5                                  */
#define FW_DC_NB_OF_MEASURES_VALUE_INDEX (0)
#define FW_DC_NB_OF_MEASURES_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_NB_OF_MEASURES_PROTECT */
/* ATONT/FW_TOP/FW/DC_NB_OF_MEASURES: Nb of measures for the DC initialization */
#define FW_DC_NB_OF_MEASURES_RESET_VALUE (0x00000005)
#define FW_DC_NB_OF_MEASURES (FW_BASE_ADDR + FW_DC_NB_OF_MEASURES_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_VGA_TH                                       ***/
/* DC tracking VGA threshold (linear gain*128)                              */
#ifndef FW_DC_CORR_VGA_TH_PROTECT
#define FW_DC_CORR_VGA_TH_PROTECT

#define FW_DC_CORR_VGA_TH_OFFS (0xC8)

/* DC_CORR_VGA_TH BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x2D0                                */
#define FW_DC_CORR_VGA_TH_VALUE_INDEX (0)
#define FW_DC_CORR_VGA_TH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_CORR_VGA_TH_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_VGA_TH: DC tracking VGA threshold (linear gain*  *
 * 128)                                                                     */
#define FW_DC_CORR_VGA_TH_RESET_VALUE (0x000002d0)
#define FW_DC_CORR_VGA_TH (FW_BASE_ADDR + FW_DC_CORR_VGA_TH_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_SF_TH                                        ***/
/* DC tracking sign filter threshold                                        */
#ifndef FW_DC_CORR_SF_TH_PROTECT
#define FW_DC_CORR_SF_TH_PROTECT

#define FW_DC_CORR_SF_TH_OFFS (0xCC)

/* DC_CORR_SF_TH BITS 31..0: Value                                          */
/* numeric RW unsigned field, reset to 0x400                                */
#define FW_DC_CORR_SF_TH_VALUE_INDEX (0)
#define FW_DC_CORR_SF_TH_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_CORR_SF_TH_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_SF_TH: DC tracking sign filter threshold         */
#define FW_DC_CORR_SF_TH_RESET_VALUE (0x00000400)
#define FW_DC_CORR_SF_TH (FW_BASE_ADDR + FW_DC_CORR_SF_TH_OFFS)

/*** ATONT/FW_TOP/FW/DC_CORR_RUN_PERIOD                                   ***/
/* DC tracking period ms                                                    */
#ifndef FW_DC_CORR_RUN_PERIOD_PROTECT
#define FW_DC_CORR_RUN_PERIOD_PROTECT

#define FW_DC_CORR_RUN_PERIOD_OFFS (0xD0)

/* DC_CORR_RUN_PERIOD BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x8                                  */
#define FW_DC_CORR_RUN_PERIOD_VALUE_INDEX (0)
#define FW_DC_CORR_RUN_PERIOD_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_CORR_RUN_PERIOD_PROTECT */
/* ATONT/FW_TOP/FW/DC_CORR_RUN_PERIOD: DC tracking period ms                */
#define FW_DC_CORR_RUN_PERIOD_RESET_VALUE (0x00000008)
#define FW_DC_CORR_RUN_PERIOD (FW_BASE_ADDR + FW_DC_CORR_RUN_PERIOD_OFFS)

/*** ATONT/FW_TOP/FW/DC_LOW_THRESHOLD                                     ***/
/* DC low threshold to determine the SF filter time constant                */
#ifndef FW_DC_LOW_THRESHOLD_PROTECT
#define FW_DC_LOW_THRESHOLD_PROTECT

#define FW_DC_LOW_THRESHOLD_OFFS (0xD4)

/* DC_LOW_THRESHOLD BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x2000                               */
#define FW_DC_LOW_THRESHOLD_VALUE_INDEX (0)
#define FW_DC_LOW_THRESHOLD_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_LOW_THRESHOLD_PROTECT */
/* ATONT/FW_TOP/FW/DC_LOW_THRESHOLD: DC low threshold to determine the SF   *
 * filter time constant                                                     */
#define FW_DC_LOW_THRESHOLD_RESET_VALUE (0x00002000)
#define FW_DC_LOW_THRESHOLD (FW_BASE_ADDR + FW_DC_LOW_THRESHOLD_OFFS)

/*** ATONT/FW_TOP/FW/DC_HIGH_THRESHOLD                                    ***/
/* DC high threshold to determine the SF filter time constant               */
#ifndef FW_DC_HIGH_THRESHOLD_PROTECT
#define FW_DC_HIGH_THRESHOLD_PROTECT

#define FW_DC_HIGH_THRESHOLD_OFFS (0xD8)

/* DC_HIGH_THRESHOLD BITS 31..0: Value                                      */
/* numeric RW unsigned field, reset to 0x6000                               */
#define FW_DC_HIGH_THRESHOLD_VALUE_INDEX (0)
#define FW_DC_HIGH_THRESHOLD_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_HIGH_THRESHOLD_PROTECT */
/* ATONT/FW_TOP/FW/DC_HIGH_THRESHOLD: DC high threshold to determine the    *
 * SF filter time constant                                                  */
#define FW_DC_HIGH_THRESHOLD_RESET_VALUE (0x00006000)
#define FW_DC_HIGH_THRESHOLD (FW_BASE_ADDR + FW_DC_HIGH_THRESHOLD_OFFS)

/*** ATONT/FW_TOP/FW/DC_SHORT_DELAY                                       ***/
/* Short delay between 2 DC measurements [us] (DC above DC_HIGH_THRESHOLD)  */
#ifndef FW_DC_SHORT_DELAY_PROTECT
#define FW_DC_SHORT_DELAY_PROTECT

#define FW_DC_SHORT_DELAY_OFFS (0xDC)

/* DC_SHORT_DELAY BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x32                                 */
#define FW_DC_SHORT_DELAY_VALUE_INDEX (0)
#define FW_DC_SHORT_DELAY_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_SHORT_DELAY_PROTECT */
/* ATONT/FW_TOP/FW/DC_SHORT_DELAY: Short delay between 2 DC measurements [  *
 * us] (DC above DC_HIGH_THRESHOLD)                                         */
#define FW_DC_SHORT_DELAY_RESET_VALUE (0x00000032)
#define FW_DC_SHORT_DELAY (FW_BASE_ADDR + FW_DC_SHORT_DELAY_OFFS)

/*** ATONT/FW_TOP/FW/DC_MIDDLE_DELAY                                      ***/
/* Middle delay between 2 DC measurements [us](DC above DC_LOW_THRESHOLD)   */
#ifndef FW_DC_MIDDLE_DELAY_PROTECT
#define FW_DC_MIDDLE_DELAY_PROTECT

#define FW_DC_MIDDLE_DELAY_OFFS (0xE0)

/* DC_MIDDLE_DELAY BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0x12C                                */
#define FW_DC_MIDDLE_DELAY_VALUE_INDEX (0)
#define FW_DC_MIDDLE_DELAY_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_MIDDLE_DELAY_PROTECT */
/* ATONT/FW_TOP/FW/DC_MIDDLE_DELAY: Middle delay between 2 DC measurements  *
 * [us](DC above DC_LOW_THRESHOLD)                                          */
#define FW_DC_MIDDLE_DELAY_RESET_VALUE (0x0000012c)
#define FW_DC_MIDDLE_DELAY (FW_BASE_ADDR + FW_DC_MIDDLE_DELAY_OFFS)

/*** ATONT/FW_TOP/FW/DC_LONG_DELAY                                        ***/
/* Long delay between 2 DC measurements [us] (DC below DC_LOW_THRESHOLD)    */
#ifndef FW_DC_LONG_DELAY_PROTECT
#define FW_DC_LONG_DELAY_PROTECT

#define FW_DC_LONG_DELAY_OFFS (0xE4)

/* DC_LONG_DELAY BITS 31..0: Value                                          */
/* numeric RW unsigned field, reset to 0x258                                */
#define FW_DC_LONG_DELAY_VALUE_INDEX (0)
#define FW_DC_LONG_DELAY_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_LONG_DELAY_PROTECT */
/* ATONT/FW_TOP/FW/DC_LONG_DELAY: Long delay between 2 DC measurements [us  *
 * ] (DC below DC_LOW_THRESHOLD)                                            */
#define FW_DC_LONG_DELAY_RESET_VALUE (0x00000258)
#define FW_DC_LONG_DELAY (FW_BASE_ADDR + FW_DC_LONG_DELAY_OFFS)

/*** ATONT/FW_TOP/FW/DC_SF_SHORT_TIME_CONST                               ***/
/* Short SF time contant during DC init. Fc=126*2^x (applied for DC above   *
 * DC_HIGH_THRESHOLD)                                                       */
#ifndef FW_DC_SF_SHORT_TIME_CONST_PROTECT
#define FW_DC_SF_SHORT_TIME_CONST_PROTECT

#define FW_DC_SF_SHORT_TIME_CONST_OFFS (0xE8)

/* DC_SF_SHORT_TIME_CONST BITS 31..0: Value                                 */
/* numeric RW unsigned field, reset to 0x9                                  */
#define FW_DC_SF_SHORT_TIME_CONST_VALUE_INDEX (0)
#define FW_DC_SF_SHORT_TIME_CONST_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_SF_SHORT_TIME_CONST_PROTECT */
/* ATONT/FW_TOP/FW/DC_SF_SHORT_TIME_CONST: Short SF time contant during DC  *
 * init. Fc=126*2^x (applied for DC above DC_HIGH_THRESHOLD)                */
#define FW_DC_SF_SHORT_TIME_CONST_RESET_VALUE (0x00000009)
#define FW_DC_SF_SHORT_TIME_CONST (FW_BASE_ADDR + FW_DC_SF_SHORT_TIME_CONST_OFFS)

/*** ATONT/FW_TOP/FW/DC_SF_MIDDLE_TIME_CONST                              ***/
/* Middle SF time contant during DC init. Fc=126*2^x (applied for DC above  *
 * DC_LOW_THRESHOLD)                                                        */
#ifndef FW_DC_SF_MIDDLE_TIME_CONST_PROTECT
#define FW_DC_SF_MIDDLE_TIME_CONST_PROTECT

#define FW_DC_SF_MIDDLE_TIME_CONST_OFFS (0xEC)

/* DC_SF_MIDDLE_TIME_CONST BITS 31..0: Value                                */
/* numeric RW unsigned field, reset to 0x6                                  */
#define FW_DC_SF_MIDDLE_TIME_CONST_VALUE_INDEX (0)
#define FW_DC_SF_MIDDLE_TIME_CONST_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_SF_MIDDLE_TIME_CONST_PROTECT */
/* ATONT/FW_TOP/FW/DC_SF_MIDDLE_TIME_CONST: Middle SF time contant during   *
 * DC init. Fc=126*2^x (applied for DC above DC_LOW_THRESHOLD)              */
#define FW_DC_SF_MIDDLE_TIME_CONST_RESET_VALUE (0x00000006)
#define FW_DC_SF_MIDDLE_TIME_CONST (FW_BASE_ADDR + FW_DC_SF_MIDDLE_TIME_CONST_OFFS)

/*** ATONT/FW_TOP/FW/DC_SF_LONG_TIME_CONST                                ***/
/* Long SF time contant during DC init. Fc=126*2^x (applied for DC below    *
 * DC_LOW_THRESHOLD)                                                        */
#ifndef FW_DC_SF_LONG_TIME_CONST_PROTECT
#define FW_DC_SF_LONG_TIME_CONST_PROTECT

#define FW_DC_SF_LONG_TIME_CONST_OFFS (0xF0)

/* DC_SF_LONG_TIME_CONST BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x5                                  */
#define FW_DC_SF_LONG_TIME_CONST_VALUE_INDEX (0)
#define FW_DC_SF_LONG_TIME_CONST_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_SF_LONG_TIME_CONST_PROTECT */
/* ATONT/FW_TOP/FW/DC_SF_LONG_TIME_CONST: Long SF time contant during DC    *
 * init. Fc=126*2^x (applied for DC below DC_LOW_THRESHOLD)                 */
#define FW_DC_SF_LONG_TIME_CONST_RESET_VALUE (0x00000005)
#define FW_DC_SF_LONG_TIME_CONST (FW_BASE_ADDR + FW_DC_SF_LONG_TIME_CONST_OFFS)

/*** ATONT/FW_TOP/FW/DC_SF_TRACKING_TIME_CONST                            ***/
/* Very long SF time contant for DC tracking. Fc=126*2^x                    */
#ifndef FW_DC_SF_TRACKING_TIME_CONST_PROTECT
#define FW_DC_SF_TRACKING_TIME_CONST_PROTECT

#define FW_DC_SF_TRACKING_TIME_CONST_OFFS (0xF4)

/* DC_SF_TRACKING_TIME_CONST BITS 31..0: Value                              */
/* numeric RW unsigned field, reset to 0xFFFFFFFF                           */
#define FW_DC_SF_TRACKING_TIME_CONST_VALUE_INDEX (0)
#define FW_DC_SF_TRACKING_TIME_CONST_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_DC_SF_TRACKING_TIME_CONST_PROTECT */
/* ATONT/FW_TOP/FW/DC_SF_TRACKING_TIME_CONST: Very long SF time contant     *
 * for DC tracking. Fc=126*2^x                                              */
#define FW_DC_SF_TRACKING_TIME_CONST_RESET_VALUE (0xffffffff)
#define FW_DC_SF_TRACKING_TIME_CONST (FW_BASE_ADDR + FW_DC_SF_TRACKING_TIME_CONST_OFFS)

/*** ATONT/FW_TOP/FW/REF_DC_VALUE                                         ***/
/* DC tuning reference (see specification of RF_DC register)                */
#ifndef FW_REF_DC_VALUE_PROTECT
#define FW_REF_DC_VALUE_PROTECT

#define FW_REF_DC_VALUE_OFFS (0xF8)

/* REF_DC_VALUE BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0xC                                  */
#define FW_REF_DC_VALUE_VALUE_INDEX (0)
#define FW_REF_DC_VALUE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_REF_DC_VALUE_PROTECT */
/* ATONT/FW_TOP/FW/REF_DC_VALUE: DC tuning reference (see specification of  *
 * RF_DC register)                                                          */
#define FW_REF_DC_VALUE_RESET_VALUE (0x0000000c)
#define FW_REF_DC_VALUE (FW_BASE_ADDR + FW_REF_DC_VALUE_OFFS)

/*** ATONT/FW_TOP/FW/VGA_G_FOR_GLOBAL_DC_CORR_INIT                        ***/
/* VGA gain for the global DC correction initialization [dB/256]            */
#ifndef FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_PROTECT
#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_PROTECT

#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_OFFS (0xFC)

/* VGA_G_FOR_GLOBAL_DC_CORR_INIT BITS 31..0: Value                          */
/* numeric RW unsigned field, reset to 0x1400                               */
#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_VALUE_INDEX (0)
#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_PROTECT */
/* ATONT/FW_TOP/FW/VGA_G_FOR_GLOBAL_DC_CORR_INIT: VGA gain for the global   *
 * DC correction initialization [dB/256]                                    */
#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_RESET_VALUE (0x00001400)
#define FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT (FW_BASE_ADDR + FW_VGA_G_FOR_GLOBAL_DC_CORR_INIT_OFFS)

/*** ATONT/FW_TOP/FW/VGA_G_FOR_VGA_DC_CORR_INIT                           ***/
/* VGA gain for the VGA only DC correction initialization [dB/256]          */
#ifndef FW_VGA_G_FOR_VGA_DC_CORR_INIT_PROTECT
#define FW_VGA_G_FOR_VGA_DC_CORR_INIT_PROTECT

#define FW_VGA_G_FOR_VGA_DC_CORR_INIT_OFFS (0x100)

/* VGA_G_FOR_VGA_DC_CORR_INIT BITS 31..0: Value                             */
/* numeric RW unsigned field, reset to 0x1B00                               */
#define FW_VGA_G_FOR_VGA_DC_CORR_INIT_VALUE_INDEX (0)
#define FW_VGA_G_FOR_VGA_DC_CORR_INIT_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_VGA_G_FOR_VGA_DC_CORR_INIT_PROTECT */
/* ATONT/FW_TOP/FW/VGA_G_FOR_VGA_DC_CORR_INIT: VGA gain for the VGA only    *
 * DC correction initialization [dB/256]                                    */
#define FW_VGA_G_FOR_VGA_DC_CORR_INIT_RESET_VALUE (0x00001b00)
#define FW_VGA_G_FOR_VGA_DC_CORR_INIT (FW_BASE_ADDR + FW_VGA_G_FOR_VGA_DC_CORR_INIT_OFFS)

/*** ATONT/FW_TOP/FW/LNA_GAIN_MIN                                         ***/
/* LNA minimum gain [dB/256]                                                */
#ifndef FW_LNA_GAIN_MIN_PROTECT
#define FW_LNA_GAIN_MIN_PROTECT

#define FW_LNA_GAIN_MIN_OFFS (0x104)

/* LNA_GAIN_MIN BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0xFFFFFA00                           */
#define FW_LNA_GAIN_MIN_VALUE_INDEX (0)
#define FW_LNA_GAIN_MIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_GAIN_MIN_PROTECT */
/* ATONT/FW_TOP/FW/LNA_GAIN_MIN: LNA minimum gain [dB/256]                  */
#define FW_LNA_GAIN_MIN_RESET_VALUE (0xfffffa00)
#define FW_LNA_GAIN_MIN (FW_BASE_ADDR + FW_LNA_GAIN_MIN_OFFS)

/*** ATONT/FW_TOP/FW/LNA_GAIN_MAX                                         ***/
/* LNA maximum gain [dB/256]                                                */
#ifndef FW_LNA_GAIN_MAX_PROTECT
#define FW_LNA_GAIN_MAX_PROTECT

#define FW_LNA_GAIN_MAX_OFFS (0x108)

/* LNA_GAIN_MAX BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x14C0                               */
#define FW_LNA_GAIN_MAX_VALUE_INDEX (0)
#define FW_LNA_GAIN_MAX_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_GAIN_MAX_PROTECT */
/* ATONT/FW_TOP/FW/LNA_GAIN_MAX: LNA maximum gain [dB/256]                  */
#define FW_LNA_GAIN_MAX_RESET_VALUE (0x000014c0)
#define FW_LNA_GAIN_MAX (FW_BASE_ADDR + FW_LNA_GAIN_MAX_OFFS)

/*** ATONT/FW_TOP/FW/LNA_AGC_INC_TIME                                     ***/
/* LNA: time to increment the gain [ms]                                     */
#ifndef FW_LNA_AGC_INC_TIME_PROTECT
#define FW_LNA_AGC_INC_TIME_PROTECT

#define FW_LNA_AGC_INC_TIME_OFFS (0x10C)

/* LNA_AGC_INC_TIME BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x19                                 */
#define FW_LNA_AGC_INC_TIME_VALUE_INDEX (0)
#define FW_LNA_AGC_INC_TIME_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_AGC_INC_TIME_PROTECT */
/* ATONT/FW_TOP/FW/LNA_AGC_INC_TIME: LNA: time to increment the gain [ms]   */
#define FW_LNA_AGC_INC_TIME_RESET_VALUE (0x00000019)
#define FW_LNA_AGC_INC_TIME (FW_BASE_ADDR + FW_LNA_AGC_INC_TIME_OFFS)

/*** ATONT/FW_TOP/FW/LNA_AGC_DEC_TIME                                     ***/
/* LNA: time to decrement the gain [ms]                                     */
#ifndef FW_LNA_AGC_DEC_TIME_PROTECT
#define FW_LNA_AGC_DEC_TIME_PROTECT

#define FW_LNA_AGC_DEC_TIME_OFFS (0x110)

/* LNA_AGC_DEC_TIME BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x1                                  */
#define FW_LNA_AGC_DEC_TIME_VALUE_INDEX (0)
#define FW_LNA_AGC_DEC_TIME_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_AGC_DEC_TIME_PROTECT */
/* ATONT/FW_TOP/FW/LNA_AGC_DEC_TIME: LNA: time to decrement the gain [ms]   */
#define FW_LNA_AGC_DEC_TIME_RESET_VALUE (0x00000001)
#define FW_LNA_AGC_DEC_TIME (FW_BASE_ADDR + FW_LNA_AGC_DEC_TIME_OFFS)

/*** ATONT/FW_TOP/FW/LNA_AGC_INC_OFFSET                                   ***/
/* LNA: gain increment step [dB/256]                                        */
#ifndef FW_LNA_AGC_INC_OFFSET_PROTECT
#define FW_LNA_AGC_INC_OFFSET_PROTECT

#define FW_LNA_AGC_INC_OFFSET_OFFS (0x114)

/* LNA_AGC_INC_OFFSET BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x36                                 */
#define FW_LNA_AGC_INC_OFFSET_VALUE_INDEX (0)
#define FW_LNA_AGC_INC_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_AGC_INC_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/LNA_AGC_INC_OFFSET: LNA: gain increment step [dB/256]    */
#define FW_LNA_AGC_INC_OFFSET_RESET_VALUE (0x00000036)
#define FW_LNA_AGC_INC_OFFSET (FW_BASE_ADDR + FW_LNA_AGC_INC_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/LNA_AGC_DEC_OFFSET                                   ***/
/* LNA: gain decrement step [dB/256]                                        */
#ifndef FW_LNA_AGC_DEC_OFFSET_PROTECT
#define FW_LNA_AGC_DEC_OFFSET_PROTECT

#define FW_LNA_AGC_DEC_OFFSET_OFFS (0x118)

/* LNA_AGC_DEC_OFFSET BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x36                                 */
#define FW_LNA_AGC_DEC_OFFSET_VALUE_INDEX (0)
#define FW_LNA_AGC_DEC_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_AGC_DEC_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/LNA_AGC_DEC_OFFSET: LNA: gain decrement step [dB/256]    */
#define FW_LNA_AGC_DEC_OFFSET_RESET_VALUE (0x00000036)
#define FW_LNA_AGC_DEC_OFFSET (FW_BASE_ADDR + FW_LNA_AGC_DEC_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/LNA2PMA_GAIN_THRESH                                  ***/
/* LNA gain threshold (level) above which a PMA overload acts on LNA AGC    *
 * first [dB/256]                                                           */
#ifndef FW_LNA2PMA_GAIN_THRESH_PROTECT
#define FW_LNA2PMA_GAIN_THRESH_PROTECT

#define FW_LNA2PMA_GAIN_THRESH_OFFS (0x11C)

/* LNA2PMA_GAIN_THRESH BITS 31..16: Threahold for UHF band                  */
/* numeric RW unsigned field, reset to 0xCC0                                */
#define FW_LNA2PMA_GAIN_THRESH_UHF_INDEX (16)
#define FW_LNA2PMA_GAIN_THRESH_UHF_MASK (0xFFFF0000)

/* LNA2PMA_GAIN_THRESH BITS 15..0: Threahold for VHF band                   */
/* numeric RW unsigned field, reset to 0xEC0                                */
#define FW_LNA2PMA_GAIN_THRESH_VHF_INDEX (0)
#define FW_LNA2PMA_GAIN_THRESH_VHF_MASK (0xFFFF)
#endif /* FW_LNA2PMA_GAIN_THRESH_PROTECT */
/* ATONT/FW_TOP/FW/LNA2PMA_GAIN_THRESH: LNA gain threshold (level) above    *
 * which a PMA overload acts on LNA AGC first [dB/256]                      */
#define FW_LNA2PMA_GAIN_THRESH_RESET_VALUE (0x0cc00ec0)
#define FW_LNA2PMA_GAIN_THRESH (FW_BASE_ADDR + FW_LNA2PMA_GAIN_THRESH_OFFS)

/*** ATONT/FW_TOP/FW/LNA_BIAS                                             ***/
/* LNA bias                                                                 */
#ifndef FW_LNA_BIAS_PROTECT
#define FW_LNA_BIAS_PROTECT

#define FW_LNA_BIAS_OFFS (0x120)

/* LNA_BIAS BITS 31..0: Value                                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_LNA_BIAS_VALUE_INDEX (0)
#define FW_LNA_BIAS_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_BIAS_PROTECT */
/* ATONT/FW_TOP/FW/LNA_BIAS: LNA bias                                       */
#define FW_LNA_BIAS_RESET_VALUE (0x00000000)
#define FW_LNA_BIAS (FW_BASE_ADDR + FW_LNA_BIAS_OFFS)

/*** ATONT/FW_TOP/FW/LPTHR                                                ***/
/* Loop-through attenuation for modes: tuner A, tuner B or cascaded         */
#ifndef FW_LPTHR_PROTECT
#define FW_LPTHR_PROTECT

#define FW_LPTHR_OFFS (0x124)

/* LPTHR BITS 23..16: Value for TUNERA_TUNERB                               */
/* numeric RW unsigned field, reset to 0x1                                  */
#define FW_LPTHR_TUNERA_TUNERB_INDEX (16)
#define FW_LPTHR_TUNERA_TUNERB_MASK (0xFF0000)

/* LPTHR BITS 15..8: Value for TUNERB_LOOPTHROUGH                           */
/* numeric RW unsigned field, reset to 0x1                                  */
#define FW_LPTHR_B_LOOPTHROUGH_INDEX (8)
#define FW_LPTHR_B_LOOPTHROUGH_MASK (0xFF00)

/* LPTHR BITS 7..0: Value for TUNERA_LOOPTHROUGH                            */
/* numeric RW unsigned field, reset to 0x10                                 */
#define FW_LPTHR_A_LOOPTHROUGH_INDEX (0)
#define FW_LPTHR_A_LOOPTHROUGH_MASK (0xFF)
#endif /* FW_LPTHR_PROTECT */
/* ATONT/FW_TOP/FW/LPTHR: Loop-through attenuation for modes: tuner A,      *
 * tuner B or cascaded                                                      */
#define FW_LPTHR_RESET_VALUE (0x010110)
#define FW_LPTHR (FW_BASE_ADDR + FW_LPTHR_OFFS)

/*** ATONT/FW_TOP/FW/LNA_INIT_PAUSE                                       ***/
/* LNA pause between each iteration of the search [us]                      */
#ifndef FW_LNA_INIT_PAUSE_PROTECT
#define FW_LNA_INIT_PAUSE_PROTECT

#define FW_LNA_INIT_PAUSE_OFFS (0x128)

/* LNA_INIT_PAUSE BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x32                                 */
#define FW_LNA_INIT_PAUSE_VALUE_INDEX (0)
#define FW_LNA_INIT_PAUSE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_INIT_PAUSE_PROTECT */
/* ATONT/FW_TOP/FW/LNA_INIT_PAUSE: LNA pause between each iteration of the  *
 * search [us]                                                              */
#define FW_LNA_INIT_PAUSE_RESET_VALUE (0x00000032)
#define FW_LNA_INIT_PAUSE (FW_BASE_ADDR + FW_LNA_INIT_PAUSE_OFFS)

/*** ATONT/FW_TOP/FW/LNA_COARSE2FINE_PAUSE                                ***/
/* LNA pause between coarse search and fine search  [us]                    */
#ifndef FW_LNA_COARSE2FINE_PAUSE_PROTECT
#define FW_LNA_COARSE2FINE_PAUSE_PROTECT

#define FW_LNA_COARSE2FINE_PAUSE_OFFS (0x12C)

/* LNA_COARSE2FINE_PAUSE BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x1F4                                */
#define FW_LNA_COARSE2FINE_PAUSE_VALUE_INDEX (0)
#define FW_LNA_COARSE2FINE_PAUSE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_COARSE2FINE_PAUSE_PROTECT */
/* ATONT/FW_TOP/FW/LNA_COARSE2FINE_PAUSE: LNA pause between coarse search   *
 * and fine search  [us]                                                    */
#define FW_LNA_COARSE2FINE_PAUSE_RESET_VALUE (0x000001f4)
#define FW_LNA_COARSE2FINE_PAUSE (FW_BASE_ADDR + FW_LNA_COARSE2FINE_PAUSE_OFFS)

/*** ATONT/FW_TOP/FW/WBD_THRESH1                                          ***/
/* WBD1 threshold                                                           */
#ifndef FW_WBD_THRESH1_PROTECT
#define FW_WBD_THRESH1_PROTECT

#define FW_WBD_THRESH1_OFFS (0x130)

/* WBD_THRESH1 BITS 31..0: Value                                            */
/* numeric RW unsigned field, reset to 0x6                                  */
#define FW_WBD_THRESH1_VALUE_INDEX (0)
#define FW_WBD_THRESH1_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_WBD_THRESH1_PROTECT */
/* ATONT/FW_TOP/FW/WBD_THRESH1: WBD1 threshold                              */
#define FW_WBD_THRESH1_RESET_VALUE (0x00000006)
#define FW_WBD_THRESH1 (FW_BASE_ADDR + FW_WBD_THRESH1_OFFS)

/*** ATONT/FW_TOP/FW/WBD_THRESH1_LNA_STARTUP                              ***/
/* WBD1 threshold for the LNA startup                                       */
#ifndef FW_WBD_THRESH1_LNA_STARTUP_PROTECT
#define FW_WBD_THRESH1_LNA_STARTUP_PROTECT

#define FW_WBD_THRESH1_LNA_STARTUP_OFFS (0x134)

/* WBD_THRESH1_LNA_STARTUP BITS 31..0: Value                                */
/* numeric RW unsigned field, reset to 0x2                                  */
#define FW_WBD_THRESH1_LNA_STARTUP_VALUE_INDEX (0)
#define FW_WBD_THRESH1_LNA_STARTUP_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_WBD_THRESH1_LNA_STARTUP_PROTECT */
/* ATONT/FW_TOP/FW/WBD_THRESH1_LNA_STARTUP: WBD1 threshold for the LNA startup */
#define FW_WBD_THRESH1_LNA_STARTUP_RESET_VALUE (0x00000002)
#define FW_WBD_THRESH1_LNA_STARTUP (FW_BASE_ADDR + FW_WBD_THRESH1_LNA_STARTUP_OFFS)

/*** ATONT/FW_TOP/FW/LNA_STARTUP_GAIN_STEP                                ***/
/* Gain step for LNA startup                                                */
#ifndef FW_LNA_STARTUP_GAIN_STEP_PROTECT
#define FW_LNA_STARTUP_GAIN_STEP_PROTECT

#define FW_LNA_STARTUP_GAIN_STEP_OFFS (0x138)

/* LNA_STARTUP_GAIN_STEP BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x300                                */
#define FW_LNA_STARTUP_GAIN_STEP_VALUE_INDEX (0)
#define FW_LNA_STARTUP_GAIN_STEP_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_STARTUP_GAIN_STEP_PROTECT */
/* ATONT/FW_TOP/FW/LNA_STARTUP_GAIN_STEP: Gain step for LNA startup         */
#define FW_LNA_STARTUP_GAIN_STEP_RESET_VALUE (0x00000300)
#define FW_LNA_STARTUP_GAIN_STEP (FW_BASE_ADDR + FW_LNA_STARTUP_GAIN_STEP_OFFS)

/*** ATONT/FW_TOP/FW/PMA_GAIN_MIN                                         ***/
/* PMA min gain [dB/256]                                                    */
#ifndef FW_PMA_GAIN_MIN_PROTECT
#define FW_PMA_GAIN_MIN_PROTECT

#define FW_PMA_GAIN_MIN_OFFS (0x13C)

/* PMA_GAIN_MIN BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x200                                */
#define FW_PMA_GAIN_MIN_VALUE_INDEX (0)
#define FW_PMA_GAIN_MIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_GAIN_MIN_PROTECT */
/* ATONT/FW_TOP/FW/PMA_GAIN_MIN: PMA min gain [dB/256]                      */
#define FW_PMA_GAIN_MIN_RESET_VALUE (0x00000200)
#define FW_PMA_GAIN_MIN (FW_BASE_ADDR + FW_PMA_GAIN_MIN_OFFS)

/*** ATONT/FW_TOP/FW/PMA_GAIN_MAX                                         ***/
/* PMA max gain [dB/256]                                                    */
#ifndef FW_PMA_GAIN_MAX_PROTECT
#define FW_PMA_GAIN_MAX_PROTECT

#define FW_PMA_GAIN_MAX_OFFS (0x140)

/* PMA_GAIN_MAX BITS 31..0: Value                                           */
/* numeric RW unsigned field, reset to 0x1600                               */
#define FW_PMA_GAIN_MAX_VALUE_INDEX (0)
#define FW_PMA_GAIN_MAX_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_GAIN_MAX_PROTECT */
/* ATONT/FW_TOP/FW/PMA_GAIN_MAX: PMA max gain [dB/256]                      */
#define FW_PMA_GAIN_MAX_RESET_VALUE (0x00001600)
#define FW_PMA_GAIN_MAX (FW_BASE_ADDR + FW_PMA_GAIN_MAX_OFFS)

/*** ATONT/FW_TOP/FW/PMA_AGC_INC_TIME                                     ***/
/* PMA: time to increment the gain [ms]                                     */
#ifndef FW_PMA_AGC_INC_TIME_PROTECT
#define FW_PMA_AGC_INC_TIME_PROTECT

#define FW_PMA_AGC_INC_TIME_OFFS (0x144)

/* PMA_AGC_INC_TIME BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x19                                 */
#define FW_PMA_AGC_INC_TIME_VALUE_INDEX (0)
#define FW_PMA_AGC_INC_TIME_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_AGC_INC_TIME_PROTECT */
/* ATONT/FW_TOP/FW/PMA_AGC_INC_TIME: PMA: time to increment the gain [ms]   */
#define FW_PMA_AGC_INC_TIME_RESET_VALUE (0x00000019)
#define FW_PMA_AGC_INC_TIME (FW_BASE_ADDR + FW_PMA_AGC_INC_TIME_OFFS)

/*** ATONT/FW_TOP/FW/PMA_AGC_DEC_TIME                                     ***/
/* PMA: time to decrement the gain [ms]                                     */
#ifndef FW_PMA_AGC_DEC_TIME_PROTECT
#define FW_PMA_AGC_DEC_TIME_PROTECT

#define FW_PMA_AGC_DEC_TIME_OFFS (0x148)

/* PMA_AGC_DEC_TIME BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x1                                  */
#define FW_PMA_AGC_DEC_TIME_VALUE_INDEX (0)
#define FW_PMA_AGC_DEC_TIME_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_AGC_DEC_TIME_PROTECT */
/* ATONT/FW_TOP/FW/PMA_AGC_DEC_TIME: PMA: time to decrement the gain [ms]   */
#define FW_PMA_AGC_DEC_TIME_RESET_VALUE (0x00000001)
#define FW_PMA_AGC_DEC_TIME (FW_BASE_ADDR + FW_PMA_AGC_DEC_TIME_OFFS)

/*** ATONT/FW_TOP/FW/PMA_AGC_INC_OFFSET                                   ***/
/* PMA: gain increment step [dB/256]                                        */
#ifndef FW_PMA_AGC_INC_OFFSET_PROTECT
#define FW_PMA_AGC_INC_OFFSET_PROTECT

#define FW_PMA_AGC_INC_OFFSET_OFFS (0x14C)

/* PMA_AGC_INC_OFFSET BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x40                                 */
#define FW_PMA_AGC_INC_OFFSET_VALUE_INDEX (0)
#define FW_PMA_AGC_INC_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_AGC_INC_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/PMA_AGC_INC_OFFSET: PMA: gain increment step [dB/256]    */
#define FW_PMA_AGC_INC_OFFSET_RESET_VALUE (0x00000040)
#define FW_PMA_AGC_INC_OFFSET (FW_BASE_ADDR + FW_PMA_AGC_INC_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/PMA_AGC_DEC_OFFSET                                   ***/
/* PMA: gain decrement step [dB/256]                                        */
#ifndef FW_PMA_AGC_DEC_OFFSET_PROTECT
#define FW_PMA_AGC_DEC_OFFSET_PROTECT

#define FW_PMA_AGC_DEC_OFFSET_OFFS (0x150)

/* PMA_AGC_DEC_OFFSET BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x40                                 */
#define FW_PMA_AGC_DEC_OFFSET_VALUE_INDEX (0)
#define FW_PMA_AGC_DEC_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_AGC_DEC_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/PMA_AGC_DEC_OFFSET: PMA: gain decrement step [dB/256]    */
#define FW_PMA_AGC_DEC_OFFSET_RESET_VALUE (0x00000040)
#define FW_PMA_AGC_DEC_OFFSET (FW_BASE_ADDR + FW_PMA_AGC_DEC_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/PMA_INIT_ITERATION                                   ***/
/* PMA: number of iteration during the setup                                */
#ifndef FW_PMA_INIT_ITERATION_PROTECT
#define FW_PMA_INIT_ITERATION_PROTECT

#define FW_PMA_INIT_ITERATION_OFFS (0x154)

/* PMA_INIT_ITERATION BITS 31..0: Value                                     */
/* numeric RW unsigned field, reset to 0x6                                  */
#define FW_PMA_INIT_ITERATION_VALUE_INDEX (0)
#define FW_PMA_INIT_ITERATION_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_INIT_ITERATION_PROTECT */
/* ATONT/FW_TOP/FW/PMA_INIT_ITERATION: PMA: number of iteration during the  *
 * setup                                                                    */
#define FW_PMA_INIT_ITERATION_RESET_VALUE (0x00000006)
#define FW_PMA_INIT_ITERATION (FW_BASE_ADDR + FW_PMA_INIT_ITERATION_OFFS)

/*** ATONT/FW_TOP/FW/PMA_INIT_PAUSE                                       ***/
/* PMA: delay between each iteration (setup) [us]                           */
#ifndef FW_PMA_INIT_PAUSE_PROTECT
#define FW_PMA_INIT_PAUSE_PROTECT

#define FW_PMA_INIT_PAUSE_OFFS (0x158)

/* PMA_INIT_PAUSE BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x32                                 */
#define FW_PMA_INIT_PAUSE_VALUE_INDEX (0)
#define FW_PMA_INIT_PAUSE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_INIT_PAUSE_PROTECT */
/* ATONT/FW_TOP/FW/PMA_INIT_PAUSE: PMA: delay between each iteration (      *
 * setup) [us]                                                              */
#define FW_PMA_INIT_PAUSE_RESET_VALUE (0x00000032)
#define FW_PMA_INIT_PAUSE (FW_BASE_ADDR + FW_PMA_INIT_PAUSE_OFFS)

/*** ATONT/FW_TOP/FW/PMA_COARSE2FINE_PAUSE                                ***/
/* PMA pause between coarse search and fine search  [us]                    */
#ifndef FW_PMA_COARSE2FINE_PAUSE_PROTECT
#define FW_PMA_COARSE2FINE_PAUSE_PROTECT

#define FW_PMA_COARSE2FINE_PAUSE_OFFS (0x15C)

/* PMA_COARSE2FINE_PAUSE BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x1F4                                */
#define FW_PMA_COARSE2FINE_PAUSE_VALUE_INDEX (0)
#define FW_PMA_COARSE2FINE_PAUSE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_COARSE2FINE_PAUSE_PROTECT */
/* ATONT/FW_TOP/FW/PMA_COARSE2FINE_PAUSE: PMA pause between coarse search   *
 * and fine search  [us]                                                    */
#define FW_PMA_COARSE2FINE_PAUSE_RESET_VALUE (0x000001f4)
#define FW_PMA_COARSE2FINE_PAUSE (FW_BASE_ADDR + FW_PMA_COARSE2FINE_PAUSE_OFFS)

/*** ATONT/FW_TOP/FW/WBD_THRESH2                                          ***/
/* Standard threshold for WBD2 used in PMA                                  */
#ifndef FW_WBD_THRESH2_PROTECT
#define FW_WBD_THRESH2_PROTECT

#define FW_WBD_THRESH2_OFFS (0x160)

/* WBD_THRESH2 BITS 31..0: Value                                            */
/* numeric RW unsigned field, reset to 0x8                                  */
#define FW_WBD_THRESH2_VALUE_INDEX (0)
#define FW_WBD_THRESH2_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_WBD_THRESH2_PROTECT */
/* ATONT/FW_TOP/FW/WBD_THRESH2: Standard threshold for WBD2 used in PMA     */
#define FW_WBD_THRESH2_RESET_VALUE (0x00000008)
#define FW_WBD_THRESH2 (FW_BASE_ADDR + FW_WBD_THRESH2_OFFS)

/*** ATONT/FW_TOP/FW/WBD_THRESH2_PMA_STARTUP                              ***/
/* Threshold for WBD2 used in PMA startup                                   */
#ifndef FW_WBD_THRESH2_PMA_STARTUP_PROTECT
#define FW_WBD_THRESH2_PMA_STARTUP_PROTECT

#define FW_WBD_THRESH2_PMA_STARTUP_OFFS (0x164)

/* WBD_THRESH2_PMA_STARTUP BITS 31..0: Value                                */
/* numeric RW unsigned field, reset to 0x1                                  */
#define FW_WBD_THRESH2_PMA_STARTUP_VALUE_INDEX (0)
#define FW_WBD_THRESH2_PMA_STARTUP_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_WBD_THRESH2_PMA_STARTUP_PROTECT */
/* ATONT/FW_TOP/FW/WBD_THRESH2_PMA_STARTUP: Threshold for WBD2 used in PMA  *
 * startup                                                                  */
#define FW_WBD_THRESH2_PMA_STARTUP_RESET_VALUE (0x00000001)
#define FW_WBD_THRESH2_PMA_STARTUP (FW_BASE_ADDR + FW_WBD_THRESH2_PMA_STARTUP_OFFS)

/*** ATONT/FW_TOP/FW/PMA_STARTUP_GAIN_STEP                                ***/
/* Gain step for LNA startup                                                */
#ifndef FW_PMA_STARTUP_GAIN_STEP_PROTECT
#define FW_PMA_STARTUP_GAIN_STEP_PROTECT

#define FW_PMA_STARTUP_GAIN_STEP_OFFS (0x168)

/* PMA_STARTUP_GAIN_STEP BITS 31..0: Value                                  */
/* numeric RW unsigned field, reset to 0x300                                */
#define FW_PMA_STARTUP_GAIN_STEP_VALUE_INDEX (0)
#define FW_PMA_STARTUP_GAIN_STEP_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_PMA_STARTUP_GAIN_STEP_PROTECT */
/* ATONT/FW_TOP/FW/PMA_STARTUP_GAIN_STEP: Gain step for LNA startup         */
#define FW_PMA_STARTUP_GAIN_STEP_RESET_VALUE (0x00000300)
#define FW_PMA_STARTUP_GAIN_STEP (FW_BASE_ADDR + FW_PMA_STARTUP_GAIN_STEP_OFFS)

/*** ATONT/FW_TOP/FW/ATONT_SUPPLY_MV                                      ***/
/* ATONT_SUPPLY_MV                                                          */
#ifndef FW_ATONT_SUPPLY_MV_PROTECT
#define FW_ATONT_SUPPLY_MV_PROTECT

#define FW_ATONT_SUPPLY_MV_OFFS (0x16C)

/* ATONT_SUPPLY_MV BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0x708                                */
#define FW_ATONT_SUPPLY_MV_VALUE_INDEX (0)
#define FW_ATONT_SUPPLY_MV_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_ATONT_SUPPLY_MV_PROTECT */
/* ATONT/FW_TOP/FW/ATONT_SUPPLY_MV: ATONT_SUPPLY_MV                         */
#define FW_ATONT_SUPPLY_MV_RESET_VALUE (0x00000708)
#define FW_ATONT_SUPPLY_MV (FW_BASE_ADDR + FW_ATONT_SUPPLY_MV_OFFS)

/*** ATONT/FW_TOP/FW/RCC_K                                                ***/
/* RCC K factor *256                                                        */
#ifndef FW_RCC_K_PROTECT
#define FW_RCC_K_PROTECT

#define FW_RCC_K_OFFS (0x170)

/* RCC_K BITS 31..0: Value                                                  */
/* numeric RW unsigned field, reset to 0x100                                */
#define FW_RCC_K_VALUE_INDEX (0)
#define FW_RCC_K_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_RCC_K_PROTECT */
/* ATONT/FW_TOP/FW/RCC_K: RCC K factor *256                                 */
#define FW_RCC_K_RESET_VALUE (0x00000100)
#define FW_RCC_K (FW_BASE_ADDR + FW_RCC_K_OFFS)

/*** ATONT/FW_TOP/FW/LPF_INPUT_GAIN                                       ***/
/* LPF gain offset                                                          */
#ifndef FW_LPF_INPUT_GAIN_PROTECT
#define FW_LPF_INPUT_GAIN_PROTECT

#define FW_LPF_INPUT_GAIN_OFFS (0x174)

/* LPF_INPUT_GAIN BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x8                                  */
#define FW_LPF_INPUT_GAIN_VALUE_INDEX (0)
#define FW_LPF_INPUT_GAIN_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LPF_INPUT_GAIN_PROTECT */
/* ATONT/FW_TOP/FW/LPF_INPUT_GAIN: LPF gain offset                          */
#define FW_LPF_INPUT_GAIN_RESET_VALUE (0x00000008)
#define FW_LPF_INPUT_GAIN (FW_BASE_ADDR + FW_LPF_INPUT_GAIN_OFFS)

/*** ATONT/FW_TOP/FW/TUNER_GAIN_OFFSET                                    ***/
/* Gain offset added to the global tuner current gain [dB/256].             */
#ifndef FW_TUNER_GAIN_OFFSET_PROTECT
#define FW_TUNER_GAIN_OFFSET_PROTECT

#define FW_TUNER_GAIN_OFFSET_OFFS (0x178)

/* TUNER_GAIN_OFFSET BITS 31..0: Gain applied                               */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_TUNER_GAIN_OFFSET_VALUE_INDEX (0)
#define FW_TUNER_GAIN_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_TUNER_GAIN_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/TUNER_GAIN_OFFSET: Gain offset added to the global       *
 * tuner current gain [dB/256].                                             */
#define FW_TUNER_GAIN_OFFSET_RESET_VALUE (0x00000000)
#define FW_TUNER_GAIN_OFFSET (FW_BASE_ADDR + FW_TUNER_GAIN_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/SCHEDULER_TICK                                       ***/
/* Scheduler tick length [us]                                               */
#ifndef FW_SCHEDULER_TICK_PROTECT
#define FW_SCHEDULER_TICK_PROTECT

#define FW_SCHEDULER_TICK_OFFS (0x17C)

/* SCHEDULER_TICK BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0x3E8                                */
#define FW_SCHEDULER_TICK_VALUE_INDEX (0)
#define FW_SCHEDULER_TICK_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_SCHEDULER_TICK_PROTECT */
/* ATONT/FW_TOP/FW/SCHEDULER_TICK: Scheduler tick length [us]               */
#define FW_SCHEDULER_TICK_RESET_VALUE (0x000003e8)
#define FW_SCHEDULER_TICK (FW_BASE_ADDR + FW_SCHEDULER_TICK_OFFS)

/*** ATONT/FW_TOP/FW/LOG_CONTROL                                          ***/
/* FW log control. LOG_ENABLE enables all logs except ENTER & LEAVE, while  *
 * FULL_LOG_ENABLE  enables all logs                                        */
#ifndef FW_LOG_CONTROL_PROTECT
#define FW_LOG_CONTROL_PROTECT

#define FW_LOG_CONTROL_OFFS (0x180)

/* LOG_CONTROL BIT 2: FW log ENTER_IT control                               */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LOG_CONTROL_IT_INDEX (2)
#define FW_LOG_CONTROL_IT_MASK (0x4)
#define FW_LOG_CONTROL_IT_DISABLED (0x0 << (FW_LOG_CONTROL_IT_INDEX))
#define FW_LOG_CONTROL_IT_ENABLE (0x1 << (FW_LOG_CONTROL_IT_INDEX))

/* LOG_CONTROL BIT 1: FW log ENTER/LEAVE control                            */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LOG_CONTROL_FUNCTION_INDEX (1)
#define FW_LOG_CONTROL_FUNCTION_MASK (0x2)
#define FW_LOG_CONTROL_FUNCTION_DISABLED (0x0 << (FW_LOG_CONTROL_FUNCTION_INDEX))
#define FW_LOG_CONTROL_FUNCTION_ENABLE (0x1 << (FW_LOG_CONTROL_FUNCTION_INDEX))

/* LOG_CONTROL BIT 0: FW main log activation                                */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LOG_CONTROL_MODE_INDEX (0)
#define FW_LOG_CONTROL_MODE_MASK (0x1)
#define FW_LOG_CONTROL_MODE_DISABLED (0x0 << (FW_LOG_CONTROL_MODE_INDEX))
#define FW_LOG_CONTROL_MODE_ENABLE (0x1 << (FW_LOG_CONTROL_MODE_INDEX))
#endif /* FW_LOG_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/LOG_CONTROL: FW log control. LOG_ENABLE enables all      *
 * logs except ENTER & LEAVE, while FULL_LOG_ENABLE  enables all logs       */
#define FW_LOG_CONTROL_RESET_VALUE (0x0)
#define FW_LOG_CONTROL (FW_BASE_ADDR + FW_LOG_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/LOG_START                                            ***/
/* Start address of FW log                                                  */
#ifndef FW_LOG_START_PROTECT
#define FW_LOG_START_PROTECT

#define FW_LOG_START_OFFS (0x184)

/* LOG_START BITS 31..0: Value                                              */
/* numeric RW unsigned field, reset to 0x14100                              */
#define FW_LOG_START_VALUE_INDEX (0)
#define FW_LOG_START_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOG_START_PROTECT */
/* ATONT/FW_TOP/FW/LOG_START: Start address of FW log                       */
#define FW_LOG_START_RESET_VALUE (0x00014100)
#define FW_LOG_START (FW_BASE_ADDR + FW_LOG_START_OFFS)

/*** ATONT/FW_TOP/FW/LOG_POS                                              ***/
/* Current address of FW log                                                */
#ifndef FW_LOG_POS_PROTECT
#define FW_LOG_POS_PROTECT

#define FW_LOG_POS_OFFS (0x188)

/* LOG_POS BITS 31..0: Value                                                */
/* numeric RW unsigned field, reset to 0x14100                              */
#define FW_LOG_POS_VALUE_INDEX (0)
#define FW_LOG_POS_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOG_POS_PROTECT */
/* ATONT/FW_TOP/FW/LOG_POS: Current address of FW log                       */
#define FW_LOG_POS_RESET_VALUE (0x00014100)
#define FW_LOG_POS (FW_BASE_ADDR + FW_LOG_POS_OFFS)

/*** ATONT/FW_TOP/FW/LOG_END                                              ***/
/* End address of FW log                                                    */
#ifndef FW_LOG_END_PROTECT
#define FW_LOG_END_PROTECT

#define FW_LOG_END_OFFS (0x18C)

/* LOG_END BITS 31..0: Value                                                */
/* numeric RW unsigned field, reset to 0x14500                              */
#define FW_LOG_END_VALUE_INDEX (0)
#define FW_LOG_END_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOG_END_PROTECT */
/* ATONT/FW_TOP/FW/LOG_END: End address of FW log                           */
#define FW_LOG_END_RESET_VALUE (0x00014500)
#define FW_LOG_END (FW_BASE_ADDR + FW_LOG_END_OFFS)

/*** ATONT/FW_TOP/FW/LOG_STAT_CONTROL                                     ***/
/* FW profiling: averaging every N ticks of 1 ms.  Deactivated if 0         */
#ifndef FW_LOG_STAT_CONTROL_PROTECT
#define FW_LOG_STAT_CONTROL_PROTECT

#define FW_LOG_STAT_CONTROL_OFFS (0x190)

/* LOG_STAT_CONTROL BITS 31..0: Value                                       */
/* numeric RW unsigned field, reset to 0x0                                  */
#define FW_LOG_STAT_CONTROL_VALUE_INDEX (0)
#define FW_LOG_STAT_CONTROL_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LOG_STAT_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/LOG_STAT_CONTROL: FW profiling: averaging every N ticks  *
 * of 1 ms.  Deactivated if 0                                               */
#define FW_LOG_STAT_CONTROL_RESET_VALUE (0x00000000)
#define FW_LOG_STAT_CONTROL (FW_BASE_ADDR + FW_LOG_STAT_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/LED_CONTROL                                          ***/
/* Led blincking control                                                    */
#ifndef FW_LED_CONTROL_PROTECT
#define FW_LED_CONTROL_PROTECT

#define FW_LED_CONTROL_OFFS (0x194)

/* LED_CONTROL BIT 0: LED blincking activation                              */
/* symbolic RW field, reset to 0x0                                          */
#define FW_LED_CONTROL_MODE_INDEX (0)
#define FW_LED_CONTROL_MODE_MASK (0x1)
#define FW_LED_CONTROL_MODE_DISABLED (0x0 << (FW_LED_CONTROL_MODE_INDEX))
#define FW_LED_CONTROL_MODE_ENABLE (0x1 << (FW_LED_CONTROL_MODE_INDEX))
#endif /* FW_LED_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/LED_CONTROL: Led blincking control                       */
#define FW_LED_CONTROL_RESET_VALUE (0x0)
#define FW_LED_CONTROL (FW_BASE_ADDR + FW_LED_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/VCO_SURVEY_CONTROL                                   ***/
/* VCO survey control                                                       */
#ifndef FW_VCO_SURVEY_CONTROL_PROTECT
#define FW_VCO_SURVEY_CONTROL_PROTECT

#define FW_VCO_SURVEY_CONTROL_OFFS (0x198)

/* VCO_SURVEY_CONTROL BIT 0: Survey activation                              */
/* symbolic RW field, reset to 0x0                                          */
#define FW_VCO_SURVEY_CONTROL_MODE_INDEX (0)
#define FW_VCO_SURVEY_CONTROL_MODE_MASK (0x1)
#define FW_VCO_SURVEY_CONTROL_MODE_DISABLED (0x0 << (FW_VCO_SURVEY_CONTROL_MODE_INDEX))
#define FW_VCO_SURVEY_CONTROL_MODE_ENABLE (0x1 << (FW_VCO_SURVEY_CONTROL_MODE_INDEX))
#endif /* FW_VCO_SURVEY_CONTROL_PROTECT */
/* ATONT/FW_TOP/FW/VCO_SURVEY_CONTROL: VCO survey control                   */
#define FW_VCO_SURVEY_CONTROL_RESET_VALUE (0x0)
#define FW_VCO_SURVEY_CONTROL (FW_BASE_ADDR + FW_VCO_SURVEY_CONTROL_OFFS)

/*** ATONT/FW_TOP/FW/LNA_GAIN_SCALE                                       ***/
/* LNA gain scale factor (on index of LNA table)                            */
#ifndef FW_LNA_GAIN_SCALE_PROTECT
#define FW_LNA_GAIN_SCALE_PROTECT

#define FW_LNA_GAIN_SCALE_OFFS (0x19C)

/* LNA_GAIN_SCALE BITS 31..0: Value                                         */
/* numeric RW unsigned field, reset to 0xD8                                 */
#define FW_LNA_GAIN_SCALE_VALUE_INDEX (0)
#define FW_LNA_GAIN_SCALE_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LNA_GAIN_SCALE_PROTECT */
/* ATONT/FW_TOP/FW/LNA_GAIN_SCALE: LNA gain scale factor (on index of LNA   *
 * table)                                                                   */
#define FW_LNA_GAIN_SCALE_RESET_VALUE (0x000000d8)
#define FW_LNA_GAIN_SCALE (FW_BASE_ADDR + FW_LNA_GAIN_SCALE_OFFS)

/*** ATONT/FW_TOP/FW/LPF_5MHZ_OFFSET                                      ***/
/* Offset for the LPF used when the bandwidth is 5 of MHz                   */
#ifndef FW_LPF_5MHZ_OFFSET_PROTECT
#define FW_LPF_5MHZ_OFFSET_PROTECT

#define FW_LPF_5MHZ_OFFSET_OFFS (0x1A0)

/* LPF_5MHZ_OFFSET BITS 31..0: Value                                        */
/* numeric RW unsigned field, reset to 0xD                                  */
#define FW_LPF_5MHZ_OFFSET_VALUE_INDEX (0)
#define FW_LPF_5MHZ_OFFSET_VALUE_MASK (0xFFFFFFFF)
#endif /* FW_LPF_5MHZ_OFFSET_PROTECT */
/* ATONT/FW_TOP/FW/LPF_5MHZ_OFFSET: Offset for the LPF used when the        *
 * bandwidth is 5 of MHz                                                    */
#define FW_LPF_5MHZ_OFFSET_RESET_VALUE (0x0000000d)
#define FW_LPF_5MHZ_OFFSET (FW_BASE_ADDR + FW_LPF_5MHZ_OFFSET_OFFS)

/*** ATONT/FW_TOP/FW/CROWBAR_TYPE                                         ***/
/* Select the crowbar type. EXTERNAL means using the external RF switch     */
#ifndef FW_CROWBAR_TYPE_PROTECT
#define FW_CROWBAR_TYPE_PROTECT

#define FW_CROWBAR_TYPE_OFFS (0x1A4)

/* CROWBAR_TYPE BIT 0: type: external or internal                           */
/* symbolic RW field, reset to 0x0                                          */
#define FW_CROWBAR_TYPE_VALUE_INDEX (0)
#define FW_CROWBAR_TYPE_VALUE_MASK (0x1)
#define FW_CROWBAR_TYPE_VALUE_EXTERNAL (0x0 << (FW_CROWBAR_TYPE_VALUE_INDEX))
#define FW_CROWBAR_TYPE_VALUE_DISABLED (0x1 << (FW_CROWBAR_TYPE_VALUE_INDEX))
#endif /* FW_CROWBAR_TYPE_PROTECT */
/* ATONT/FW_TOP/FW/CROWBAR_TYPE: Select the crowbar type. EXTERNAL means    *
 * using the external RF switch                                             */
#define FW_CROWBAR_TYPE_RESET_VALUE (0x0)
#define FW_CROWBAR_TYPE (FW_BASE_ADDR + FW_CROWBAR_TYPE_OFFS)

#define FW_SIZE (0x1A4)

#define FW_TOP_SIZE (0x140A4)

#define ATONT_SIZE (0x830035)
#endif /* ATONT_MODULE_PROTECT */
