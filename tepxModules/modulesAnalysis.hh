#ifndef MODULESANALYSIS_HH
#define MODULESANALYSIS_HH

#include "moduleMeasurement.hh"

#include "TFile.h"
#include "TH1.h"
#include "TProfile.h"
#include <vector>
#include <map>

// ---------------------------------------------------------------------- 
class modulesAnalysis {
  public:
  modulesAnalysis(int mode = 1, std::string directory = "pdf");
  ~modulesAnalysis();

  void bookHistograms();
  void doAll();
  void calcAll();
  void anaAll();
  void plotAll();
  void plotGlueTests();

  bool chipWellMeasured(compound &c, int index);

  private:
  std::vector<moduleMeasurement*> fModules;
  std::string fFilename, fDirectory;
  TFile *fFile;
  std::map<std::string, TH1 *> fHists;
  std::map<std::string, TProfile *> fProfiles;
};

#endif
