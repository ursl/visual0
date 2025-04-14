#include "compound.hh"

#include "util.hh"

#include "TMath.h"
#include "TString.h"
#include "TH1.h"

#include <fstream>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
compound::compound(string name)
    : fSF(0.), fAlpha(0.0), fOrthogonality(9999.), fName(name) {
  TVector2 a(0., 0.);
  pHDI = {a, a, a, a, a, a, a, a};
  pHDIPrime = {a, a, a, a, a, a, a, a};
  pROCs = {a, a, a, a, a, a, a, a};
  pROCsPrime = {a, a, a, a, a, a, a, a};
  pACsPrime = {a, a, a, a, a, a, a, a};
  pMarkers = {a, a, a};
  pMarkersPrime = {a, a, a};
  fOffset = a;
}

// ----------------------------------------------------------------------
void compound::set(compound &other) {
  for (unsigned it = 0; it < other.pHDI.size(); ++it) {
    pHDI[it] = other.pHDI[it];
    pHDIPrime[it] = other.pHDIPrime[it];
  }

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
}

// ----------------------------------------------------------------------
void compound::parseSvgFile() {
  int    VERBOSE(0);
  string bareName(fName);
  string csv1 = Form("%s.chips", bareName.c_str());
  string csv2 = Form("%s.hdi", bareName.c_str());
  string csv3 = Form("%s.marks", bareName.c_str());

  system(
      Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s --query-id "
           "chip00,chip01,chip10,chip11,chip20,chip21,chip30,chip31 -X -Y > %s",
           fName.c_str(), csv1.c_str()));

  system(
      Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s --query-id "
           "HDI00,HDI01,HDI10,HDI11,HDI20,HDI21,HDI30,HDI31 -X -Y > %s",
           fName.c_str(), csv2.c_str()));

  system(Form("/Applications/Inkscape.app/Contents/MacOS/inkscape %s "
              "--query-id mark1,mark2,mark3 -X -Y > %s",
              fName.c_str(), csv3.c_str()));

  bool     doParse(false);
  ifstream INS;
  string   sline;

  // -- read 2 lines of chip coordinates
  cout << "read 1: chip coordinates" << endl;
  INS.open(csv1);
  vector<string> vlines;
  while (getline(INS, sline)) {
    vlines.push_back(sline);
  }
  INS.close();

  vector<string> schipsX = split(vlines[0], ',');
  vector<string> schipsY = split(vlines[1], ',');

  for (unsigned int i = 0; i < schipsX.size(); ++i) {
    pROCs[i] = TVector2(::stof(schipsX[i]), ::stof(schipsY[i]));
  }

  // -- read 2 lines of HDI coordinates
  cout << "read 2: HDI coordinates" << endl;
  INS.open(csv2);
  vlines.clear();
  while (getline(INS, sline)) {
    vlines.push_back(sline);
  }
  INS.close();

  vector<string> shdiX = split(vlines[0], ',');
  vector<string> shdiY = split(vlines[1], ',');

  for (unsigned int i = 0; i < shdiX.size(); ++i) {
    pHDI[i] = TVector2(::stof(shdiX[i]), ::stof(shdiY[i]));
  }

  // -- read 2 lines of markers
  cout << "read 3: marker coordinates" << endl;
  INS.open(csv3);
  vlines.clear();
  while (getline(INS, sline)) {
    vlines.push_back(sline);
  }
  INS.close();

  vector<string> smarkersX = split(vlines[0], ',');
  vector<string> smarkersY = split(vlines[1], ',');

  for (unsigned int i = 0; i < smarkersX.size(); ++i) {
    pMarkers[i] = TVector2(::stof(smarkersX[i]), ::stof(smarkersY[i]));
  }

  // cout << "Chips:" << endl;
  // for (unsigned int i = 0; i < 4; ++i) {
  //   cout << Form("%+09.3f/%+09.3f ", pROCs[i].X(), pROCs[i].Y());
  // }
  // cout << endl;
  // for (unsigned int i = 7; i > 3; --i) {
  //   cout << Form("%+09.3f/%+09.3f ", pROCs[i].X(), pROCs[i].Y());
  // }
  // cout << endl;

  // cout << "HDI:" << endl;
  // for (unsigned int i = 0; i < 4; ++i) {
  //   cout << Form("%+09.3f/%+09.3f ", pHDI[i].X(), pHDI[i].Y());
  // }
  // cout << endl;
  // for (unsigned int i = 7; i > 3; --i) {
  //   cout << Form("%+09.3f/%+09.3f ", pHDI[i].X(), pHDI[i].Y());
  // }
  // cout << endl;

  // cout << "Markers:" << endl;
  // for (unsigned int i = 0; i < 3; ++i) {
  //   cout << Form("%+09.3f/%+09.3f ", pMarkers[i].X(), pMarkers[i].Y());
  // }
  // cout << endl;
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
  fOrthogonality = TMath::ACos(xSvg * ySvg);

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
void compound::calcAll() {
  parseSvgFile();
  determineSF();
  determineTransformation();
  transform(pMarkers, pMarkersPrime);
  transform(pHDI, pHDIPrime);
  transform(pROCs, pROCsPrime);
  determineAlignmentCrosses();
  dump();
}

// ----------------------------------------------------------------------
void compound::dump() {
  cout << "compound ->" << fName << "<- alpha = " << fAlpha << " offset = ("
       << fOffset.X() << "/" << fOffset.Y() << ")"
       << " orthogonality = " << 100. * fOrthogonality / TMath::PiOver2() << "%"
       << " SF = " << fSF << "mm/px" << endl;

  cout << "Markers:        ";
  for (int i = 0; i < pMarkers.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pMarkers[i].X(), pMarkers[i].Y());
    if (i < 2)
      cout << ", ";
  }
  cout << endl;

  cout << "HDI:            ";
  for (int i = 0; i < pHDI.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pHDI[i].X(), pHDI[i].Y());
    if (i < 7)
      cout << ", ";
  }
  cout << endl;

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

  cout << "HDIPrime:       ";
  for (int i = 0; i < pHDIPrime.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pHDIPrime[i].X(), pHDIPrime[i].Y());
    if (i < 7)
      cout << ", ";
  }
  cout << endl;

  cout << "ROCsPrime:      ";
  for (int i = 0; i < pROCsPrime.size(); ++i) {
    cout << Form("(%8.3f/%8.3f)", pROCsPrime[i].X(), pROCsPrime[i].Y());
    if (i < 7)
      cout << ", ";
  }
  cout << endl;

  cout << "Test chip dimensions? (Correct answer: 21.673mm)" << endl;
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
  cout << "RMS = " << h0->GetRMS() << endl;


  cout << "Test HDI marker precision" << endl;
  TH1D *h1 = new TH1D("hdi", "hdi", 40, 20.4, 20.6);
  for (int i = 0; i < 4; ++i) {
    double diff = fSF * TMath::Abs((pHDIPrime[2 * i + 1].X() - pHDIPrime[2 * i].X())); 
    cout << "HDI " << i << ": "
         << TMath::Abs(pHDIPrime[2 * i + 1].X() - pHDIPrime[2 * i].X())
         << " px, "
         << diff
         << " mm" << endl;
         h1->Fill(diff);
  }
  cout << "RMS = " << h1->GetRMS() << endl;
}