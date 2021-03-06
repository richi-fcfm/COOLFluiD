// Copyright (C) 2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/NullableObject.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

NullableObject::NullableObject() {}

NullableObject::~NullableObject() {}

bool NullableObject::isNull() const { return false; }

//////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace COOLFluiD
