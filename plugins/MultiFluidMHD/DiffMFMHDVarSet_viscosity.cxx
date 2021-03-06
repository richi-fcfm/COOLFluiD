#include "DiffMFMHDVarSet.hh"
#include "EulerMFMHDTerm.hh"
#include "Framework/PhysicalConsts.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::MathTools;
using namespace COOLFluiD::Common;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Physics {

    namespace MultiFluidMHD {

//////////////////////////////////////////////////////////////////////////////
       
DiffMFMHDVarSet::DiffMFMHDVarSet(const std::string& name, 
				 SafePtr<PhysicalModelImpl> model) :
  DiffusiveVarSet(name, model),
  m_model(model->getDiffusiveTerm().d_castTo<DiffMFMHDTerm>()),
  m_eulerModel(model->getConvectiveTerm().d_castTo<PTERM>()),
  _twoThird(2./3.),
  _useBackUpValues(false),
  _setBackUpValues(false),
  _wallDistance(0.),
  _gradState(),
  _normal(),
  _heatFlux(),
  m_dynViscCoeff(),
  m_thermCondCoeff(),
  m_tau(),
  m_uID(),
  m_vID(),
  m_wID(),
  m_TID(),
  m_lambda()
{
}
  
//////////////////////////////////////////////////////////////////////////////
      
DiffMFMHDVarSet::~DiffMFMHDVarSet()
{
}
    
//////////////////////////////////////////////////////////////////////////////
   
void DiffMFMHDVarSet::setup()
{
  DiffusiveVarSet::setup();
  _gradState.resize(PhysicalModelStack::getActive()->getNbEq());
 
  const CFuint dim = PhysicalModelStack::getActive()->getDim();
  _normal.resize(dim);
  const CFuint nbSpecies = getModel().getNbSpecies(); 
  const CFuint endEM = 8;
  
  _heatFlux.resize(nbSpecies);
  m_tau.resize(nbSpecies);
  m_uID.resize(nbSpecies);
  m_vID.resize(nbSpecies);
  m_wID.resize(nbSpecies);
  m_TID.resize(nbSpecies);
  m_dynViscCoeff.resize(nbSpecies);
  m_currDynViscosity.resize(nbSpecies);
  m_lambda.resize(nbSpecies);
  if(getModel().isBraginskii() == true) {
    if(nbSpecies == 2){
      m_thermCondCoeff.resize(8);
    }
  }
  else{		// Default case with constant viscosity and thermal conductivity
    m_thermCondCoeff.resize(nbSpecies);
  }
  
  
  for (CFuint i = 0; i < nbSpecies; ++i) {
    m_uID[i] = endEM + nbSpecies + i*dim;
    m_vID[i] = endEM + nbSpecies + i*dim + 1;
    m_TID[i] = endEM + (dim + 1)*nbSpecies + i;
   
    if (dim == DIM_3D) {
      m_uID[i] = endEM + nbSpecies + i*dim;
      m_vID[i] = endEM + nbSpecies + i*dim + 1;
      m_wID[i] = endEM + nbSpecies + i*dim + 2;
      m_TID[i] = endEM + nbSpecies + nbSpecies*dim + i;
    }
    m_tau[i].resize(dim,dim);
  }
}

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHDVarSet::computeTransportProperties(const RealVector& state,
						 const std::vector<RealVector*>& gradients,
						 const RealVector& normal)
{
  RealVector& diffMFMHDData = getModel().getPhysicalData();
  const CFuint nbSpecies = getModel().getNbSpecies();
  const CFuint dim = PhysicalModelStack::getActive()->getDim();
      
  if (_useBackUpValues || _freezeDiffCoeff) {
    for (CFuint i = 0; i < nbSpecies; ++i){
      diffMFMHDData[i] = m_dynViscCoeff[i];
      diffMFMHDData[nbSpecies+i] = m_thermCondCoeff[i];
    }
  }
  else {		//this is the default case
    const RealVector& mu = getDynViscosityVec(state, gradients);
    
    for (CFuint i = 0; i < nbSpecies; ++i) {
      diffMFMHDData[i] = mu[i]; 						//reads the dimensional value imposed in the options but only takes the max
      diffMFMHDData[nbSpecies+i] = getThermConductivity(state, mu[i]);		//reads the dimensional value imposed in the options but only takes the max
    }
    
    if (_setBackUpValues) {
      for (CFuint i = 0; i < nbSpecies; ++i){
        m_dynViscCoeff[i] = diffMFMHDData[i];
        m_thermCondCoeff[i] = diffMFMHDData[nbSpecies+i];
      }
    }
  }
  //Implementation of Braginskii heat Flux in 2D
  if(getModel().isBraginskii() == true) {
    if (nbSpecies == 2){
      if (_useBackUpValues || _freezeDiffCoeff) {
        for (CFuint i = 0; i < nbSpecies; ++i){
          diffMFMHDData[i] = m_dynViscCoeff[i];
        }
        // AL: here to be commented properly
        for(CFuint i = 0; i < 8; i++){
          diffMFMHDData[nbSpecies + i] = m_thermCondCoeff[i];
        }
      } //End of freezeDiffCoeff
      else {		//this is the default case
        if(dim == DIM_2D){
          const RealVector& mu = getDynViscosityVec(state, gradients);
          for (CFuint i = 0; i < nbSpecies; ++i) {
            diffMFMHDData[i] = mu[i]; 						//reads the dimensional value imposed in the options
          }

          computeBraginskiiThermConduct(state);             // Compute the Braginskii coeffs.
          CFreal kappaParallel = _kappaParallel;
          CFreal kappaPerpendicular = _kappaPerpendicular;
          CFreal betaWedge = _betaWedge;
	  
          //unitary vector in B direction
          CFreal Bx = state[0];
          CFreal By = state[1];
          CFreal B2 = Bx*Bx + By*By;
          CFreal B = std::sqrt(B2);

          CFreal bx = 0;
          CFreal by = 0;
	  
          if (std::abs(B) > 1e-15){//controlling that one doesn't divide by zero
            bx = Bx/B;
            by = By/B;
          }
	  
          //matrix
          CFreal bxx = bx*bx;
          CFreal bxy = bx*by;
          CFreal byy = by*by;
	  
          const CFuint endVisc = 2;
          diffMFMHDData[endVisc + 0] = kappaParallel*bxx;
          diffMFMHDData[endVisc + 1] = kappaParallel*bxy;
          diffMFMHDData[endVisc + 2] = kappaParallel*byy;
	  
          diffMFMHDData[endVisc + 3] = kappaPerpendicular*(1-bxx);
          diffMFMHDData[endVisc + 4] = -kappaPerpendicular*bxy;
          diffMFMHDData[endVisc + 5] = kappaPerpendicular*(1-byy);
	  
          diffMFMHDData[endVisc + 6] = betaWedge;
	  
          diffMFMHDData[endVisc + 7] = getThermConductivity(state, mu[1]);		//neutrals' thermal conduction
        }
      }
      if (_setBackUpValues) {			//To be implenemted
        for (CFuint i = 0; i < nbSpecies; ++i){
          m_dynViscCoeff[i] = diffMFMHDData[i];
        }
        for(CFuint i = 0; i < 8; i++){
          m_thermCondCoeff[i] = diffMFMHDData[nbSpecies + i];
        }
      }
    } // End of if nbSpecies==2
  } // End of if Braginskii==true

  /// Implementation to increase the value of the viscosity in the extended domain
  //if(getModel().isExtendedDomain() == true){
    //const RealVector& mu_0 = getDynViscosityVec(state, gradients);
    //const RealVector& mu_f = getModel().getIncreasedDynViscosityDim();
    //const CFreal y_boundary = getModel().getTopHeight();
    //const CFreal y_0 = getModel().getDampingHeight();
    //const CFreal Delta_y = std::abs(y_0 - y_boundary)/5;   // It's taken 5 since tanh(5) is almost 1
    //const CFreal y_coord = (state).getCoordinates()[YY];
    //for (CFuint i = 0; i < nbSpecies; ++i) {
      //if(y_coord >= y_boundary){ //If the coordinate of the state is in the extended domain, it increases the viscosity
        //diffMFMHDData[i] =  0.5*mu_f[i]*std::tanh((y_coord - y_0)/Delta_y) + mu_0[i] - 0.5*mu_f[i]*std::tanh((y_boundary - y_0)/Delta_y); 						//reads the dimensional value imposed in the options
      //}
      //else{//If not, it uses the same viscosity
        //diffMFMHDData[i] = mu_0[i];
      //}
    //}
  //}
}

//////////////////////////////////////////////////////////////////////////////

void DiffMFMHDVarSet::computeStressTensor(const RealVector& state,
					     const vector<RealVector*>& gradients,
					     const CFreal& radius) 
{
 const CFuint dim = PhysicalModelStack::getActive()->getDim();
 const CFuint nbSpecies = getModel().getNbSpecies();   
 
 for (CFuint i = 0; i < nbSpecies; ++i) {
      CFreal divTerm = 0.0;
    if (dim == DIM_2D && radius > MathConsts::CFrealEps()) {
      // if the face is a boundary face, the radius could be 0
      // check against eps instead of 0. for safety
      divTerm = _gradState[m_vID[i]]/radius;
    }
    else if (dim == DIM_3D) {
      const RealVector& gradW = *gradients[m_wID[i]];
      divTerm = gradW[ZZ];
    }
  
    const RealVector& gradU = *gradients[m_uID[i]];
    const RealVector& gradV = *gradients[m_vID[i]];
    const CFreal twoThirdDivV = _twoThird*(gradU[XX] + gradV[YY] + divTerm);
    const RealVector& diffMFMHDData = getModel().getPhysicalData();
    const CFreal coeffTauMu = diffMFMHDData[i];    
//    cout <<"DiffMFMHDVarSet::computeStressTensor coeffTauMu --> = " << coeffTauMu <<"\n";
    
    m_tau[i](XX,XX) = coeffTauMu*(2.*gradU[XX] - twoThirdDivV);
    m_tau[i](XX,YY) = m_tau[i](YY,XX) = coeffTauMu*(gradU[YY] + gradV[XX]);
    m_tau[i](YY,YY) = coeffTauMu*(2.*gradV[YY] - twoThirdDivV);
  
    if (dim == DIM_3D) {
      const RealVector& gradW = *gradients[m_wID[i]];
      m_tau[i](XX,ZZ) = m_tau[i](ZZ,XX) = coeffTauMu*(gradU[ZZ] + gradW[XX]);
      m_tau[i](YY,ZZ) = m_tau[i](ZZ,YY) = coeffTauMu*(gradV[ZZ] + gradW[YY]);
      m_tau[i](ZZ,ZZ) = coeffTauMu*(2.*gradW[ZZ] - twoThirdDivV);
    }
 }
 
 for (CFuint i = 0; i < nbSpecies; ++i) {  
  CFreal divTerm = 0.0;
  if (dim == DIM_2D && radius > MathConsts::CFrealEps()) {
    // if the face is a boundary face, the radius could be 0
    // check against eps instead of 0. for safety
    divTerm = _gradState[m_vID[i]]/radius;
  }
  else if (dim == DIM_3D) {
    const RealVector& gradW = *gradients[m_wID[i]];
    divTerm = gradW[ZZ];
  }
  
  const RealVector& gradU = *gradients[m_uID[i]];
  const RealVector& gradV = *gradients[m_vID[i]];
  const CFreal twoThirdDivV = _twoThird*(gradU[XX] + gradV[YY] + divTerm);
  const RealVector& diffMFMHDData = getModel().getPhysicalData();
  const CFreal coeffTauMu = diffMFMHDData[i];
  m_tau[i](XX,XX) = coeffTauMu*(2.*gradU[XX] - twoThirdDivV);
  m_tau[i](XX,YY) = m_tau[i](YY,XX) = coeffTauMu*(gradU[YY] + gradV[XX]);
  m_tau[i](YY,YY) = coeffTauMu*(2.*gradV[YY] - twoThirdDivV);
  
  if (dim == DIM_3D) {
    const RealVector& gradW = *gradients[m_wID[i]];
    m_tau[i](XX,ZZ) = m_tau[i](ZZ,XX) = coeffTauMu*(gradU[ZZ] + gradW[XX]);
    m_tau[i](YY,ZZ) = m_tau[i](ZZ,YY) = coeffTauMu*(gradV[ZZ] + gradW[YY]);
    m_tau[i](ZZ,ZZ) = coeffTauMu*(2.*gradW[ZZ] - twoThirdDivV);
  }
 }
}
      
//////////////////////////////////////////////////////////////////////////////

RealVector& DiffMFMHDVarSet::getHeatFlux(const RealVector& state,
				    const vector<RealVector*>& gradients,
				    const RealVector& normal)
{
  const CFuint nbSpecies = getModel().getNbSpecies(); 
  const CFuint dim = PhysicalModelStack::getActive()->getDim();
  const CFreal mu = getDynViscosity(state, gradients);
  m_lambda = getThermConductivityVec(state, mu);
  
  //Model with Braginskii 
  if(getModel().isBraginskii() == true) {
    if (nbSpecies == 2){
      if(dim == DIM_2D){
	const RealVector& diffMFMHDData = getModel().getPhysicalData();
	const RealVector& gradTi = *gradients[m_TID[0]];
	const CFreal Tx = gradTi[XX];
	const CFreal Ty = gradTi[YY];
	
	const CFuint endVisc = 2;
	
	const CFreal TBx = ((diffMFMHDData[endVisc + 0] + diffMFMHDData[endVisc + 3])*Tx + (diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Ty);
	const CFreal TBy = ((diffMFMHDData[endVisc + 1] + diffMFMHDData[endVisc + 4])*Tx + (diffMFMHDData[endVisc + 2] + diffMFMHDData[endVisc + 5])*Ty);
      
	_heatFlux[0] = -TBx*normal[XX] - TBy*normal[YY];
	
	const RealVector& gradT = *gradients[m_TID[1]];
	_heatFlux[1] = (-m_lambda[1]*(MathFunctions::innerProd(gradT,normal)));
      }
      if(dim == DIM_3D){
        for (CFuint i = 0; i < nbSpecies; ++i) {
          std::cout<<"DiffMFMHDVarSet::getHeatFlux ==> Braginskii 3D not implemented";
        }
      }
    }
  }
  else {    
    for (CFuint i = 0; i < nbSpecies; ++i) {  
      const RealVector& gradT = *gradients[m_TID[i]];
      _heatFlux[i] = (-m_lambda[i]*(MathFunctions::innerProd(gradT,normal)));
    }
  } 
  return _heatFlux;
}
//////////////////////////////////////////////////////////////////////////////

void DiffMFMHDVarSet::computeBraginskiiThermConduct(const RealVector& state){
  
  //Implemented for a 2D case, to be extended to 3D in the future
  //Physical magnitudes in S.I. units
  CFreal Bx = state[0];
  CFreal By = state[1];
  CFreal Bz = state[2]; 
  CFreal Bnorm = std::sqrt(Bx*Bx + By*By + Bz*Bz);	//[T]
  _kappaParallel = 0;
  _kappaPerpendicular = 0;
  _betaWedge = 0; 

  const CFuint endEM = 8;
  const CFuint TiID = endEM + 6;
  const CFuint rhoiID = endEM;
  const CFuint rhonID = endEM + 1;
  const CFuint TnID = endEM + 7;
  CFreal T_i     = state[TiID];				//[K]
  CFreal rho_i   = state[rhoiID];			//[kg/m3]
  CFreal T_n     = state[TnID];				//[K]
  CFreal rho_n   = state[rhonID];			//[kg/m3]
  CFreal m_i     = m_eulerModel->getMolecularMass3();	//[kg]
  CFreal m_e     = m_eulerModel->getMolecularMass1();	//[kg]
  CFreal n_i     = rho_i/m_i;				//[m-3]
  CFreal n_n     = rho_n/m_n;				//[m-3]
  CFreal c       = m_eulerModel->getLightSpeed();	//[m/s]
  CFreal eCharge = Framework::PhysicalConsts::ElectronCharge();	//[C]
  CFreal kBoltz = 1.380648813e-16*11604.5052;				//[erg/eV]  
  
  //Conversion to cgs system
  Bnorm *= 1e4;						//[gauss]
  T_i *= 1/11604.5052;					//[erg]
  n_i *= 1e-6;						//[cm-3]
  c *= 100;						//[cm/s]
  eCharge *= c/10;					//[Fr]
  m_i *= 1000;						//[gr]
  m_e *= 1000;						//[gr]
  
  CFreal n_e = n_i; 
  CFreal T_e = T_i;
  
  //General parameters
  CFreal lambda = 23 - 0.5*std::log(n_i) + 1.5*std::log(T_i); /*23.4 - 1.15*std::log(n_i) + 3.45*std::log(T_i);*/
  const CFreal pi = MathTools::MathConsts::CFrealPi();
  //ion's parameters
  CFreal mu = 1; //m_i/m_p ion-proton mass ratio. Here ions are considered protons
  CFreal tau_i = 2.09e7*std::pow(T_i, 3/2)/(lambda*n_i)*mu;// NRL Plasma Formulary /*3*std::sqrt(m_i)*std::pow(T_i, 3/2)/(4*std::sqrt(pi)*lambda*std::pow(eCharge, 4)*n_i);*/
  CFreal omega_i = (eCharge*Bnorm)/(m_i*c);
  CFreal chi_i = omega_i*tau_i;
  CFreal Delta_i = std::pow(chi_i, 4) + 2.7*std::pow(chi_i, 2) + 0.677; 
  //electron's parameters
  CFreal tau_e = 3.44e5*std::pow(T_i, 3/2)/(lambda*n_i)*mu;// NRL Plasma Formulary 3*std::sqrt(m_e)*std::pow(T_e, 3/2)/(4*std::sqrt(2*pi)*lambda*std::pow(eCharge, 4)*n_e);
  CFreal omega_e = (eCharge*Bnorm)/(m_e*c);
  CFreal chi_e = omega_e*tau_e;
  CFreal Delta_e = std::pow(chi_e, 4) + 14.79*std::pow(chi_e, 2) + 3.7703;  
  
  _kappaParallel = (3.906*tau_i/m_i + 3.1616*tau_e/m_e)*n_i*kBoltz*kBoltz*T_i; 
  _kappaPerpendicular = (((2*std::pow(chi_i, 2)+ 2.645)/Delta_i)*(tau_i/m_i) + ((4.664*std::pow(chi_e, 2)+ 11.92)/Delta_e)*(tau_e/m_e))*n_i*kBoltz*kBoltz*T_i;
  _betaWedge = (chi_e*(3/2*std::pow(chi_e, 2) + 3.053)/Delta_e)*n_e*kBoltz*kBoltz*T_e;

  //Conversion into S.I. units
  _kappaParallel *= 1/(1e5);//sure about this
  _kappaPerpendicular *= 1/(1e5);//sure about this
  _betaWedge *= 1/(1e5); //Not sure about this

  //Conversion to cgs system
  Bnorm *= 1e4;						//[gauss]
  T_i *= 1/11604.5052;					//[erg]
  n_i *= 1e-6;						//[cm-3]
  c *= 100;						//[cm/s]
  eCharge *= c/10;					//[Fr]
  m_i *= 1000;						//[gr]
  m_e *= 1000;						//[gr]

// adding the calculation for the dynamical viscosity:
//YM:
  
  CFreal dyn_visc_i = 0.96*n_i*kBoltz*T_i*tau_i;
  CFreal dyn_visc_e = 0.733*n_e*kBoltz*T_e*tau_e;
  CFreal dyn_visc_p = dyn_visc_i + dyn_visc_e; //plasma viscosity due to electron ion collisions
  CFconst Sigma_en  = 1e-18;
  CFconst Sigma_in  = 1.16e-18;
  CFreal m_en       = m_e*m_n/(m_e+m_n);
  CFreal m_in       = m_i*m_n/(m_i+m_n);
  CFreal T_en       = (T_e + T_n)/2;
  CFreal T_in       = (T_i + T_n)/2;
  CFreal nu_en      = n_n*Sigma_en*std::sqrt(8.*kBoltz*T_en/(pi*m_en));
  CFreal nu_in      = n_n*Sigma_in*std::sqrt(8.*kBoltz*T_in/(pi*m_in));
  CFreal dyn_visc_n = 0.96*n_n*kBoltz*T_n*nu_en + 0.96*n_n*kBoltz*T_n*nu_in;

 //Conversion to cgs system

  dyn_visc_p *= 1e-3; // to convert from [g/(cm.s)] to [kg/(m.s)]
  dyn_visc_n *= 1e-3;

// CFreal epsilon0 = Framework::PhysicalConsts::VacuumPermittivity();
// CFreal r_debye = std::pow(eCharge, 2)/(4*pi*epsilon0*kBoltz*T_i);
// CFreal Sigma_ei   = lambda*pi*std::pow(r_debye, 2);
// CFconst Sigma_en  = 1e-18;
// CFconst Sigma_in  = 1.16e-18;
// CFreal m_ei       = m_e*m_i/(m_e+m_i);
// CFreal m_en       = m_e*m_n/(m_e+m_n);
// CFreal m_in       = m_i*m_n/(m_i+m_n);
// CFreal T_ei       = T_i;
// CFreal T_en       = (T_e + T_n)/2;
// CFreal T_in       = (T_i + T_n)/2;
// CFreal nu_ei      = n_i*Sigma_ei*std::sqrt(8.*kBoltz*T_ei/(pi*m_ei));
// CFreal nu_en      = n_n*Sigma_en*std::sqrt(8.*kBoltz*T_en/(pi*m_en));
// CFreal nu_in      = n_n*Sigma_in*std::sqrt(8.*kBoltz*T_in/(pi*m_in));
// CFreal dyn_visc_i = 0.96*n_i*(8.621738e-11)*kBoltz*T_i*nu_ei;
// CFreal dyn_visc_n = 0.73*n_n*T_i*nu_nn; //neutral viscosity is the sum of the ions and the electrons
  
//   
//   std::cout<<"\n";
//   std::cout<<"Ion's Properties in cgs \n";
//   std::cout<<"lambda  = " << lambda <<"\n";
//   std::cout<<"\t log(n_i)  = " << std::log(n_i) <<"\n";
//   std::cout<<"\t log(T_i)  = " << std::log(T_i) <<"\n";
//   std::cout<<"tau_i   = " << tau_i <<"\n";
//   std::cout<<"omega_i = " << omega_i <<"\n";
//   std::cout<<"chi_i   = " << chi_i <<"\n";
//   std::cout<<"Delta_i = " << Delta_i <<"\n";
//   
//   std::cout<<"\n";
//   std::cout<<"Electron's Properties in cgs \n";
//   std::cout<<"tau_e   = " << tau_e <<"\n";
//   std::cout<<"omega_e = " << omega_e <<"\n";
//   std::cout<<"chi_e   = " << chi_e <<"\n";
//   std::cout<<"Delta_e = " << Delta_e <<"\n";
 
  //std::cout<<"kappaParallel[W/mK] = "<< _kappaParallel <<"\n";
  //std::cout <<"DiffMFMHDVarSet::computeBraginskiiThermConduct => _kappaParallel      =" << _kappaParallel <<"\n";
  //std::cout <<"DiffMFMHDVarSet::computeBraginskiiThermConduct => _kappaPerpendicular =" << _kappaPerpendicular <<"\n";
  //std::cout <<"DiffMFMHDVarSet::computeBraginskiiThermConduct => _betaWedge          =" << _betaWedge <<"\n";
  //This finction should be debugged
//   _kappaParallel = 0.;
//   _kappaPerpendicular = 0.;
//   _betaWedge = 0.;
//   bool stop = true;
//   cf_assert(stop == false);
  
}

//////////////////////////////////////////////////////////////////////////////

    } // namespace NavierStokes

  } // namespace Physics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
