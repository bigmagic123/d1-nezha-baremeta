// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/type.h
 *
 * Copyright (c) 2018 Allwinner.
 * 2018-09-14 Written by fanqinghua (fanqinghua@allwinnertech.com).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __TYPE_H__
#define __TYPE_H__

#include "stdint.h"


#define __io_br()	do {} while (0)
#define __io_ar()	__asm__ __volatile__ ("fence i,r" : : : "memory");
#define __io_bw()	__asm__ __volatile__ ("fence w,o" : : : "memory");
#define __io_aw()	do {} while (0)

static inline void __raw_writeb(uint8_t val, volatile void *addr)
{
	asm volatile("sb %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline void __raw_writew(uint16_t val, volatile void *addr)
{
	asm volatile("sh %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline void __raw_writel(uint32_t val, volatile void *addr)
{
	asm volatile("sw %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline uint8_t __raw_readb(const volatile void *addr)
{
	uint8_t val;

	asm volatile("lb %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

static inline uint16_t __raw_readw(const volatile void *addr)
{
	uint16_t val;

	asm volatile("lh %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

static inline uint32_t __raw_readl(const volatile void *addr)
{
	uint32_t val;

	asm volatile("lw %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

#define writeb(v, c)                                  \
	({                                            \
		__io_bw();                            \
		__raw_writeb((v), ((void *)(long)c)); \
		__io_aw();                            \
	})
#define writew(v, c)                                  \
	({                                            \
		__io_bw();                            \
		__raw_writew((v), ((void *)(long)c)); \
		__io_aw();                            \
	})
#define writel(v, c)                                  \
	({                                            \
		__io_bw();                            \
		__raw_writel((v), ((void *)(long)c)); \
		__io_aw();                            \
	})

#define readb(c)                              \
	({                                    \
		u8 __v;                       \
		__io_br();                    \
		__v = __raw_readb((void *)(long)c); \
		__io_ar();                    \
		__v;                          \
	})
#define readw(c)                              \
	({                                    \
		u16 __v;                      \
		__io_br();                    \
		__v = __raw_readw((void *)(long)c); \
		__io_ar();                    \
		__v;                          \
	})
#define readl(c)                              \
	({                                    \
		u32 __v;                      \
		__io_br();                    \
		__v = __raw_readl((void *)(long)c); \
		__io_ar();                    \
		__v;                          \
	})

typedef int8_t	s8;
typedef int16_t	s16;
typedef int32_t	s32;
typedef int64_t	s64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
/*
 * typedef char	s8;
 * typedef short		s16;
 * typedef int			s32;
 * typedef long long int	s64;
 *
 * typedef unsigned char		u8;
 * typedef unsigned short		u16;
 * typedef unsigned int		u32;
 * typedef unsigned long long int	u64;
 */

#define BIT(nr)			(1 << (nr))
#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))
#endif /*__TYPE_H__*/
