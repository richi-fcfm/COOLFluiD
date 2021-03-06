#include "DiffMFMHD3DVarSet.hh"
#include "EulerMFMHDTerm.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::MathTools;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Physics {

    namespace MultiFluidMHD {

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHD3DVarSet::setup()
{
  DiffMFMHDVarSet::setup();
}

//////////////////////////////////////////////////////////////////////////////

RealMatrix& DiffMFMHD3DVarSet::getFlux(const RealVector& state,
				       const vector<RealVector*>& gradients,
				       const CFreal& radius)
{
  setGradientState(state);
  // dummy normal here
  computeTransportProperties(state, gradients, _normal);
  computeStressTensor(state, gradients, radius);

  const CFuint endEM = 8;
  for (CFuint i = 0; i < endEM; ++i){
    _fluxVec(i,XX) = _fluxVec(i,YY) = _fluxVec(i,ZZ) = 0.;
  }
  
  const CFuint nbSpecies = getModel().getNbSpecies();    
  for (CFuint i = 0; i < nbSpecies; ++i) {
    const RealVector& gradT = *gradients[m_TID[i]];
    const CFreal avU = _gradState[m_uID[i]];
    const CFreal avV = _gradState[m_vID[i]];
    const CFreal avW = _gradState[m_wID[i]];
    const CFreal tauXX = m_tau[i](XX, XX);
    const CFreal tauXY = m_tau[i](XX, YY);
    const CFreal tauXZ = m_tau[i](XX, ZZ);
    const CFreal tauYY = m_tau[i](YY, YY);
    const CFreal tauYZ = m_tau[i](YY, ZZ);
    const CFreal tauZZ = m_tau[i](ZZ, ZZ);

    if(getModel().isBraginskii()) {
      computeHeatFluxBraginskii(gradients, i);
    }
    else if (getModel().isSolarTransport1F()) {
      computeHeatFluxSolar1F(gradients, i);
    }
    else {
      computeHeatFluxScalar(gradients, i);
    }
    
    _fluxVec(m_uID[i],XX) = tauXX;
    _fluxVec(m_uID[i],YY) = tauXY;
    _fluxVec(m_uID[i],ZZ) = tauXZ;
    
    _fluxVec(m_vID[i],XX) = tauXY;
    _fluxVec(m_vID[i],YY) = tauYY;
    _fluxVec(m_vID[i],ZZ) = tauYZ;
    
    _fluxVec(m_wID[i],XX) = tauXZ;
    _fluxVec(m_wID[i],YY) = tauYZ;
    _fluxVec(m_wID[i],ZZ) = tauZZ;
    
    _fluxVec(m_TID[i],XX) = tauXX*avU + tauXY*avV + tauXZ*avW - _qFluxVect[i][XX];
    _fluxVec(m_TID[i],YY) = tauXY*avU + tauYY*avV + tauYZ*avW - _qFluxVect[i][YY];
    _fluxVec(m_TID[i],ZZ) = tauXZ*avU + tauYZ*avV + tauZZ*avW - _qFluxVect[i][ZZ];
  }
  
  return _fluxVec;
}

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHD3DVarSet::getAxiSourceTerm(const RealVector& physicalData,
					 const RealVector& state,
					 const vector<RealVector*>& gradients,
					 const CFreal& radius,
					 RealVector& source)
{
  throw Common::NotImplementedException (FromHere(),"DiffMFMHD3DVarSet::getAxiSourceTerm()");
}

//////////////////////////////////////////////////////////////////////////////

RealVector& DiffMFMHD3DVarSet::getFlux(const RealVector& state,
				       const vector<RealVector*>& gradients,
				       const RealVector& normal,
				       const CFreal& radius)
{  
  setGradientState(state);
  computeTransportProperties(state, gradients, normal);
  computeStressTensor(state, gradients, radius);
  
  const CFuint endEM = 8;
  for (CFuint i = 0; i < endEM; ++i) {_flux[i] = 0;}

  const CFuint nbSpecies = getModel().getNbSpecies();  
  for (CFuint i = 0; i < nbSpecies; ++i) {
    _flux[m_uID[i]] =
      m_tau[i](XX,XX)*normal[XX] + m_tau[i](XX,YY)*normal[YY] + m_tau[i](XX,ZZ)*normal[ZZ];
    _flux[m_vID[i]] =
      m_tau[i](YY,XX)*normal[XX] + m_tau[i](YY,YY)*normal[YY] + m_tau[i](YY,ZZ)*normal[ZZ];
    _flux[m_wID[i]] =
      m_tau[i](ZZ,XX)*normal[XX] + m_tau[i](YY,ZZ)*normal[YY] + m_tau[i](ZZ,ZZ)*normal[ZZ];
    
    if (getModel().isBraginskii()) {
      computeHeatFluxBraginskii(gradients, i);
    }
    else if (getModel().isSolarTransport1F()) {
      computeHeatFluxSolar1F(gradients, i);
    }
    else {
      computeHeatFluxScalar(gradients, i);
    }
    _qFlux[i] = MathFunctions::innerProd(_qFluxVect[i], normal);
    
    _flux[m_TID[i]] =
      (m_tau[i](XX,XX)*_gradState[m_uID[i]] + m_tau[i](XX,YY)*_gradState[m_vID[i]] +
       m_tau[i](XX,ZZ)*_gradState[m_wID[i]])*normal[XX] +
      (m_tau[i](XX,YY)*_gradState[m_uID[i]] + m_tau[i](YY,YY)*_gradState[m_vID[i]] +
       m_tau[i](YY,ZZ)*_gradState[m_wID[i]])*normal[YY] +
      (m_tau[i](XX,ZZ)*_gradState[m_uID[i]] + m_tau[i](YY,ZZ)*_gradState[m_vID[i]] +
       m_tau[i](ZZ,ZZ)*_gradState[m_wID[i]])*normal[ZZ]- _qFlux[i];
  }
  
  return _flux;
}

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHD3DVarSet::computeHeatFluxBraginskii(const vector<RealVector*>& gradients,
						  const CFuint i)
{
  const CFuint nbSpecies = getModel().getNbSpecies();  
  if (nbSpecies == 2){ // Model with plasma + neutrals
    const RealVector& diffMFMHDData = getModel().getPhysicalData();
    const CFuint endVisc = 2;
    const RealVector& gradTi = *gradients[m_TID[i]];

    if(i == 0) {
      const CFreal Tx = gradTi[XX];
      const CFreal Ty = gradTi[YY];
      const CFreal Tz = gradTi[ZZ];
      
      // AL: the following needs to be extended properly to 3D, is even the 2D correct?
      _qFluxVect[0][XX] = -((diffMFMHDData[endVisc + 0] + diffMFMHDData[endVisc + 3])*Tx + (diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Ty);
      _qFluxVect[0][YY] = -((diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Tx + (diffMFMHDData[endVisc + 2] + diffMFMHDData[endVisc + 5])*Ty);
      _qFluxVect[0][ZZ] = 0.; // missing implementation
    }
    else { //neutrals
      _qFluxVect[i] = -diffMFMHDData[endVisc + 7]*gradTi;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHD3DVarSet::computeHeatFluxSolar1F(const vector<RealVector*>& gradients,
					       const CFuint i)
{
 
  const CFuint nbSpecies = getModel().getNbSpecies();  
  cf_assert(nbSpecies == 1);
  cf_assert(i == 0);
  
  const RealVector& diffMFMHDData = getModel().getPhysicalData();
  const CFuint endVisc = 1;
  const RealVector& gradTi = *gradients[m_TID[i]];
  const CFreal Tx = gradTi[XX];
  const CFreal Ty = gradTi[YY];
  const CFreal Tz = gradTi[ZZ];
  
  //--------------------------------------------------------//
  
  // Peter: here is your part to fill in !!!!
  // AL: the following needs to be extended properly to 3D, is even the 2D correct?

  //_qFluxVect[0][XX] = - comm_fact*(Bx*Bx*Tx + Bx*By*Ty + Bx*Bz*Tz);
  _qFluxVect[0][XX] = -(diffMFMHDData[endVisc+0]*Tx + diffMFMHDData[endVisc+1]*Ty + diffMFMHDData[endVisc+2]*Tz);
  //_qFluxVect[0][YY] = - comm_fact*(By*Bx*Tx + By*By*Ty + By*Bz*Tz);
  _qFluxVect[0][YY] = -(diffMFMHDData[endVisc+3]*Tx + diffMFMHDData[endVisc+4]*Ty + diffMFMHDData[endVisc+5]*Tz);
  //_qFluxVect[0][ZZ] = - comm_fact*(Bz*Bx*Tx + Bz*By*Ty + Bz*Bz*Tz);
  _qFluxVect[0][ZZ] = -(diffMFMHDData[endVisc+6]*Tx + diffMFMHDData[endVisc+7]*Ty + diffMFMHDData[endVisc+8]*Tz);


  // AL: the following needs to be extended properly to 3D, is even the 2D correct?
  //_qFluxVect[0][XX] = -((diffMFMHDData[endVisc + 0] + diffMFMHDData[endVisc + 3])*Tx + (diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Ty);
  //_qFluxVect[0][YY] = -((diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Tx + (diffMFMHDData[endVisc + 2] + diffMFMHDData[endVisc + 5])*Ty);
  //_qFluxVect[0][ZZ] = 0.; // missing implementation
}

//////////////////////////////////////////////////////////////////////////////

} // namespace MultiFluidMHD

  } // namespace Physics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
