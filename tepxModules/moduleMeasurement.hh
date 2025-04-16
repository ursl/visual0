#ifndef MODULEMEASUREMENT_HH
#define MODULEMEASUREMENT_HH

#include "compound.hh"

#include "TVector2.h"


// ----------------------------------------------------------------------
class moduleMeasurement {
  public:
  moduleMeasurement(std::string filename = "nada", int position = 0);
  
  void calcAll(int verbose = 0, int doParse = 1);

  // -- overall transformation from SVG CS to HDI CS
  TVector2 transform(TVector2 r, double theta, TVector2 t);
  // -- rotation transformation, used by transform()
  TVector2 rotate(TVector2 r, double theta);
  // -- get chip width
  double getChipWidth(int ichip);
  // -- get marker distance
  double getMarkerDistance(std::string dir = "x");
  
  compound &getCompound() {return fCompound;}

  // -- dump
  void dump() {fCompound.dump();}
  
  // -- variables
  std::string getFileName() {return fFilename;}
  void setFileName(std::string name);
  void testCoordinates(int mode = 1); // COMPLETELY DECREPIT!!!!
  std::string dump(TVector2);
  TVector2 PerpAntiClockwise(TVector2);
  TVector2 PerpClockwise(TVector2);

  private:
  compound fCompound;
  std::string fFilename;
  int fPosition;
  int fIndex;
};

#endif
