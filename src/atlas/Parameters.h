/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef atlas_Parameters_h
#define atlas_Parameters_h

#include <cmath>
#include <string>

#include "atlas/atlas_defines.h"

//------------------------------------------------------------------------------------------------------

namespace atlas {

//------------------------------------------------------------------------------------------------------

enum { XX = 0, YY = 1, ZZ = 2 };

enum { LON = 0, LAT = 1 };

// ----------------------------------------------------------------------------------------------------

enum AngleUnit{ DEG=0, RAD=1 };

//------------------------------------------------------------------------------------------------------

/// Some usefull constants
/// @note These could be static const constants, but then the initialization would be in the .cc
///       which would preclude constant optimization.
///       With C++11 constexpr the constants can be initialized in the class header.
struct Constants
{
    static double radiansToDegrees() { return 180. * M_1_PI; }
    static double degreesToRadians() { return M_PI / 180; }

};

//------------------------------------------------------------------------------------------------------

struct Earth
{
    static double radiusInMeters() { return 6371229; }
    static double radiusInKm()     { return radiusInMeters() / 1.0E3; }

    static double areaInSqMeters() { return 4 * M_PI * radiusInMeters() * radiusInMeters(); }
    static double areaInSqKm()     { return 4 * M_PI * radiusInKm() * radiusInKm(); }

};

//------------------------------------------------------------------------------------------------------

/*===========================================================*/
/* TO BE REMOVED SOON                                        */
/*===========================================================*/

struct Entity
{
    enum Type { NODES=0, FACES=1, ELEMS=2 };
};

struct ElementRef {
  ElementRef() {}

  ElementRef(int func_space_idx, int elem_idx) : f(func_space_idx), e(elem_idx) {}

  int f;
  int e;

  bool operator<(const ElementRef& other) const {
    if (f < other.f) return true;
    if (f == other.f) return (e < other.e);
    return false;
  };
};

//------------------------------------------------------------------------------------------------------

} // namespace atlas

#endif // atlas_Parameters_h
