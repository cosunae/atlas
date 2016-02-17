/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/actions/BuildCellCentres.h"
#include "atlas/Parameters.h"
#include "atlas/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/mesh/HybridElements.h"
#include "atlas/FunctionSpace.h"
#include "atlas/Field.h"
#include "atlas/util/ArrayView.h"
#include "atlas/util/IndexView.h"

namespace atlas {
namespace actions {

#if !DEPRECATE_OLD_FUNCTIONSPACE
void build_cell_centres_convert_to_old(Mesh& mesh);
#endif

void BuildCellCentres::operator()( Mesh& mesh ) const
{
  mesh::Nodes& nodes     = mesh.nodes();
  ArrayView<double,2> coords  ( nodes.field("xyz") );

  size_t nb_cells = mesh.cells().size();
  ArrayView<double,2> centroids ( mesh.cells().add( Field::create<double>("centre", make_shape(nb_cells,3))) );
  const mesh::HybridElements::Connectivity& cell_node_connectivity = mesh.cells().node_connectivity();

  for (size_t e=0; e<nb_cells; ++e)
  {
    centroids(e,XX) = 0.;
    centroids(e,YY) = 0.;
    centroids(e,ZZ) = 0.;
    const size_t nb_nodes_per_elem = cell_node_connectivity.cols(e);
    const double average_coefficient = 1./static_cast<double>(nb_nodes_per_elem);
    for (size_t n=0; n<nb_nodes_per_elem; ++n)
    {
      centroids(e,XX) += coords( cell_node_connectivity(e,n), XX );
      centroids(e,YY) += coords( cell_node_connectivity(e,n), YY );
      centroids(e,ZZ) += coords( cell_node_connectivity(e,n), ZZ );
    }
    centroids(e,XX) *= average_coefficient;
    centroids(e,YY) *= average_coefficient;
    centroids(e,ZZ) *= average_coefficient;
  }
#if !DEPRECATE_OLD_FUNCTIONSPACE
  build_cell_centres_convert_to_old(mesh);
#endif
}

#if !DEPRECATE_OLD_FUNCTIONSPACE
void build_cell_centres_convert_to_old(Mesh& mesh)
{
    ASSERT( mesh.has_function_space("triags") );
    ASSERT( mesh.has_function_space("quads") );

    mesh::Nodes& nodes     = mesh.nodes();
    ArrayView<double,2> coords  ( nodes.field("xyz") );

    if( mesh.has_function_space("triags") ) {

        deprecated::FunctionSpace& triags = mesh.function_space( "triags" );
        IndexView<int,2> triag_nodes ( triags.field( "nodes" ) );
        const size_t nb_triags = triags.shape(0);

        ArrayView<double,2> triags_centres ( triags.create_field<double>("centre",3) );

        const double third = 1. / 3.;
        for(size_t e = 0; e < nb_triags; ++e)
        {
            const size_t i0 =  triag_nodes(e,0);
            const size_t i1 =  triag_nodes(e,1);
            const size_t i2 =  triag_nodes(e,2);

            triags_centres(e,XX) = third * ( coords(i0,XX) + coords(i1,XX) + coords(i2,XX) );
            triags_centres(e,YY) = third * ( coords(i0,YY) + coords(i1,YY) + coords(i2,YY) );
            triags_centres(e,ZZ) = third * ( coords(i0,ZZ) + coords(i1,ZZ) + coords(i2,ZZ) );

        }
    }

    if( mesh.has_function_space("quads") ) {
        deprecated::FunctionSpace& quads  = mesh.function_space( "quads" );
        IndexView<int,2> quads_nodes ( quads.field( "nodes" ) );
        const size_t nb_quads = quads.shape(0);

        ArrayView<double,2> quads_centres ( quads.create_field<double>("centre",3) );

        const double fourth = 1. / 4.;
        for(size_t e = 0; e < nb_quads; ++e)
        {
            const size_t i0 =  quads_nodes(e,0);
            const size_t i1 =  quads_nodes(e,1);
            const size_t i2 =  quads_nodes(e,2);
            const size_t i3 =  quads_nodes(e,3);

            quads_centres(e,XX) = fourth * ( coords(i0,XX) + coords(i1,XX) + coords(i2,XX) + coords(i3,XX) );
            quads_centres(e,YY) = fourth * ( coords(i0,YY) + coords(i1,YY) + coords(i2,YY) + coords(i3,YY) );
            quads_centres(e,ZZ) = fourth * ( coords(i0,ZZ) + coords(i1,ZZ) + coords(i2,ZZ) + coords(i3,ZZ) );

        }
    }
}
#endif

} // actions
} // atlas
