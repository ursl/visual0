#include "compound.hh"

#include "util.hh"
#include "json.h"

#include "TMath.h"
#include "TString.h"
#include "TH1.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

// ----------------------------------------------------------------------
compound::compound(string name, int position)
    : fSF(0.1), fAlpha(0.0), fOrthogonality(9999.), fName(name), fModuleNumber(0), fModulePosition(position) {
  cout << "ctor compound ->" << fName << "<- " << "inside constructor for object " << this << endl;
  TVector2 a(0., 0.);
  // pHDI = {a, a, a, a, a, a, a, a};
  // pHDIPrime = {a, a, a, a, a, a, a, a};
  pROCs = {a, a, a, a, a, a, a, a};
  pROCsPrime = {a, a, a, a, a, a, a, a};
  pACsPrime = {a, a, a, a, a, a, a, a};
  pMarkers = {a, a, a};
  pMarkersPrime = {a, a, a};
  fOffset = a;

  // -- extract module number from filename (e.g., "json/P1000.json" -> 1000)
  size_t pos = fName.find_last_of("/");
  if (pos != string::npos) {
    pos += 2; // skip the leading letter (P or p)
  } else {
    pos = 1; // no path, just skip the leading letter
  }
  string numStr = fName.substr(pos, 4);
  try {
    fModuleNumber = stoi(numStr);
  } catch (...) {
    fModuleNumber = 0;
  }
  cout << "fModuleNumber = " << fModuleNumber << endl;
}


// ----------------------------------------------------------------------
void compound::set(compound &other) {

  for (unsigned it = 0; it < other.pROCs.size(); ++it) {
    pROCs[it] = other.pROCs[it];
    pROCsPrime[it] = other.pROCsPrime[it];
  }

  for (unsigned it = 0; it < other.pACsPrime.size(); ++it) {
    pACsPrime[it] = other.pACsPrime[it];
  }

  for (unsigned it = 0; it < other.pMarkers.size(); ++it) {
    pMarkers[it] = other.pMarkers[it];
    pMarkersPrime[it] = other.pMarkersPrime[it];
  }

  fSF = other.fSF;
  fAlpha = other.fAlpha;
  fOffset = other.fOffset;
  fOrthogonality = other.fOrthogonality;
  fName = other.fName + string("_copy");
  fModuleNumber = other.fModuleNumber;
  fModulePosition = other.fModulePosition;
}


// ----------------------------------------------------------------------
void compound::parseJsonFile() {
  ifstream INS(fName);
  if (!INS.good()) {
    cerr << "Cannot open JSON file: " << fName << endl;
    return;
  }

  // -- Parse JSON using nlohmann::json
  nlohmann::json j;
  INS >> j;
  INS.close();

  fModuleNumber = j["moduleNumber"].get<int>();
  fModulePosition = j["modulePosition"].get<int>();

  // -- Map chip names to indices: chip00->0, chip01->1, chip10->2, chip11->3, chip20->4, chip21->5, chip30->6, chip31->7
  map<string, int> chipNameToIdx;
  chipNameToIdx["chip00"] = 0;
  chipNameToIdx["chip01"] = 1;
  chipNameToIdx["chip10"] = 2;
  chipNameToIdx["chip11"] = 3;
  chipNameToIdx["chip20"] = 4;
  chipNameToIdx["chip21"] = 5;
  chipNameToIdx["chip30"] = 6;
  chipNameToIdx["chip31"] = 7;

  // -- Parse hdiMarkers (expect 3 markers)
  if (j.contains("hdiMarkers") && j["hdiMarkers"].is_array()) {
    int markerIdx = 0;
    for (const auto& marker : j["hdiMarkers"]) {
      if (markerIdx >= 3) break;
      if (marker.contains("x") && marker.contains("y")) {
        double x = marker["x"].get<double>();
        double y = marker["y"].get<double>();
        pMarkers[markerIdx] = TVector2(x, y);
        markerIdx++;
      }
    }
    cout << "Parsed " << markerIdx << " HDI markers" << endl;
  } else {
    cerr << "JSON file missing or invalid 'hdiMarkers' array" << endl;
  }

  // -- Parse chipMarkers (expect 8 chips)
  if (j.contains("chipMarkers") && j["chipMarkers"].is_array()) {
    int chipCount = 0;
    for (const auto& chip : j["chipMarkers"]) {
      if (chip.contains("name") && chip.contains("x") && chip.contains("y")) {
        string chipName = chip["name"].get<string>();
        double x = chip["x"].get<double>();
        double y = chip["y"].get<double>();
        
        // Store in pROCs if we know this chip name
        if (chipNameToIdx.find(chipName) != chipNameToIdx.end()) {
          int idx = chipNameToIdx[chipName];
          pROCs[idx] = TVector2(x, y);
          chipCount++;
        }
      }
    }
    cout << "Parsed " << chipCount << " chip markers" << endl;
  } else {
    cerr << "JSON file missing or invalid 'chipMarkers' array" << endl;
  }
}


