#include "FiniteVolume/FiniteVolume.hh"
#include "SuperInletProjection.hh"
#include "Framework/MethodCommandProvider.hh"
#include "Framework/MethodCommandProvider.hh"
#include "Framework/MapGeoToTrsAndIdx.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::Common;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Numerics {

    namespace FiniteVolume {

//////////////////////////////////////////////////////////////////////////////

MethodCommandProvider<SuperInletProjection, CellCenterFVMData, FiniteVolumeModule> 
superInletProjectionFVMCCProvider("SuperInletProjectionFVMCC");

//////////////////////////////////////////////////////////////////////////////

void SuperInletProjection::defineConfigOptions(Config::OptionList& options)
{
  options.addConfigOption< CFreal >("refPhi","Reference phi value imposed at the supersonic inlet.");
  options.addConfigOption< vector<CFuint> >("ProjectionIDs","IDs corresponding to projection fields.");
  options.addConfigOption< CFint >("inletCoronalBC","Switch to coronal inlet BC.");
  options.addConfigOption< CFint >("Phi_divB_zero","Set Phi to zero at the ghost state.");
  options.addConfigOption< CFint >("Phi_divB_extrapolated","Copy Phi from the closest inner state to the ghost cell.");
  options.addConfigOption< CFint >("JensVelocityBC","Set velocity boundary conditions according to Pomoell 2012.");
  options.addConfigOption< CFint >("BarbarasVelocityBC","Set velocity boundary conditions according to Zanni and Ferreira 2009.");
  options.addConfigOption< CFint >("hydrodynamic_limit","Set velocity boundary conditions for the hydrodynamic limit case.");
  options.addConfigOption< CFint >("DanasVelocityBC","Set velocity boundary conditions according to Dana.");
  options.addConfigOption< CFint >("DifferentialRotation","Enable differential rotation in the azimuth momentum");
  options.addConfigOption< CFint >("JensBfieldBC","Fix Br and extrapolate Btheta and Bphi.");
  options.addConfigOption< CFint >("DanasBfieldBC","Fix Br and extrapolate Btheta*pow(r,alpha) and Bphi.");
  options.addConfigOption< CFint >("JonLinkersBfieldSuggestion","Freeze B_PFSS at the inlet.");
  options.addConfigOption< CFint >("pressure_fixed","Dirichlet condition for P");
  options.addConfigOption< CFint >("pressure_Neumann","Neumann condition for P");
  options.addConfigOption< CFint >("JensRhoIni","Dirichlet condition for Jens' grav.strat. initial condition for rho");
  options.addConfigOption< CFint >("JensPIni","Dirichlet condition for Jens' grav.strat. initial condition for P");
  options.addConfigOption< vector<CFuint> >("VarIDs","IDs of the variables from which values are read by file");
}
      
//////////////////////////////////////////////////////////////////////////////

SuperInletProjection::SuperInletProjection(const std::string& name) :
  SuperInlet(name),
  m_mapGeoToTrs(),
  m_mapTrs2Twall(),
  m_BfromFile(false)
{
  addConfigOptionsTo(this);

  _refPhi = 0.;
  setParameter("refPhi",&_refPhi);
  
  setParameter("inletCoronalBC",&_inletCoronalBC);
  setParameter("Phi_divB_zero",&_Phi_divB_zero);
  setParameter("Phi_divB_extrapolated",&_Phi_divB_extrapolated);
  setParameter("JensVelocityBC",&_JensVelocityBC);
  setParameter("BarbarasVelocityBC",&_BarbarasVelocityBC);
  setParameter("hydrodynamic_limit",&_hydrodynamic_limit);
  setParameter("DanasVelocityBC",&_DanasVelocityBC);
  setParameter("DifferentialRotation",&_DifferentialRotation);
  setParameter("JensBfieldBC",&_JensBfieldBC);
  setParameter("DanasBfieldBC",&_DanasBfieldBC);
  setParameter("JonLinkersBfieldSuggestion",&_JonLinkersBfieldSuggestion);
  setParameter("pressure_fixed",&_pressure_fixed);
  setParameter("pressure_Neumann",&_pressure_Neumann);
  setParameter("JensRhoIni",&_JensRhoIni);
  setParameter("JensPIni",&_JensPIni);
  
  _projectionIDs = vector<CFuint>();
  setParameter("ProjectionIDs",&_projectionIDs);
  m_varIDs = vector<CFuint>();
  setParameter("VarIDs",&m_varIDs);
}

//////////////////////////////////////////////////////////////////////////////

SuperInletProjection::~SuperInletProjection()
{
}

//////////////////////////////////////////////////////////////////////////////

void SuperInletProjection::setup()
{
  SuperInlet::setup();

  if (m_varIDs.size() > 0) {m_BfromFile = true;}
  
  m_mapGeoToTrs =
    MeshDataStack::getActive()->getMapGeoToTrs("MapFacesToTrs");
  
  // build the m_mapTrs2Twall storage
  vector< SafePtr<TopologicalRegionSet> >& trsList = this->getTrsList();
  
  for (CFuint iTrs = 0; iTrs < trsList.size(); ++iTrs) {
    SafePtr<TopologicalRegionSet> trs = trsList[iTrs];
    const CFuint nbTrsFaces = trs->getLocalNbGeoEnts();
    RealVector* tWall = new RealVector(0.0,nbTrsFaces);
    m_mapTrs2Twall.insert(&*trs, tWall);
  }
  m_mapTrs2Twall.sortKeys();
}

//////////////////////////////////////////////////////////////////////////////

void SuperInletProjection::unsetup()
{
  SuperInlet::unsetup();
}

//////////////////////////////////////////////////////////////////////////////

void SuperInletProjection::setGhostState(GeometricEntity *const face)
{
  State *const innerState = face->getState(0);
  State *const ghostState = face->getState(1);

  // during initialization phase we store the initial solution values to be used as BC
  // This BC (the PFSS solution fixed on the inlet) was suggested by Jon Linker and
  // is evaluated by default. When using Jens' or Dana's BCs for the magnetic field
  // the B-field values in the ghost cells are overwritten.

  RealVector B_PFSS_dimless(3);

  if (m_initialSolutionMap.size() > 0) {
    /// map faces to corresponding TRS and index inside that TRS
    SafePtr<MapGeoToTrsAndIdx> mapGeoToTrs =
      MeshDataStack::getActive()->getMapGeoToTrs("MapFacesToTrs");
    const CFuint faceIdx = mapGeoToTrs->getIdxInTrs(face->getID());
    const string name = getCurrentTRS()->getName();
    SafePtr<RealVector> initialValues = m_initialSolutionMap.find(name);
    const CFuint nbVars = m_initialSolutionIDs.size();
    const CFuint startID = faceIdx*nbVars;
    for (CFuint i = 0; i < nbVars; ++i) {
      const CFuint varID = m_initialSolutionIDs[i];
      const CFuint idx = startID+i;
      cf_assert(idx < initialValues->size());

      B_PFSS_dimless[i] = (*initialValues)[idx]; // save the Poisson PFSS solution in a vector

      (*ghostState)[varID] = 2.*(*initialValues)[idx] - (*innerState)[varID];
      CFLog(DEBUG_MIN, "SuperInletProjection::setGhostState() => [" << varID << "] => " << (*initialValues)[idx] << " | " << (*innerState)[varID] << "\n");
    }
  }

  // this interpolates Br directly from the magnetogram file
  // static CFreal maxBr = -1e-14;
  CFreal BrFromFile = 0.;
  
  if (m_BfromFile) {
    vector<Node*>& nodesInFace = *face->getNodes();
    const CFuint nbNodesInFace = nodesInFace.size();
    SafePtr<TopologicalRegionSet> trs = m_mapGeoToTrs->getTrs(face->getID());
    cf_assert(trs.isNotNull());
    
    // build the mapTrs2NodalValues storage
    SafePtr<NodalStatesExtrapolator<CellCenterFVMData> > nse =
      this->getMethodData().getNodalStatesExtrapolator();
    SafePtr<vector<NodalStatesExtrapolator<CellCenterFVMData>::MapTrs2NodalValues*> >
      mapTrs2NodalValues = nse->getMapTrs2NodalValues();
    
    cf_assert(m_varIDs.size() == 1);
    cf_assert(m_varIDs[0] == 0);
    
    const CFuint varID = 0;
    cf_assert(varID < mapTrs2NodalValues->size());
    
    RealVector& bArray = *(*mapTrs2NodalValues)[varID]->find(&*trs);
    CFMap<CFuint,CFuint>* mapNodeIDs = nse->getMapTrs2NodeIDs()->find(&*trs);
    
    BrFromFile = 0.0;
    for (CFuint iNode = 0; iNode < nbNodesInFace; ++iNode) {
      const CFuint localNodeID = nodesInFace[iNode]->getLocalID();
      // CFLog(INFO, "Barray = " << bArray[mapNodeIDs->find(localNodeID)] << "\n");
      BrFromFile += bArray[mapNodeIDs->find(localNodeID)];
    }
    BrFromFile /= nbNodesInFace;

    // maxBr = std::max(maxBr, BrFromFile);
    // CFLog(INFO, "Br from file #### varID = " << varID << ", maxBr = " << maxBr << "\n");
  }
  
  if (_inletCoronalBC==1) {

  CFreal latG = 0.;
  CFreal RSun = 6.9551e8; // m
  CFreal RSS = 1.4953465e10; // m
  CFreal B0dip = 0.00022; // Tesla
  CFreal latI = 0.;
  const CFreal PI = MathTools::MathConsts::CFrealPi();
  const CFreal xG = ghostState->getCoordinates()[XX]*RSun;
  const CFreal xG_dimless = ghostState->getCoordinates()[XX];
  const CFreal yG = ghostState->getCoordinates()[YY]*RSun;
  const CFreal yG_dimless = ghostState->getCoordinates()[YY];
  const CFreal zG = ghostState->getCoordinates()[ZZ]*RSun;
  const CFreal zG_dimless = ghostState->getCoordinates()[ZZ];
  const CFreal xI = innerState->getCoordinates()[XX]*RSun;
  const CFreal xI_dimless = innerState->getCoordinates()[XX];
  const CFreal yI = innerState->getCoordinates()[YY]*RSun;
  const CFreal yI_dimless = innerState->getCoordinates()[YY];
  const CFreal zI = innerState->getCoordinates()[ZZ]*RSun;
  const CFreal zI_dimless = innerState->getCoordinates()[ZZ];
  const CFreal xBoundary = (xG + xI)/2.0;
  const CFreal xBoundary_dimless = (xG_dimless + xI_dimless)/2.0;
  const CFreal yBoundary = (yG + yI)/2.0;
  const CFreal yBoundary_dimless = (yG_dimless + yI_dimless)/2.0;
  const CFreal zBoundary = (zG + zI)/2.0;
  const CFreal zBoundary_dimless = (zG_dimless + zI_dimless)/2.0;
  const CFreal rBoundary = std::sqrt(xBoundary*xBoundary + yBoundary*yBoundary + zBoundary*zBoundary);
  const CFreal rBoundary_dimless = std::sqrt(xBoundary_dimless*xBoundary_dimless + yBoundary_dimless*yBoundary_dimless + zBoundary_dimless*zBoundary_dimless);
  const CFreal rhoBoundary = std::sqrt(xBoundary*xBoundary + yBoundary*yBoundary);
  const CFreal rhoBoundary_dimless = std::sqrt(xBoundary_dimless*xBoundary_dimless + yBoundary_dimless*yBoundary_dimless);
  const CFreal thetaBoundary = std::atan2(std::sqrt(xBoundary*xBoundary + yBoundary*yBoundary),zBoundary);
  const CFreal thetaBoundary_dimless = std::atan2(std::sqrt(xBoundary_dimless*xBoundary_dimless + yBoundary_dimless*yBoundary_dimless),zBoundary_dimless);
  const CFreal phiBoundary = std::atan2(yBoundary,xBoundary);
  const CFreal phiBoundary_dimless = std::atan2(yBoundary_dimless,xBoundary_dimless);


  const CFreal rG = std::sqrt(xG*xG + yG*yG + zG*zG);
  const CFreal rG_dimless = std::sqrt(xG_dimless*xG_dimless + yG_dimless*yG_dimless + zG_dimless*zG_dimless);
  const CFreal rhoG = std::sqrt(xG*xG + yG*yG);
  const CFreal rhoG_dimless = std::sqrt(xG_dimless*xG_dimless + yG_dimless*yG_dimless);
  const CFreal thetaG = std::atan2(std::sqrt(xG_dimless*xG_dimless + yG_dimless*yG_dimless),zG_dimless);
  const CFreal phiG = std::atan2(yG_dimless,xG_dimless);
  const CFreal rI = std::sqrt(xI*xI + yI*yI + zI*zI);
  const CFreal rI_dimless = std::sqrt(xI_dimless*xI_dimless + yI_dimless*yI_dimless + zI_dimless*zI_dimless);
  const CFreal thetaI = std::atan2(std::sqrt(xI_dimless*xI_dimless + yI_dimless*yI_dimless),zI_dimless);
  const CFreal rhoI = std::sqrt(xI*xI + yI*yI);
  const CFreal rhoI_dimless = std::sqrt(xI_dimless*xI_dimless + yI_dimless*yI_dimless);
  const CFreal phiI = std::atan2(yI_dimless,xI_dimless);


  // Determine the latitude of the ghost cell:
  // Only needed when using the dead-zone
  if (thetaG > -PI && thetaG < -PI*0.5) {
     latG = std::abs(thetaG) - PI*0.5;
  } else if (thetaG > -PI*0.5 && thetaG < 0) {
     latG = PI*0.5 - std::abs(thetaG);
  } else if (thetaG > 0. && thetaG < PI*0.5) {
     latG = PI*0.5 - thetaG;
  } else if (thetaG > PI*0.5 && thetaG < PI) {
     latG = thetaG - PI*0.5;
  } else {
    CFLog(INFO, "Error: value of theta for the point in question outside expected range\n");
  }

  //===== D E N S I T Y   B O U N D A R Y   C O N D I T I O N =================
  

  CFreal densityBoundary_dimless = 1.0;
  // The Dirichlet BC for rho with a value used by Linker et al. and B. Perri


  if (_JensRhoIni==1) {
     // The Dirichlet BC for with a value used by Jens for the surface
     // density. This value is used for a gravitationally stratified
     // density profile
     densityBoundary_dimless = 4.03679312e-13/1.67e-13;
  }


  CFreal densityG_dimless = 2.*densityBoundary_dimless - (*innerState)[0];
  (*ghostState)[0] = densityG_dimless;
  //(*ghostState)[0] = (*innerState)[0]; // Neumann version





   //=== M A G N E T I C - F I E L D   B O U N D A R Y   C O N D I T I O N ====


      // 1. Compute Btheta, Bphi at the boundary from B_PFS_dimless
      // 2. Take Br from BrFromFile as provided by Andrea
      // 3. From these 3 spherical B-field components compute the cartesian components
      // 4. Set them accordingly in the ghost cells

      CFreal BrBoundary_dimless = BrFromFile;
      CFreal BthetaBoundary_dimless = xBoundary_dimless*zBoundary_dimless/(rhoBoundary_dimless*rBoundary_dimless)*B_PFSS_dimless[0] + yBoundary_dimless*zBoundary_dimless/(rhoBoundary_dimless*rBoundary_dimless)*B_PFSS_dimless[1] - rhoBoundary_dimless/rBoundary_dimless*B_PFSS_dimless[2];
      CFreal BphiBoundary_dimless = -yBoundary_dimless/rhoBoundary_dimless*B_PFSS_dimless[0] + xBoundary_dimless/rhoBoundary_dimless*B_PFSS_dimless[1];

      CFreal BxBoundary_dimless = xBoundary_dimless/rBoundary_dimless*BrBoundary_dimless - yBoundary_dimless/rhoBoundary_dimless*BphiBoundary_dimless + xBoundary_dimless*zBoundary_dimless/(rhoBoundary_dimless*rBoundary_dimless)*BthetaBoundary_dimless;
      CFreal ByBoundary_dimless = yBoundary_dimless/rBoundary_dimless*BrBoundary_dimless + xBoundary_dimless/rhoBoundary_dimless*BphiBoundary_dimless + yBoundary_dimless*zBoundary_dimless/(rhoBoundary_dimless*rBoundary_dimless)*BthetaBoundary_dimless;
      CFreal BzBoundary_dimless = zBoundary_dimless/rBoundary_dimless*BrBoundary_dimless - rhoBoundary_dimless/rBoundary_dimless*BthetaBoundary_dimless;




  if (_JonLinkersBfieldSuggestion==1) {
      
      // Freeze all three components of the magnetic field

      /*
      (*ghostState)[4] = 2*B_PFSS_dimless[0] - (*innerState)[4];
      (*ghostState)[5] = 2*B_PFSS_dimless[1] - (*innerState)[5];
      (*ghostState)[6] = 2*B_PFSS_dimless[2] - (*innerState)[6];
      */

      (*ghostState)[4] = 2*BxBoundary_dimless - (*innerState)[4];
      (*ghostState)[5] = 2*ByBoundary_dimless - (*innerState)[5];
      (*ghostState)[6] = 2*BzBoundary_dimless - (*innerState)[6];

  } else if (_JensBfieldBC==1) {
      //Br should be taken from the PFSS initial solution, while Btheta and
      // Bphi are linearly extrapolated to the ghost from the inner state:
      //CFreal BrG_dimless = xG_dimless/rG_dimless*(*ghostState)[4] + yG_dimless/rG_dimless*(*ghostState)[5] + zG_dimless/rG_dimless*(*ghostState)[6];

      CFreal BxI_dimless = (*innerState)[4];
      CFreal ByI_dimless = (*innerState)[5];
      CFreal BzI_dimless = (*innerState)[6];
      CFreal BrI_dimless = xI_dimless/rI_dimless*BxI_dimless + yI_dimless/rI_dimless*ByI_dimless + zI_dimless/rI_dimless*BzI_dimless;
      CFreal BthetaI_dimless = xI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*BxI_dimless + yI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*ByI_dimless - rhoI_dimless/rI_dimless*BzI_dimless;
      CFreal BphiI_dimless = -yI_dimless/rhoI_dimless*BxI_dimless + xI_dimless/rhoI_dimless*ByI_dimless;

      CFreal BthetaG_dimless = BthetaI_dimless;
      CFreal BphiG_dimless = BphiI_dimless;
      CFreal BrG_dimless = 2*BrFromFile - BrI_dimless;

      // Back-transformation to Cartesian coordinates:
      CFreal BxG_dimless = xG_dimless/rG_dimless*BrG_dimless - yG_dimless/rhoG_dimless*BphiG_dimless + xG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*BthetaG_dimless;
      CFreal ByG_dimless = yG_dimless/rG_dimless*BrG_dimless + xG_dimless/rhoG_dimless*BphiG_dimless + yG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*BthetaG_dimless;
      CFreal BzG_dimless = zG_dimless/rG_dimless*BrG_dimless - rhoG_dimless/rG_dimless*BthetaG_dimless;

      (*ghostState)[4] = BxG_dimless;
      (*ghostState)[5] = ByG_dimless;
      (*ghostState)[6] = BzG_dimless;

  } else if (_DanasBfieldBC==1) {
      // Slightly modified version of Jens' BC:
      ////CFreal BrG_dimless = xG_dimless/rG_dimless*(*ghostState)[4] + yG_dimless/rG_dimless*(*ghostState)[5] + zG_dimless/rG_dimless*(*ghostState)[6];

      CFreal BxI_dimless = (*innerState)[4];
      CFreal ByI_dimless = (*innerState)[5];
      CFreal BzI_dimless = (*innerState)[6];
      CFreal BrI_dimless = xI_dimless/rI_dimless*BxI_dimless + yI_dimless/rI_dimless*ByI_dimless + zI_dimless/rI_dimless*BzI_dimless;
      CFreal BrG_dimless = 2*BrFromFile - BrI_dimless;
      CFreal BthetaI_dimless = xI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*BxI_dimless + yI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*ByI_dimless - rhoI_dimless/rI_dimless*BzI_dimless;
      CFreal BphiI_dimless = -yI_dimless/rhoI_dimless*BxI_dimless + xI_dimless/rhoI_dimless*ByI_dimless;

      CFreal BthetaG_dimless = BthetaI_dimless*std::pow(rI_dimless,5)/pow(rG_dimless,5);
      CFreal BphiG_dimless = BphiI_dimless;
 
      // Back-transformation to Cartesian coordinates:
      CFreal BxG_dimless = xG_dimless/rG_dimless*BrG_dimless - yG_dimless/rhoG_dimless*BphiG_dimless + xG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*BthetaG_dimless;
      CFreal ByG_dimless = yG_dimless/rG_dimless*BrG_dimless + xG_dimless/rhoG_dimless*BphiG_dimless + yG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*BthetaG_dimless;
      CFreal BzG_dimless = zG_dimless/rG_dimless*BrG_dimless - rhoG_dimless/rG_dimless*BthetaG_dimless;
      (*ghostState)[4] = BxG_dimless;
      (*ghostState)[5] = ByG_dimless;
      (*ghostState)[6] = BzG_dimless;
  }






  //===== V E L O C I T Y   B O U N D A R Y   C O N D I T I O N ===============

  // Read the inner state velocity need for extrapolation radially inwards:
  CFreal VxI_dimless = (*innerState)[1];
  CFreal VyI_dimless = (*innerState)[2];
  CFreal VzI_dimless = (*innerState)[3];
  CFreal VrI_dimless = xI_dimless/rI_dimless*VxI_dimless + yI_dimless/rI_dimless*VyI_dimless + zI_dimless/rI_dimless*VzI_dimless;
  CFreal VthetaI_dimless = xI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*VxI_dimless + yI_dimless*zI_dimless/(rhoI_dimless*rI_dimless)*VyI_dimless - rhoI_dimless/rI_dimless*VzI_dimless;
  CFreal VphiI_dimless = -yI_dimless/rhoI_dimless*VxI_dimless + xI_dimless/rhoI_dimless*VyI_dimless;

  if (_JensVelocityBC==1) {
      // Momentum in spherical coordinates extrapolated to the ghost cells:
      CFreal VrG_dimless = VrI_dimless*(*innerState)[0]/densityG_dimless;
      CFreal VthetaG_dimless = VthetaI_dimless*(*innerState)[0]/densityG_dimless;
      CFreal VphiG_dimless = VphiI_dimless*(*innerState)[0]/densityG_dimless;
      // Transformation back to Cartesian coordinates:
      CFreal VxG_dimless = xG_dimless/rG_dimless*VrG_dimless - yG_dimless/rhoG_dimless*VphiG_dimless + xG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*VthetaG_dimless;
      CFreal VyG_dimless = yG_dimless/rG_dimless*VrG_dimless + xG_dimless/rhoG_dimless*VphiG_dimless + yG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*VthetaG_dimless;
      CFreal VzG_dimless = zG_dimless/rG_dimless*VrG_dimless - rhoG_dimless/rG_dimless*VthetaG_dimless;
      // Set the ghost state values:
      (*ghostState)[1] = VxG_dimless;
      (*ghostState)[2] = VyG_dimless;
      (*ghostState)[3] = VzG_dimless;

  }
  else if (_BarbarasVelocityBC==1) {
/*
      CFreal VxDimless = (*innerState)[1];
      CFreal VyDimless = (*innerState)[2];
      CFreal VzDimless = (*innerState)[3];
      CFreal BxDimless = (*innerState)[4];
      CFreal ByDimless = (*innerState)[5];
      CFreal BzDimless = (*innerState)[6];
*/

      //CFreal BDimless = std::sqrt(BxDimless*BxDimless+ByDimless*ByDimless+BzDimless*BzDimless);
      //CFreal BBoundaryDimless = std::sqrt(BxBoundaryDimless*BxBoundaryDimless + ByBoundaryDimless*ByBoundaryDimless + BzBoundaryDimless*BzBoundaryDimless);
      
      //CFreal Vparallel = (VxDimless*BxDimless + VyDimless*ByDimless + VzDimless*BzDimless)/BDimless;

      // Version 2: konstante Ausstr??mung am Inlet mit Vr = 848.1501 m/s

      CFreal VxBoundary_dimless = 848.15*(xBoundary/rBoundary)/(2.2e-4/sqrt(1.2566e-6*1.67e-13));
      CFreal VyBoundary_dimless = 848.15*(yBoundary/rBoundary)/(2.2e-4/sqrt(1.2566e-6*1.67e-13));
      CFreal VzBoundary_dimless = 848.15*(zBoundary/rBoundary)/(2.2e-4/sqrt(1.2566e-6*1.67e-13));
      
      // B at the boundary B_PFSS_dimless

      CFreal BnormBoundary_dimless = std::sqrt(B_PFSS_dimless[0]*B_PFSS_dimless[0]+B_PFSS_dimless[1]*B_PFSS_dimless[1]+B_PFSS_dimless[2]*B_PFSS_dimless[2]);
  

      CFreal Vparallel = (VxBoundary_dimless*B_PFSS_dimless[0] + VyBoundary_dimless*B_PFSS_dimless[1] + VzBoundary_dimless*B_PFSS_dimless[2])/BnormBoundary_dimless;

      CFreal VxBoundaryUpdate_dimless = Vparallel*B_PFSS_dimless[0]/BnormBoundary_dimless;
      CFreal VyBoundaryUpdate_dimless = Vparallel*B_PFSS_dimless[1]/BnormBoundary_dimless;
      CFreal VzBoundaryUpdate_dimless = Vparallel*B_PFSS_dimless[2]/BnormBoundary_dimless;



      (*ghostState)[1] = 2.*VxBoundaryUpdate_dimless - (*innerState)[1];
      (*ghostState)[2] = 2.*VyBoundaryUpdate_dimless - (*innerState)[2];
      (*ghostState)[3] = 2.*VzBoundaryUpdate_dimless - (*innerState)[3];


      if (_hydrodynamic_limit==1) {
          // Im hydrodynamischen Limit ist B_PFSS = 0 und die perfekter Rotator BC muss adaptiert werden
          // In the hydrodynamic Limit B_PFSS = 0 and thus we cannot computer V_par, because we have to divide by B there.
          // However, in this limit V_par = Vr_Inlet*e_r anyway:

          Vparallel = 848.15/(2.2e-4/sqrt(1.2566e-6*1.67e-13));  // = Vr|inlet


          VxBoundaryUpdate_dimless = Vparallel*xBoundary_dimless/rBoundary_dimless;
          VyBoundaryUpdate_dimless = Vparallel*xBoundary_dimless/rBoundary_dimless;
          VzBoundaryUpdate_dimless = Vparallel*xBoundary_dimless/rBoundary_dimless;



         (*ghostState)[1] = 2.*VxBoundaryUpdate_dimless - (*innerState)[1];
         (*ghostState)[2] = 2.*VyBoundaryUpdate_dimless - (*innerState)[2];
         (*ghostState)[3] = 2.*VzBoundaryUpdate_dimless - (*innerState)[3];

      }
      

  
  }
  else if (_DanasVelocityBC==1) {
      CFreal VrG_dimless = (*innerState)[0]*VrI_dimless*rI_dimless*rI_dimless/(densityG_dimless*rG_dimless*rG_dimless);
      CFreal VthetaG_dimless = -VthetaI_dimless;
      CFreal VphiG_dimless = 0.0;
      if (_DifferentialRotation==0) {
        VphiG_dimless = 0.0 - VphiI_dimless;
      }
      else if (_DifferentialRotation==1) {
        CFreal A = 14.713;
        CFreal B = -2.396;
        CFreal C = -1.787;
        CFreal Omega_deg_day = A + B*std::sin(latG)*std::sin(latG) + C*std::sin(latG)*std::sin(latG)*std::sin(latG)*std::sin(latG);
        CFreal Omega = Omega_deg_day*PI/(180*24*60*60);   // Now in rad/sec
        CFreal VphiBoundary_dimless = Omega*rBoundary*std::sin(thetaBoundary)/(2.2e-4/(std::sqrt(1.25e-6*1.67e-13)));
        CFreal VphiG_dimless = 2.0*VphiBoundary_dimless - VphiI_dimless;
      }
            // Transformation back to Cartesian coordinates:
      CFreal VxG_dimless = xG_dimless/rG_dimless*VrG_dimless - yG_dimless/rhoG_dimless*VphiG_dimless + xG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*VthetaG_dimless;
      CFreal VyG_dimless = yG_dimless/rG_dimless*VrG_dimless + xG_dimless/rhoG_dimless*VphiG_dimless + yG_dimless*zG_dimless/(rhoG_dimless*rG_dimless)*VthetaG_dimless;
      CFreal VzG_dimless = zG_dimless/rG_dimless*VrG_dimless - rhoG_dimless/rG_dimless*VthetaG_dimless;
      // Set the ghost state values:
      (*ghostState)[1] = VxG_dimless;
      (*ghostState)[2] = VyG_dimless;
      (*ghostState)[3] = VzG_dimless;
  }
  









  //===== P R E S S U R E   B O U N D A R Y   C O N D I T I O N ===============
  // Temperature kept constant at 1.5e6 K
  // Pressure at inner boundary:
  CFreal T_Sun = 1.5e6;   // K
  CFreal mu_cor = 1.27;   // Mean molecular weight
  CFreal mH = 1.67e-27;   // Mass hydrogen
  CFreal BRef = 2.2e-4;   // T
  CFreal mu0 = 1.2566e-6;
  CFreal rhoRef = 1.67e-13;
  CFreal kB = 1.38e-23;
  
  //V1:
  if (_pressure_fixed==1) {
  // This is a Dirichlet pressure BC for 1/r^2 initial condition like used by Linker et al
  // or B. Perri
  //CFreal PressureBoundary_dimless = (2.0*rhoRef*kB*T_Sun/(mu_cor*mH))/(pow(BRef,2)/mu0);
CFreal PressureBoundary_dimless =  0.0032549425343197064/(pow(BRef,2)/mu0);
  (*ghostState)[7] = 2.*PressureBoundary_dimless - (*innerState)[7];
  }

  else if (_JensPIni==1){
  // This is a Dirichlet pressure BC for gravitationally stratified pressure initial
  // condition as used by Jens
  CFreal PressureBoundary_dimless = 8.01260207e-03/(pow(BRef,2)/mu0);
  (*ghostState)[7] = 2.*PressureBoundary_dimless - (*innerState)[7];

  }

  else if (_pressure_Neumann==1){
  //V2:
  //CFreal PressureBoundary_dimless = (rhoRef*kB*T_Sun/(mu_cor*mH))/(pow(BRef,2)/mu0);
  //(*ghostState)[7] = 2.*PressureBoundary_dimless - (*innerState)[7];


  
  (*ghostState)[7] = (*innerState)[7];
  }
  else {
    CFLog(VERBOSE, "ERROR: Pressure not specified at inlet boundary!\n");
  }  
  
  //V4:
  //(*ghostState)[7] = 2.0*std::pow(rhoRef,1.05)/(pow(BRef,2)/mu0) - (*innerState)[7];


  //===== P S I   B O U N D A R Y   C O N D I T I O N S =======================
  if (_Phi_divB_zero==1) {
    (*ghostState)[8] = -(*innerState)[8];
  }
  else if (_Phi_divB_extrapolated==1) {
    (*ghostState)[8] = (*innerState)[8];
  }
  else {
    std::cout << "ERROR: Phi not specified at inlet boundary!" << endl;
  }
  
 } else {

  // coordinate of the boundary point
  _bCoord = (innerState->getCoordinates() +
             ghostState->getCoordinates());
  _bCoord *= 0.5;

  // ghostState = 2*bcState - innerState
  _vFunction.evaluate(_bCoord, *_dimState);

  if(_inputAdimensionalValues)
  {
    *ghostState = *_dimState;
  }
  else
  {
    _varSet->setAdimensionalValues(*_dimState, *ghostState);
  }
  
  *ghostState *= 2.;
  *ghostState -= *innerState;
  
  //  cf_assert(_projectionIDs.size() > 0);
  for (CFuint i = 0; i < _projectionIDs.size(); ++i) {
    const CFuint varID = _projectionIDs[i];
    (*ghostState)[varID] = (*innerState)[varID];
  }
  }

  //CFLog(INFO, "SuperInletProjection::setGhostState() => ghost,dimst,inner = "
  //	<< (*ghostState)[8] << ", " << (*_dimState)[8] << ", e" << (*innerState)[8] << "\n");
  
  /*if ((*ghostState)[8] < 1e-32) {
    CFLog(INFO, "SuperInletProjection::setGhostState() => ghost = " << (*ghostState)[8] <<"\n");
  }

  if ((*innerState)[8] < 1e-32) {
    CFLog(INFO, "SuperInletProjection::setGhostState() => inner = " << (*innerState)[8] <<"\n");
  }

  if ((*_dimState)[8] < 1e-32) {
    CFLog(INFO, "SuperInletProjection::setGhostState() => dimst = " << (*_dimState)[8] <<"\n");
    }*/

}


//////////////////////////////////////////////////////////////////////////////

    } // namespace FiniteVolume

  } // namespace Numerics

} // namespace COOLFluiD
