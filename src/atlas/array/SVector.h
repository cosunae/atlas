/*
* (C) Copyright 2013 ECMWF.
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
* In applying this licence, ECMWF does not waive the privileges and immunities
* granted to it by virtue of its status as an intergovernmental organisation nor
* does it submit to any jurisdiction.
*/

#pragma once

#include <cassert>
#include <cstddef>

#include "atlas/library/config.h"
#include "atlas/runtime/ErrorHandling.h"

#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
#include <cuda_runtime.h>
#endif

namespace atlas {
namespace array {

//------------------------------------------------------------------------------

template <typename T>
class SVector {
public:
    ATLAS_HOST_DEVICE
    SVector() : data_( nullptr ), size_( 0 ), externally_allocated_( false ) {}

    ATLAS_HOST_DEVICE
    SVector(const T* data, const idx_t size) : data_(data), size_(size), externally_allocated_( true ) {}

    ATLAS_HOST_DEVICE
    SVector( SVector const& other ) :
        data_( other.data_ ),
        size_( other.size_ ),
        externally_allocated_( other.externally_allocated_ ) {}

    ATLAS_HOST_DEVICE
    SVector( T* data, idx_t size ) : data_( data ), size_( size ), externally_allocated_(true) {}

    void allocate(T*& data, idx_t N) {
        if ( N != 0 ) {
#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
            cudaError_t err = cudaMallocManaged( &data, N * sizeof( T ) );
            if ( err != cudaSuccess ) throw eckit::AssertionFailed( "failed to allocate GPU memory" );
#else
            data = (T*)malloc( N * sizeof( T ) );
#endif
        }
    }
    SVector( idx_t N ) : data_( nullptr ), size_( N ), externally_allocated_( false ) {
        allocate(data_,N);
    }
    ATLAS_HOST_DEVICE
    ~SVector() {
#ifndef __CUDA_ARCH__
        if ( !externally_allocated_ ) delete_managedmem( data_ );
#endif
    }

    void delete_managedmem( T*& data ) {
        if ( data ) {
#if ATLAS_GRIDTOOLS_STORAGE_BACKEND_CUDA
            cudaError_t err = cudaDeviceSynchronize();
            if ( err != cudaSuccess ) throw eckit::AssertionFailed( "failed to synchronize memory" );

            err = cudaFree( data );
            // The following throws an invalid device memory

            if ( err != cudaSuccess ) throw eckit::AssertionFailed( "failed to free GPU memory" );

#else
            free( data );
#endif
            data = NULL;
        }
    }

    void insert(idx_t pos, idx_t dimsize) {
        T* data;
        allocate(data, size_ + dimsize);

        for(unsigned int c=0; c < pos; ++c) {
            data[c] = data_[c];
        }
        for(unsigned int c=pos; c < size_; ++c) {
            data[c+dimsize] = data_[c];
        }

        T* oldptr = data_;
        data_ = data;
        delete_managedmem(oldptr);
        size_+= dimsize;
    }

    size_t footprint() const {
        return sizeof(T) * size_;
    }

    ATLAS_HOST_DEVICE
    T* data() { return data_; }

    ATLAS_HOST_DEVICE
    T const* data() const { return data_; }

    ATLAS_HOST_DEVICE
    T& operator()( const idx_t idx ) {
        assert( data_ && idx < size_ );
        return data_[idx];
    }
    ATLAS_HOST_DEVICE
    T const& operator()( const idx_t idx ) const {
        assert( data_ && idx < size_ );
        return data_[idx];
    }

    ATLAS_HOST_DEVICE
    T& operator[]( const idx_t idx ) {
        assert( data_ && idx < size_ );
        return data_[idx];
    }
    ATLAS_HOST_DEVICE
    T const& operator[]( const idx_t idx ) const {
        assert( data_ && idx < size_ );
        return data_[idx];
    }

    ATLAS_HOST_DEVICE
    idx_t size() const { return size_; }

    void resize_impl( idx_t N ) {
        if ( N == size_ ) return;

        T* d_ = nullptr;
        allocate(d_,N);
        for ( unsigned int c = 0; c < std::min(size_, N); ++c ) {
            d_[c] = data_[c];
        }
        delete_managedmem( data_ );
        data_ = d_;
    }

    void resize( idx_t N ) {
        resize_impl( N );
        size_ = N;
    }

    void resize( idx_t N, T val ) {
        const int oldsize = size_;
        resize( N );
        for(unsigned int c=oldsize; c < size_; ++c) {
            data_[c] = val;
        }
    }

private:
    T* data_;
    idx_t size_;
    bool externally_allocated_;
};

//------------------------------------------------------------------------------

}  // namespace array
}  // namespace atlas
