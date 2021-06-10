// * SPDX-License-Identifier:	GPL-2.0+

#include <common.h>

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
	(((long)X & (sizeof(long) - 1)) | ((long)Y & (sizeof(long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof(long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

void * memcpy( void *dst0, const void* src0, size_t len0)
{
	char *dst = dst0;
	const char *src = src0;
	long *aligned_dst;
	long *aligned_src;

	/* If the size is small, or either SRC or DST is unaligned,
	   then punt into the byte copy loop.  This should be rare.  */
	if (!TOO_SMALL(len0) && !UNALIGNED(src, dst)) {
		aligned_dst = (long *)dst;
		aligned_src = (long *)src;

		/* Copy 4X long words at a time if possible.  */
		while (len0 >= BIGBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len0 -= BIGBLOCKSIZE;
		}

		/* Copy one long word at a time if possible.  */
		while (len0 >= LITTLEBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			len0 -= LITTLEBLOCKSIZE;
		}

		/* Pick up any residual with a byte copier.  */
		dst = (char *)aligned_dst;
		src = (char *)aligned_src;
	}

	while (len0--)
		*dst++ = *src++;

	return dst0;
}
