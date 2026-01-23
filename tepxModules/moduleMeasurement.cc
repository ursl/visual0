#include "moduleMeasurement.hh"

#include "TMath.h"
#include "TString.h"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
moduleMeasurement::moduleMeasurement(string filename, int position) : fCompound(filename), fFilename(filename), fPosition(position) {
  // -- extract index from filename
  fIndex = filename.find_last_of("/") + 2; // assume single-letter offset for index
  fIndex = stoi(filename.substr(fIndex, 4)); // assume 4-digit index
  cout << "fIndex = " << fIndex << endl;
}


// ----------------------------------------------------------------------
void moduleMeasurement::setFileName(string filename) {
  fFilename = filename;
  fCompound.setName(filename);
  
  // -- extract index from filename
  fIndex = filename.find_last_of("/") + 2; // assume single-letter offset for index
  fIndex = stoi(filename.substr(fIndex, 4));
  cout << "fIndex = " << fIndex << endl;
}

// ----------------------------------------------------------------------
void moduleMeasurement::calcAll(int verbose, int doParse) {
  fCompound.calcAll(verbose, doParse);
}

// ----------------------------------------------------------------------
double moduleMeasurement::getChipWidth(int ichip) {
  return fCompound.getSF() * TMath::Abs(fCompound.getROCsPrime(2*ichip).X() - fCompound.getROCsPrime(2*ichip+1).X());
  // return fCompound.getSF() 
  //        * TMath::Sqrt(TMath::Power(fCompound.getROCsPrime(2*ichip).X() - fCompound.getROCsPrime(2*ichip+1).X(), 2) 
  //                    + TMath::Power(fCompound.getROCsPrime(2*ichip).Y() - fCompound.getROCsPrime(2*ichip+1).Y(), 2));
}

// ----------------------------------------------------------------------
double moduleMeasurement::getChipMarkerSeparation(int ichip0, int ichip1) {
  return fCompound.getSF() * TMath::Abs(fCompound.getROCsPrime(ichip0).X() - fCompound.getROCsPrime(ichip1).X());
  // return fCompound.getSF() 
  //       * TMath::Sqrt(TMath::Power(fCompound.getROCsPrime(2*ichip0).X() - fCompound.getROCsPrime(2*ichip1).X(), 2) 
  //                   + TMath::Power(fCompound.getROCsPrime(2*ichip0).Y() - fCompound.getROCsPrime(2*ichip1).Y(), 2));
}

// ----------------------------------------------------------------------
double moduleMeasurement::getMarkerDistance(std::string dir) {
  if (dir == "x") {
    return fCompound.getSF() * (fCompound.getMarkersPrime(2) - fCompound.getMarkersPrime(1)).Mod();
  }
  if (dir == "y") {
    return fCompound.getSF() * (fCompound.getMarkersPrime(0) - fCompound.getMarkersPrime(1)).Mod();
  }
  return 0;
}

// ----------------------------------------------------------------------
TVector2 moduleMeasurement::rotate(TVector2 r, double theta) {
  double x = TMath::Cos(theta) * r.X() - TMath::Sin(theta) * r.Y();
  double y = TMath::Sin(theta) * r.X() + TMath::Cos(theta) * r.Y();
  return TVector2(x, y);
}

// ----------------------------------------------------------------------
TVector2 moduleMeasurement::transform(TVector2 r, double theta, TVector2 t) {
  TVector2 rp = r - t;
  double xnew = TMath::Cos(theta) * rp.X() - TMath::Sin(theta) * rp.Y();
  double ynew = TMath::Sin(theta) * rp.X() + TMath::Cos(theta) * rp.Y();
  return TVector2(xnew, ynew);
}

// ----------------------------------------------------------------------
TVector2 moduleMeasurement::PerpClockwise(TVector2 vector2) {
    return TVector2(vector2.Y(), -vector2.X());
}

// ----------------------------------------------------------------------
TVector2 moduleMeasurement::PerpAntiClockwise(TVector2 vector2) {
    return TVector2(-vector2.Y(), vector2.X());
}

// ----------------------------------------------------------------------
string moduleMeasurement::dump(TVector2 a) {
  return string(Form("%9.6f/%9.6f", a.X(), a.Y()));
}
