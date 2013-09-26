#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vec.h"

int main( int argc, char **argv ) {
	srand( time( NULL ) );
	if( argc < 2 ) {
        printf( "this program generates a dense vector.\n" );
		printf( "usage: %s <size>\n", argv[0] );
		return 0;
	}

	size_t size;
    int r;
	r = sscanf( argv[1],  "%zd", &size );
	if( r != 1 || size <= 0 ) {
		printf( "'size' must be larger than 0.\n" );
		return 1;
	}
	vec_t v;
    r = vec_new( &v, size );
    if( r != 0 ) {
        printf( "an error occured during generating a vector. return code = %d\n", r );
        return r;
    }
    size_t i;
    for( i=0; i<size; ++i )
        v.data[i] = rand() % 10;
	vec_save( &v, NULL );
	vec_delete( &v );
	return 0;
}
