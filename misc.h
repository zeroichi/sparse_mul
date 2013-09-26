
#ifndef __MISC_H__
#define __MISC_H__

#include <stdio.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MISC_DEVICE_SYNC
#define TIMESTART { cudaDeviceSynchronize(); gettimeofday(&t1,NULL); }
#define TIMEEND(name) { cudaDeviceSynchronize(); gettimeofday(&t2,NULL); printf( "%s: %llu ms\n", name, sub_tv(t1,t2) ); }
#else
#define TIMESTART { gettimeofday(&t1,NULL); }
#define TIMEEND(name) { gettimeofday(&t2,NULL); printf( "%s: %llu ms\n", name, sub_tv(t1,t2) ); }
#endif
#define TIMEINIT struct timeval t1, t2;

unsigned long long sub_tv( const struct timeval t1, const struct timeval t2 );
void dump_array( FILE *fp, int *array, int size );
int strcmp_suffix( const char *str, const char *suffix );

#ifdef __cplusplus
}
#endif

#endif
