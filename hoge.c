/*
 *  hoge.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#include "crs.h"
#include "vec.h"
#include "misc.h"

int main( int argc, char **argv ) {
	if( argc < 3 ) {
		printf( "usage: %s <matrix file> <vector file>\n", argv[0] );
		return 0;
	}
    crs_t crs;
    vec_t v;
    int r;
    TIMEINIT;
    
    TIMESTART;
	if( strcmp_suffix( argv[1], ".bin" ) == 0 )
        r = crs_load_bin( &crs, argv[1] );
    else
        r = crs_load( &crs, argv[1] );
    if( r != 0 ) {
        printf( "error: failed to load crs file, return code = %d\n", r );
        return r;
    }
    TIMEEND( "loading sparse matrix data" );

    TIMESTART;
    r = vec_load( &v, argv[2] );
    if( r != 0 ) {
        printf( "error: failed to load vector file, return code = %d\n", r );
        return r;
    }
    TIMEEND( "loading dense vector data" );

    if( crs.cols != (int)v.size ) {
		printf( "error: vector size is different with matrix column size\n" );
        crs_delete( &crs );
		vec_delete( &v );
		return 1;
	}

    printf( "matrix filename ... %s\n"
            "matrix size ....... %d x %d\n"
            "matrix nonzeros ... %d\n"
            "vector filename ... %s\n"
            "vector size ....... %d\n"
            "compute device .... cpu\n"
            "with gather ....... %s\n"
            , argv[1]
            , crs.rows, crs.cols
            , crs.nonzeros
            , argv[2]
            , (int)v.size
#ifdef WITH_GATHER
            , "true"
#else
            , "false"
#endif
            );

	int i, j;
#ifdef WITH_GATHER
    TIMESTART;
    int *gather = (int*)malloc( sizeof(int) * crs.nonzeros );
    for( i=0; i<crs.nonzeros; ++i )
        gather[i] = v.data[crs.col_ind[i]];
    TIMEEND( "gathering data" );
#endif
    
    int *p = (int*)malloc( sizeof(int) * crs.rows );
    unsigned long long total=0LL;
    int l;
    for( l=0; l<10; ++l ) {
    TIMESTART;
	for( i=0; i<crs.rows; ++i ) {
		p[i] = 0;
        int start = crs.row_ptr[i];
        int end = crs.row_ptr[i+1];
		for( j=start; j<end; ++j ) {
#ifdef WITH_GATHER
            p[i] += crs.val[j] * gather[j];
#else
            p[i] += crs.val[j] * v.data[crs.col_ind[j]];
#endif
		}
	}
    TIMEEND( "* COMPUTING TIME *" );
    total+=sub_tv(t1, t2);
    }
    printf( "average: %llu ms", total/l );

    dump_array( stderr, p, crs.rows );
#ifdef WITH_GATHER
    free( gather );
#endif
    free( p );
    crs_delete( &crs );
	vec_delete( &v );
	return 0;
}
