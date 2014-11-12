/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/atlas_config.h"
#include "atlas/meshgen/RGG.h"
#include "atlas/meshgen/RGGMeshGenerator.h"
#include "atlas/Mesh.h"
#include "atlas/mpl/MPL.h"

using namespace atlas;

namespace atlas {
namespace test {

class TestGrid: public ReducedGrid {
public:
  TestGrid(int N, int lon[]);
};

TestGrid::TestGrid(int N, int lon[])
{
  std::vector<double> colat(N);
  predict_gaussian_colatitudes_hemisphere(N,colat.data());
  setup_colat_hemisphere(N,lon,colat.data(),DEG);
}

Mesh::Ptr generate_mesh( const ReducedGrid& rgg )
{
  meshgen::ReducedGridMeshGenerator generator;
  generator.options.set("nb_parts",MPL::size());
  generator.options.set("part",MPL::rank());
  return Mesh::Ptr( generator.generate( rgg ) );
}

Mesh::Ptr generate_mesh(int nlat, int lon[] )
{
  return generate_mesh( TestGrid(nlat,lon) );
}


} // end namespace test
} // end namespace atlas
