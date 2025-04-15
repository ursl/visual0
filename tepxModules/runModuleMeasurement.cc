#include "moduleMeasurement.hh"
#include "modulesAnalysis.hh"
#include <iostream>
#include <string>


using namespace std;
 
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  // -- command line arguments
  int mode(0), run(3), verbose(0);
  string fileName("bla");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-f"))   {fileName = argv[++i];}
    if (!strcmp(argv[i], "-m"))   {mode = atoi(argv[++i]);}
  }
 
  if (1 == mode) {
    modulesAnalysis ma(1);
    ma.calcAll();
  }

  moduleMeasurement m(fileName);
  if (fileName != "bla") {
    m.setFileName(fileName);
    m.calcAll();
  } else {
    m.testCoordinates(1);
    m.testCoordinates(2);
  }

 }
