// Copyright (C) 2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COOLFluiD_IO_THOR2CFmesh_CheckNodeNumberingHexa_hh
#define COOLFluiD_IO_THOR2CFmesh_CheckNodeNumberingHexa_hh

//////////////////////////////////////////////////////////////////////////////

#include "CheckNodeNumbering.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {



    namespace THOR2CFmesh {

//////////////////////////////////////////////////////////////////////////////

/**
 * This class represents CheckNodeNumbering for Hexahedra elements.
 *
 * @author Tiago Quintino
 *
 */
class CheckNodeNumberingHexa : public CheckNodeNumbering {
public:

  /**
   * Default constructor without arguments
   */
  CheckNodeNumberingHexa(const Common::Table<CFreal> *const nodes);

  /**
   * Default destructor
   */
  ~CheckNodeNumberingHexa();

  /**
   * Checks the numbering of the nodes and makes it stick
   * to the COOLFluiD convention.
   */
  void checkElementNodalNumberingImpl()
    throw (Framework::NegativeVolumeException);

private: // data

  /// the number of nodes per element = 8
  static const CFuint nbNodesPerElem;

  /// the dimensions = 3
  static const CFuint dim;

}; // end of class CheckNodeNumberingHexa

//////////////////////////////////////////////////////////////////////////////

    } // namespace THOR2CFmesh



} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_IO_THOR2CFmesh_CheckNodeNumberingHexa_hh
