// Copyright (C) 2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COOLFluiD_ShapeFunctions_LagrangeShapeFunctionLineP0_hh
#define COOLFluiD_ShapeFunctions_LagrangeShapeFunctionLineP0_hh

//////////////////////////////////////////////////////////////////////////////

#include "ShapeFunctions/LagrangeShapeFunction.hh"
#include "ShapeFunctions/ShapeFunctionsAPI.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace ShapeFunctions {

//////////////////////////////////////////////////////////////////////////////

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P0 (constant)
/// 1D element.
/// @author Andrea Lani
class ShapeFunctions_API LagrangeShapeFunctionLineP0 : public ShapeFunctions::LagrangeShapeFunction {
public:

  /// Get the inheritant dimensionality of the ShapeFunction
  static CFuint getDimensionality()
  {
    return DIM_1D;
  }

  /// Get the number of nodal shape functions in the element
  static CFuint getNbNodes()
  {
    return 1;
  }

  /// Get the number of faces
  static CFuint getNbFaces()
  {
    return 2;
  }

  /// Gets the type of CFGeoShape::Type
  static CFGeoShape::Type getShape()
  {
    return CFGeoShape::LINE;
  }

  /// Gets the type of Interpolator
  static CFPolyForm::Type getInterpolatorType()
  {
    return CFPolyForm::LAGRANGE;
  }

  /// Gets the Interpolator order
  static CFPolyOrder::Type getInterpolatorOrder()
  {
    return CFPolyOrder::ORDER0;
  }

  /// Gets the states mapped coordinates
  static void getStatesMappedCoordinates(std::vector<RealVector>& mappedCoords)
  {
    cf_assert (mappedCoords.size() == 1);
    mappedCoords[0][KSI] = 0. ;
  }

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  static void computeShapeFunctions(
          const std::vector<RealVector>& mappedCoord, std::vector<RealVector>& shapeFunc)
  {
    for (CFuint ip = 0; ip < mappedCoord.size(); ++ip) {
      computeShapeFunction(mappedCoord[ip],shapeFunc[ip]);
    }
  }

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  static void computeShapeFunction(
        const RealVector& mappedCoord, RealVector& shapeFunc)
  {
      cf_assert(mappedCoord.size() == 1);
      shapeFunc[0] = 1.0;
  }

