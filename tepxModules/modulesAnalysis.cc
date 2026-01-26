#include "modulesAnalysis.hh"
#include "moduleMeasurement.hh"
#include "util.hh"

#include <glob.h>

#include <iostream>

#include "TStyle.h"
#include "TMath.h"
#include "TGraph.h"
#include "TCanvas.h"

#include <TH2.h>
#include <TVersionCheck.h>

using namespace std;

// ----------------------------------------------------------------------
modulesAnalysis::modulesAnalysis(int mode, string directory): fDirectory(directory) {
  cout << "fDirectory = " << fDirectory << endl;
  if (1 == mode) {
    // -- legacy mode - manual measurements with inkscape
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1000.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1001.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1002.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1003.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1004.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1005.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1006.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1007.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1008.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1009.svg2", 3));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1010.svg2", 4));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1011.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1012.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1013.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1014.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1015.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1016.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1017.svg2", 3));
    //badly-aligned   fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1018.svg2", -1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1019.svg2", 3));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1020.svg2", 4));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1021.svg2", 5));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1022.svg2", 4));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1023.svg2", 4));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1024.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1025.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1026.svg2", 2));
  } else if (2 == mode) {
    // -- validation with same modules as legacy
    fModules.push_back(new moduleMeasurement("json/P1000.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1001.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1002.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1003.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1004.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1005.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1006.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1007.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1008.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1009.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1010.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1011.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1012.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1013.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1014.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1015.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1016.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1017.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1018.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1019.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1020.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1021.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1022.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1023.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1024.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1025.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1026.json", 2));
  } else if (3 == mode) {
    // -- complete statistics
    fModules.push_back(new moduleMeasurement("json/P1000.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1001.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1002.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1003.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1004.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1005.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1006.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1007.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1008.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1009.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1010.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1011.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1012.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1013.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1014.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1015.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1016.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1017.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1018.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1019.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1020.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1021.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1022.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1023.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1024.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1025.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1026.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1027.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1028.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1029.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1030.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1031.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1032.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1033.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1034.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1035.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1036.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1037.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1038.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1039.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1040.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1041.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1042.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1043.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1044.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1045.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1046.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1047.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1048.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1049.json", 1));

    fModules.push_back(new moduleMeasurement("json/P1050.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1051.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1052.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1053.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1054.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1055.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1056.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1057.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1058.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1059.json", 3));
  
    fModules.push_back(new moduleMeasurement("json/P1060.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1061.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1062.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1063.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1064.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1065.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1066.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1067.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1068.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1069.json", 3));
  
    fModules.push_back(new moduleMeasurement("json/P1070.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1071.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1072.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1073.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1074.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1075.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1076.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1077.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1078.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1079.json", 0));

    fModules.push_back(new moduleMeasurement("json/P1080.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1081.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1082.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1083.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1084.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1085.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1086.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1087.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1088.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1089.json", 2));

    fModules.push_back(new moduleMeasurement("json/P1090.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1091.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1092.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1093.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1094.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1095.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1096.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1097.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1098.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1099.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1100.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1101.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1102.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1103.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1104.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1105.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1106.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1107.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1108.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1109.json", 2));


    fModules.push_back(new moduleMeasurement("json/P1110.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1111.json", 4));
 //   fModules.push_back(new moduleMeasurement("json/P1112.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1113.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1114.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1115.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1116.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1117.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1118.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1119.json", 3));

    fModules.push_back(new moduleMeasurement("json/P1120.json", 4));
    fModules.push_back(new moduleMeasurement("json/P1121.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1122.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1123.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1124.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1125.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1126.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1127.json", 2));
    fModules.push_back(new moduleMeasurement("json/P1128.json", 3));
    fModules.push_back(new moduleMeasurement("json/P1129.json", 4));

    fModules.push_back(new moduleMeasurement("json/P1130.json", 5));
    fModules.push_back(new moduleMeasurement("json/P1131.json", 0));
    fModules.push_back(new moduleMeasurement("json/P1132.json", 1));
    fModules.push_back(new moduleMeasurement("json/P1133.json", 2));
  } else if (4 == mode) {
    // -- Find all files matching pattern "modules/p????.jpg"
    vector<string> jsonFiles;
    glob_t globResult;
    int globRet = glob("json/p????.json", GLOB_TILDE, NULL, &globResult);
    
    if (globRet == 0) {
      for (size_t i = 0; i < globResult.gl_pathc; ++i) {
        string jsonFile = string(globResult.gl_pathv[i]);
        if (jsonFile.find("p1112") == string::npos) continue;
        jsonFiles.push_back(jsonFile);
      }
    }
    globfree(&globResult);

    for (const auto& jsonFile : jsonFiles) {
      fCompounds.push_back(new compound(jsonFile));
    }
  }
  bookHistograms();
}

