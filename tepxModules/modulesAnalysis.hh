#ifndef MODULESANALYSIS_HH
#define MODULESANALYSIS_HH

#include "moduleMeasurement.hh"

#include <vector>
// ----------------------------------------------------------------------
class modulesAnalysis {
  public:
  modulesAnalysis(int mode = 1);
  
  void calcAll();
  void anaAll();

  private:
  std::vector<moduleMeasurement> fModules;
  std::string fFilename;
};

#endif
