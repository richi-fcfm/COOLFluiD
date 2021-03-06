#ifndef COOLFluiD_FluxReconstructionMethod_SA2DSourceTerm_hh
#define COOLFluiD_FluxReconstructionMethod_SA2DSourceTerm_hh

//////////////////////////////////////////////////////////////////////////////

#include "FluxReconstructionMethod/StdSourceTerm.hh"
#include "SA/NavierStokesSAVarSetTypes.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
  
  namespace Physics {
    namespace NavierStokes {
      class Euler2DVarSet;
    }
  }

  namespace FluxReconstructionMethod {

//////////////////////////////////////////////////////////////////////////////

/**
 * A base command for adding the source term for 2D Spalart Allmaras
 *
 * @author Ray Vandenhoeck
 */
class SA2DSourceTerm : public StdSourceTerm {
public:

  /**
   * Defines the Config Option's of this class
   * @param options a OptionList where to add the Option's
   */
  static void defineConfigOptions(Config::OptionList& options);

  /**
   * Constructor.
   */
  explicit SA2DSourceTerm(const std::string& name);

  /**
   * Destructor.
   */
  ~SA2DSourceTerm();

  /**
   * Set up the member data
   */
  virtual void setup();

  /**
   * UnSet up private data and data of the aggregated classes
   * in this command after processing phase
   */
  virtual void unsetup();

  /**
   * Returns the DataSocket's that this command needs as sinks
   * @return a vector of SafePtr with the DataSockets
   */
  std::vector< Common::SafePtr< Framework::BaseDataSocketSink > >
      needsSockets();
  
  /// Returns the DataSocket's that this command provides as sources
  /// @return a vector of SafePtr with the DataSockets
  virtual std::vector< Common::SafePtr< Framework::BaseDataSocketSource > >
    providesSockets();

protected:

  /**
   * get data required for source term computation
   */
  void getSourceTermData();

  /**
   * add the source term
   */
  void addSourceTerm(RealVector& resUpdates);

  /**
   * Configures the command.
   */
  virtual void configure ( Config::ConfigArgs& args );

protected: // data
  
  /// socket for gradients
  Framework::DataSocketSink< std::vector< RealVector > > socket_gradients;
  
  /// handle to the wall distance
  Framework::DataSocketSink<CFreal> socket_wallDistance;
  
  /// storage for the wall shear stress velocity
  Framework::DataSocketSource<CFreal> socket_wallShearStressVelocity;

  /// the source term for one state
  RealVector m_srcTerm;

  /// dimensionality
  CFuint m_dim;
  
  /// physical model (in conservative variables)
  Common::SafePtr<Physics::NavierStokes::Euler2DVarSet> m_eulerVarSet;
  
  /// corresponding diffusive variable set
  Common::SafePtr<Physics::SA::NavierStokes2DSA> m_diffVarSet;
  
  /// variable for physical data of sol
  RealVector m_solPhysData;
  
  ///Dummy vector for the gradients
  std::vector<RealVector*> m_dummyGradients;
  
  /// the gradients in the current cell
  std::vector< std::vector< RealVector* > > m_cellGrads;
  
  /// wall distance in current sol point
  RealVector m_currWallDist; 
  
  /// boolean telling whether to add the compressibility correction term
  bool m_compTerm;
  
}; // class SA2DSourceTerm

//////////////////////////////////////////////////////////////////////////////

    } // namespace FluxReconstructionMethod

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_FluxReconstructionMethod_SA2DSourceTerm_hh

