#include "Framework/MethodStrategyProvider.hh"

#include "SpectralFV/SpectralFV.hh"
#include "SpectralFV/FaceDiffusiveFlux.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace SpectralFV {

//////////////////////////////////////////////////////////////////////////////

FaceDiffusiveFlux::FaceDiffusiveFlux(const std::string& name) :
  SpectralFVMethodStrategy(name),
  m_diffusiveVarSet(),
  m_maxNbrFlxPnts(),
  m_multiDiffFlux(),
  m_nbrEqs()
{
  CFAUTOTRACE;
}

//////////////////////////////////////////////////////////////////////////////

FaceDiffusiveFlux::~FaceDiffusiveFlux()
{
  CFAUTOTRACE;
}

//////////////////////////////////////////////////////////////////////////////

void FaceDiffusiveFlux::setup()
{
  CFAUTOTRACE;

  // get number of equations
  m_nbrEqs = Framework::PhysicalModelStack::getActive()->getNbEq();

  // get maximum number of flux points
  m_maxNbrFlxPnts = getMethodData().getMaxNbrRFluxPnts();

  // get the diffusive varset
  if (getMethodData().hasDiffTerm())
  {
    m_diffusiveVarSet = getMethodData().getDiffusiveVar();
  }

  m_multiDiffFlux.resize(m_maxNbrFlxPnts);
  for (CFuint iFlx = 0; iFlx < m_maxNbrFlxPnts; ++iFlx)
  {
    m_multiDiffFlux[iFlx].resize(m_nbrEqs);
  }
}

//////////////////////////////////////////////////////////////////////////////

void FaceDiffusiveFlux::unsetup()
{
  CFAUTOTRACE;
}

//////////////////////////////////////////////////////////////////////////////

  }  // namespace SpectralFV

}  // namespace COOLFluiD

