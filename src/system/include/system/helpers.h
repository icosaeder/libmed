/* SPDX-License-Identifier: GPL-3.0-only */

/*
 * helpers.h - various helper definitions.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>

#define BIT(x) (1 << x)

#define container_of(ptr, type, member) \
	((type *)((void*)(ptr) - offsetof(type, member)))

#endif /* HELPERS_H */
