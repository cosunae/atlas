# (C) Copyright 1996-2016 ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

if( HAVE_FCTEST )

  add_fctest( TARGET atlas_fctest_functionspace
    MPI             4
    LINKER_LANGUAGE Fortran
    SOURCES         fctest_functionspace.F90
    LIBS            atlas_f
  )

endif()

ecbuild_add_test( TARGET atlas_test_functionspace
  BOOST
  MPI      1
  SOURCES  test_functionspace.cc
  LIBS     atlas
)