// ----------------------------------------------------------------------
void compound::parseSvgFile(int doParse) {

  int    VERBOSE(0);
  string bareName(fName);
  string csv1 = Form("%s.chips", bareName.c_str());
  string csv2 = Form("%s.hdi", bareName.c_str());
  string csv3 = Form("%s.marks", bareName.c_str());

  if (doParse) {
    system(
      Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s --query-id "
           "chip00,chip01,chip10,chip11,chip20,chip21,chip30,chip31 -X -Y > %s",
           fName.c_str(), csv1.c_str()));
  }

  if (doParse) {
    system(
      Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s --query-id "
           "HDI00,HDI01,HDI10,HDI11,HDI20,HDI21,HDI30,HDI31 -X -Y > %s",
           fName.c_str(), csv2.c_str()));
  }

  if (doParse) {
    system(Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s "
                "--query-id mark1,mark2,mark3 -X -Y > %s",
                fName.c_str(), csv3.c_str()));
  }

  ifstream INS;
  string   sline;

  // -- read 2 lines of chip coordinates
  cout << "read 1: chip coordinates ->" << csv1 << "<- ";
  INS.open(csv1);
  vector<string> vlines;
  while (getline(INS, sline)) {
    vlines.push_back(sline);
  }
  INS.close();

  vector<string> schipsX = split(vlines[0], ',');
  vector<string> schipsY = split(vlines[1], ',');
  cout << "schips.size() = " << schipsX.size() << "/" << schipsY.size() << endl;

  for (unsigned int i = 0; i < schipsX.size(); ++i) {
    pROCs[i] = TVector2(::stof(schipsX[i]), ::stof(schipsY[i]));
  }

  // // -- read 2 lines of HDI coordinates
  // cout << "read 2: HDI coordinates ->" << csv2 << "<- ";
  // INS.open(csv2);
  // vlines.clear();
  // while (getline(INS, sline)) {
  //   vlines.push_back(sline);
  // }
  // INS.close();

  // vector<string> shdiX = split(vlines[0], ',');
  // vector<string> shdiY = split(vlines[1], ',');
  // cout << "shdi.size() = " << shdiX.size() << "/" << shdiY.size() << endl;

  // for (unsigned int i = 0; i < shdiX.size(); ++i) {
  //   pHDI[i] = TVector2(::stof(shdiX[i]), ::stof(shdiY[i]));
  // }

  // -- read 2 lines of markers
  cout << "read 3: marker coordinates ->" << csv3 << "<- ";
  INS.open(csv3);
  vlines.clear();
  while (getline(INS, sline)) {
    vlines.push_back(sline);
  }
  INS.close();

  vector<string> smarkersX = split(vlines[0], ',');
  vector<string> smarkersY = split(vlines[1], ',');
  cout << "smarkers.size() = " << smarkersX.size() << "/" << smarkersY.size() << endl;

  for (unsigned int i = 0; i < smarkersX.size(); ++i) {
    pMarkers[i] = TVector2(::stof(smarkersX[i]), ::stof(smarkersY[i]));
  }

}


// ----------------------------------------------------------------------
TVector2 compound::transform(TVector2 r, double theta, TVector2 t) {
  TVector2 rp = r - t;
  double   xnew = TMath::Cos(theta) * rp.X() - TMath::Sin(theta) * rp.Y();
  double   ynew = TMath::Sin(theta) * rp.X() + TMath::Cos(theta) * rp.Y();
  return TVector2(xnew, ynew);
}


// ----------------------------------------------------------------------
void compound::transform(std::vector<TVector2> &orig,
                         std::vector<TVector2> &trsf) {
  for (unsigned it = 0; it < orig.size(); ++it) {
    trsf[it] = transform(orig[it], fAlpha, fOffset);
  }
}


// ----------------------------------------------------------------------
string compound::dump(TVector2 a) {
  return string(Form("%9.6f/%9.6f", a.X(), a.Y()));
}


