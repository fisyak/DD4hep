//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  \author   Markus Frank, David Blyth
//  \date     2018-02-20
//  \version  1.0
//
//==========================================================================
#ifndef DD4HEP_CARTESIANSTRIPY_H
#define DD4HEP_CARTESIANSTRIPY_H 1

// Framework include files
#include <DD4hep/Segmentations.h>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {
/// Namespace for base segmentations
namespace DDSegmentation {
class CartesianStripY;
}

/// We need some abbreviation to make the code more readable.
typedef Handle<SegmentationWrapper<DDSegmentation::CartesianStripY> > CartesianStripYHandle;

/// Implementation class for the strip Y segmentation.
/**
 *  Concrete user handle to serve specific needs of client code
 *  which requires access to the base functionality not served
 *  by the super-class Segmentation.
 *
 *  Note:
 *  We only check the validity of the underlying handle.
 *  If for whatever reason the implementation object is not valid
 *  This is not checked.
 *  In principle this CANNOT happen unless some brain-dead has
 *  fiddled with the handled object directly.....
 *
 *  Note:
 *  The handle base corresponding to this object in for
 *  conveniance reasons instantiated in dd4hep/src/Segmentations.cpp.
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \ingroup DD4HEP_CORE
 */
class CartesianStripY : public CartesianStripYHandle {
   public:
    /// Default constructor
    CartesianStripY() = default;
    /// Copy constructor
    CartesianStripY(const CartesianStripY& e) = default;
    /// Copy Constructor from segmentation base object
    CartesianStripY(const Segmentation& e) : Handle<Object>(e) {}
    /// Copy constructor from handle
    CartesianStripY(const Handle<Object>& e) : Handle<Object>(e) {}
    /// Copy constructor from other equivalent handle
    template <typename Q>
    CartesianStripY(const Handle<Q>& e) : Handle<Object>(e) {}
    /// Assignment operator
    CartesianStripY& operator=(const CartesianStripY& seg) = default;
    /// Equality operator
    bool operator==(const CartesianStripY& seg) const { return m_element == seg.m_element; }

    /// determine the position based on the cell ID
    Position position(const CellID& cellID) const;
    /// determine the cell ID based on the position
    CellID cellID(const Position& local, const Position& global, const VolumeID& volID) const;
    /// access the strip size in Y
    double stripSizeY() const;
    /// set the strip size in Y
    void setStripSizeY(double cellSize) const;
    /// access the coordinate offset in Y
    double offsetY() const;
    /// set the coordinate offset in Y
    void setOffsetY(double offset) const;
    /// access the field name used for Y
    const std::string& fieldNameY() const;
    /** \brief Returns a vector<double> of the cellDimensions of the given cell ID
        in natural order of dimensions, e.g., dx/dy/dz, or dr/r*dPhi

        Returns a vector of the cellDimensions of the given cell ID
        \param cellID is ignored as all cells have the same dimension
        \return std::vector<double> size 1:
        -# size in y
    */
    std::vector<double> cellDimensions(const CellID& cellID) const;
};
} /* End namespace dd4hep                */
#endif // DD4HEP_CARTESIANSTRIPY_H
