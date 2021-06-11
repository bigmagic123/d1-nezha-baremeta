// SPDX-License-Identifier: BSD-2-Clause
#ifndef _MCTL_STANDBY_H
#define _MCTL_STANDBY_H

#include "head.h"

extern unsigned int dram_power_save_process(void);
extern unsigned int dram_power_up_process(__dram_para_t *para);
extern void dram_enable_all_master(void);
extern void dram_disable_all_master(void);
#endif
