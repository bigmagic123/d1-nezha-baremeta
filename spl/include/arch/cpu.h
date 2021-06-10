/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
* SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CPU_H
#define _SUNXI_CPU_H

#include <config.h>

#if defined(CONFIG_SUNXI_NCAT)
#include <arch/cpu_ncat.h>
#elif defined(CONFIG_SUNXI_NCAT_2_0)
#include <arch/cpu_ncat_2_0.h>
#elif defined(CONFIG_SUNXI_NCAT_V2)
#include <arch/cpu_ncat_v2.h>
#elif defined(CONFIG_SUNXI_VERSION1)
#include <arch/cpu_version1.h>
#else
#error "Unsupported plat"
#endif

#endif /* _SUNXI_CPU_H */
