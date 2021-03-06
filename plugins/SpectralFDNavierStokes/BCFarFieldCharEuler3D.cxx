#include "Framework/MethodStrategyProvider.hh"

#include "NavierStokes/Euler3DVarSet.hh"
#include "NavierStokes/EulerTerm.hh"

#include "SpectralFDNavierStokes/SpectralFDNavierStokes.hh"
#include "SpectralFDNavierStokes/BCFarFieldCharEuler3D.hh"

#include "Common/NotImplementedException.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::Physics::NavierStokes;
using namespace COOLFluiD::Common;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace SpectralFD {

//////////////////////////////////////////////////////////////////////////////

Framework::MethodStrategyProvider<
    BCFarFieldCharEuler3D,SpectralFDMethodData,BCStateComputer,SpectralFDNavierStokesModule >
  BCFarFieldCharEuler3DProvider("FarFieldCharEuler3D");

//////////////////////////////////////////////////////////////////////////////

void BCFarFieldCharEuler3D::defineConfigOptions(Config::OptionList& options)
{
  options.addConfigOption< CFreal >("Uinf","x velocity");
  options.addConfigOption< CFreal >("Vinf","y velocity");
  options.addConfigOption< CFreal >("Winf","z velocity");
  options.addConfigOption< CFreal >("Pinf","static pressure");
  options.addConfigOption< CFreal >("Tinf","static temperature");
}

//////////////////////////////////////////////////////////////////////////////

BCFarFieldCharEuler3D::BCFarFieldCharEuler3D(const std::string& name) :
  BCStateComputer(name),
  m_eulerVarSet(CFNULL),
  m_ghostSolPhysData(),
  m_intSolPhysData(),
  m_tempInf(),
  m_presInf(),
  m_uInf(),
  m_vInf(),
  m_wInf(),
  m_rhoInf(),
  m_cInf()
{
  CFAUTOTRACE;

  addConfigOptionsTo(this);

  m_tempInf = 0.0;
  setParameter("Tinf",&m_tempInf);

  m_presInf = 0.0;
  setParameter("Pinf",&m_presInf);

  m_uInf = 0.0;
  setParameter("Uinf",&m_uInf);

  m_vInf = 0.0;
  setParameter("Vinf",&m_vInf);

  m_wInf = 0.0;
  setParameter("Winf",&m_wInf);
}

//////////////////////////////////////////////////////////////////////////////

BCFarFieldCharEuler3D::~BCFarFieldCharEuler3D()
{
  CFAUTOTRACE;
}

//////////////////////////////////////////////////////////////////////////////

void BCFarFieldCharEuler3D::computeGhostStates(const vector< State* >& intStates,
                                               vector< State* >& ghostStates,
                                               const std::vector< RealVector >& normals,
                                               const std::vector< RealVector >& coords)
{
  // number of states
  const CFuint nbrStates = ghostStates.size();
  cf_assert(nbrStates == intStates.size());
  cf_assert(nbrStates == normals.size());

  // get some physical data from the model
  const CFreal gamma = m_eulerVarSet->getModel()->getGamma();
  const CFreal gammaMinus1 = gamma - 1.0;
  const CFreal gammaDivGammaMinus1 = gamma/gammaMinus1;

  // loop over the states
  for (CFuint iState = 0; iState < nbrStates; ++iState)
  {
    // normal
    const RealVector& normal = normals[iState];

    // dereference states
    State& intState   = (*intStates  [iState]);
    State& ghostState = (*ghostStates[iState]);

    cf_assert(intState  .size() == 5);
    cf_assert(ghostState.size() == 5);

    // set the physical data starting from the inner state
    m_eulerVarSet->computePhysicalData(intState,m_intSolPhysData);

    // some auxiliary variables
    const CFreal rho = m_intSolPhysData[EulerTerm::RHO];
    const CFreal p   = m_intSolPhysData[EulerTerm::P];
    const CFreal u   = m_intSolPhysData[EulerTerm::VX];
    const CFreal v   = m_intSolPhysData[EulerTerm::VY];
    const CFreal w   = m_intSolPhysData[EulerTerm::VZ];
    const CFreal vn  = u*normal[XX] + v*normal[YY] + w*normal[ZZ];
    const CFreal c   = m_intSolPhysData[EulerTerm::A];
    const CFreal mach = vn/c;

    // depending on the sign and magnitude of the local Mach number,
    // number of variables to be specified are determined

    // supersonic outlet case
    if (mach >= 1.0)
    {
      ghostState[0] = intState[0];
      ghostState[1] = intState[1];
      ghostState[2] = intState[2];
      ghostState[3] = intState[3];
      ghostState[4] = intState[4];
    }
    // supersonic inlet case
    else if (mach <= -1.0)
    {
      // set all the physical data corresponding to the ghost state
      m_ghostSolPhysData[EulerTerm::RHO] = 2.0*m_rhoInf  - rho;
      m_ghostSolPhysData[EulerTerm::VX]  = 2.0*m_uInf    - u;
      m_ghostSolPhysData[EulerTerm::VY]  = 2.0*m_vInf    - v;
      m_ghostSolPhysData[EulerTerm::VZ]  = 2.0*m_wInf    - w;
      m_ghostSolPhysData[EulerTerm::P]   = 2.0*m_presInf - p;
      const CFreal vSq = m_ghostSolPhysData[EulerTerm::VX]*m_ghostSolPhysData[EulerTerm::VX]+
                         m_ghostSolPhysData[EulerTerm::VY]*m_ghostSolPhysData[EulerTerm::VY]+
                         m_ghostSolPhysData[EulerTerm::VZ]*m_ghostSolPhysData[EulerTerm::VZ];
      m_ghostSolPhysData[EulerTerm::H] = (gammaDivGammaMinus1*m_ghostSolPhysData[EulerTerm::P]
                                            + 0.5*m_ghostSolPhysData[EulerTerm::RHO]*vSq
                                         )/m_ghostSolPhysData[EulerTerm::RHO];

      // set the ghost state starting from the physical data
      m_eulerVarSet->computeStateFromPhysicalData(m_ghostSolPhysData,ghostState);
    }
    else
    {
      // far field data
      const CFreal vnInf = m_uInf*normal[XX] + m_vInf*normal[YY] + m_wInf*normal[ZZ];

      // Riemann invariants
      const CFreal rPlus_n = vn     + 2.*c     /gammaMinus1;
      const CFreal rMin_n  = vnInf  - 2.*m_cInf/gammaMinus1;
      CFreal rMinRMax = 0.5*(rPlus_n + rMin_n);

      // subsonic outlet case
      if (rMinRMax > 0.)
      {
        const CFreal rMinRMaxMinusVn = rMinRMax - vn;
        const CFreal ubnd = u + rMinRMaxMinusVn*normal[XX];
        const CFreal vbnd = v + rMinRMaxMinusVn*normal[YY];
        const CFreal wbnd = w + rMinRMaxMinusVn*normal[ZZ];

        m_ghostSolPhysData[EulerTerm::RHO] = rho;
        m_ghostSolPhysData[EulerTerm::P]   = p;
        m_ghostSolPhysData[EulerTerm::VX]  = 2.0*ubnd - u;
        m_ghostSolPhysData[EulerTerm::VY]  = 2.0*vbnd - v;
        m_ghostSolPhysData[EulerTerm::VZ]  = 2.0*wbnd - w;
      }
      // subsonic inlet case
      else
      {
        const CFreal rMinRMaxMinusVn = rMinRMax - vnInf;
        const CFreal ubnd = m_uInf + rMinRMaxMinusVn*normal[XX];
        const CFreal vbnd = m_vInf + rMinRMaxMinusVn*normal[YY];
        const CFreal wbnd = m_wInf + rMinRMaxMinusVn*normal[ZZ];

        m_ghostSolPhysData[EulerTerm::RHO] = m_rhoInf;
        m_ghostSolPhysData[EulerTerm::P]   = m_presInf;
        m_ghostSolPhysData[EulerTerm::VX]  = 2.0*ubnd - u;
        m_ghostSolPhysData[EulerTerm::VY]  = 2.0*vbnd - v;
        m_ghostSolPhysData[EulerTerm::VZ]  = 2.0*wbnd - w;
      }
      const CFreal vSq = m_ghostSolPhysData[EulerTerm::VX]*m_ghostSolPhysData[EulerTerm::VX]+
                         m_ghostSolPhysData[EulerTerm::VY]*m_ghostSolPhysData[EulerTerm::VY]+
                         m_ghostSolPhysData[EulerTerm::VZ]*m_ghostSolPhysData[EulerTerm::VZ];
      m_ghostSolPhysData[EulerTerm::H] = (gammaDivGammaMinus1*m_ghostSolPhysData[EulerTerm::P]
                                            + 0.5*m_ghostSolPhysData[EulerTerm::RHO]*vSq
                                         )/m_ghostSolPhysData[EulerTerm::RHO];

      // set the ghost state starting from the physical data
      m_eulerVarSet->computeStateFromPhysicalData(m_ghostSolPhysData,ghostState);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void BCFarFieldCharEuler3D::computeGhostGradients(const std::vector< std::vector< RealVector* > >& intGrads,
                                                  std::vector< std::vector< RealVector* > >& ghostGrads,
                                                  const std::vector< RealVector >& normals,
                                                  const std::vector< RealVector >& coords)
{
  // number of state gradients
  const CFuint nbrStateGrads = intGrads.size();
  cf_assert(nbrStateGrads == ghostGrads.size());
  cf_assert(nbrStateGrads == normals.size());

  // number of gradient variables
  cf_assert(nbrStateGrads > 0);
  const CFuint nbrGradVars = intGrads[0].size();

  // set the ghost gradients
  for (CFuint iState = 0; iState < nbrStateGrads; ++iState)
  {
    for (CFuint iGradVar = 0; iGradVar < nbrGradVars; ++iGradVar)
    {
      *ghostGrads[iState][iGradVar] = *intGrads[iState][iGradVar];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void BCFarFieldCharEuler3D::setup()
{
  CFAUTOTRACE;

  // setup of the parent class
  BCStateComputer::setup();

  // no flux point coordinates required
  m_needsSpatCoord = false;

  // get Euler 3D varset
  m_eulerVarSet = getMethodData().getUpdateVar().d_castTo<Euler3DVarSet>();
  if (m_eulerVarSet.isNull())
  {
    throw Common::ShouldNotBeHereException (FromHere(),"Update variable set is not Euler3DVarSet in BCFarFieldCharEuler3D!");
  }

  // resize the physical data for internal and ghost solution points
  m_eulerVarSet->getModel()->resizePhysicalData(m_ghostSolPhysData);
  m_eulerVarSet->getModel()->resizePhysicalData(m_intSolPhysData  );

  // non-dimensionalize pressure and temperature
  m_presInf /= m_eulerVarSet->getModel()->getPressRef();
  m_tempInf /= m_eulerVarSet->getModel()->getTempRef ();
  m_uInf    /= m_eulerVarSet->getModel()->getVelRef  ();
  m_vInf    /= m_eulerVarSet->getModel()->getVelRef  ();

  // compute density and speed of sound
  m_rhoInf = m_presInf/m_eulerVarSet->getModel()->getR()/m_tempInf;
  m_cInf   = sqrt(m_eulerVarSet->getModel()->getGamma()*m_eulerVarSet->getModel()->getR()*m_tempInf);
}

//////////////////////////////////////////////////////////////////////////////

  }  // namespace SpectralFD

}  // namespace COOLFluiD

