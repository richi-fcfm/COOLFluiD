#ifndef COOLFluiD_Physics_SA_EulerSAVarSet_hh
#define COOLFluiD_Physics_SA_EulerSAVarSet_hh

//////////////////////////////////////////////////////////////////////////////

#include "Framework/ConvectiveVarSet.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Physics {

    namespace SA {

//////////////////////////////////////////////////////////////////////////////

  /**
   * This class represents a convective var set for SA turbulence model
   *
   * @author Andrea Lani
   */
template <typename BASE, CFuint SGROUP>
class EulerSAVarSet : public BASE {
public: // classes

  /**
   * Constructor
   */
  EulerSAVarSet(Common::SafePtr<Framework::BaseTerm> term);
  
  /**
   * Default destructor
   */
  virtual ~EulerSAVarSet();

  /**
   * Set up the private data and give the maximum size of states physical
   * data to store
   */
  virtual void setup();
  
  /**
   * Get extra variable names
   */
  virtual std::vector<std::string> getExtraVarNames() const;
  
  /**
   * Gets the block separator for this variable set
   */
  virtual CFuint getBlockSeparator() const;
  
  /**
   * Get the speed
   */
  virtual CFreal getSpeed(const Framework::State& state) const;
  
  /**
   * Split the jacobian
   */
  virtual void splitJacobian(RealMatrix& jacobPlus,
			     RealMatrix& jacobMin,
			     RealVector& eValues,
			     const RealVector& normal);
  /**
   * Set the matrix of the right eigenvectors and the matrix of the eigenvalues
   */
  virtual void computeEigenValuesVectors(RealMatrix& rightEv,
					 RealMatrix& leftEv,
					 RealVector& eValues,
					 const RealVector& normal);
  
  /**
   * Give dimensional values to the adimensional state variables
   */
  virtual void setDimensionalValues(const Framework::State& state,
				    RealVector& result);
  
  /**
   * Give adimensional values to the dimensional state variables
   */
  virtual void setAdimensionalValues(const Framework::State& state,
				     RealVector& result);
  
  
  /**
   * Set other adimensional values for useful physical quantities
   */
  virtual void setDimensionalValuesPlusExtraValues
  (const Framework::State& state, RealVector& result, RealVector& extra);
  
  /// Compute the perturbed physical data
  virtual void computePerturbedPhysicalData(const Framework::State& state,
					    const RealVector& pdataBkp,
					    RealVector& pdata,
					    CFuint iVar);
  
  /**
   * Set the PhysicalData corresponding to the given State
   */
  virtual void computePhysicalData(const Framework::State& state,
				   RealVector& data);
  
  /**
   * Set a State starting from the given PhysicalData
   * @see EulerPhysicalModel
   */
  virtual void computeStateFromPhysicalData(const RealVector& data,
					    Framework::State& state);
  
private:
    
  /// start ID for the turbulent equations
  CFuint m_startNutil;
  
  /// temporary array
  RealVector m_tmpResult;
  
  /// physical data
  RealVector m_pdataNutil;

}; // end of class EulerSAVarSet

//////////////////////////////////////////////////////////////////////////////

    } // namespace SA

  } // namespace Physics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#include "EulerSAVarSet.ci"

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Physics_SA_EulerSAVarSet_hh
