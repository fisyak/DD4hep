//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : F.Gaede
//
//==========================================================================
#include "DDRec/MaterialManager.h"
#include "DD4hep/Exceptions.h"
#include "DD4hep/Detector.h"

#include "TGeoVolume.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TVirtualGeoTrack.h"

#define MINSTEP 1.e-5

namespace dd4hep {
  namespace rec {

    MaterialManager::MaterialManager(Volume world) : _mV(0), _m( Material() ), _p0(),_p1(),_pos() {
      _tgeoMgr = world->GetGeoManager();
    }
    
    MaterialManager::~MaterialManager(){
      
    }
    
    const PlacementVec& MaterialManager::placementsBetween(const Vector3D& p0, const Vector3D& p1 , double epsilon) {
      materialsBetween(p0,p1,epsilon);
      return _placeV;
    }

    const MaterialVec& MaterialManager::materialsBetween(const Vector3D& p0, const Vector3D& p1 , double epsilon) {
      if( ( p0 != _p0 ) || ( p1 != _p1 ) ) {	
        // A backup is needed to restore the state of the navigator after the track is done
        // see https://github.com/AIDASoft/DD4hep/issues/1413
        _tgeoMgr->DoBackupState();
        //---------------------------------------	
        _mV.clear() ;
        _placeV.clear();
        //
        // algorithm copied from TGeoGearDistanceProperties.cc (A.Munnich):
        // 
	
        double startpoint[3], endpoint[3], direction[3];
        double L=0;
        for(unsigned int i=0; i<3; i++) {
          startpoint[i] = p0[i];
          endpoint[i]   = p1[i];
          direction[i] = endpoint[i] - startpoint[i];
          L+=direction[i]*direction[i];
        }
        double totDist = sqrt( L ) ;
	
        //normalize direction
        for(unsigned int i=0; i<3; i++)
          direction[i]=direction[i]/totDist;
	
        _tgeoMgr->AddTrack(0, 12 ) ; // electron neutrino

        TGeoNode *node1 = _tgeoMgr->InitTrack(startpoint, direction);

        //check if there is a node at startpoint
        if(!node1)
          throw std::runtime_error("No geometry node found at given location. Either there is no node placed here or position is outside of top volume.");

        while ( !_tgeoMgr->IsOutside() )  {
	  
          // TGeoNode *node2;
          // TVirtualGeoTrack *track; 
	  
          // step to (and over) the next Boundary
          TGeoNode * node2 = _tgeoMgr->FindNextBoundaryAndStep( 500, 1) ;
	  
          if( !node2 || _tgeoMgr->IsOutside() )
            break;
	  
          const double *position    =  _tgeoMgr->GetCurrentPoint();
          const double *previouspos =  _tgeoMgr->GetLastPoint();
	  
          double length = _tgeoMgr->GetStep();

          TVirtualGeoTrack *track = _tgeoMgr->GetLastTrack();

          //protection against infinitive loop in root which should not happen, but well it does...
          //work around until solution within root can be found when the step gets very small e.g. 1e-10
          //and the next boundary is never reached
 	  
#if 1   //fg: is this still needed ?
          if( length < MINSTEP ) {
	    
            _tgeoMgr->SetCurrentPoint( position[0] + MINSTEP * direction[0], 
                                       position[1] + MINSTEP * direction[1], 
                                       position[2] + MINSTEP * direction[2] );
	    
            length = _tgeoMgr->GetStep();
            node2  = _tgeoMgr->FindNextBoundaryAndStep(500, 1) ;
	    
            position    = _tgeoMgr->GetCurrentPoint();
            previouspos = _tgeoMgr->GetLastPoint();
          }
#endif 	  
          //	printf( " --  step length :  %1.8e %1.8e   %1.8e   %1.8e   %1.8e   %1.8e   %1.8e   - %s \n" , length ,
          //		position[0], position[1], position[2], previouspos[0], previouspos[1], previouspos[2] , node1->GetMedium()->GetMaterial()->GetName() ) ;
	  
          Vector3D posV( position ) ;
	  
          double currDistance = ( posV - p0 ).r() ;
	  
          // //if the next boundary is further than end point
          //  if(fabs(position[0])>fabs(endpoint[0]) || fabs(position[1])>fabs(endpoint[1]) 
          //  || fabs(position[2])>fabs(endpoint[2]))
	  
          //if we travelled too far:
          if( currDistance > totDist  ) {
	    
            length = sqrt( pow(endpoint[0]-previouspos[0],2) + 
                           pow(endpoint[1]-previouspos[1],2) +
                           pow(endpoint[2]-previouspos[2],2)   );
	    
            track->AddPoint( endpoint[0], endpoint[1], endpoint[2], 0. );
	    
	    
            if( length > epsilon )   {
              _mV.emplace_back(node1->GetMedium(), length ); 
              _placeV.emplace_back(node1,length);
            }
            break;
          }
	  
          track->AddPoint( position[0], position[1], position[2], 0.);
	  
          if( length > epsilon )   {
            _mV.emplace_back(node1->GetMedium(), length); 
            _placeV.emplace_back(node1,length);
          }
          node1 = node2;
        }
	

        //fg: protect against empty list:
        if( _mV.empty() ){
          _mV.emplace_back(node1->GetMedium(), totDist); 
          _placeV.emplace_back(node1,totDist);
        }


        _tgeoMgr->ClearTracks();

        _tgeoMgr->CleanGarbage();

        _tgeoMgr->DoRestoreState();
	
        //---------------------------------------	
	
        _p0 = p0 ;
        _p1 = p1 ;
      }

      return _mV ;
    }

    
    const Material& MaterialManager::materialAt(const Vector3D& pos )   {
      if( pos != _pos ) {
        TGeoNode *node = _tgeoMgr->FindNode( pos[0], pos[1], pos[2] ) ;	
        if( ! node ) {
          std::stringstream err ;
          err << " MaterialManager::material: No geometry node found at location: " << pos ;
          throw std::runtime_error( err.str() );
        }
        _m = Material( node->GetMedium() );
        _pv = node;
        _pos = pos ;
      }
      return _m ;
    }
    
