// Copyright (C) 2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COOLFluiD_Numerics_NewtonMethod_LinearizedBDF2Setup_hh
#define COOLFluiD_Numerics_NewtonMethod_LinearizedBDF2Setup_hh

//////////////////////////////////////////////////////////////////////////////

#include "BDF2Setup.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Numerics {

    namespace NewtonMethod {

//////////////////////////////////////////////////////////////////////////////

  /// This class represents a NumericalCommand action to be
  /// sent to Domain to be executed in order to setup the convergence method.

class LinearizedBDF2Setup : public BDF2Setup {
public:

  /// Constructor.
  explicit LinearizedBDF2Setup(std::string name);

  /// Destructor.
  ~LinearizedBDF2Setup();

  /// Returns the DataSocket's that this command provides as sources
  /// @return a vector of SafePtr with the DataSockets
  virtual std::vector<Common::SafePtr<Framework::BaseDataSocketSource> > providesSockets();

  /// Execute Processing actions
  void execute();

protected:

  /// socket for past time component
  Framework::DataSocketSource<Framework::State*> socket_linearizedStates;

  /// socket for PastStates's
  Framework::DataSocketSource<CFreal> socket_pastRhs;

}; // class Setup

//////////////////////////////////////////////////////////////////////////////

    } // namespace NewtonMethod

  } // namespace Numerics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Numerics_NewtonMethod_LinearizedBDF2Setup_hh

