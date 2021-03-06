/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#pragma once

#include <array>

#include "eckit/config/Parametrisation.h"
#include "eckit/memory/Builder.h"
#include "eckit/memory/Owned.h"
#include "eckit/utils/Hash.h"

#include "atlas/util/Config.h"
#include "atlas/util/Point.h"
#include "atlas/util/Rotation.h"

namespace atlas {
namespace projection {
namespace detail {

class ProjectionImpl : public eckit::Owned {
public:
    using ARG1      = const eckit::Parametrisation&;
    using builder_t = eckit::BuilderT1<ProjectionImpl>;
    using Spec      = atlas::util::Config;
    static std::string className() { return "atlas.Projection"; }

public:
    static ProjectionImpl* create();  // creates the LonLatProjection
    static ProjectionImpl* create( const eckit::Parametrisation& p );

    ProjectionImpl() {}
    virtual ~ProjectionImpl() {}  // destructor should be virtual

    virtual std::string type() const = 0;

    virtual void xy2lonlat( double crd[] ) const = 0;
    virtual void lonlat2xy( double crd[] ) const = 0;

    PointLonLat lonlat( const PointXY& ) const;
    PointXY xy( const PointLonLat& ) const;

    virtual bool strictlyRegional() const = 0;

    virtual Spec spec() const = 0;

    virtual std::string units() const = 0;

    virtual operator bool() const { return true; }

    virtual void hash( eckit::Hash& ) const = 0;
};

inline PointLonLat ProjectionImpl::lonlat( const PointXY& xy ) const {
    PointLonLat lonlat( xy );
    xy2lonlat( lonlat.data() );
    return lonlat;
}

inline PointXY ProjectionImpl::xy( const PointLonLat& lonlat ) const {
    PointXY xy( lonlat );
    lonlat2xy( xy.data() );
    return xy;
}

class Rotated : public util::Rotation {
public:
    using Spec = ProjectionImpl::Spec;

    Rotated( const PointLonLat& south_pole, double rotation_angle = 0. );
    Rotated( const eckit::Parametrisation& );
    virtual ~Rotated() {}

    static std::string classNamePrefix() { return "Rotated"; }
    static std::string typePrefix() { return "rotated_"; }

    void spec( Spec& ) const;

    void hash( eckit::Hash& ) const;
};

class NotRotated {
public:
    using Spec = ProjectionImpl::Spec;

    NotRotated() {}
    NotRotated( const eckit::Parametrisation& ) {}
    virtual ~NotRotated() {}

    static std::string classNamePrefix() { return ""; }  // deliberately empty
    static std::string typePrefix() { return ""; }       // deliberately empty

    void rotate( double crd[] ) const { /* do nothing */
    }
    void unrotate( double crd[] ) const { /* do nothing */
    }

    bool rotated() const { return false; }

    void spec( Spec& ) const {}

    void hash( eckit::Hash& ) const {}
};

}  // namespace detail
}  // namespace projection
}  // namespace atlas