    PlacedVolume MaterialManager::placementAt(const Vector3D& pos )   {
      if( pos != _pos ) {	
        TGeoNode *node = _tgeoMgr->FindNode( pos[0], pos[1], pos[2] ) ;	
        if( ! node ) {
          std::stringstream err ;
          err << " MaterialManager::material: No geometry node found at location: " << pos ;
          throw std::runtime_error( err.str() );
        }
        _m = Material( node->GetMedium() );
        _pv = node;
        _pos = pos;
      }
      return _pv;
    }
    
    MaterialData MaterialManager::createAveragedMaterial( const MaterialVec& materials ) {
      
      std::stringstream sstr ;
      
      double sum_l = 0 ;
      double sum_rho_l = 0 ;
      double sum_rho_l_over_A = 0 ;
      double sum_rho_l_Z_over_A = 0 ;
      //double sum_rho_l_over_x = 0 ;
      double sum_l_over_x = 0 ;
      //double sum_rho_l_over_lambda = 0 ;
      double sum_l_over_lambda = 0 ;

      for(unsigned i=0,n=materials.size(); i<n ; ++i){

        Material mat = materials[i].first ;
        double   l   = materials[i].second ;

        if( i != 0 ) sstr << "_" ; 
        sstr << mat.name() << "_" << l ;

        double rho      = mat.density() ;
        double  A       = mat.A() ;
        double  Z       = mat.Z() ;
        double  x       = mat.radLength() ;
        double  lambda  = mat.intLength() ;
	
        sum_l                 +=   l ;
        sum_rho_l             +=   rho * l  ;
        sum_rho_l_over_A      +=   rho * l / A ;
        sum_rho_l_Z_over_A    +=   rho * l * Z / A ;
        sum_l_over_x          +=   l / x ;
        sum_l_over_lambda     +=   l / lambda ;
        // sum_rho_l_over_x      +=   rho * l / x ;
        // sum_rho_l_over_lambda +=   rho * l / lambda ;
      }

      double rho      =  sum_rho_l / sum_l ;

      double  A       =  sum_rho_l / sum_rho_l_over_A ;
      double  Z       =  sum_rho_l_Z_over_A / sum_rho_l_over_A ;

      // radiation and interaction lengths already given in cm - average by length
     
      // double  x       =  sum_rho_l / sum_rho_l_over_x ;
      double  x       =  sum_l / sum_l_over_x ;

      //     double  lambda  =  sum_rho_l / sum_rho_l_over_lambda ;
      double  lambda  =  sum_l / sum_l_over_lambda ;

     
      return MaterialData( sstr.str() , Z, A, rho, x, lambda ) ;

    }
    
  } /* namespace rec */
} /* namespace dd4hep */
