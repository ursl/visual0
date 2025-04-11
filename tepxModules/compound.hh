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
struct compound {
  compound(std::string name = "unset");

  // -- set (circumvent [past?] issues with copy c'tor)
  void set(compound &c);

  // -- calculate all
  void calcAll();
  // -- read information
  void parseSvgFile();
  // -- calculate scale factor (pixel to mm)
  void determineSF();
  // -- calculate fOffset and fAlpha
  void determineTransformation();
  // -- calculate position of alignment crosses (markers) on chips
  void determineAlignmentCrosses();
  // -- calculate coordinates in HDI frame (starting from coordinates in SVG frame)
  TVector2 transform(TVector2 r, double theta, TVector2 t);
  void     transform(std::vector<TVector2> &orig, std::vector<TVector2> &trsf);

  // -- print it
  void dump();
  // -- cast a TVector2 into a string
  std::string dump(TVector2 a);

  // -- 8 points for HDI. SVG coordinates and primed coordinates
  std::vector<TVector2> pHDI, pHDIPrime;
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
};

#endif