  /// Compute the Gradient of the Shape Function
  static void computeGradientStates(
         const std::vector<RealMatrix>& jacob,
         const std::vector<RealVector>& mappedCoord,
               std::vector<RealMatrix>& grad)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeGradientStates()");
  }

  /// Computes the normal to a face at the given mapped coordinates,
  /// scaled with the 'face Jacobian determinant'.
  /// (Normal has the dimensionality of the Face + 1)
  static void computeFaceJacobDetVectorAtMappedCoords(const std::vector<RealVector>& mappedCoord,
      const std::vector<Framework::Node*>& nodes,
      std::vector<RealVector>& normal)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeFaceJacobDetVectorAtMappedCoords()");
  }

  /// Computes the normal to a given mapped coordinate plane, at the given mapped coordinates
  static void computeMappedCoordPlaneNormal(const std::vector<CFuint>& planeIdx,
                                            const std::vector<RealVector>& mappedCoord,
                                            const std::vector<Framework::Node*>& nodes,
                                            std::vector<RealVector>& normal)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeMappedCoordPlaneNormal()");
  }

  /// Compute the Jacobian
  static void computeJacobian(
         const std::vector<Framework::Node*>& nodes,
         const std::vector<RealVector>& mappedCoord,
               std::vector<RealMatrix>& jacob)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobian()");
  }

  /// Compute the Jacobian
  static void computeJacobianPlus1D(
         const std::vector<Framework::Node*>& nodes,
         const std::vector<RealVector>& mappedCoord,
               std::vector<RealMatrix>& jacob)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobianPlus1D()");
  }

  /// Compute the Jacobian
  static void computeJacobianPlus2D(
         const std::vector<Framework::Node*>& nodes,
         const std::vector<RealVector>& mappedCoord,
               std::vector<RealMatrix>& jacob)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobianPlus2D()");
  }

  /// Compute the jacobian determinant at the given
  /// mapped coordinates
  static void computeJacobianDeterminant(
         const std::vector<RealVector>& mappedCoord,
         const std::vector<Framework::Node*>& nodes,
               std::valarray<CFreal>& detJacobian)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobianDeterminant()");
  }

  /// Compute the jacobian determinant at the given
  /// mapped coordinates
  static void computeJacobianDeterminantPlus1D(
         const std::vector<RealVector>& mappedCoord,
         const std::vector<Framework::Node*>& nodes,
               std::valarray<CFreal>& detJacobian)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobianDeterminantPlus1D()");
  }

  /// Compute the jacobian determinant at the given
  /// mapped coordinates
  static void computeJacobianDeterminantPlus2D(
         const std::vector<RealVector>& mappedCoord,
         const std::vector<Framework::Node*>& nodes,
               std::valarray<CFreal>& detJacobian)
  {
    throw Common::NotImplementedException (FromHere(),getName() + "::computeJacobianDeterminantPlus2D()");
  }

  /// Get the name of this shape function
  static const std::string getName()
  {
    static const std::string name = "LagrangeLineP0";
    return name;
  }

  /// Get the ID for the solution integration
  static Framework::InterpolatorID getInterpolatorID()
  {
    return _interpolatorID;
  }

  /// Get the ID for the solution integration
  static void setInterpolatorID(const Framework::InterpolatorID& id)
  {
    _interpolatorID = id;
  }

  /// Get the volume
  static CFreal computeVolume(const std::vector<Framework::Node*>& nodes)
  {
    throw Common::NotImplementedException
      (FromHere(), getName()+"::computeVolume()");
    return 0.;
  }

  /// Get the centroid
  static RealVector computeCentroid(const std::vector<Framework::Node*>& nodes)
  {
    throw Common::NotImplementedException
      (FromHere(), getName()+"::computeCentroid()");
  }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @return RealVector containing the Mapped Coordinates
  static RealVector computeMappedCoordinates(const RealVector& coord, const std::vector<Framework::Node*>& nodes)
  {
    throw Common::NotImplementedException (FromHere(),"LagrangeShapeFunctionLineP0::computeMappedCoordinates()");
  }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @return RealVector containing the Mapped Coordinates
  static RealVector computeMappedCoordinatesPlus1D(const RealVector& coord, const std::vector<Framework::Node*>& nodes)
  {
    throw Common::NotImplementedException (FromHere(),"LagrangeShapeFunctionLineP0::computeMappedCoordinatesPlus1D()");
  }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @return RealVector containing the Mapped Coordinates
  static RealVector computeMappedCoordinatesPlus2D(const RealVector& coord, const std::vector<Framework::Node*>& nodes)
  {
    throw Common::NotImplementedException
      (FromHere(), getName()+"::computeMappedCoordinatesPlus2D()");
  }

  /// Compute Face Average Normals
  /// @param nodes contains the nodes
  /// @return vector<RealVector> containing the average face normals
  static std::vector<RealVector> computeAvgFaceNormals(const std::vector<Framework::Node*>& nodes)
  {
    throw Common::ShouldNotBeHereException (FromHere(),getName()+"::computeFaceNormals()");
  }

  /// Compute Face Average Normals
  /// @param nodes contains the nodes
  /// @return vector<RealVector> containing the average face normals
  static std::vector<RealVector> computeFaceNormals(const RealVector& mappedCoord, const std::vector<Framework::Node*>& nodes)
  {
    throw Common::ShouldNotBeHereException (FromHere(),getName()+"::computeFaceNormals()");
  }

  /// Compute Cell Average Normal
  /// @param nodes contains the nodes
  /// @return RealVector containing the average cell normal
  static RealVector computeAvgCellNormal(const std::vector<Framework::Node*>& nodes)
  {
    RealVector normal(DIM_2D);

    const RealVector x0 = (*nodes[0]);
    const RealVector x1 = (*nodes[1]);

    _vec = x1-x0;
    normal[XX] = _vec[YY];
    normal[YY] = -_vec[XX];

    return normal;
  }

  /// Compute Cell Normal at a given mapped coord
  /// @param nodes contains the nodes
  /// @return RealVector containing the cell normal
  static RealVector computeCellNormal(const RealVector& mappedCoord, const std::vector<Framework::Node*>& nodes)
  {
    ///P0 element -> same as average cell normal
    RealVector normal(DIM_2D);

    const RealVector x0 = (*nodes[0]);
    const RealVector x1 = (*nodes[1]);

    _vec = x1-x0;
    normal[XX] = _vec[YY];
    normal[YY] = -_vec[XX];

    return normal;
  }

  /// Check if a point (defined with mapped coordinates) is inside an element
  static bool isInMappedElement(const RealVector& mappedCoord)
  {
    cf_assert(mappedCoord.size() == 1);
    if((mappedCoord[0] < 0.) || (mappedCoord[0] > 1.))
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  /// Check if a point is inside an element
  static bool isInElement(const std::vector<Framework::Node*>& nodes, const RealVector& coord)
  {
    throw Common::NotImplementedException
      (FromHere(), getName()+"::isInElement()");
  }

private:

  /// Default constructor without arguments
  LagrangeShapeFunctionLineP0();

  /// Default destructor
  ~LagrangeShapeFunctionLineP0();

private:

  /// solution integrator ID
  static CFuint _interpolatorID;

  /// Temporary Vector for the computation of normals
  static RealVector _vec;

  /// Vector of normals
  static std::vector<RealVector> _normals;

}; // end of class LagrangeShapeFunctionLineP0

//////////////////////////////////////////////////////////////////////////////

    } // namespace ShapeFunctions



} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_ShapeFunctions_LagrangeShapeFunctionLineP0_hh
