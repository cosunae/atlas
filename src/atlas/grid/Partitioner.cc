/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "atlas/grid/Partitioner.h"
#include "atlas/grid/detail/partitioner/Partitioner.h"
#include "atlas/parallel/mpi/mpi.h"
#include "atlas/runtime/Trace.h"

namespace atlas {
namespace grid {

using Factory = detail::partitioner::PartitionerFactory;

bool Partitioner::exists( const std::string& type ) {
    return Factory::has( type );
}

Partitioner::Partitioner( const detail::partitioner::Partitioner* partitioner ) : partitioner_( partitioner ) {}

Partitioner::Partitioner( const std::string& type ) : partitioner_( Factory::build( type ) ) {}

Partitioner::Partitioner( const std::string& type, const size_t nb_partitions ) :
    partitioner_( Factory::build( type, nb_partitions ) ) {}

namespace {
detail::partitioner::Partitioner* partitioner_from_config( const Partitioner::Config& config ) {
    std::string type;
    long partitions = mpi::comm().size();
    if ( not config.get( "type", type ) )
        throw eckit::BadParameter( "'type' missing in configuration for Partitioner", Here() );
    config.get( "partitions", partitions );
    return Factory::build( type, partitions );
}
}  // namespace

Partitioner::Partitioner( const Config& config ) : partitioner_( partitioner_from_config( config ) ) {}

void Partitioner::partition( const Grid& grid, int part[] ) const {
    ATLAS_TRACE();
    partitioner_->partition( grid, part );
}

MatchingMeshPartitioner::MatchingMeshPartitioner() : Partitioner() {}

grid::detail::partitioner::Partitioner* matching_mesh_partititioner( const Mesh& mesh,
                                                                     const Partitioner::Config& config ) {
    std::string type( "lonlat-polygon" );
    config.get( "type", type );
    return MatchedPartitionerFactory::build( type, mesh );
}

MatchingMeshPartitioner::MatchingMeshPartitioner( const Mesh& mesh, const Config& config ) :
    Partitioner( matching_mesh_partititioner( mesh, config ) ) {}

extern "C" {

detail::partitioner::Partitioner* atlas__grid__Partitioner__new( const Partitioner::Config* config ) {
    detail::partitioner::Partitioner* p;
    {
        Partitioner partitioner( *config );
        p = const_cast<detail::partitioner::Partitioner*>( partitioner.get() );
        p->attach();
    }
    p->detach();
    return p;
}

detail::partitioner::Partitioner* atlas__grid__MatchingMeshPartitioner__new( const Mesh::Implementation* mesh,
                                                                             const Partitioner::Config* config ) {
    detail::partitioner::Partitioner* p;
    {
        MatchingMeshPartitioner partitioner( Mesh( mesh ), *config );
        p = const_cast<detail::partitioner::Partitioner*>( partitioner.get() );
        p->attach();
    }
    p->detach();
    return p;
}

Distribution::impl_t* atlas__grid__Partitioner__partition( const Partitioner::Implementation* This,
                                                           const Grid::Implementation* grid ) {
    Distribution::impl_t* d;
    {
        Distribution distribution = This->partition( Grid( grid ) );
        d                         = const_cast<Distribution::impl_t*>( distribution.get() );
        d->attach();
    }
    d->detach();
    return d;
}

void atlas__grid__Partitioner__delete( detail::partitioner::Partitioner* This ) {
    delete This;
}

}  // extern "C"

}  // namespace grid
}  // namespace atlas
