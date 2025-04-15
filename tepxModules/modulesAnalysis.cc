#include "modulesAnalysis.hh"
#include "moduleMeasurement.hh"

modulesAnalysis::modulesAnalysis(int mode) {
  if (1 == mode) {
    // -- manually setup list of modules
    fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1000.svg"));
    // fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1001.svg"));
    // fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1002.svg"));
    // fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1003.svg"));
    // fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1004.svg"));
    // fModules.push_back(moduleMeasurement("~/inkscape/tepx-modules/P1005.svg"));
   }

}

void modulesAnalysis::calcAll() {
  for (auto module : fModules) {
    module.calcAll();
  }
}

void::modulesAnalysis::anaAll() {
}