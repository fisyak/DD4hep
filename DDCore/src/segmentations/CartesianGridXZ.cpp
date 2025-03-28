//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  Created: Jun 28, 2013
//  Author:  Christian Grefe, CERN
//
//==========================================================================

/// Framework include files
#include <DDSegmentation/CartesianGridXZ.h>

namespace dd4hep {
namespace DDSegmentation {

using std::make_pair;
using std::vector;

/// default constructor using an encoding string
CartesianGridXZ::CartesianGridXZ(const std::string& cellEncoding) :
	CartesianGrid(cellEncoding) {
	// define type and description
	_type = "CartesianGridXZ";
	_description = "Cartesian segmentation in the local XZ-plane";

	// register all necessary parameters
	registerParameter("grid_size_x", "Cell size in X", _gridSizeX, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_z", "Cell size in Z", _gridSizeZ, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_x", "Cell offset in X", _offsetX, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_z", "Cell offset in Z", _offsetZ, 0., SegmentationParameter::LengthUnit, true);
	registerIdentifier("identifier_x", "Cell ID identifier for X", _xId, "x");
	registerIdentifier("identifier_z", "Cell ID identifier for Z", _zId, "z");
}

/// Default constructor used by derived classes passing an existing decoder
CartesianGridXZ::CartesianGridXZ(const BitFieldCoder* decode) :
	CartesianGrid(decode) {
	// define type and description
	_type = "CartesianGridXZ";
	_description = "Cartesian segmentation in the local XZ-plane";

	// register all necessary parameters
	registerParameter("grid_size_x", "Cell size in X", _gridSizeX, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_z", "Cell size in Z", _gridSizeZ, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_x", "Cell offset in X", _offsetX, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_z", "Cell offset in Z", _offsetZ, 0., SegmentationParameter::LengthUnit, true);
	registerIdentifier("identifier_x", "Cell ID identifier for X", _xId, "x");
	registerIdentifier("identifier_z", "Cell ID identifier for Z", _zId, "z");
}

/// destructor
CartesianGridXZ::~CartesianGridXZ() {

}

/// determine the position based on the cell ID
Vector3D CartesianGridXZ::position(const CellID& cID) const {
	vector<double> localPosition(3);
	Vector3D cellPosition;
	cellPosition.X = binToPosition( _decoder->get(cID,_xId ), _gridSizeX, _offsetX);
	cellPosition.Z = binToPosition( _decoder->get(cID,_zId ), _gridSizeZ, _offsetZ);
	return cellPosition;
}

/// determine the cell ID based on the position
  CellID CartesianGridXZ::cellID(const Vector3D& localPosition, const Vector3D& /* globalPosition */, const VolumeID& vID) const {
        CellID cID = vID ;
        _decoder->set( cID,_xId, positionToBin(localPosition.X, _gridSizeX, _offsetX) );
	_decoder->set( cID,_zId, positionToBin(localPosition.Z, _gridSizeZ, _offsetZ) );
	return cID ;
}

std::vector<double> CartesianGridXZ::cellDimensions(const CellID&) const {
  return {_gridSizeX, _gridSizeZ};
}


} /* namespace DDSegmentation */
} /* namespace dd4hep */
