/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction. and Interpolation
 */

#pragma once

#include "StructuredInterpolation2D.h"


#include "eckit/exception/Exceptions.h"

#include "atlas/array/ArrayView.h"
#include "atlas/field/Field.h"
#include "atlas/field/FieldSet.h"
#include "atlas/functionspace/NodeColumns.h"
#include "atlas/functionspace/PointCloud.h"
#include "atlas/functionspace/StructuredColumns.h"
#include "atlas/grid/Grid.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/parallel/mpi/mpi.h"
#include "atlas/parallel/omp/omp.h"
#include "atlas/runtime/Log.h"
#include "atlas/runtime/Trace.h"
#include "atlas/util/CoordinateEnums.h"
#include "atlas/util/NormaliseLongitude.h"
#include "atlas/util/Point.h"

namespace atlas {
namespace interpolation {
namespace method {

namespace {
static double convert_units_multiplier( const Field& field ) {
    std::string units = field.metadata().getString( "units", "degrees" );
    if ( units == "degrees" ) { return 1.; }
    if ( units == "radians" ) { return 180. / M_PI; }
    NOTIMP;
}
}  // namespace

template <typename Kernel>
StructuredInterpolation2D<Kernel>::StructuredInterpolation2D( const Method::Config& config ) :
    Method( config ),
    matrix_free_{false} {
    config.get( "matrix_free", matrix_free_ );
}


template <typename Kernel>
void StructuredInterpolation2D<Kernel>::setup( const Grid& source, const Grid& target ) {
    if ( mpi::comm().size() > 1 ) { NOTIMP; }


    ASSERT( grid::StructuredGrid( source ) );
    FunctionSpace source_fs =
        functionspace::StructuredColumns( source, option::halo( std::max( kernel_->stencil_halo(), 1 ) ) );
    // guarantee "1" halo for pole treatment!
    FunctionSpace target_fs = functionspace::PointCloud( target );

    setup( source_fs, target_fs );
}


template <typename Kernel>
void StructuredInterpolation2D<Kernel>::setup( const FunctionSpace& source, const FunctionSpace& target ) {
    ATLAS_TRACE( "atlas::interpolation::method::StructuredInterpolation::setup()" );

    source_ = source;
    target_ = target;

    if ( functionspace::NodeColumns tgt = target ) {
        target_lonlat_ = tgt.mesh().nodes().lonlat();
        target_ghost_  = tgt.mesh().nodes().ghost();
    }
    else if ( functionspace::PointCloud tgt = target ) {
        target_lonlat_ = tgt.lonlat();
        target_ghost_  = tgt.ghost();
    }
    else {
        NOTIMP;
    }

    setup( source );
}

template <typename Kernel>
void StructuredInterpolation2D<Kernel>::setup( const FunctionSpace& source, const Field& target ) {
    ATLAS_TRACE( "atlas::interpolation::method::StructuredInterpolation2D::setup(FunctionSpace source, Field target)" );

    source_ = source;

    if ( target.functionspace() ) { target_ = target.functionspace(); }

    target_lonlat_ = target;

    setup( source );
}

template <typename Kernel>
void StructuredInterpolation2D<Kernel>::setup( const FunctionSpace& source, const FieldSet& target ) {
    ATLAS_TRACE( "atlas::interpolation::method::StructuredInterpolation::setup(FunctionSpace source,FieldSet target)" );

    source_ = source;

    ASSERT( target.size() >= 2 );
    if ( target[0].functionspace() ) { target_ = target[0].functionspace(); }

    target_lonlat_fields_ = target;

    setup( source );
}

template <typename Kernel>
void StructuredInterpolation2D<Kernel>::print( std::ostream& ) const {
    NOTIMP;
}


template <typename Kernel>
void StructuredInterpolation2D<Kernel>::setup( const FunctionSpace& source ) {
    kernel_.reset( new Kernel( source ) );

    if ( functionspace::StructuredColumns( source ).halo() < 1 ) {
        throw eckit::Exception( "The source functionspace must have (halo >= 1) for pole treatment" );
    }

    if ( not matrix_free_ ) {
        idx_t inp_npts = source.size();
        idx_t out_npts = target_lonlat_.shape( 0 );

        auto ghost  = array::make_view<int, 1>( target_ghost_ );
        auto lonlat = array::make_view<double, 2>( target_lonlat_ );

        double convert_units = convert_units_multiplier( target_lonlat_ );

        auto triplets = kernel_->allocate_triplets( out_npts );

        constexpr NormaliseLongitude normalise;
        //auto normalise = []( double x ) { return x; };
        ATLAS_TRACE_SCOPE( "Precomputing interpolation matrix" ) {
            atlas_omp_parallel {
                typename Kernel::WorkSpace workspace;
                atlas_omp_for( idx_t n = 0; n < out_npts; ++n ) {
                    if ( not ghost( n ) ) {
                        PointLonLat p{normalise( lonlat( n, LON ) ) * convert_units, lonlat( n, LAT ) * convert_units};
                        kernel_->insert_triplets( n, p, triplets, workspace );
                    }
                }
            }
            // fill sparse matrix and return
            Matrix A( out_npts, inp_npts, triplets );
            matrix_.swap( A );
        }
    }
}


template <typename Kernel>
void StructuredInterpolation2D<Kernel>::execute( const Field& src_field, Field& tgt_field ) const {
    FieldSet tgt( tgt_field );
    execute( FieldSet( src_field ), tgt );
}


template <typename Kernel>
void StructuredInterpolation2D<Kernel>::execute( const FieldSet& src_fields, FieldSet& tgt_fields ) const {
    if ( not matrix_free_ ) {
        Method::execute( src_fields, tgt_fields );
        return;
    }

    ATLAS_TRACE( "atlas::interpolation::method::StructuredInterpolation::execute()" );

    const idx_t N = src_fields.size();
    ASSERT( N == tgt_fields.size() );

    if ( N == 0 ) return;

    haloExchange( src_fields );

    array::DataType datatype = src_fields[0].datatype();
    int rank                 = src_fields[0].rank();

    for ( idx_t i = 0; i < N; ++i ) {
        ASSERT( src_fields[i].datatype() == datatype );
        ASSERT( src_fields[i].rank() == rank );
        ASSERT( tgt_fields[i].datatype() == datatype );
        ASSERT( tgt_fields[i].rank() == rank );
    }

    if ( datatype.kind() == array::DataType::KIND_REAL64 && rank == 1 ) {
        execute_impl<double, 1>( *kernel_, src_fields, tgt_fields );
    }
    if ( datatype.kind() == array::DataType::KIND_REAL32 && rank == 1 ) {
        execute_impl<float, 1>( *kernel_, src_fields, tgt_fields );
    }
    if ( datatype.kind() == array::DataType::KIND_REAL64 && rank == 2 ) {
        execute_impl<double, 2>( *kernel_, src_fields, tgt_fields );
    }
    if ( datatype.kind() == array::DataType::KIND_REAL32 && rank == 2 ) {
        execute_impl<float, 2>( *kernel_, src_fields, tgt_fields );
    }

    tgt_fields.set_dirty();
}


template <typename Kernel>
template <typename Value, int Rank>
void StructuredInterpolation2D<Kernel>::execute_impl( const Kernel& kernel, const FieldSet& src_fields,
                                                      FieldSet& tgt_fields ) const {
    const idx_t N = src_fields.size();

    std::vector<array::ArrayView<Value, Rank> > src_view;
    std::vector<array::ArrayView<Value, Rank> > tgt_view;
    src_view.reserve( N );
    tgt_view.reserve( N );

    for ( idx_t i = 0; i < N; ++i ) {
        src_view.emplace_back( array::make_view<Value, Rank>( src_fields[i] ) );
        tgt_view.emplace_back( array::make_view<Value, Rank>( tgt_fields[i] ) );
    }
    if ( target_lonlat_ ) {
        double convert_units = convert_units_multiplier( target_lonlat_ );

        if ( target_ghost_ ) {
            idx_t out_npts    = target_lonlat_.shape( 0 );
            const auto ghost  = array::make_view<int, 1, array::Intent::ReadOnly>( target_ghost_ );
            const auto lonlat = array::make_view<double, 2, array::Intent::ReadOnly>( target_lonlat_ );

            atlas_omp_parallel {
                typename Kernel::Stencil stencil;
                typename Kernel::Weights weights;
                atlas_omp_for( idx_t n = 0; n < out_npts; ++n ) {
                    if ( not ghost( n ) ) {
                        PointLonLat p{lonlat( n, LON ) * convert_units, lonlat( n, LAT ) * convert_units};
                        kernel.compute_stencil( p.lon(), p.lat(), stencil );
                        kernel.compute_weights( p.lon(), p.lat(), stencil, weights );
                        for ( idx_t i = 0; i < N; ++i ) {
                            kernel.interpolate( stencil, weights, src_view[i], tgt_view[i], n );
                        }
                    }
                }
            }
        }
        else {
            idx_t out_npts    = target_lonlat_.shape( 0 );
            const auto lonlat = array::make_view<double, 2, array::Intent::ReadOnly>( target_lonlat_ );

            atlas_omp_parallel {
                typename Kernel::Stencil stencil;
                typename Kernel::Weights weights;
                atlas_omp_for( idx_t n = 0; n < out_npts; ++n ) {
                    PointLonLat p{lonlat( n, LON ) * convert_units, lonlat( n, LAT ) * convert_units};
                    kernel.compute_stencil( p.lon(), p.lat(), stencil );
                    kernel.compute_weights( p.lon(), p.lat(), stencil, weights );
                    for ( idx_t i = 0; i < N; ++i ) {
                        kernel.interpolate( stencil, weights, src_view[i], tgt_view[i], n );
                    }
                }
            }
        }
    }
    else if ( not target_lonlat_fields_.empty() ) {
        idx_t out_npts       = target_lonlat_fields_[0].shape( 0 );
        const auto lon       = array::make_view<double, 1, array::Intent::ReadOnly>( target_lonlat_fields_[LON] );
        const auto lat       = array::make_view<double, 1, array::Intent::ReadOnly>( target_lonlat_fields_[LAT] );
        double convert_units = convert_units_multiplier( target_lonlat_fields_[LON] );

        atlas_omp_parallel {
            typename Kernel::Stencil stencil;
            typename Kernel::Weights weights;
            atlas_omp_for( idx_t n = 0; n < out_npts; ++n ) {
                PointLonLat p{lon( n ) * convert_units, lat( n ) * convert_units};
                kernel.compute_stencil( p.lon(), p.lat(), stencil );
                kernel.compute_weights( p.lon(), p.lat(), stencil, weights );
                for ( idx_t i = 0; i < N; ++i ) {
                    kernel.interpolate( stencil, weights, src_view[i], tgt_view[i], n );
                }
            }
        }
    }
    else {
        NOTIMP;
    }
}

}  // namespace method
}  // namespace interpolation
}  // namespace atlas
