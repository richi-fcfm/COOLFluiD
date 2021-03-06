#include "FiniteVolumeMultiFluidMHD/MultiFluidMHDSTRadiation.hh"
#include "FiniteVolumeMultiFluidMHD/FiniteVolumeMultiFluidMHD.hh"
#include "Framework/MethodStrategyProvider.hh"
#include "Maxwell/Maxwell2DProjectionVarSet.hh"
//#include "Maxwell/Maxwell3DProjectionVarSet.hh"
#include "MultiFluidMHD/MultiFluidMHDVarSet.hh"
#include "Framework/MethodStrategyProvider.hh"
#include "FiniteVolume/CellCenterFVMData.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::Common;
using namespace COOLFluiD::Physics::Maxwell;
using namespace COOLFluiD::Physics::MultiFluidMHD;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Numerics {

    namespace FiniteVolume {

//////////////////////////////////////////////////////////////////////////////

MethodStrategyProvider<MultiFluidMHDSTRadiation<MultiFluidMHDVarSet<Maxwell2DProjectionVarSet> >,
		       CellCenterFVMData,
		       Framework::ComputeSourceTerm<CellCenterFVMData>,
		       FiniteVolumeMultiFluidMHDModule>
multiFluidMHDSTRadiation2DProvider("MultiFluidMHDSTRadiation2D");

// MethodStrategyProvider<MultiFluidMHDSTRadiation<MultiFluidMHDVarSet<Maxwell3DProjectionVarSet> >,
// 		       CellCenterFVMData,
// 		       Framework::ComputeSourceTerm<CellCenterFVMData>,
// 		       FiniteVolumeMultiFluidMHDModule>
// multiFluidMHDSTRadiation3DProvider("MultiFluidMHDST3D");

//////////////////////////////////////////////////////////////////////////////

    } // namespace FiniteVolume

  } // namespace Numerics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
