/*
* (C) Copyright 2013 ECMWF.
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
* In applying this licence, ECMWF does not waive the privileges and immunities
* granted to it by virtue of its status as an intergovernmental organisation nor
* does it submit to any jurisdiction.
*/


#include "Allocate.h"

#include "atlas/runtime/ErrorHandling.h"
#include "atlas/library/config.h"
#include "eckit/log/CodeLocation.h"

#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
#include <cuda_runtime.h>
#endif

namespace atlas {
namespace util {

//------------------------------------------------------------------------------
namespace detail {
//------------------------------------------------------------------------------

void allocate_cudamanaged( void **ptr, size_t size ) {
#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
    cudaError_t err = cudaMallocManaged( ptr, size );
    if ( err != cudaSuccess )  throw eckit::AssertionFailed( "failed to allocate GPU memory", Here() );
#else
    *ptr = malloc( size );
#endif
}

void deallocate_cudamanaged( void* ptr ) {
#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
    cudaError_t err = cudaDeviceSynchronize();
    if ( err != cudaSuccess ) throw eckit::AssertionFailed( "failed to synchronize memory", Here() );

    err = cudaFree( ptr );
    // The following throws an invalid device memory
    if ( err != cudaSuccess ) throw eckit::AssertionFailed( "failed to free GPU memory", Here() );
#else
    free( ptr );
#endif
}

//------------------------------------------------------------------------------
} // namespace detail
//------------------------------------------------------------------------------

extern "C" {
    void atlas__allocate_managedmem_double(double* &a, int N) {
        allocate_managedmem( a, N );
    }
    void atlas__allocate_managedmem_float(float* &a, int N) {
        allocate_managedmem( a, N );
    }
    void atlas__allocate_managedmem_int(int* &a, int N) {
        allocate_managedmem( a, N );
    }
    void atlas__allocate_managedmem_long(long* &a, int N) {
        allocate_managedmem( a, N );
    }
    void atlas__deallocate_managedmem(void * &a) {
        delete_managedmem( a );
    }
}

//------------------------------------------------------------------------------

}  // namespace util
}  // namespace atlas
