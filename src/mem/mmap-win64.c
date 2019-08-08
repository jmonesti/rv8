/*
 *  mmap-win64.c
 *
 *  mmap interception to move maps with default address to high memory
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <unistd.h>

#include <stdio.h>

#include "mmap-core.h"

static size_t page_size = 0;
static uintptr_t map_base = 0x7fff00000000UL;
static mmap_fn real_mmap = NULL;
static munmap_fn real_munmap = NULL;

// pre-declaration
int munmap(void *addr, size_t len);

void* mmap_win64(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	printf( "addr=%p len=%lu prot=0x%x flags=0x%x fd=%d offset=%lu\n",
			addr, len, prot, flags, fd, offset);

#if 0
	void * _addr = VirtualAlloc( NULL, len, 0x3000, 0x04 );

	if ( _addr == NULL ) {
		// failed
		printf( "failed 1 (%d)\n", GetLastError() );
		return MAP_FAILED;
	}
	else if ( _addr != addr ) {
		// failed
		printf( "failed 2 %p != %p (%d)\n", _addr, addr, GetLastError() );
		return MAP_FAILED;
	}
	else {
		// yeeha !
		return _addr;
	}
#else
	return MAP_FAILED;
#endif
}

int munmap_win64(void *addr, size_t len)
{
	return -1;
}

static uintptr_t round_page(uintptr_t x)
{
	if (!page_size) {
		page_size = 4096;
	}
	return ((x + page_size - 1UL) & ~(page_size - 1UL));
}

int guest_munmap(void *addr, size_t len)
{
	return __guest_munmap(munmap, addr, len);
}

void* guest_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	if (!real_mmap) {
		*(void **)(&real_mmap) = mmap_win64;
	}
	return __guest_mmap(real_mmap, addr, len, prot, flags, fd, offset);
}

void* mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	if (!real_mmap) {
		*(void **)(&real_mmap) = mmap_win64;
	}
	size_t incr = 0;
	if (addr == 0) {
		addr = (void*)map_base;
		incr = round_page(len);
	}
	void *rv = real_mmap(addr, len, prot, flags, fd, offset);
	if (rv != MAP_FAILED) map_base += incr;
	return rv;
}

int munmap(void *addr, size_t len)
{
	if (!real_munmap) {
		*(void **)(&real_mmap) = munmap_win64;
	}
	return real_munmap(addr, len);
}
