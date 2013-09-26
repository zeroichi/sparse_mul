// -*- mode: c++ -*-

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <cuda.h>
#include <cuda_runtime.h>

#include "crs.h"
#include "vec.h"
// this macro requests calling cudaDeviceSynchronize() before gettimeofday()
#define MISC_DEVICE_SYNC
#include "misc.h"

__global__ void collect( int *row_ptr, int *tmp, int *result, int rows ) {
    int total = gridDim.x * gridDim.y * blockDim.x; // the number of total threads
    int tid = threadIdx.x + blockIdx.x * blockDim.x + blockIdx.y * gridDim.x * blockDim.x; // thread id
    int i, j, start, end;
    for( i=tid; i<rows; i+=total ) {
        start = row_ptr[i];
        end   = row_ptr[i+1];
        result[i] = 0;
        for( j=start; j<end; ++j )
            result[i] += tmp[j];
    }
}

#ifdef WITH_GATHER

__global__ void gather( int *v, int *col_ind, int *gather, int len ) {
    int total = gridDim.x * gridDim.y * blockDim.x; // the number of total threads
    int tid = threadIdx.x + blockIdx.x * blockDim.x + blockIdx.y * gridDim.x * blockDim.x; // thread id
    int i;
    for( i=tid; i<len; i+=total )
        gather[i] = v[col_ind[i]];
}

__global__ void compute( int *val, int *gather, int *tmp, int len ) {
    int total = gridDim.x * gridDim.y * blockDim.x; // the number of total threads
    int tid = threadIdx.x + blockIdx.x * blockDim.x + blockIdx.y * gridDim.x * blockDim.x; // thread id
    int i;
    for( i=tid; i<len; i+=total )
        tmp[i] = val[i] * gather[i];
}

#else

__global__ void compute( int *val, int *col_ind, int *v, int *tmp, int len ) {
    int total = gridDim.x * gridDim.y * blockDim.x; // the number of total threads
    int tid = threadIdx.x + blockIdx.x * blockDim.x + blockIdx.y * gridDim.x * blockDim.x; // thread id
    int i;
    for( i=tid; i<len; i+=total )
        tmp[i] = val[i] * v[col_ind[i]];
}

#endif

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
            "compute device .... gpu\n"
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

    struct {
        int *val;     // non-zero values
        int *col_ind; // column index
        int *row_ptr; // row pointer
        int *vector;
        int *tmp;
        int *result;
#ifdef WITH_GATHER
        int *gather;
#endif
    } device;

    size_t alloc_total = 0;
    cudaMalloc( (void**)&device.val, sizeof(*device.val) * crs.nonzeros );
    alloc_total += sizeof(*device.val) * crs.nonzeros;
    cudaMalloc( (void**)&device.col_ind, sizeof(*device.col_ind) * crs.nonzeros );
    alloc_total += sizeof(*device.col_ind) * crs.nonzeros;
    cudaMalloc( (void**)&device.row_ptr, sizeof(*device.row_ptr) * (crs.rows+1) );
    alloc_total += sizeof(*device.row_ptr) * (crs.rows+1);
    cudaMalloc( (void**)&device.vector, sizeof(*device.vector) * v.size );
    alloc_total += sizeof(*device.vector) * v.size;
    cudaMalloc( (void**)&device.tmp, sizeof(*device.tmp) * crs.nonzeros );
    alloc_total += sizeof(*device.tmp) * crs.nonzeros;
    cudaMalloc( (void**)&device.result, sizeof(*device.result) * crs.rows );
    alloc_total += sizeof(*device.result) * crs.rows;
#ifdef WITH_GATHER
    cudaMalloc( (void**)&device.gather, sizeof(*device.gather) * crs.nonzeros );
    alloc_total += sizeof(*device.gather) * crs.nonzeros;
#endif
    printf( "GPU memory allocation size: %zd bytes\n", alloc_total );
    
    TIMESTART;
    cudaMemcpy( device.val, crs.val, sizeof(*device.val)*crs.nonzeros, cudaMemcpyHostToDevice );
    cudaMemcpy( device.vector, v.data, sizeof(*device.vector)*v.size, cudaMemcpyHostToDevice );
    cudaMemcpy( device.col_ind, crs.col_ind, sizeof(*device.col_ind)*crs.nonzeros, cudaMemcpyHostToDevice );
    cudaMemcpy( device.row_ptr, crs.row_ptr, sizeof(*device.row_ptr)*(crs.rows+1), cudaMemcpyHostToDevice );
    TIMEEND("data transfer time (host to device)");

    dim3 blocks(32,32);
    dim3 threads(32,1);
    int loopcount = 20;

#ifdef WITH_GATHER
    TIMESTART;
    gather<<<blocks, threads>>>( device.vector, device.col_ind, device.gather, crs.nonzeros );
    TIMEEND( "gathering data" );
#endif

    TIMESTART;
    for( int loop=0; loop<loopcount; loop++ ) {
        //__global__ void compute( int *val, int *gather, int *tmp, int len ) {
#ifdef WITH_GATHER
        compute<<<blocks, threads>>>( device.val, device.gather, device.tmp, crs.nonzeros );
#else
        compute<<<blocks, threads>>>( device.val, device.col_ind, device.vector, device.tmp, crs.nonzeros );
#endif
        collect<<<blocks, threads>>>( device.row_ptr, device.tmp, device.result, crs.rows );
    }
    TIMEEND( "* COMPUTING TIME *" );

    // transfer result data to host and dump it to stderr
    int *p = (int*)malloc( sizeof(int) * crs.rows );
    cudaMemcpy( p, device.result, sizeof(*p)*crs.rows, cudaMemcpyDeviceToHost );
    dump_array( stderr, p, crs.rows );

    // clean up
    crs_delete( &crs );
    vec_delete( &v );
    free(p);
    cudaFree( device.val );
    cudaFree( device.col_ind );
    cudaFree( device.row_ptr );
    cudaFree( device.vector );
    cudaFree( device.tmp );
    cudaFree( device.result );

    return 0;
}