// ----------------------------------------------------------------------
void compound::determineTransformation() {
  TVector2 ySvg = pMarkers[0] - pMarkers[1];
  ySvg /= ySvg.Mod();

  TVector2 xSvg = pMarkers[2] - pMarkers[1];
  xSvg /= xSvg.Mod();

  // -- this is a x-check on how well the markers have been measured
  fOrthogonality = 100. * TMath::ACos(xSvg * ySvg) / TMath::PiOver2();

  fOffset = pMarkers[1];

  // -- average rotation angle from  both x- and y-axes (always < pi/2)
  double theta0 = TMath::ACos(xSvg * TVector2(1., 0.));
  double theta1 = TMath::ACos(ySvg * TVector2(0., 1.));
  double alpha = 0.5 * (theta0 + theta1);

  // -- make sure rotation is towards y-axis
  TVector2 xOld = TVector2(1, 0);
  double   rsgn = xOld.X() * xSvg.Y() - xOld.Y() * xSvg.X();
  alpha -= TMath::Pi();
  fAlpha = -TMath::Pi() + (rsgn < 0 ? alpha : -1. * alpha);
}


// ----------------------------------------------------------------------
void compound::determineSF() {
  double dm0m1 = 23.0; // mm
  double dm1m2 = 40.0; // mm

  double pxDiff1 = (pMarkers[0] - pMarkers[1]).Mod();
  // cout << "pxDiff1 = " << pxDiff1 << " is 23mm, sf = " << dm0m1 / pxDiff1
  //      << endl;

  double pxDiff2 = (pMarkers[1] - pMarkers[2]).Mod();
  // cout << "pxDiff2 = " << pxDiff2 << " is 40mm, sf = " << dm1m2 / pxDiff2
  //      << endl;

  fSF = 0.5 * (dm0m1 / pxDiff1 + dm1m2 / pxDiff2);
  // cout << "fSF = " << fSF << endl;
}


// ----------------------------------------------------------------------
double compound::getChipWidth(int ichip) {
  return fSF * TMath::Abs(pROCsPrime[2*ichip].X() - pROCsPrime[2*ichip+1].X());
}


// ----------------------------------------------------------------------
double compound::getChipMarkerSeparation(int ichip0, int ichip1) {
  return fSF * TMath::Abs(pROCsPrime[ichip0].X() - pROCsPrime[ichip1].X());
}


// ----------------------------------------------------------------------
double compound::getMarkerDistance(std::string dir) {
  if (dir == "x") {
    return fSF * (pMarkersPrime[2] - pMarkersPrime[1]).Mod();
  }
  if (dir == "y") {
    return fSF * (pMarkersPrime[0] - pMarkersPrime[1]).Mod();
  }
  return 0;
}


// ----------------------------------------------------------------------
bool compound::chipWellMeasured(int iChip) {
  bool result(true); 

  if (pROCs[2*iChip].X() < 1.) result = false;
  if (pROCs[2*iChip].Y() < 1.) result = false;

  if (pROCs[2*iChip+1].X() < 1.) result = false;
  if (pROCs[2*iChip+1].Y() < 1.) result = false;

  if (!markersWellMeasured()) result = false;

  return result;
}


// ----------------------------------------------------------------------
bool compound::markersWellMeasured() {
  bool result(true); 
  if (pMarkers[0].X() < 1.) result = false;
  if (pMarkers[0].Y() < 1.) result = false;
  if (pMarkers[1].X() < 1.) result = false;
  if (pMarkers[1].Y() < 1.) result = false;
  if (pMarkers[2].X() < 1.) result = false;
  if (pMarkers[2].Y() < 1.) result = false;

  return result;
}


// ----------------------------------------------------------------------
double compound::getAngleChipMarkerHDIyAxis(int ichip0, int ichip1) {
  TVector2 chipVector = pROCsPrime[ichip0] - pROCsPrime[ichip1];
  return TMath::PiOver2() - TMath::ATan2(chipVector.Y(), chipVector.X());
}


// ----------------------------------------------------------------------
double compound::getAngleChipMarkerHDIxAxis(int ichip0, int ichip1) {
  TVector2 chipVector = pROCsPrime[ichip1] - pROCsPrime[ichip0];
  return TMath::ATan2(chipVector.Y(), chipVector.X());
}


// ----------------------------------------------------------------------
void compound::determineAlignmentCrosses() {
  // FIXME
  // -- Chip 0
  pACsPrime[0] =
      TVector2(pROCsPrime[0].X() + 6.75 - 30, pROCsPrime[0].Y() + 8.3);
  pACsPrime[1] =
      TVector2(pROCsPrime[1].X() + 6.75 + 165, pROCsPrime[1].Y() + 8.3);

  // -- Chip 1
  pACsPrime[2] =
      TVector2(pROCsPrime[2].X() + 6.75 - 30, pROCsPrime[2].Y() + 8.3);
  pACsPrime[3] =
      TVector2(pROCsPrime[3].X() + 6.75 + 165, pROCsPrime[3].Y() + 8.3);

  // -- Chip 2
  pACsPrime[4] =
      TVector2(pROCsPrime[4].X() + 6.75 + 165, pROCsPrime[4].Y() + 8.3);
  pACsPrime[5] =
      TVector2(pROCsPrime[5].X() + 6.75 - 30, pROCsPrime[5].Y() + 8.3);

  // -- Chip 3
  pACsPrime[6] =
      TVector2(pROCsPrime[6].X() + 6.75 + 165, pROCsPrime[6].Y() + 8.3);
  pACsPrime[7] =
      TVector2(pROCsPrime[7].X() + 6.75 - 30, pROCsPrime[7].Y() + 8.3);
}


