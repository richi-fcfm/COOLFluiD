#ifndef COOLFluiD_Physics_LESvki_WALES2DVarSet_hh
#define COOLFluiD_Physics_LESvki_WALES2DVarSet_hh

//////////////////////////////////////////////////////////////////////////////

#include "NavierStokes/NavierStokesTurbVarSetTypes.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Physics {

    namespace LESvki {

//////////////////////////////////////////////////////////////////////////////

  /**
   * This class represents a NavierStokes physical model 2D for primitive
   * variables
   *
   * @author Andrea Lani
   */
class WALES2DVarSet : public NavierStokes::NavierStokesTurb2DVarSet {
public: // classes

  /**
   * Constructor
   * @see Smagorinsky2D
   */
   WALES2DVarSet(const std::string& name,
		       Common::SafePtr<Framework::PhysicalModelImpl> model) :
     NavierStokes::NavierStokesTurb2DVarSet(name, model)
  {
  }

  /**
   * Default destructor
   */
  virtual ~WALES2DVarSet()
  {
  }

  /**
   * Set up private data
   */
  virtual void setup();

  /**
   * Get the diffusive flux
   */
  virtual RealVector& getFlux(const RealVector& values,
                              const std::vector<RealVector*>& gradients,
                              const RealVector& normal,
                              const CFreal& radius);

  /**
   * Get the diffusive flux vector
   */
  virtual RealMatrix& getFlux(const RealVector& values,
                              const std::vector<RealVector*>& gradients,
                              const CFreal& radius);
  
  /**
   * Get the axisymmetric source term
   */
  virtual void getAxiSourceTerm(const RealVector& physicalData,
				const RealVector& state,
				const std::vector<RealVector*>& gradients,
				const CFreal& radius,
				RealVector& source);
  
  /**
   * Get the heat flux
   */
  virtual CFreal getHeatFlux(const RealVector& state,
                             const std::vector<RealVector*>& gradients,
                             const RealVector& normal);
  
}; // end of class WALES2DVarSet

//////////////////////////////////////////////////////////////////////////////

    } // namespace LESvki

  } // namespace Physics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Physics_LESvki_Smagorinsky2DVarSet_hh
