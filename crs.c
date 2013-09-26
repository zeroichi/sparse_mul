
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crs.h"
#include "misc.h"

int crs_new( crs_t *crs, int rows, int cols, int nonzeros ) {
    // parameter check
    if( crs == NULL ) return 1;
    if( rows <= 0 ) return 1;
    if( cols <= 0 ) return 1;
    if( nonzeros <= 0 ) return 1;
    crs->rows = rows;
    crs->cols = cols;
    crs->nonzeros = nonzeros;
    crs->val = (int *)malloc( sizeof(*crs->val) * nonzeros );
    crs->col_ind = (int *)malloc( sizeof(*crs->col_ind) * nonzeros );
    crs->row_ptr = (int *)malloc( sizeof(*crs->row_ptr) * (rows+1) );
    if( crs->val == NULL || crs->col_ind == NULL || crs->row_ptr == NULL ) {
        crs_delete( crs );
        return 4;
    }
    return 0;
}

int crs_new_identity( crs_t *crs, int size ) {
    if( crs == NULL ) return 1;
    if( size <= 0 ) return 1;
    int ret;
    if( 0 != ( ret = crs_new( crs, size, size, size ) ) )
        return ret;
    int i;
    for( i=0; i<crs->nonzeros; ++i ) {
        crs->val[i] = 1;
        crs->col_ind[i] = i;
    }
    for( i=0; i<crs->rows+1; ++i )
        crs->row_ptr[i] = i;
    return 0;
}

int crs_delete( crs_t *crs ) {
    if( crs == NULL ) return 1;
    crs->rows = crs->cols = crs->nonzeros = 0;
    free( crs->val );
    free( crs->col_ind );
    free( crs->row_ptr );
    crs->val = crs->col_ind = crs->row_ptr = NULL;
    return 0;
}

int crs_load( crs_t *crs, const char *filename ) {
    // parameter check
    if( crs == NULL ) return 1;
    if( filename == NULL ) return 1;
	FILE *fp = fopen( filename, "rt" );
	if( fp == NULL ) return 2;
    int ret;
    crs->val = crs->col_ind = crs->row_ptr = NULL;
    ret = fscanf( fp, "%d %d %d", &crs->rows, &crs->cols, &crs->nonzeros );
    if( ret < 3 || crs->rows <= 0 || crs->cols <= 0 || crs->nonzeros <= 0 )
        goto invalid_format;
    if( 0 != ( ret = crs_new( crs, crs->rows, crs->cols, crs->nonzeros ) ) ) {
        fclose( fp );
        return ret;
    }

    int i;
    for( i=0; i<crs->nonzeros; ++i )
        if( 1 != fscanf( fp, "%d", &crs->val[i] ) )
            goto invalid_format;
    for( i=0; i<crs->nonzeros; ++i )
        if( 1 != fscanf( fp, "%d", &crs->col_ind[i] ) )
            goto invalid_format;
    for( i=0; i<crs->rows+1; ++i )
        if( 1 != fscanf( fp, "%d", &crs->row_ptr[i] ) )
            goto invalid_format;
    fclose( fp );
	return 0;
    
    invalid_format:
    fclose( fp );
    crs_delete( crs );
    return 3;
}

int crs_load_bin( crs_t *crs, const char *filename ) {
    if( crs==NULL ) return 1;
    if( filename==NULL ) return 1;
    FILE *fp = fopen( filename, "rb" );
    if( fp == NULL ) return 2;
    fread( &crs->rows, sizeof(crs->rows), 1, fp );
    fread( &crs->cols, sizeof(crs->cols), 1, fp );
    fread( &crs->nonzeros, sizeof(crs->nonzeros), 1, fp );
    crs_new( crs, crs->rows, crs->cols, crs->nonzeros );
    fread( crs->val, sizeof(*crs->val), crs->nonzeros, fp );
    fread( crs->col_ind, sizeof(*crs->col_ind), crs->nonzeros, fp );
    fread( crs->row_ptr, sizeof(*crs->row_ptr), crs->rows+1, fp );
    fclose( fp );
    return 0;
}

int crs_save( crs_t *crs, const char *filename ) {
    // parameter validation
    if( crs == NULL ) return 1;
    if( filename == NULL ) return 1;
    FILE *fp = fopen( filename, "wt" );
    if( fp == NULL ) return 2;
    fprintf( fp, "%d %d %d\n", crs->rows, crs->cols, crs->nonzeros );
    dump_array( fp, crs->val, crs->nonzeros );
    dump_array( fp, crs->col_ind, crs->nonzeros );
    dump_array( fp, crs->row_ptr, crs->rows + 1 );
    fclose( fp );
    return 0;
}

int crs_save_bin( crs_t *crs, const char *filename ) {
    if( crs==NULL ) return 1;
    if( filename==NULL ) return 1;
    FILE *fp = fopen( filename, "wb" );
    if( fp == NULL ) return 2;
    if( 1 != fwrite( &crs->rows, sizeof(crs->rows), 1, fp ) )
        goto ioerror;
    if( 1 != fwrite( &crs->cols, sizeof(crs->cols), 1, fp ) )
        goto ioerror;
    if( 1 != fwrite( &crs->nonzeros, sizeof(crs->nonzeros), 1, fp ) )
        goto ioerror;
    if( crs->nonzeros != fwrite( crs->val, sizeof(*crs->val), crs->nonzeros, fp ) )
        goto ioerror;
    if( crs->nonzeros != fwrite( crs->col_ind, sizeof(*crs->col_ind), crs->nonzeros, fp ) )
        goto ioerror;
    if( crs->rows+1 != fwrite( crs->row_ptr, sizeof(*crs->row_ptr), crs->rows+1, fp ) )
        goto ioerror;
    fclose( fp );
    return 0;

    ioerror:
    fclose( fp );
    return 2;
}
