// * SPDX-License-Identifier:	GPL-2.0+
#ifndef LZ4_H
#define LZ4_H

#ifdef __cplusplus
extern "C" {
#endif

/* lib/lz4_wrapper.c */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

#ifdef __cplusplus
}
#endif

#endif /* LZ4_H */
