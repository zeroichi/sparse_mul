
#ifndef __SPARSE_H__
#define __SPARSE_H__

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
    int rows, cols, nonzeros;
    int *val;
    int *col_ind;
    int *row_ptr;
} crs_t;

int crs_save_bin( crs_t *crs, const char *filename );
int crs_save( crs_t *crs, const char *filename );
int crs_load_bin( crs_t *crs, const char *filename );
int crs_load( crs_t *crs, const char *filename );
int crs_delete( crs_t *crs );
int crs_new_identity( crs_t *crs, int size );
int crs_new( crs_t *crs, int rows, int cols, int nonzeros );

#ifdef __cplusplus
}
#endif


#endif /* __SPARSE_H__ */
