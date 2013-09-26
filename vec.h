
#ifndef __VEC_H__
#define __VEC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
return values
0. no error (success)
1. invalid parameter
2. i/o error
3. invalid format
4. memory allocation error
 */

typedef struct {
	size_t size;
	int *data;
} vec_t;

void vec_print( const vec_t* v );
int vec_write( const vec_t* v, FILE *file );
int vec_save( const vec_t* v, const char *filename );
int vec_load( vec_t *v, const char *filename );
void vec_delete( vec_t *v );
int vec_new( vec_t* v, size_t size );

#ifdef __cplusplus
}
#endif

#endif
