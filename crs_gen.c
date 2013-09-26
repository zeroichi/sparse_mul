
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "crs.h"

// used for qsort()
int compare_int( const void *a, const void *b ) {
    return *((int*)a) - *((int*)b);
}

void usage( const char *argv0 ) {
    printf( "usage: %s <options>\n", argv0 );
    puts(
         "\t-r NUMBER    number of rows\n"
         "\t-c NUMBER    number of columns\n"
         "\t-n NUMBER    number of non-zero elements\n"
         "\t-o FILENAME  specify output file name ('matrix.crs' by default)\n"
         "\t-i           identity matrix (-c and -n is ignored)\n"
         "\t-t           output in text format (binary format by default)\n"
         );
}

int main( int argc, char **argv ) {
    int c;
    crs_t crs;
    char output[4000];
    int identity = 0;
    int save_binary = 1;
    int ret;

    // set default values
    crs.rows = 1024;
    crs.cols = 1024;
    crs.nonzeros = 65536;
    strcpy( output, "matrix.crs" );
    srand( time( NULL ) );

    while( -1 != ( c = getopt( argc, argv, "c:hin:o:r:t" ) ) ) {
        switch( c ) {
        case 'c':
            crs.cols = atoi( optarg );
            break;
        case 'h':
            usage( argv[0] );
            return 0;
        case 'i':
            identity = 1;
            break;
        case 'n':
            crs.nonzeros = atoi( optarg );
            break;
        case 'o':
            strncpy( output, optarg, sizeof(output) );
            // make it certainly terminate with null
            output[sizeof(output)-1] = '\0';
            break;
        case 'r':
            crs.rows = atoi( optarg );
            break;
        case 't':
            save_binary = 0;
            break;
        case '?':
            usage( argv[0] );
            return 1;
        }
    }

    if( identity != 0 ) {
        if( 0 != ( ret = crs_new_identity( &crs, crs.rows ) ) ) {
            fprintf( stderr, "crs_new_identity() failed. return value = %d\n", ret );
            return ret;
        }
    } else {
        int i;
        if( 0 != ( ret = crs_new( &crs, crs.rows, crs.cols, crs.nonzeros ) ) ) {
            fprintf( stderr, "crs_new() failed. return value = %d\n", ret );
            return ret;
        }
        for( i=0; i<crs.nonzeros; ++i )
            crs.val[i] = 1 + rand() % 9; // random value from 1 to 9
        for( i=0; i<crs.rows; ++i )
            crs.row_ptr[i] = rand() % crs.nonzeros;
        qsort( crs.row_ptr, crs.rows, sizeof(*crs.row_ptr), compare_int );
        crs.row_ptr[0] = 0;
        crs.row_ptr[crs.rows] = crs.nonzeros;

        int *p = crs.col_ind;
        for( i=0; i<crs.rows; ++i ) {
            int nelements_row = crs.row_ptr[i+1] - crs.row_ptr[i];
            int *a = (int *)malloc(sizeof(*crs.col_ind)*nelements_row);
            int j;
            for( j=0; j<nelements_row; ++j )
                a[j] = rand() % crs.cols;
            for(;;) {
                int dup=0;
                int again=0;
                qsort( a, nelements_row, sizeof(*a), compare_int );
                for( j=1; j<nelements_row; ++j ) {
                    if( a[dup] == a[j] ) {
                        a[j] = rand() % crs.cols;
                        again=1;
                    } else {
                        dup = j;
                    }
                }
                if( !again ) break;
            }
            memcpy( p, a, sizeof(*p)*nelements_row );
            p += nelements_row;
            free(a);
        }
        assert( p == crs.col_ind + crs.nonzeros );
    }

    printf( "number of rows .............. %d\n", crs.rows );
    printf( "number of columns ........... %d\n", crs.cols );
    printf( "number of non-zero elements . %d\n", crs.nonzeros );
    printf( "identity matrix ............. %s\n", identity ? "true" : "false" );
    printf( "save as binary format ....... %s\n", save_binary ? "true" : "false" );
    printf( "output file name ............ %s\n", output );
    
    if( save_binary != 0 ) {
        ret = crs_save_bin( &crs, output );
        if( ret != 0 ) {
            fprintf( stderr, "crs_save_bin() failed. return value = %d\n", ret );
            crs_delete( &crs );
            return ret;
        }
    } else {
        ret = crs_save( &crs, output );
        if( ret != 0 ) {
            fprintf( stderr, "crs_save() failed. return value = %d\n", ret );
            crs_delete( &crs );
            return ret;
        }
    }

    crs_delete( &crs );
    return 0;
}