// ----------------------------------------------------------------------
void compound::calcAll(int verbose, int doParse) {
  cout << "compound ->" << fName << "<- " << "inside calcAll() for object " << this << endl;
  if (fName.find(".json") != string::npos) {
    parseJsonFile();
  } else {
    parseSvgFile(doParse);
  }
  determineSF();
  determineTransformation();
  transform(pMarkers, pMarkersPrime);
  // transform(pHDI, pHDIPrime);
  transform(pROCs, pROCsPrime);
  determineAlignmentCrosses();
  dump();
}


// ----------------------------------------------------------------------
void compound::dump() {
  cout << "compound ->" << fName << "<- alpha = " << fAlpha << " offset = ("
       << fOffset.X() << "/" << fOffset.Y() << ")"
       << " orthogonality = " << fOrthogonality  << "%"
       << " SF = " << fSF << "mm/px" << " at address " << this << endl;

  cout << "Markers:        ";
  for (int i = 0; i < pMarkers.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pMarkers[i].X(), pMarkers[i].Y());
    if (i < 2)
      cout << ", ";
  }
  cout << endl;

  // cout << "HDI:            ";
  // for (int i = 0; i < pHDI.size(); ++i) {
  //   cout << Form("(%8.3f/%8.3f)", pHDI[i].X(), pHDI[i].Y());
  //   if (i < 7)
  //     cout << ", ";
  // }
  // cout << endl;

  cout << "ROCs:           ";
  for (int i = 0; i < pROCs.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pROCs[i].X(), pROCs[i].Y());
    if (i < 7)
      cout << ", ";
  }
  cout << endl;

  cout << "MarkersPrime:   ";
  for (int i = 0; i < pMarkersPrime.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pMarkersPrime[i].X(), pMarkersPrime[i].Y());
    if (i < 2)
      cout << ", ";
  }
  cout << endl;

  // cout << "HDIPrime:       ";
  // for (int i = 0; i < pHDIPrime.size(); ++i) {
  //   cout << Form("(%8.3f/%8.3f)", pHDIPrime[i].X(), pHDIPrime[i].Y());
  //   if (i < 7)
  //     cout << ", ";
  // }
  // cout << endl;

  cout << "ROCsPrime:      ";
  for (int i = 0; i < pROCsPrime.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pROCsPrime[i].X(), pROCsPrime[i].Y());
    if (i < 7)
      cout << ", ";
  }
  cout << endl;

  cout << "Test chip dimensions? (Correct answer: 21.500mm)" << endl;
  TH1D *h0 = new TH1D("hdr", "hdr", 40, 21.4, 21.8);
  for (int i = 0; i < 4; ++i) {
    double diff = fSF * TMath::Abs((pROCsPrime[2 * i + 1].X() - pROCsPrime[2 * i].X()));
    cout << "chip " << i << ": "
         << TMath::Abs(pROCsPrime[2 * i + 1].X() - pROCsPrime[2 * i].X())
         << " px, "
         << diff
         << " mm" << endl;
    h0->Fill(diff);
  }
  cout << "RMS = " << h0->GetRMS() << "mm" << endl;
  delete h0;


  // cout << "Test HDI marker precision" << endl;
  // TH1D *h1 = new TH1D("hdi", "hdi", 40, 20.4, 20.6);
  // for (int i = 0; i < 4; ++i) {
  //   double diff = fSF * TMath::Abs((pHDIPrime[2 * i + 1].X() - pHDIPrime[2 * i].X())); 
  //   cout << "HDI " << i << ": "
  //        << TMath::Abs(pHDIPrime[2 * i + 1].X() - pHDIPrime[2 * i].X())
  //        << " px, "
  //        << diff
  //        << " mm" << endl;
  //        h1->Fill(diff);
  // }
  // cout << "RMS = " << h1->GetRMS() << "mm" << endl;
  // delete h1;
}


// ----------------------------------------------------------------------
bool exists_test0(const std::string& name) {
  ifstream f(name.c_str());
  return f.good();
}
