#ifndef MODULEMEASUREMENT_HH
#define MODULEMEASUREMENT_HH

#include "compound.hh"

#include "TVector2.h"


// ----------------------------------------------------------------------
class moduleMeasurement {
  public:
  moduleMeasurement(std::string filename = "nada");
  
  void calcAll();

  // -- overall transformation from SVG CS to HDI CS
  TVector2 transform(TVector2 r, double theta, TVector2 t);
  // -- rotation transformation, used by transform()
  TVector2 rotate(TVector2 r, double theta);
    
  // -- varia
  void setFileName(std::string name);
  void testCoordinates(int mode = 1); // COMPLETELY DECREPIT!!!!
  std::string dump(TVector2);
  TVector2 PerpAntiClockwise(TVector2);
  TVector2 PerpClockwise(TVector2);

  private:
  compound fCompound;
  std::string fFilename;
};

#endif
