#include "DD4hep/DDTest.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <exception>

#include "DD4hep/Plugins.h"
#include "DD4hep/Primitives.h"
#include "DDG4/Geant4InputAction.h"
#include "DDG4/Geant4Particle.h"
#include "DDG4/Geant4Vertex.h"

//using namespace dd4hep::sim;
typedef dd4hep::sim::Geant4Vertex   Vertex;
typedef dd4hep::sim::Geant4Particle Particle;

static dd4hep::DDTest test( "EventReader" ) ;

class TestTuple {
public:
  std::string readerType;
  std::string inputFile;
  bool skipEOF; //LCIO skipNEvents does not signal end of file??
  TestTuple( std::string const& rT, std::string const& iF, bool sEOF=false): readerType(rT), inputFile(iF), skipEOF(sEOF) {}
};


int main(int argc, char** argv ){

  if( argc < 2 ) {
    std::cout << " usage:  test_EventReaders Path/To/InputFiles " << std::endl ;
    exit(1) ;
  }

  std::string inputFileFolder = argv[1];

  std::vector<TestTuple> tests;
  #ifdef DD4HEP_USE_LCIO
  tests.push_back( TestTuple( "LCIOStdHepReader", "bbudsc_3evt.stdhep" ) );
  tests.push_back( TestTuple( "LCIOFileReader",   "muons.slcio" , /*skipEOF= */ true ) );
  #endif
  tests.push_back( TestTuple( "Geant4EventReaderHepEvtShort", "Muons10GeV.HEPEvt" ) );
  #ifdef DD4HEP_USE_HEPMC3
  tests.push_back( TestTuple( "HEPMC3FileReader", "g4pythia.hepmc", /*skipEOF= */ true) );
  tests.push_back( TestTuple( "HEPMC3FileReader", "Pythia_output.hepmc", /*skipEOF= */ true) );
  #endif
  #ifdef DD4HEP_USE_EDM4HEP
  tests.push_back( TestTuple( "EDM4hepFileReader", "ZH250_ISR.edm4hep.root", /*skipEOF= */ true) );
  #endif

  try{
    for(std::vector<TestTuple>::const_iterator it = tests.begin(); it != tests.end(); ++it) {
      std::string readerType = (*it).readerType;
      std::string fileName = (*it).inputFile;
      bool skipEOF =  (*it).skipEOF;
      //InputFiles are in DDTest/inputFiles, argument is cmake_source directory
      std::string inputFile = argv[1]+ std::string("/inputFiles/") + fileName;
      dd4hep::sim::Geant4EventReader* thisReader = dd4hep::PluginService::Create<dd4hep::sim::Geant4EventReader*>(readerType, inputFile);
      if ( not thisReader ) {
        test.log( "Plugin not found" );
        test.log( readerType );
        continue;
      }
      test( thisReader->currentEventNumber() == 0 , readerType + std::string("Initial Event Number") );
      if (!thisReader->hasDirectAccess()) {
        thisReader->moveToEvent(1);
        test( thisReader->currentEventNumber() == 1 , readerType + std::string("Event Number after Skip") );
      }
      std::vector<Particle*> particles;
      std::vector<Vertex*> vertices ;
      dd4hep::sim::Geant4EventReader::EventReaderStatus sc = thisReader->readParticles(2,vertices,particles);
      std::for_each(particles.begin(),particles.end(),dd4hep::detail::deleteObject<Particle>);
      test( thisReader->currentEventNumber() == 2 && sc == dd4hep::sim::Geant4EventReader::EVENT_READER_OK,
            readerType + std::string("Event Number Read") );

      //Reset Reader to check what happens if moving too far in the file
      if (not skipEOF) {
        thisReader = dd4hep::PluginService::Create<dd4hep::sim::Geant4EventReader*>(readerType, std::move(inputFile));
        sc = thisReader->moveToEvent(1000000);
        test( sc != dd4hep::sim::Geant4EventReader::EVENT_READER_OK , readerType + std::string("EventReader False") );
      }
    }
  } catch( std::exception &e ){
    test.error("Exception occurred:");
    test.log(e.what());
  }
  return 0;
}

