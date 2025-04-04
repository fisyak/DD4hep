//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  Created: Sep 03, 2014
//  Author:  F.Gaede CERN/Desy
//
//==========================================================================

/// Framework include files
#include <DDSegmentation/CartesianGridYZ.h>

namespace dd4hep {
namespace DDSegmentation {

/// default constructor using an encoding string
CartesianGridYZ::CartesianGridYZ(const std::string& cellEncoding) :
		CartesianGrid(cellEncoding) {
	// define type and description
	_type = "CartesianGridYZ";
	_description = "Cartesian segmentation in the local YZ-plane";

	// register all necessary parameters
	registerParameter("grid_size_y", "Cell size in Y", _gridSizeY, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_z", "Cell size in Z", _gridSizeZ, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_y", "Cell offset in Y", _offsetY, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_z", "Cell offset in Z", _offsetZ, 0., SegmentationParameter::LengthUnit, true);
	registerIdentifier("identifier_y", "Cell ID identifier for Y", _yId, "y");
	registerIdentifier("identifier_z", "Cell ID identifier for Z", _zId, "z");
}


/// Default constructor used by derived classes passing an existing decoder
CartesianGridYZ::CartesianGridYZ(const BitFieldCoder* decode) : CartesianGrid(decode)
{
	// define type and description
	_type = "CartesianGridYZ";
	_description = "Cartesian segmentation in the local YZ-plane";

	// register all necessary parameters
	registerParameter("grid_size_y", "Cell size in Y", _gridSizeY, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_z", "Cell size in Z", _gridSizeZ, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_y", "Cell offset in Y", _offsetY, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_z", "Cell offset in Z", _offsetZ, 0., SegmentationParameter::LengthUnit, true);
	registerIdentifier("identifier_y", "Cell ID identifier for Y", _yId, "y");
	registerIdentifier("identifier_z", "Cell ID identifier for Z", _zId, "z");
}

/// destructor
CartesianGridYZ::~CartesianGridYZ() {

}

/// determine the position based on the cell ID
Vector3D CartesianGridYZ::position(const CellID& cID) const {
	Vector3D cellPosition;
	cellPosition.Y = binToPosition( _decoder->get(cID,_yId ), _gridSizeY, _offsetY);
	cellPosition.Z = binToPosition( _decoder->get(cID,_zId ), _gridSizeZ, _offsetZ);
	return cellPosition;
}

/// determine the cell ID based on the position
  CellID CartesianGridYZ::cellID(const Vector3D& localPosition, const Vector3D& /* globalPosition */, const VolumeID& vID) const {
        CellID cID = vID ;
	_decoder->set( cID,_yId, positionToBin(localPosition.Y, _gridSizeY, _offsetY) );
	_decoder->set( cID,_zId, positionToBin(localPosition.Z, _gridSizeZ, _offsetZ) );
	return cID ;
}

std::vector<double> CartesianGridYZ::cellDimensions(const CellID&) const {
  return {_gridSizeY, _gridSizeZ};
}


} /* namespace DDSegmentation */
} /* namespace dd4hep */
