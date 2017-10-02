/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/functionspace/PointCloud.h"

#include "atlas/array.h"

#include "tests/AtlasTestEnvironment.h"
#include "eckit/testing/Test.h"


using namespace eckit;
using namespace eckit::testing;
using namespace atlas::functionspace;
using namespace atlas::util;

namespace atlas {
namespace test {

//-----------------------------------------------------------------------------

CASE( "test_functionspace_PointCloud" )
{

  Field points( "points", array::make_datatype<double>(), array::make_shape(10,2) );
  auto xy = array::make_view<double,2>(points);
  xy.assign( {
    00. , 0.,
    10. , 0.,
    20. , 0.,
    30. , 0.,
    40. , 0.,
    50. , 0.,
    60. , 0.,
    70. , 0.,
    80. , 0.,
    90. , 0.
  } );
  
  functionspace::PointCloud pointcloud( points );
  EXPECT( pointcloud.size() == 10 );

}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace atlas


int main(int argc, char **argv) {
    atlas::test::AtlasTestEnvironment env( argc, argv );
    return run_tests ( argc, argv, false );
}