// ----------------------------------------------------------------------
modulesAnalysis::~modulesAnalysis() {
  fFile->cd();
  for (auto hist : fHists) {
    hist.second->SetDirectory(fFile);
    hist.second->Write();
  }
  for (auto prof : fProfiles) {
    prof.second->SetDirectory(fFile);
    prof.second->Write();
  }
  fFile->Write();
  fFile->Close();
}


// ----------------------------------------------------------------------
void modulesAnalysis::doAll() {
  calcAll();
  anaAll();
  plotAll();
  cout << "----------------------------------------" << endl;
  cout << "Chip widths:" << endl;
  for (auto mm : fModules) {
    cout << mm->getFileName()
         <<  " " << Form("%7.4f", mm->getChipWidth(0)) 
         << " " << Form("%7.4f", mm->getChipWidth(1)) 
         << " " << Form("%7.4f", mm->getChipWidth(2)) 
         << " " << Form("%7.4f", mm->getChipWidth(3)) 
         << " chip separations:"
         << " " << Form("%7.4f", mm->getChipMarkerSeparation(1, 2)) 
         << " " << Form("%7.4f", mm->getChipMarkerSeparation(6, 5)) 
         << endl;
  }
}


// ---------------------------------------------------------------------- 
void modulesAnalysis::bookHistograms() {
  fFile = new TFile((fDirectory + "/modulesAnalysis.root").c_str(), "RECREATE");

  int nbins = 100;
  // -- calibration plots
  TH1D *h = new TH1D("chipWidth", " ", nbins, 21.4, 21.6);
  setTitles(h, "chip width [mm]", "entries");
  fHists.insert({"chipWidth", h});
  h = new TH1D("markerDistanceX", " ", nbins, 39.95, 40.05);
  setTitles(h, "HDI marker distance X axis [mm]", "entries");
  fHists.insert({"markerDistanceX", h});
  h = new TH1D("markerDistanceY", " ", nbins, 22.95, 23.05);
  setTitles(h, "HDI marker distance Y axis [mm]", "entries");
  fHists.insert({"markerDistanceY", h});

  h = new TH1D("scaleFactor", "", nbins, 0.0090, 0.0092);
  setTitles(h, "scale factor [mm/pixel]", "entries");
  fHists.insert({"scaleFactor", h}); 
  h = new TH1D("scaleFactorInkScape", "", nbins, 0.00678, 0.00682);
  setTitles(h, "scale factor (inkscape) [mm/pixel]", "entries");
  fHists.insert({"scaleFactorInkScape", h}); 

  h = new TH1D("alpha", "", nbins, -3.18, -3.12);
  setTitles(h, "alpha(svg, HDI) [rad]", "entries");
  fHists.insert({"alpha", h});
  h = new TH1D("orthogonality", "", nbins, 99.8, 100.2);
  setTitles(h, "orthogonality [100. * acos(xHDI * yHDI) / (PI/2)]", "entries");
  fHists.insert({"orthogonality", h});
  h = new TH1D("diffXChips", "", nbins, -0.1, 0.1);
  setTitles(h, "#Delta(x_{chip 0}, x_{chip 3}) [mm]", "entries");
  fHists.insert({"diffXChips", h});
  h = new TH1D("diffYChips", "", nbins, 37.1, 37.4);
  setTitles(h, "#Delta(y_{chip 0}, y_{chip 3}) [mm]", "entries");
  fHists.insert({"diffYChips", h});

  // -- distance plots
  h = new TH1D("chip00", "chip0 position top left (absolute)", nbins, 41.0, 42.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip00", h});
  h = new TH1D("chip01", "chip0 position top right (absolute)", nbins, 19.5, 21.0);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip01", h});
  h = new TH1D("chip10", "chip1 position top left (absolute)", nbins, 19.0, 20.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip10", h});
  h = new TH1D("chip11", "chip1 position top right (absolute)", nbins, -2.5, -1.0);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip11", h});

  h = new TH1D("chip20", "chip2 position bottom right (absolute)", nbins, -2.5, -1.0);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip20", h});
  h = new TH1D("chip21", "chip2 position bottom left (absolute)", nbins, 19.0, 20.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip21", h});
  h = new TH1D("chip30", "chip3 position bottom right (absolute)", nbins, 19.5, 21.0);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip30", h});
  h = new TH1D("chip31", "chip3 position bottom left (absolute)", nbins, 41.0, 42.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip31", h});

  // -- angle plots
  h = new TH1D("angleChips", " ", nbins, -0.005, 0.005);
  setTitles(h, "Pi/2 - angle(HDI, chips) [rad]", "entries");
  fHists.insert({"angleChips", h});

  h = new TH1D("position", "position", 6, 0, 6);
  setTitles(h, "position", "entries");
  h->SetMinimum(0.);
  fHists.insert({"position", h}); 

  TProfile *h2 = new TProfile("prfChipWidth", "chip width vs Index", fModules.size(), 0, fModules.size());
  //  setTitles(h2, "index", "chip width [mm]");
  h2->GetYaxis()->SetRangeUser(21.4, 21.6);
  fProfiles.insert({"prfChipWidth", h2});

  h2 = new TProfile("prfAngleChips", "angle(chips) vs Index", fModules.size(), 0, fModules.size());
  //setTitles(h2, "index", "angle(chips) [rad]");
  h2->GetYaxis()->SetRangeUser(-0.01, 0.01);
  fProfiles.insert({"prfAngleChips", h2});

  h2 = new TProfile("prfdiffXChips", "#Delta(x_{chip 0}, x_{chip 3}) vs Index", fModules.size(), 0, fModules.size());
  //setTitles(h2, "index", "#Delta(x_{chip 0}, x_{chip 3}) [mm]");
  h2->GetYaxis()->SetRangeUser(-0.1, 0.1);
  fProfiles.insert({"prfdiffXChips", h2});

  h2 = new TProfile("prfdiffYChips", "#Delta(y_{chip 0}, y_{chip 3}) vs Index", fModules.size(), 0, fModules.size());
  //setTitles(h2, "index", "#Delta(y_{chip 0}, y_{chip 3}) [mm]");
  h2->GetYaxis()->SetRangeUser(37., 37.4);
  fProfiles.insert({"prfdiffYChips", h2});

  h2 = new TProfile("prfscaleFactor", "scale factor vs Index", fModules.size(), 0, fModules.size());
  //setTitles(h2, "index", "scale factor [mm/pixel]");
  h2->GetYaxis()->SetRangeUser(0.0090, 0.0092);
  fProfiles.insert({"prfscaleFactor", h2});
}

// ----------------------------------------------------------------------
void modulesAnalysis::calcAll() {
  for (auto mm : fModules) {
    if (mm->getPosition() < 0) continue;
    fHists["position"]->Fill(mm->getPosition());
    mm->calcAll(0, 0);
  }
}

// ----------------------------------------------------------------------
void::modulesAnalysis::anaAll() {
  for (auto mm : fModules) {
    if (mm->getPosition() < 0) {
      cout << "filling -999 for missing module " << mm->getFileName() << endl;
      // -- avoid "zero" as entry for missing modules
      fProfiles["prfAngleChips"]->Fill(mm->getHistIndex(), -0.01);
      fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), 21.4);
      fProfiles["prfdiffXChips"]->Fill(mm->getHistIndex(), -0.1);
      fProfiles["prfdiffYChips"]->Fill(mm->getHistIndex(), 37.);
      fProfiles["prfscaleFactor"]->Fill(mm->getHistIndex(), 0.00678);
      continue;  
    }

    for (int ichip = 0; ichip < 4; ichip++) {
      if (!mm->getCompound().chipWellMeasured(ichip)) {
        cout << "**************************** skipping chip " << ichip << " of " << mm->getFileName() << endl;
        continue;
      }
      fHists["chipWidth"]->Fill(mm->getChipWidth(ichip));
    }

    double sf = mm->getCompound().getSF();
    if (mm->getCompound().markersWellMeasured()) {
      fHists["markerDistanceX"]->Fill(mm->getMarkerDistance("x"));
      fHists["markerDistanceY"]->Fill(mm->getMarkerDistance("y"));
      fHists["scaleFactor"]->Fill(sf);
      fHists["scaleFactorInkScape"]->Fill(sf);
      fHists["alpha"]->Fill(mm->getCompound().getAlpha());
      fHists["orthogonality"]->Fill(mm->getCompound().getOrthogonality());
    }

    if (mm->getCompound().chipWellMeasured(0)) {
       fHists["chip00"]->Fill(sf * mm->getCompound().getROCsPrime(0).X());
       fHists["chip01"]->Fill(sf * mm->getCompound().getROCsPrime(1).X());
    }
    if (mm->getCompound().chipWellMeasured(1)) {
      fHists["chip10"]->Fill(sf * mm->getCompound().getROCsPrime(2).X());
      fHists["chip11"]->Fill(sf * mm->getCompound().getROCsPrime(3).X());
      if (sf * mm->getCompound().getROCsPrime(3).X() >-1. ) {
        cout << "**************************** chip11 is badly placed **************************** " << mm->getFileName() << endl;
        cout << "sf * mm->getCompound().getROCsPrime(3).X() = " << sf * mm->getCompound().getROCsPrime(3).X() << endl;
        cout << "sf * mm->getCompound().getROCsPrime(3).Y() = " << sf * mm->getCompound().getROCsPrime(3).Y() << endl;
        cout << "mm->getCompound().getROCsPrime(3).X() = " << mm->getCompound().getROCsPrime(3).X() << endl;
        cout << "mm->getCompound().getROCsPrime(3).Y() = " << mm->getCompound().getROCsPrime(3).Y() << endl;
        cout << "mm->getCompound().getROCs(3).X() = " << mm->getCompound().getROCs(3).X() << endl;
        cout << "mm->getCompound().getROCs(3).Y() = " << mm->getCompound().getROCs(3).Y() << endl;
      }
    }
    if (mm->getCompound().chipWellMeasured(2)) {
      fHists["chip20"]->Fill(sf * mm->getCompound().getROCsPrime(4).X());
      fHists["chip21"]->Fill(sf * mm->getCompound().getROCsPrime(5).X());
    }
    if (mm->getCompound().chipWellMeasured(3)) {
      fHists["chip30"]->Fill(sf * mm->getCompound().getROCsPrime(6).X());
      fHists["chip31"]->Fill(sf * mm->getCompound().getROCsPrime(7).X());
      if (sf * mm->getCompound().getROCsPrime(7).X() < 40.) {
        cout << "**************************** chip31 is too small " << mm->getFileName() << endl;
        cout << "sf * mm->getCompound().getROCsPrime(7).X() = " << sf * mm->getCompound().getROCsPrime(7).X() << endl;
        cout << "sf * mm->getCompound().getROCsPrime(7).Y() = " << sf * mm->getCompound().getROCsPrime(7).Y() << endl;
        cout << "mm->getCompound().getROCsPrime(7).X() = " << mm->getCompound().getROCsPrime(7).X() << endl;
        cout << "mm->getCompound().getROCsPrime(7).Y() = " << mm->getCompound().getROCsPrime(7).Y() << endl;
        cout << "mm->getCompound().getROCs(7).X() = " << mm->getCompound().getROCs(7).X() << endl;
        cout << "mm->getCompound().getROCs(7).Y() = " << mm->getCompound().getROCs(7).Y() << endl;
        cout << "mm->getCompound().getROCsPrime(7).X() = " << mm->getCompound().getROCsPrime(7).X() << endl;
        cout << "mm->getCompound().getROCsPrime(7).Y() = " << mm->getCompound().getROCsPrime(7).Y() << endl;
      }
    }

    if (mm->getCompound().chipWellMeasured(0) && mm->getCompound().chipWellMeasured(3)) {
      fHists["diffXChips"]->Fill(sf * (mm->getCompound().getROCsPrime(0).X() - mm->getCompound().getROCsPrime(7).X()));
      fHists["diffYChips"]->Fill(sf * (mm->getCompound().getROCsPrime(0).Y() - mm->getCompound().getROCsPrime(7).Y()));
    }

    TVector2 rocEdge = mm->getCompound().getROCsPrime(1) - mm->getCompound().getROCsPrime(6);
    double angle = TMath::PiOver2() - TMath::ATan2(rocEdge.Y(), rocEdge.X());
    fHists["angleChips"]->Fill(angle);


    fProfiles["prfAngleChips"]->Fill(mm->getHistIndex(), angle);
    if (mm->getCompound().chipWellMeasured(0)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(0));
    if (mm->getCompound().chipWellMeasured(1)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(1));
    if (mm->getCompound().chipWellMeasured(2)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(2));
    if (mm->getCompound().chipWellMeasured(3)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(3));

    if (mm->getCompound().chipWellMeasured(0) && mm->getCompound().chipWellMeasured(3)) {
      fProfiles["prfdiffXChips"]->Fill(mm->getHistIndex(), sf * (mm->getCompound().getROCsPrime(0).X() - mm->getCompound().getROCsPrime(7).X()));
      fProfiles["prfdiffYChips"]->Fill(mm->getHistIndex(), sf * (mm->getCompound().getROCsPrime(0).Y() - mm->getCompound().getROCsPrime(7).Y()));
    }
    cout << "filling scale factor " << mm->getHistIndex() << " " << sf << endl;
    fProfiles["prfscaleFactor"]->Fill(mm->getHistIndex(), sf);
  }
}

// ----------------------------------------------------------------------
void modulesAnalysis::plotAll() {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  shrinkPad(0.15, 0.15, 0.1, 0.1);
  gStyle->SetOptStat(111111);
  for (auto h : fHists) {
    h.second->Draw();
    c1->SaveAs((fDirectory + "/" + h.first + ".pdf").c_str());
  }

  gStyle->SetOptStat(0);
  for (auto prof : fProfiles) {
    prof.second->Draw();
    c1->SaveAs((fDirectory + "/" + prof.first + ".pdf").c_str());
  }
}


// ----------------------------------------------------------------------
void modulesAnalysis::plotGlueTests() {
  vector<double> x1 = {0, 1, 2, 3, 4, 5};
  vector<double> x2 = {0, 1, 2, 3, 4, 5};
  vector<double> y1 = {0.0146, 0.0174, 0.0222, 0.0259, 0.0309, 0.0336};
  vector<double> y2 = {0.0143, 0.0154, 0.0204, 0.0222, 0.0243, 0.0235};

  TGraph *g1 = new TGraph(x1.size(), x1.data(), y1.data());
  TGraph *g2 = new TGraph(x2.size(), x2.data(), y2.data());

  g1->SetMarkerStyle(20);
  g1->SetMarkerColor(kRed);
  g2->SetMarkerStyle(20);
  g2->SetMarkerColor(kBlue);

  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  shrinkPad(0.1, 0.15, 0.1, 0.1);
  g1->Draw("AP");
  g2->Draw("P");
  g1->SetMarkerSize(1.5); 
  g2->SetMarkerSize(1.5); 
  g1->SetTitle(" ");
  g1->GetXaxis()->SetTitle("position");
  g1->GetYaxis()->SetTitle("glue [gram]");
  g1->GetYaxis()->SetRangeUser(0.0, 0.035);

  TLegend *leg = newLegend(0.16, 0.72, 0.3, 0.85); 
  leg->AddEntry(g1, "3 Araldite presses", "p");
  leg->AddEntry(g2, "2 Araldite presses", "p");
  leg->SetHeader("Glue tests");
  leg->Draw();

  c1->Draw();
  c1->SaveAs((fDirectory + "/glueTests.pdf").c_str());

}
