/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file HybridElements.h
/// @author Willem Deconinck
/// @date October 2015
///
/// This file describes the HybridElements class for a Mesh.

#pragma once

#include "eckit/memory/Owned.h"
#include "eckit/memory/SharedPtr.h"

#include "atlas/field/Field.h"
#include "atlas/mesh/Connectivity.h"
#include "atlas/util/Metadata.h"

#include "atlas/field/detail/FieldImpl.h"  // only included for Fortran interface

namespace atlas {
class Mesh;
}
namespace atlas {
namespace mesh {
class ElementType;
}
}  // namespace atlas
namespace atlas {
namespace mesh {
class Elements;
}
}  // namespace atlas

namespace atlas {
namespace mesh {

// -------------------------------------------------------------------------------

/// @brief HybridElements class that describes elements of different types
class HybridElements : public eckit::Owned {
    friend class Elements;

public:
    typedef MultiBlockConnectivity Connectivity;

public:  // methods
         //-- Constructors
    HybridElements();
    virtual ~HybridElements();

    //-- Accessors

    /// @brief Number of elements
    size_t size() const;

    /// @brief Number of nodes for given element
    size_t nb_nodes( size_t elem_idx ) const;

    /// @brief Number of edges for given element
    size_t nb_edges( size_t elem_idx ) const;

    /// @brief Element type index for given element
    size_t type_idx( size_t elem_idx ) const;

    /// @brief Element type name for given element
    const std::string& name( size_t elem_idx ) const;

    /// @brief Element to Node connectivity table
    const HybridElements::Connectivity& node_connectivity() const;
    HybridElements::Connectivity& node_connectivity();

    /// @brief Element to Edge connectivity table
    const HybridElements::Connectivity& edge_connectivity() const;
    HybridElements::Connectivity& edge_connectivity();

    /// @brief Element to Cell connectivity table
    const HybridElements::Connectivity& cell_connectivity() const;
    HybridElements::Connectivity& cell_connectivity();

    /// @brief Number of types present in HybridElements
    size_t nb_types() const;

    /// @brief The element_type description for given type
    const ElementType& element_type( size_t type_idx ) const;

    /// @brief Sub-elements convenience class for given type
    /// This allows optimized access to connectivities and loops.
    const Elements& elements( size_t type_idx ) const;
    Elements& elements( size_t type_idx );

    const Field& field( const std::string& name ) const;
    Field& field( const std::string& name );
    bool has_field( const std::string& name ) const { return ( fields_.find( name ) != fields_.end() ); }

    const Field& field( size_t ) const;
    Field& field( size_t );
    size_t nb_fields() const { return fields_.size(); }

    const util::Metadata& metadata() const { return metadata_; }
    util::Metadata& metadata() { return metadata_; }

    const Field& global_index() const { return field( "glb_idx" ); }
    Field& global_index() { return field( "glb_idx" ); }

    const Field& remote_index() const { return field( "remote_idx" ); }
    Field& remote_index() { return field( "remote_idx" ); }

    const Field& partition() const { return field( "partition" ); }
    Field& partition() { return field( "partition" ); }

    const Field& halo() const { return field( "halo" ); }
    Field& halo() { return field( "halo" ); }

    // -- Modifiers

    /// @brief Add a new element type with given number of elements
    /// @return type_idx of the added element type
    size_t add( const ElementType*, size_t nb_elements );

    /// @brief Add a new element type with given number of elements and
    /// node-connectivity
    /// @return type_idx of the added element type
    size_t add( const ElementType*, size_t nb_elements, const std::vector<idx_t>& node_connectivity );

    /// @brief Add a new element type with given number of elements and
    /// node-connectivity
    /// @return type_idx of the added element type
    size_t add( const ElementType*, size_t nb_elements, const idx_t node_connectivity[] );

    /// @brief Add a new element type with given number of elements and
    /// node-connectivity
    /// @return type_idx of the added element type
    size_t add( const ElementType*, size_t nb_elements, const idx_t node_connectivity[], bool fortran_array );

    /// @brief Add a new element type from existing Elements.
    /// Data will be copied.
    /// @return type_idx of the added element type
    size_t add( const Elements& );

    Field add( const Field& field );

    void remove_field( const std::string& name );

    void insert( size_t type_idx, size_t position, size_t nb_elements = 1 );

    void cloneToDevice() const;

    void cloneFromDevice() const;

    void syncHostDevice() const;

    void clear();

    /// @brief Return the memory footprint of the elements
    size_t footprint() const;

private:  // -- types
    typedef std::map<std::string, Field> FieldMap;
    typedef std::map<std::string, eckit::SharedPtr<Connectivity>> ConnectivityMap;

private:  // -- methods
    void resize( size_t size );

