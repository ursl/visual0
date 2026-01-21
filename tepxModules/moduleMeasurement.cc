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


// ----------------------------------------------------------------------
// -- COMPLETELY DECREPIT!!!!
void moduleMeasurement::testCoordinates(int mode) {
 cout << "==============================" << endl;
  // -- normalized unit vectors of new coordinate system on HDI, in SVG coordinates
  compound a("test");
  if (0) {
    a.getMarkers(0) = TVector2(8120, 1308);
    a.getMarkers(1) = TVector2(8139, 4671);
    a.getMarkers(2) = TVector2(2245, 4670);
  }

  // -- test perfect case (flipped, perfectly aligned)
  if (0) {
    a.getMarkers(0) = TVector2(8120, 1308);
    a.getMarkers(1) = TVector2(8120, 4671);
    a.getMarkers(2) = TVector2(2245, 4671);
  }

  // -- test perfect case (rotated by 90 degrees, perfectly aligned)
  if (0) {
    a.getMarkers(2) = TVector2(8120, 6000);
    a.getMarkers(1) = TVector2(8120, 4671);
    a.getMarkers(0) = TVector2(2245, 4671);
  }

  TVector2 aTestPoint = TVector2(8125, 4000);
  // -- test slightly overrotated case (flipped)
  if (1 == mode) {
    a.getName() = "mode 1";
    cout << "alpha more negative than -3.1415" << endl;
    a.getMarkers(0) = TVector2(8160, 1308);
    a.getMarkers(1) = TVector2(8120, 4671);
    a.getMarkers(2) = TVector2(2245, 4650);
  }

   // -- test slightly underrotated case (flipped)
   if (2 == mode) {
    a.getName() = "mode 2";
    cout << "alpha less negative than -3.1415" << endl;
    a.getMarkers(0) = TVector2(8080, 1308);
    a.getMarkers(1) = TVector2(8120, 4671);
    a.getMarkers(2) = TVector2(2245, 4691);
  }

  a.dump();

  TVector2 ySvg = a.getMarkers(0) - a.getMarkers(1);
  cout << "ySvgRaw  = " << Form("%9.7f", ySvg .X()) << "/" << Form("%9.7f", ySvg .Y()) << endl;
  ySvg  /= ySvg .Mod();
  TVector2 xSvg  = a.getMarkers(2) - a.getMarkers(1);
  //cout << "xSvgRaw  = " << Form("%9.7f", xSvg .X()) << "/" << Form("%9.7f", xSvg .Y()) << endl;
  xSvg  /= xSvg .Mod();
  
  // -- this is calculated from ySvg ONLY. xSvg is irrelevant!
  TVector2 xClc = PerpClockwise(ySvg);

  //xSvg = xClc;

  cout << "ySvg  = " << Form("%9.7f", ySvg .X()) << "/" << Form("%9.7f", ySvg .Y()) << endl;
  cout << "xSvg  = " << Form("%9.7f", xSvg .X()) << "/" << Form("%9.7f", xSvg .Y()) << endl;
  cout << "xClc  = " << Form("%9.7f", xClc .X()) << "/" << Form("%9.7f", xClc .Y()) << endl;

  double theta0 = TMath::ACos(xSvg  * TVector2(1., 0.));
  double theta1 = TMath::ACos(ySvg  * TVector2(0., 1.));
  double alpha  = 0.5*(theta0 + theta1);
  TVector2 offset = a.getMarkers(1);

  // -- rotation sign
  TVector2 xOld = TVector2(1, 0);
  //  x1*y2 - x2*y1 
  double rsgn = xOld.X()*xSvg.Y() - xOld.Y()*xSvg.X();
  alpha -= TMath::Pi();
  alpha = -TMath::Pi() + (rsgn < 0 ? alpha : -1.*alpha);
  cout << "rsgn     = " << rsgn << endl;
  cout << "theta0/1 = " << theta0 << "/" << theta1 << endl;
  cout << "alpha    = " << alpha << endl;
  cout << "offset   = " << dump(a.getMarkers(1)) << endl;
  a.setAlpha(alpha); 
  a.setOffset(offset);

  TVector2 origPrime = transform(a.getMarkers(1), alpha, offset);
  cout << "origPrime = " << dump(origPrime) << endl;

  TVector2 aPrime = transform(aTestPoint, alpha, offset);
  cout << "aPrime    = " << dump(aPrime) << endl;
  
  a.dump();
  fCompound.set(a);

  TVector2 aPrime2 = transform(aTestPoint, fCompound.getAlpha(), fCompound.getOffset());
  cout << "aPrime2   = " << dump(aPrime2) << endl;

  fCompound.set(a);
  // fCompound.transform(fCompound.pMarkers, fCompound.pMarkersPrime);
  // fCompound.dump();
  TVector2 aPrime3 = fCompound.transform(aTestPoint, fCompound.getAlpha(), fCompound.getOffset());
  cout << "aPrime3   = " << dump(aPrime3) << endl;
}
