#include "moduleMeasurement.hh"

#include "TMath.h"
#include "TString.h"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
moduleMeasurement::moduleMeasurement(string filename) : fCompound(), fFilename(filename) {}


// ----------------------------------------------------------------------
void moduleMeasurement::setFileName(string filename) {
  fFilename = filename;
  fCompound.fName = filename;
}

// ----------------------------------------------------------------------
void moduleMeasurement::calcAll() {
  fCompound.calcAll();
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
    a.pMarkers[0] = TVector2(8120, 1308);
    a.pMarkers[1] = TVector2(8139, 4671);
    a.pMarkers[2] = TVector2(2245, 4670);
  }

  // -- test perfect case (flipped, perfectly aligned)
  if (0) {
    a.pMarkers[0] = TVector2(8120, 1308);
    a.pMarkers[1] = TVector2(8120, 4671);
    a.pMarkers[2] = TVector2(2245, 4671);
  }

  // -- test perfect case (rotated by 90 degrees, perfectly aligned)
  if (0) {
    a.pMarkers[2] = TVector2(8120, 6000);
    a.pMarkers[1] = TVector2(8120, 4671);
    a.pMarkers[0] = TVector2(2245, 4671);
  }

  TVector2 aTestPoint = TVector2(8125, 4000);
  // -- test slightly overrotated case (flipped)
  if (1 == mode) {
    a.fName = "mode 1";
    cout << "alpha more negative than -3.1415" << endl;
    a.pMarkers[0] = TVector2(8160, 1308);
    a.pMarkers[1] = TVector2(8120, 4671);
    a.pMarkers[2] = TVector2(2245, 4650);
  }

   // -- test slightly underrotated case (flipped)
   if (2 == mode) {
    a.fName = "mode 2";
    cout << "alpha less negative than -3.1415" << endl;
    a.pMarkers[0] = TVector2(8080, 1308);
    a.pMarkers[1] = TVector2(8120, 4671);
    a.pMarkers[2] = TVector2(2245, 4691);
  }

  a.dump();

  TVector2 ySvg = a.pMarkers[0] - a.pMarkers[1];
  cout << "ySvgRaw  = " << Form("%9.7f", ySvg .X()) << "/" << Form("%9.7f", ySvg .Y()) << endl;
  ySvg  /= ySvg .Mod();
  TVector2 xSvg  = a.pMarkers[2] - a.pMarkers[1];
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
  TVector2 offset = a.pMarkers[1];

  // -- rotation sign
  TVector2 xOld = TVector2(1, 0);
  //  x1*y2 - x2*y1 
  double rsgn = xOld.X()*xSvg.Y() - xOld.Y()*xSvg.X();
  alpha -= TMath::Pi();
  alpha = -TMath::Pi() + (rsgn < 0 ? alpha : -1.*alpha);
  cout << "rsgn     = " << rsgn << endl;
  cout << "theta0/1 = " << theta0 << "/" << theta1 << endl;
  cout << "alpha    = " << alpha << endl;
  cout << "offset   = " << dump(a.pMarkers[1]) << endl;
  a.fAlpha = alpha; 
  a.fOffset = offset;

  TVector2 origPrime = transform(a.pMarkers[1], alpha, offset);
  cout << "origPrime = " << dump(origPrime) << endl;

  TVector2 aPrime = transform(aTestPoint, alpha, offset);
  cout << "aPrime    = " << dump(aPrime) << endl;
  
  a.dump();
  fCompound.set(a);

  TVector2 aPrime2 = transform(aTestPoint, fCompound.fAlpha, fCompound.fOffset);
  cout << "aPrime2   = " << dump(aPrime2) << endl;

  fCompound.set(a);
  // fCompound.transform(fCompound.pMarkers, fCompound.pMarkersPrime);
  // fCompound.dump();
  TVector2 aPrime3 = fCompound.transform(aTestPoint, fCompound.fAlpha, fCompound.fOffset);
  cout << "aPrime3   = " << dump(aPrime3) << endl;
}