    size_t elemtype_nb_nodes( size_t elem_idx ) const;
    size_t elemtype_nb_edges( size_t elem_idx ) const;

    Connectivity& add( Connectivity* );

private:  // -- Data
          // -- Total number of elements
    size_t size_;

    // -- Data: one value per type
    std::vector<size_t> elements_size_;
    std::vector<size_t> elements_begin_;
    std::vector<eckit::SharedPtr<const ElementType>> element_types_;

    // -- Data: one value per element
    std::vector<size_t> type_idx_;

    // -- Sub elements
    std::vector<eckit::SharedPtr<Elements>> elements_;

    // -- Fields and connectivities
    FieldMap fields_;
    ConnectivityMap connectivities_;

    // -- Metadata
    util::Metadata metadata_;

    // -- Cached shortcuts to specific connectivities in connectivities_
    Connectivity* node_connectivity_;
    Connectivity* edge_connectivity_;
    Connectivity* cell_connectivity_;
};

// -----------------------------------------------------------------------------------------------------

inline size_t HybridElements::size() const {
    return size_;
}

inline size_t HybridElements::nb_types() const {
    return element_types_.size();
}

inline const ElementType& HybridElements::element_type( size_t type_idx ) const {
    return *element_types_[type_idx].get();
}

inline const HybridElements::Connectivity& HybridElements::node_connectivity() const {
    return *node_connectivity_;
}

inline HybridElements::Connectivity& HybridElements::node_connectivity() {
    return *node_connectivity_;
}

inline const HybridElements::Connectivity& HybridElements::edge_connectivity() const {
    return *edge_connectivity_;
}

inline HybridElements::Connectivity& HybridElements::edge_connectivity() {
    return *edge_connectivity_;
}

inline const HybridElements::Connectivity& HybridElements::cell_connectivity() const {
    return *cell_connectivity_;
}

inline HybridElements::Connectivity& HybridElements::cell_connectivity() {
    return *cell_connectivity_;
}

inline const Elements& HybridElements::elements( size_t type_idx ) const {
    return *elements_[type_idx].get();
}

inline Elements& HybridElements::elements( size_t type_idx ) {
    return *elements_[type_idx].get();
}

inline size_t HybridElements::nb_nodes( size_t elem_idx ) const {
    return node_connectivity_->rows() ? node_connectivity_->cols( elem_idx ) : elemtype_nb_nodes( elem_idx );
}

inline size_t HybridElements::nb_edges( size_t elem_idx ) const {
    return edge_connectivity_->rows() ? edge_connectivity_->cols( elem_idx ) : elemtype_nb_edges( elem_idx );
}

inline size_t HybridElements::type_idx( size_t elem_idx ) const {
    return type_idx_[elem_idx];
}

// ------------------------------------------------------------------------------------------------------

extern "C" {
HybridElements* atlas__mesh__HybridElements__create();
void atlas__mesh__HybridElements__delete( HybridElements* This );
MultiBlockConnectivity* atlas__mesh__HybridElements__node_connectivity( HybridElements* This );
MultiBlockConnectivity* atlas__mesh__HybridElements__edge_connectivity( HybridElements* This );
MultiBlockConnectivity* atlas__mesh__HybridElements__cell_connectivity( HybridElements* This );

size_t atlas__mesh__HybridElements__size( const HybridElements* This );
void atlas__mesh__HybridElements__add_elements( HybridElements* This, ElementType* elementtype, size_t nb_elements );
void atlas__mesh__HybridElements__add_elements_with_nodes( HybridElements* This, ElementType* elementtype,
                                                           size_t nb_elements, int node_connectivity[],
                                                           int fortran_array );
void atlas__mesh__HybridElements__add_field( HybridElements* This, field::FieldImpl* field );
int atlas__mesh__HybridElements__has_field( const HybridElements* This, char* name );
int atlas__mesh__HybridElements__nb_fields( const HybridElements* This );
int atlas__mesh__HybridElements__nb_types( const HybridElements* This );
field::FieldImpl* atlas__mesh__HybridElements__field_by_name( HybridElements* This, char* name );
field::FieldImpl* atlas__mesh__HybridElements__field_by_idx( HybridElements* This, size_t idx );
field::FieldImpl* atlas__mesh__HybridElements__global_index( HybridElements* This );
field::FieldImpl* atlas__mesh__HybridElements__remote_index( HybridElements* This );
field::FieldImpl* atlas__mesh__HybridElements__partition( HybridElements* This );
field::FieldImpl* atlas__mesh__HybridElements__halo( HybridElements* This );

Elements* atlas__mesh__HybridElements__elements( HybridElements* This, size_t idx );
}

}  // namespace mesh
}  // namespace atlas
