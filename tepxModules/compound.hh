#ifndef COMPOUND_HH
#define COMPOUND_HH

#include "TVector2.h"

#include <vector>

// ----------------------------------------------------------------------
// compound -- describes HDI measurements 
// ======================================
//
// Orientation of frames (SVG and HDI, primed) and marker numbering (M0, M1, M2)
//
// +--->x(SVG)
// |
// |
// v
// y(SVG)
//
//       +-------------------------------+
//       |                               |
//  +----+                       M0      |
//  |                                    |
//  |                            ^ y'    |
//  |                            |       |
//  +----+    M2              <- M1      |
//       |                    x'         |
//       +-------------------------------+
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
class compound {
  public:
  compound(std::string name = "unset");

  // -- set (circumvent [past?] issues with copy c'tor)
  void set(compound &c);

  // -- calculate all - call this before doing anything else!
  void calcAll(int verbose = 0, int doParse = 1);

  // -- read information
  void parseSvgFile(int doParse = 1);
  void parseJsonFile();

  // -- calculate scale factor (pixel to mm)
  void determineSF();
  // -- calculate fOffset and fAlpha
  void determineTransformation();
  // -- calculate position of alignment crosses (markers) on chips
  void determineAlignmentCrosses();
  // -- calculate coordinates in HDI frame (starting from coordinates in SVG frame)
  TVector2 transform(TVector2 r, double theta, TVector2 t);
  void     transform(std::vector<TVector2> &orig, std::vector<TVector2> &trsf);

  // -- get chip width
  double getChipWidth(int ichip);
  // -- get chip marker separation
  double getChipMarkerSeparation(int ichip0, int ichip1);
  // -- get marker distance
  double getMarkerDistance(std::string dir = "x");
  // -- check if chip is well measured
  bool chipWellMeasured(int iChip);
  // -- check if markers are well measured
  bool markersWellMeasured();

  // -- get module number
  int getModuleNumber() {return fModuleNumber;}
  // -- get module position
  int getModulePosition() {return fModulePosition;}

  // -- setters
  void setSF(double sf) {fSF = sf;}
  void setAlpha(double alpha) {fAlpha = alpha;}
  void setOrthogonality(double orthogonality) {fOrthogonality = orthogonality;}
  void setOffset(TVector2 offset) {fOffset = offset;}
  void setName(std::string name) {fName = name;}

  // -- getters
  double getSF() {return fSF;}
  double getAlpha() {return fAlpha;}
  double getOrthogonality() {return fOrthogonality;}
  TVector2 getOffset() {return fOffset;}
  std::string getName() {return fName;}

  // TVector2 getHDI(int i) {return pHDI[i];}
  // TVector2 getHDIPrime(int i) {return pHDIPrime[i];}
  TVector2 getROCs(int i) {return pROCs[i];}
  TVector2 getROCsPrime(int i) {return pROCsPrime[i];}
  TVector2 getACsPrime(int i) {return pACsPrime[i];}
  TVector2 getMarkers(int i) {return pMarkers[i];}
  TVector2 getMarkersPrime(int i) {return pMarkersPrime[i];}

  // -- print it
  void dump();
  // -- cast a TVector2 into a string
  std::string dump(TVector2 a);

private:
  // -- 8 points for HDI. SVG coordinates and primed coordinates
  // std::vector<TVector2> pHDI, pHDIPrime;
  // 8 points for ROCs. SVG coordinates and primed coordinates
  std::vector<TVector2> pROCs, pROCsPrime;
  // 8 alignment crosses for ROCs. SVG coordinates and primed coordinates
  // Note: This is only calculated/filled for the primed (HDI) coordinates
  std::vector<TVector2> pACsPrime;
  // 3 points for HDI markers. SVG coordinates and primed coordinates
  std::vector<TVector2> pMarkers, pMarkersPrime;
  // convert pixel (distance) to mm (distance)
  double fSF;
  double fAlpha;
  double fOrthogonality;
  TVector2 fOffset;
  std::string fName;
  int fModuleNumber, fModulePosition;
};

bool exists_test0(const std::string& name);

#endif