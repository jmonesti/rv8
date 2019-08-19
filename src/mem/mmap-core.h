/*
 *  mmap-core.h
 *
 *  mmap bitmap tracker
 */

#ifndef _mmap_core_h
#define _mmap_core_h

#if defined __MINGW32__
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define PROT_NONE  0x0

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20

#define MAP_FAILED ((void*) -1)

void* mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t len);
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*munmap_fn)(void *addr, size_t len);
typedef void* (*mmap_fn)(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

int guest_munmap(void *addr, size_t len);
void* guest_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

int __host_munmap(munmap_fn munmap, void *addr, size_t len);
int __guest_munmap(munmap_fn munmap, void *addr, size_t len);
void* __host_mmap(mmap_fn mmap, void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void* __guest_mmap(mmap_fn mmap, void *addr, size_t len, int prot, int flags, int fd, off_t offset);

#ifdef __cplusplus
}
#endif

#endif
