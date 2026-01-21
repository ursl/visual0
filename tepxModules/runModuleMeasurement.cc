#include "moduleMeasurement.hh"
#include "modulesAnalysis.hh"
#include <iostream>
#include <string>
#include <vector>
#include <glob.h>


using namespace std;
 
// ----------------------------------------------------------------------
// moor>./bin/runModuleMeasurement -m 10
// moor>./bin/runModuleMeasurement -m 2
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  // -- command line arguments
  int mode(0), verbose(0);
  string fileName("bla");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-f"))   {fileName = argv[++i];}
    if (!strcmp(argv[i], "-m"))   {mode = atoi(argv[++i]);}
  }
 
  // -- run analysis on processed image files (either legacy CSV or JSON)
  if (mode < 10) {
    modulesAnalysis ma(mode);
    ma.doAll();
    if (1 == mode) ma.plotGlueTests();
    return 0;
  }

  // -- process images
  if (mode >= 10) {
    // -- Find all files matching pattern "modules/p????.jpg"
    vector<string> imageFiles;
    glob_t globResult;
    int globRet = glob("modules/p????.jpg", GLOB_TILDE, NULL, &globResult);
    
    if (globRet == 0) {
      for (size_t i = 0; i < globResult.gl_pathc; ++i) {
        imageFiles.push_back(string(globResult.gl_pathv[i]));
      }
    }
    globfree(&globResult);
        
    // -- Loop over all found files
    for (const auto& imgFile : imageFiles) {
      cout << "Processing: " << imgFile << endl;
      system(("bin/ocvMarkersFromJPG -f " + imgFile).c_str());
    }
    return 0;
  }

  moduleMeasurement m(fileName, 0);
  if (fileName != "bla") {
    m.setFileName(fileName);
    m.calcAll();
  } 
  return 0;
 }
