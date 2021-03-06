#ifndef COOLFluiD_ShapeFunctions_SetTetraLagrangeP1SpectralFVP3StateCoord_hh
#define COOLFluiD_ShapeFunctions_SetTetraLagrangeP1SpectralFVP3StateCoord_hh

//////////////////////////////////////////////////////////////////////////////

#include "Framework/SetElementStateCoord.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Framework { class Node; }
  namespace Framework { class State; }

  namespace SpectralFV {


//////////////////////////////////////////////////////////////////////////////

/**
 * This class is a functor and offers an abstract interface
 * for setting the corresponding space coordinates (Framework::Node) in
 * the State's in a tetrahedron with P1 geometrical and P3 solution
 * interpolation.
 *
 * @warning No coordinates are set, as the states are not really related to a specific points in space
 *
 * @author Kris Van den Abeele
 */
class SetTetraLagrangeP1SpectralFVP3StateCoord : public Framework::SetElementStateCoord {

public:

  /**
   * Constructor
   */
  SetTetraLagrangeP1SpectralFVP3StateCoord() : Framework::SetElementStateCoord()
  {
  }

  /**
   * Destructor
   */
  ~SetTetraLagrangeP1SpectralFVP3StateCoord()
  {
  }

  /**
   * Overloading of the operator () to make this class act as a
   * functor
   * @param nodes   list of the nodes in the current element
   * @param states  list of the states in the current element
   */
  void operator() (const std::vector<Framework::Node*>& nodes,
                   std::vector<Framework::State*>& states);

  /**
   * Function allowing to update the StateCoord
   * @param nodes   list of the nodes in the current element
   * @param states  list of the states in the current element
   */
  void update(const std::vector<Framework::Node*>& nodes,
                            std::vector<Framework::State*>& states);

}; // end of class SetTetraLagrangeP1SpectralFVP3StateCoord

//////////////////////////////////////////////////////////////////////////////

  } // namespace SpectralFV

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_ShapeFunctions_SetTetraLagrangeP1SpectralFVP3StateCoord_hh
