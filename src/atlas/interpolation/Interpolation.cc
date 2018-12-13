/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"

#include "atlas/field/Field.h"
#include "atlas/field/FieldSet.h"
#include "atlas/functionspace/FunctionSpace.h"
#include "atlas/interpolation/Interpolation.h"

namespace atlas {

Interpolation::Interpolation( const Config& config, const FunctionSpace& source, const FunctionSpace& target ) :
    implementation_( [&]() -> Implementation* {
        std::string type;
        ASSERT( config.get( "type", type ) );
        Implementation* impl = interpolation::MethodFactory::build( type, config );
        impl->setup( source, target );
        return impl;
    }() ) {
    std::string path;
    if ( config.get( "output", path ) ) {
        std::ofstream file( path );
        print( file );
    }
}

Interpolation::Interpolation( const Config& config, const Grid& source, const Grid& target ) :
    implementation_( [&]() -> Implementation* {
        std::string type;
        ASSERT( config.get( "type", type ) );
        Implementation* impl = interpolation::MethodFactory::build( type, config );
        impl->setup( source, target );
        return impl;
    }() ) {
    std::string path;
    if ( config.get( "output", path ) ) {
        std::ofstream file( path );
        print( file );
    }
}

Interpolation::Interpolation( const Config& config, const FunctionSpace& source, const Field& target ) :
    implementation_( [&]() -> Implementation* {
        std::string type;
        ASSERT( config.get( "type", type ) );
        Implementation* impl = interpolation::MethodFactory::build( type, config );
        impl->setup( source, target );
        return impl;
    }() ) {
    std::string path;
    if ( config.get( "output", path ) ) {
        std::ofstream file( path );
        print( file );
    }
}

Interpolation::Interpolation( const Interpolation::Config& config, const FunctionSpace& source,
                              const FieldSet& target ) :
    implementation_( [&]() -> Implementation* {
        std::string type;
        ASSERT( config.get( "type", type ) );
        Implementation* impl = interpolation::MethodFactory::build( type, config );
        impl->setup( source, target );
        return impl;
    }() ) {
    std::string path;
    if ( config.get( "output", path ) ) {
        std::ofstream file( path );
        print( file );
    }
}

Interpolation::Interpolation( const Interpolation& other ) : implementation_( other.implementation_ ) {}

void Interpolation::execute( const FieldSet& source, FieldSet& target ) const {
    get()->execute( source, target );
}

void Interpolation::execute( const Field& source, Field& target ) const {
    get()->execute( source, target );
}

const Interpolation::Implementation* Interpolation::get() const {
    return implementation_.get();
}

void Interpolation::print( std::ostream& out ) const {
    implementation_->print( out );
}

const FunctionSpace& Interpolation::source() const {
    return implementation_->source();
}

const FunctionSpace& Interpolation::target() const {
    return implementation_->target();
}

atlas::Interpolation::operator bool() const {
    return implementation_;
}

extern "C" {
Interpolation::Implementation* atlas__Interpolation__new( const eckit::Parametrisation* config,
                                                          const functionspace::FunctionSpaceImpl* source,
                                                          const functionspace::FunctionSpaceImpl* target ) {
    Interpolation::Implementation* interpolator;
    {
        Interpolation im( *config, FunctionSpace( source ), FunctionSpace( target ) );
        interpolator = const_cast<Interpolation::Implementation*>( im.get() );
        interpolator->attach();
    }
    interpolator->detach();
    return interpolator;
}

Interpolation::Implementation* atlas__Interpolation__new_tgt_field( const eckit::Parametrisation* config,
                                                                    const functionspace::FunctionSpaceImpl* source,
                                                                    const field::FieldImpl* target ) {
    Interpolation::Implementation* interpolator;
    {
        Interpolation im( *config, FunctionSpace( source ), Field( target ) );
        interpolator = const_cast<Interpolation::Implementation*>( im.get() );
        interpolator->attach();
    }
    interpolator->detach();
    return interpolator;
}

Interpolation::Implementation* atlas__Interpolation__new_tgt_fieldset( const eckit::Parametrisation* config,
                                                                       const functionspace::FunctionSpaceImpl* source,
                                                                       const field::FieldSetImpl* target ) {
    Interpolation::Implementation* interpolator;
    {
        Interpolation im( *config, FunctionSpace( source ), FieldSet( target ) );
        interpolator = const_cast<Interpolation::Implementation*>( im.get() );
        interpolator->attach();
    }
    interpolator->detach();
    return interpolator;
}

void atlas__Interpolation__delete( Interpolation::Implementation* This ) {
    delete This;
}

void atlas__Interpolation__execute_field( Interpolation::Implementation* This, const field::FieldImpl* source,
                                          field::FieldImpl* target ) {
    Field t( target );
    This->execute( Field( source ), t );
}

void atlas__Interpolation__execute_fieldset( Interpolation::Implementation* This, const field::FieldSetImpl* source,
                                             field::FieldSetImpl* target ) {
    FieldSet t( target );
    This->execute( FieldSet( source ), t );
}

}  // extern "C"

}  // namespace atlas
