
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

unsigned long long sub_tv( const struct timeval t1, const struct timeval t2 ) {
    return (unsigned long long)( t2.tv_sec - t1.tv_sec ) * 1000 + ( t2.tv_usec - t1.tv_usec ) / 1000;
}

void dump_array( FILE *fp, int *array, int size ) {
    if( size <= 0 ) return;
    if( fp == NULL || array == NULL ) return;
    fprintf( fp, "%d", array[0] );
    int i;
    for( i=1; i<size; ++i )
        fprintf( fp, " %d", array[i] );
    fputs( "\n", fp );
}

int strcmp_suffix( const char *str, const char *suffix ) {
    size_t str_p = strlen( str );
    size_t suf_p = strlen( suffix );
    for(;;) {
        if( str[str_p] != suffix[suf_p] ) return 1;
        if( suf_p == 0 ) return 0;
        if( str_p == 0 ) return 1;
        --str_p;
        --suf_p;
    }
}
