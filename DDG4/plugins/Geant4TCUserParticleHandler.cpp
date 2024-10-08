//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

/** \addtogroup Geant4Action
 *
 @{
   \package Geant4TCUserParticleHandler
 * \brief Rejects to keep particles, which are created outside a tracking cylinder.
 *
 *
@}
 */

#ifndef DD4HEP_DDG4_GEANT4TCUSERPARTICLEHANDLER_H
#define DD4HEP_DDG4_GEANT4TCUSERPARTICLEHANDLER_H

// Framework include files
#include <DD4hep/Primitives.h>
#include <DDG4/Geant4UserParticleHandler.h>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
  namespace sim {

    ///  Rejects to keep particles, which are created outside a tracking cylinder.
    /** Geant4TCUserParticleHandler
     *
     *  TC stands for TrackingCylinder ;-)
     *
     * @author  M.Frank
     * @version 1.0
     */
    class Geant4TCUserParticleHandler : public Geant4UserParticleHandler  {
      double m_zTrackerMin, m_zTrackerMax, m_rTracker;
    public:
      /// Standard constructor
      Geant4TCUserParticleHandler(Geant4Context* context, const std::string& nam);

      /// Default destructor
      virtual ~Geant4TCUserParticleHandler() {}

      /// Post-track action callback
      /** Allow the user to force the particle handling in the post track action
       *  set the reason mask to NULL in order to drop the particle.
       *  The parent's reasoning mask will be or'ed with the particle's mask
       *  to preserve the MC truth for the hit creation.
       *  The default implementation is empty.
       *
       *  Note: The particle passed is a temporary and will be copied if kept.
       */
      virtual void end(const G4Track* track, Particle& particle);

      /// Post-event action callback: avoid warning (...) was hidden [-Woverloaded-virtual]
      virtual void end(const G4Event* event);

    };
  }    // End namespace sim
}      // End namespace dd4hep

#endif // DD4HEP_DDG4_GEANT4TCUSERPARTICLEHANDLER_H

//====================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
// Framework include files
//#include <DDG4/Geant4TCUserParticleHandler.h>
#include <DDG4/Geant4Particle.h>
#include <DDG4/Factories.h>
#include "Geant4UserParticleHandlerHelper.h"


using namespace dd4hep::sim;
DECLARE_GEANT4ACTION(Geant4TCUserParticleHandler)

/// Standard constructor
Geant4TCUserParticleHandler::Geant4TCUserParticleHandler(Geant4Context* ctxt, const std::string& nam)
: Geant4UserParticleHandler(ctxt,nam)
{
  declareProperty("TrackingVolume_Zmin",m_zTrackerMin=-1e100);
  declareProperty("TrackingVolume_Zmax",m_zTrackerMax=1e100);
  declareProperty("TrackingVolume_Rmax",m_rTracker=1e100);
}

/// Post-track action callback
void Geant4TCUserParticleHandler::end(const G4Track* /* track */, Particle& p)  {

  double r_prod = std::sqrt(p.vsx*p.vsx + p.vsy*p.vsy);
  double z_prod = p.vsz;
  bool starts_in_trk_vol = ( r_prod <= m_rTracker
    && z_prod >= (m_zTrackerMin == -1e100? -m_zTrackerMax : m_zTrackerMin)
    && z_prod <= m_zTrackerMax
  )  ;

  double r_end  = std::sqrt(p.vex*p.vex + p.vey*p.vey);
  double z_end  = p.vez;
  bool ends_in_trk_vol =  ( r_end <= m_rTracker
     && z_end >= (m_zTrackerMin == -1e100? -m_zTrackerMax : m_zTrackerMin)
     && z_end <= m_zTrackerMax
  ) ;

  setReason(p, starts_in_trk_vol, ends_in_trk_vol);
  setSimulatorStatus(p, starts_in_trk_vol, ends_in_trk_vol);
}

/// Post-event action callback
void Geant4TCUserParticleHandler::end(const G4Event* /* event */)   {

}

