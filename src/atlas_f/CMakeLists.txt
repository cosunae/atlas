# (C) Copyright 2013 ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/atlas_f.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/../atlas/atlas_f.h )

install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/../atlas/atlas_f.h
  DESTINATION
  ${INSTALL_INCLUDE_DIR}/atlas )


### fortran bindings

function(generate_fortran_bindings output filename)

  set( options "" )
  set( single_value_args OUTPUT MODULE )
  set( multi_value_args "" )
  cmake_parse_arguments( _PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN} )

  get_filename_component(base ${filename} NAME_WE)
  set(base_abs ${CMAKE_CURRENT_SOURCE_DIR}/${base})
  set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${base}_c_binding.f90)

  if( _PAR_OUTPUT )
    set(outfile ${_PAR_OUTPUT})
  endif()
  set(${output} ${${output}} ${outfile} PARENT_SCOPE)

  if( NOT _PAR_MODULE )
    set( _PAR_MODULE "atlas_${base}_c_binding" )
  endif()

  add_custom_command(
    OUTPUT ${outfile}
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/c2f.py ${CMAKE_CURRENT_SOURCE_DIR}/${filename} -o ${outfile} -m ${_PAR_MODULE}
    DEPENDS ${filename} )
  set_source_files_properties(${outfile} PROPERTIES GENERATED TRUE)
endfunction()

set( PYTHON_VERSION "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}")
set( PYTHON_REQUIRED_VERSION 2.7.8 )
if( PYTHON_VERSION VERSION_LESS ${PYTHON_REQUIRED_VERSION} )
  ecbuild_warn("Python version ${PYTHON_VERSION} is inadequate to use Fortran preprocessor 'fypp'."
          "Required version is ${PYTHON_REQUIRED_VERSION}. No problem, previously preprocessed files will be used."
          "Just be aware that these will not be updated." )

  function(preprocess_fypp output filename)

    set( options "" )
    set( single_value_args OUTPUT )
    set( multi_value_args "" )
    cmake_parse_arguments( _PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN} )

    get_filename_component(base ${filename} NAME_WE)
    set(base_abs ${CMAKE_CURRENT_SOURCE_DIR}/${base})
    set(outfile ${CMAKE_CURRENT_SOURCE_DIR}/autogenerated/${base}_fypp.F90)

    if( _PAR_OUTPUT )
      set(outfile ${_PAR_OUTPUT})
    endif()
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)
  endfunction()

else()

  function(preprocess_fypp output filename)

    set( options "" )
    set( single_value_args OUTPUT )
    set( multi_value_args "" )
    cmake_parse_arguments( _PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN} )

    get_filename_component(base ${filename} NAME_WE)
    set(base_abs ${CMAKE_CURRENT_SOURCE_DIR}/${base})
    set(outfile ${CMAKE_CURRENT_SOURCE_DIR}/autogenerated/${base}_fypp.F90)
    # set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${filename})

    if( _PAR_OUTPUT )
      set(outfile ${_PAR_OUTPUT})
    endif()
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)

    add_custom_command(
      OUTPUT ${outfile}
      COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/fypp -l 132 -p ${CMAKE_CURRENT_SOURCE_DIR}/${filename} ${outfile}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${filename} )
    set_source_files_properties(${outfile} PROPERTIES GENERATED TRUE)
  endfunction()

endif()

generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/grid.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/grid/detail/grid/Structured.h
    MODULE    atlas_grid_Structured_c_binding
    OUTPUT    grid_Structured_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/grid/Distribution.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/grid/Partitioner.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/detail/MeshIntf.h
    MODULE    atlas_mesh_c_binding
    OUTPUT    atlas_mesh_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/Nodes.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/Connectivity.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/HybridElements.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/Elements.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/ElementType.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/BuildParallelFields.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/BuildPeriodicBoundaries.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/BuildHalo.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/BuildEdges.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/BuildDualMesh.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/mesh/actions/WriteLoadBalanceReport.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/meshgenerator/MeshGenerator.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/output/Output.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/output/Gmsh.h
  MODULE    atlas_output_gmsh_c_binding
  OUTPUT    outout_Gmsh_c_binding.f90)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/field/State.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/field/detail/FieldImpl.h
    MODULE    atlas_field_c_binding
    OUTPUT    field_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/field/FieldSet.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/functionspace/FunctionSpace.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/functionspace/Spectral.h
    MODULE    atlas_functionspace_Spectral_c_binding
    OUTPUT    functionspace_Spectral_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/functionspace/StructuredColumns.h
    MODULE    atlas_functionspace_StructuredColumns_c_binding
    OUTPUT    functionspace_StructuredColumns_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/functionspace/NodeColumnsInterface.h
    MODULE    atlas_functionspace_NodeColumns_c_binding
    OUTPUT    functionspace_NodeColumns_c_binding.f90)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/functionspace/EdgeColumns.h
    MODULE    atlas_functionspace_EdgeColumns_c_binding
    OUTPUT    functionspace_EdgeColumns_c_binding.f90)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/numerics/Nabla.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/numerics/Nabla.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/numerics/Method.h )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/numerics/fvm/Method.h
    MODULE    atlas_fvm_method_c_binding
    OUTPUT    fvm_method_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/interpolation/Interpolation.h
      MODULE    atlas_interpolation_c_binding
      OUTPUT    interpolation_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/trans/ifs/TransIFS.h
  MODULE    atlas_trans_c_binding
  OUTPUT    trans_c_binding.f90 )
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/util/Metadata.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/util/Config.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/output/detail/GmshIO.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/parallel/HaloExchange.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/parallel/GatherScatter.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/parallel/Checksum.h)
generate_fortran_bindings(FORTRAN_BINDINGS ../atlas/runtime/ErrorHandling.h)
generate_fortran_bindings(FORTRAN_BINDINGS internals/atlas_read_file.h)
generate_fortran_bindings(FORTRAN_BINDINGS internals/Library.h)

preprocess_fypp( atlas_f_src field/atlas_Field_module.F90 )

## gridtools_storage_files ##
add_custom_target( pre_processed_files SOURCES field/atlas_Field_module.F90 )

ecbuild_debug_var( atlas_f_src )

### atlas fortran lib
ecbuild_add_library( TARGET atlas_f
    AUTO_VERSION
    CONDITION CMAKE_Fortran_COMPILER_LOADED
    SOURCES
        ${FORTRAN_BINDINGS}
        ${atlas_f_src}
        atlas_module.F90
        util/atlas_kinds_module.F90
        util/atlas_JSON_module.F90
        util/atlas_Config_module.F90
        util/atlas_Metadata_module.F90
        util/atlas_Error_module.F90
        output/atlas_output_module.F90
        functionspace/atlas_FunctionSpace_module.F90
        functionspace/atlas_functionspace_EdgeColumns_module.F90
        functionspace/atlas_functionspace_NodeColumns_module.F90
        functionspace/atlas_functionspace_StructuredColumns_module.F90
        functionspace/atlas_functionspace_Spectral_module.F90
        field/atlas_FieldSet_module.F90
        field/atlas_State_module.F90
        grid/atlas_Grid_module.F90
        grid/atlas_GridDistribution_module.F90
        grid/atlas_Partitioner_module.F90
        mesh/atlas_MeshGenerator_module.F90
        mesh/atlas_Mesh_module.F90
        mesh/atlas_mesh_Nodes_module.F90
        mesh/atlas_mesh_Edges_module.F90
        mesh/atlas_mesh_Cells_module.F90
        mesh/atlas_Connectivity_module.F90
        mesh/atlas_HybridElements_module.F90
        mesh/atlas_Elements_module.F90
        mesh/atlas_ElementType_module.F90
        mesh/atlas_mesh_actions_module.F90
        numerics/atlas_Method_module.F90
        numerics/atlas_fvm_module.F90
        numerics/atlas_Nabla_module.F90
        interpolation/atlas_Interpolation_module.F90
        parallel/atlas_mpi_module.F90
        parallel/atlas_GatherScatter_module.F90
        parallel/atlas_Checksum_module.F90
        parallel/atlas_HaloExchange_module.F90
        trans/atlas_Trans_module.F90
        internals/atlas_read_file.h
        internals/atlas_read_file.cc
        internals/atlas_write_to_fortran_unit.F90
        internals/Library.h
        internals/Library.cc

  PRIVATE_INCLUDES
        ${FCKIT_INCLUDE_DIRS}

  LIBS atlas fckit
)
