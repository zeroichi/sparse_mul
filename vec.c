
#include <stdio.h>
#include <stdlib.h>
#include "vec.h"

int vec_new( vec_t* v, size_t size ) {
    if( v == NULL ) return 1;
    if( size < 1 ) return 1;
    v->size = size;
    v->data = (int*)malloc( sizeof(*v->data)*size );
    if( v->data == NULL ) return 4;
    return 0;
}

void vec_delete( vec_t *v ) {
    if( v == NULL ) return;
	free( v->data );
	v->size = 0;
	v->data = 0;
}

int vec_load( vec_t *v, const char *filename ) {
    if( v == NULL ) return 1;
    if( filename == NULL ) return 1;
	FILE *fp = fopen( filename, "rt" );
	if( fp == NULL ) return 2;
	if( 1 != fscanf( fp, "%zd", &v->size ) ) {
		fclose( fp );
		return 3;
	}
    int r = vec_new( v, v->size );
    if( r != 0 ) {
        fclose( fp );
        return r;
    }
	size_t i;
	for( i=0; i<v->size; ++i ) {
		if( 1 != fscanf( fp, "%d", (v->data)+i ) ) {
			fclose( fp );
            vec_delete( v );
			return 3;
		}
	}
	fclose( fp );
	return 0;
}

int vec_write( const vec_t* v, FILE *fp ) {
    if( v == NULL ) return 1;
    if( fp == NULL ) return 1;
	fprintf( fp, "%zd\n", v->size );
	size_t i;
    fprintf( fp, "%d", v->data[0] );
	for( i=1; i<v->size; ++i )
		fprintf( fp, " %d", v->data[i] );
	fputs( "\n", fp );
    return 0;
}

int vec_save( const vec_t* v, const char *filename ) {
    if( v == NULL ) return 1;
    if( filename == NULL ) {
        vec_write( v, stdout );
    } else {
        FILE *fp = fopen( filename, "wt" );
        if( fp == NULL ) return 2;
        vec_write( v, fp );
        fclose( fp );
    }
    return 0;
}

void vec_print( const vec_t* v ) {
	size_t i;
	printf( "(size=%zd) [", v->size );
    printf( "%d", v->data[0] );
	for( i=1; i<v->size; ++i )
		printf( ",%d", v->data[i] );
	puts("]");
	return;
}
