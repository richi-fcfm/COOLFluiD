#include "Framework/State.hh"
#include "Environment/ObjectProvider.hh"

#include "FluxReconstructionMethod/FluxReconstruction.hh"
#include "FluxReconstructionMethod/SetQuadLagrangeP2FluxReconstructionP3StateCoord.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace FluxReconstructionMethod {

//////////////////////////////////////////////////////////////////////////////

Environment::ObjectProvider<SetQuadLagrangeP2FluxReconstructionP3StateCoord,
               Framework::SetElementStateCoord,
               FluxReconstructionModule>
SetQuadLagrangeP2FluxReconstructionP3StateCoord("QuadLagrangeP2FluxReconstructionP3");

//////////////////////////////////////////////////////////////////////////////

void SetQuadLagrangeP2FluxReconstructionP3StateCoord::operator() (const vector<Framework::Node*>& nodes,
                                                          vector<Framework::State*>& states)
{
  cf_assert(states.size() == 16);
  cf_assert(nodes.size() == 9);
}

//////////////////////////////////////////////////////////////////////////////

void SetQuadLagrangeP2FluxReconstructionP3StateCoord::update(const vector<Framework::Node*>& nodes,
                                                     vector<Framework::State*>& states)
{
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace ShapeFunctions

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
