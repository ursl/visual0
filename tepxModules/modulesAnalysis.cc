#include "modulesAnalysis.hh"
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
modulesAnalysis::modulesAnalysis(int mode, string directory, string filename): fDirectory(directory), fFilename(filename) {
  cout << "fDirectory = " << fDirectory << endl;
  if (0 == mode) {
    // -- single module
    fCompunds.push_back(new compound(fFilename, 0));
  }
  else if (1 == mode) {
    // -- legacy mode - manual measurements with inkscape
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1000.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1001.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1002.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1003.svg2", 2));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1004.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1005.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1006.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1007.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1008.svg2", 2));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1009.svg2", 3));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1010.svg2", 4));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1011.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1012.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1013.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1014.svg2", 2));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1015.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1016.svg2", 2));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1017.svg2", 3));
    //badly-aligned   fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1018.svg2", -1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1019.svg2", 3));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1020.svg2", 4));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1021.svg2", 5));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1022.svg2", 4));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1023.svg2", 4));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1024.svg2", 0));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1025.svg2", 1));
    fCompunds.push_back(new compound("/Users/ursl/inkscape/tepx-modules/P1026.svg2", 2));
  } else if (2 == mode) {
    // -- validation with same modules as legacy
    fCompunds.push_back(new compound("json/P1000.json", 0));
    fCompunds.push_back(new compound("json/P1001.json", 0));
    fCompunds.push_back(new compound("json/P1002.json", 1));
    fCompunds.push_back(new compound("json/P1003.json", 2));
    fCompunds.push_back(new compound("json/P1004.json", 0));
    fCompunds.push_back(new compound("json/P1005.json", 1));
    fCompunds.push_back(new compound("json/P1006.json", 0));
    fCompunds.push_back(new compound("json/P1007.json", 1));
    fCompunds.push_back(new compound("json/P1008.json", 2));
    fCompunds.push_back(new compound("json/P1009.json", 3));
    fCompunds.push_back(new compound("json/P1010.json", 4));
    fCompunds.push_back(new compound("json/P1011.json", 0));
    fCompunds.push_back(new compound("json/P1012.json", 0));
    fCompunds.push_back(new compound("json/P1013.json", 1));
    fCompunds.push_back(new compound("json/P1014.json", 2));
    fCompunds.push_back(new compound("json/P1015.json", 1));
    fCompunds.push_back(new compound("json/P1016.json", 2));
    fCompunds.push_back(new compound("json/P1017.json", 3));
    fCompunds.push_back(new compound("json/P1018.json", 0));
    fCompunds.push_back(new compound("json/P1019.json", 3));
    fCompunds.push_back(new compound("json/P1020.json", 4));
    fCompunds.push_back(new compound("json/P1021.json", 5));
    fCompunds.push_back(new compound("json/P1022.json", 4));
    fCompunds.push_back(new compound("json/P1023.json", 4));
    fCompunds.push_back(new compound("json/P1024.json", 0));
    fCompunds.push_back(new compound("json/P1025.json", 1));
    fCompunds.push_back(new compound("json/P1026.json", 2));
  } else if (3 == mode) {
    // -- complete statistics
    fCompunds.push_back(new compound("json/P1000.json", 0));
    fCompunds.push_back(new compound("json/P1001.json", 0));
    fCompunds.push_back(new compound("json/P1002.json", 1));
    fCompunds.push_back(new compound("json/P1003.json", 2));
    fCompunds.push_back(new compound("json/P1004.json", 0));
    fCompunds.push_back(new compound("json/P1005.json", 1));
    fCompunds.push_back(new compound("json/P1006.json", 0));
    fCompunds.push_back(new compound("json/P1007.json", 1));
    fCompunds.push_back(new compound("json/P1008.json", 2));
    fCompunds.push_back(new compound("json/P1009.json", 3));
    fCompunds.push_back(new compound("json/P1010.json", 4));
    fCompunds.push_back(new compound("json/P1011.json", 0));
    fCompunds.push_back(new compound("json/P1012.json", 0));
    fCompunds.push_back(new compound("json/P1013.json", 1));
    fCompunds.push_back(new compound("json/P1014.json", 2));
    fCompunds.push_back(new compound("json/P1015.json", 1));
    fCompunds.push_back(new compound("json/P1016.json", 2));
    fCompunds.push_back(new compound("json/P1017.json", 3));
    fCompunds.push_back(new compound("json/P1018.json", 0));
    fCompunds.push_back(new compound("json/P1019.json", 3));
    fCompunds.push_back(new compound("json/P1020.json", 4));
    fCompunds.push_back(new compound("json/P1021.json", 5));
    fCompunds.push_back(new compound("json/P1022.json", 4));
    fCompunds.push_back(new compound("json/P1023.json", 4));
    fCompunds.push_back(new compound("json/P1024.json", 0));
    fCompunds.push_back(new compound("json/P1025.json", 1));
    fCompunds.push_back(new compound("json/P1026.json", 2));
    fCompunds.push_back(new compound("json/P1027.json", 1));
    fCompunds.push_back(new compound("json/P1028.json", 2));
    fCompunds.push_back(new compound("json/P1029.json", 3));
    fCompunds.push_back(new compound("json/P1030.json", 4));
    fCompunds.push_back(new compound("json/P1031.json", 0));
    fCompunds.push_back(new compound("json/P1032.json", 1));
    fCompunds.push_back(new compound("json/P1033.json", 3));
    fCompunds.push_back(new compound("json/P1034.json", 4));
    fCompunds.push_back(new compound("json/P1035.json", 0));
    fCompunds.push_back(new compound("json/P1036.json", 0));
    fCompunds.push_back(new compound("json/P1037.json", 1));
    fCompunds.push_back(new compound("json/P1038.json", 2));
    fCompunds.push_back(new compound("json/P1039.json", 3));
    fCompunds.push_back(new compound("json/P1040.json", 4));
    fCompunds.push_back(new compound("json/P1041.json", 5));
    fCompunds.push_back(new compound("json/P1042.json", 0));
    fCompunds.push_back(new compound("json/P1043.json", 1));
    fCompunds.push_back(new compound("json/P1044.json", 2));
    fCompunds.push_back(new compound("json/P1045.json", 3));
    fCompunds.push_back(new compound("json/P1046.json", 4));
    fCompunds.push_back(new compound("json/P1047.json", 5));
    fCompunds.push_back(new compound("json/P1048.json", 0));
    fCompunds.push_back(new compound("json/P1049.json", 1));

    fCompunds.push_back(new compound("json/P1050.json", 2));
    fCompunds.push_back(new compound("json/P1051.json", 0));
    fCompunds.push_back(new compound("json/P1052.json", 1));
    fCompunds.push_back(new compound("json/P1053.json", 2));
    fCompunds.push_back(new compound("json/P1054.json", 3));
    fCompunds.push_back(new compound("json/P1055.json", 2));
    fCompunds.push_back(new compound("json/P1056.json", 3));
    fCompunds.push_back(new compound("json/P1057.json", 0));
    fCompunds.push_back(new compound("json/P1058.json", 1));
    fCompunds.push_back(new compound("json/P1059.json", 3));
  
    fCompunds.push_back(new compound("json/P1060.json", 0));
    fCompunds.push_back(new compound("json/P1061.json", 1));
    fCompunds.push_back(new compound("json/P1062.json", 2));
    fCompunds.push_back(new compound("json/P1063.json", 3));
    fCompunds.push_back(new compound("json/P1064.json", 4));
    fCompunds.push_back(new compound("json/P1065.json", 5));
    fCompunds.push_back(new compound("json/P1066.json", 0));
    fCompunds.push_back(new compound("json/P1067.json", 1));
    fCompunds.push_back(new compound("json/P1068.json", 2));
    fCompunds.push_back(new compound("json/P1069.json", 3));
  
    fCompunds.push_back(new compound("json/P1070.json", 4));
    fCompunds.push_back(new compound("json/P1071.json", 0));
    fCompunds.push_back(new compound("json/P1072.json", 5));
    fCompunds.push_back(new compound("json/P1073.json", 0));
    fCompunds.push_back(new compound("json/P1074.json", 1));
    fCompunds.push_back(new compound("json/P1075.json", 2));
    fCompunds.push_back(new compound("json/P1076.json", 3));
    fCompunds.push_back(new compound("json/P1077.json", 4));
    fCompunds.push_back(new compound("json/P1078.json", 5));
    fCompunds.push_back(new compound("json/P1079.json", 0));

    fCompunds.push_back(new compound("json/P1080.json", 1));
    fCompunds.push_back(new compound("json/P1081.json", 2));
    fCompunds.push_back(new compound("json/P1082.json", 3));
    fCompunds.push_back(new compound("json/P1083.json", 4));
    fCompunds.push_back(new compound("json/P1084.json", 5));
    fCompunds.push_back(new compound("json/P1085.json", 0));
    fCompunds.push_back(new compound("json/P1086.json", 1));
    fCompunds.push_back(new compound("json/P1087.json", 0));
    fCompunds.push_back(new compound("json/P1088.json", 1));
    fCompunds.push_back(new compound("json/P1089.json", 2));

    fCompunds.push_back(new compound("json/P1090.json", 3));
    fCompunds.push_back(new compound("json/P1091.json", 4));
    fCompunds.push_back(new compound("json/P1092.json", 5));
    fCompunds.push_back(new compound("json/P1093.json", 0));
    fCompunds.push_back(new compound("json/P1094.json", 1));
    fCompunds.push_back(new compound("json/P1095.json", 2));
    fCompunds.push_back(new compound("json/P1096.json", 0));
    fCompunds.push_back(new compound("json/P1097.json", 1));
    fCompunds.push_back(new compound("json/P1098.json", 2));
    fCompunds.push_back(new compound("json/P1099.json", 3));

    fCompunds.push_back(new compound("json/P1100.json", 4));
    fCompunds.push_back(new compound("json/P1101.json", 5));
    fCompunds.push_back(new compound("json/P1102.json", 0));
    fCompunds.push_back(new compound("json/P1103.json", 1));
    fCompunds.push_back(new compound("json/P1104.json", 2));
    fCompunds.push_back(new compound("json/P1105.json", 3));
    fCompunds.push_back(new compound("json/P1106.json", 4));
    fCompunds.push_back(new compound("json/P1107.json", 0));
    fCompunds.push_back(new compound("json/P1108.json", 1));
    fCompunds.push_back(new compound("json/P1109.json", 2));


    fCompunds.push_back(new compound("json/P1110.json", 3));
    fCompunds.push_back(new compound("json/P1111.json", 4));
 //   fModules.push_back(new compound("json/P1112.json", 5));
    fCompunds.push_back(new compound("json/P1113.json", 0));
    fCompunds.push_back(new compound("json/P1114.json", 1));
    fCompunds.push_back(new compound("json/P1115.json", 2));
    fCompunds.push_back(new compound("json/P1116.json", 0));
    fCompunds.push_back(new compound("json/P1117.json", 1));
    fCompunds.push_back(new compound("json/P1118.json", 2));
    fCompunds.push_back(new compound("json/P1119.json", 3));

    fCompunds.push_back(new compound("json/P1120.json", 4));
    fCompunds.push_back(new compound("json/P1121.json", 5));
    fCompunds.push_back(new compound("json/P1122.json", 0));
    fCompunds.push_back(new compound("json/P1123.json", 1));
    fCompunds.push_back(new compound("json/P1124.json", 2));
    fCompunds.push_back(new compound("json/P1125.json", 0));
    fCompunds.push_back(new compound("json/P1126.json", 1));
    fCompunds.push_back(new compound("json/P1127.json", 2));
    fCompunds.push_back(new compound("json/P1128.json", 3));
    fCompunds.push_back(new compound("json/P1129.json", 4));

    fCompunds.push_back(new compound("json/P1130.json", 5));
    fCompunds.push_back(new compound("json/P1131.json", 0));
    fCompunds.push_back(new compound("json/P1132.json", 1));
    fCompunds.push_back(new compound("json/P1133.json", 2));
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
      fCompunds.push_back(new compound(jsonFile));
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
  for (auto mm : fCompunds) {
    cout << mm->getName()
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

  h = new TH1D("aveAngleChips", " ", nbins, -0.005, 0.005);
  setTitles(h, "Pi/2 - aveAngle(HDI, chips) [rad]", "entries");
  fHists.insert({"aveAngleChips", h});

  h = new TH1D("position", "position", 6, 0, 6);
  setTitles(h, "position", "entries");
  h->SetMinimum(0.);
  fHists.insert({"position", h}); 

  TProfile *h2 = new TProfile("prfChipWidth", "chip width vs Index", fCompunds.size(), 0, fCompunds.size());
  //  setTitles(h2, "index", "chip width [mm]");
  h2->GetYaxis()->SetRangeUser(21.4, 21.6);
  fProfiles.insert({"prfChipWidth", h2});

  h2 = new TProfile("prfAngleChips", "angle(chips) vs Index", fCompunds.size(), 0, fCompunds.size());
  //setTitles(h2, "index", "angle(chips) [rad]");
  h2->GetYaxis()->SetRangeUser(-0.01, 0.01);
  fProfiles.insert({"prfAngleChips", h2});

  h2 = new TProfile("prfdiffXChips", "#Delta(x_{chip 0}, x_{chip 3}) vs Index", fCompunds.size(), 0, fCompunds.size());
  //setTitles(h2, "index", "#Delta(x_{chip 0}, x_{chip 3}) [mm]");
  h2->GetYaxis()->SetRangeUser(-0.1, 0.1);
  fProfiles.insert({"prfdiffXChips", h2});

  h2 = new TProfile("prfdiffYChips", "#Delta(y_{chip 0}, y_{chip 3}) vs Index", fCompunds.size(), 0, fCompunds.size());
  //setTitles(h2, "index", "#Delta(y_{chip 0}, y_{chip 3}) [mm]");
  h2->GetYaxis()->SetRangeUser(37., 37.4);
  fProfiles.insert({"prfdiffYChips", h2});

  h2 = new TProfile("prfscaleFactor", "scale factor vs Index", fCompunds.size(), 0, fCompunds.size());
  //setTitles(h2, "index", "scale factor [mm/pixel]");
  h2->GetYaxis()->SetRangeUser(0.0090, 0.0092);
  fProfiles.insert({"prfscaleFactor", h2});
}

// ----------------------------------------------------------------------
void modulesAnalysis::calcAll() {
  for (auto mm : fCompunds) {
    if (mm->getModulePosition() < 0) continue;
    fHists["position"]->Fill(mm->getModulePosition());
    mm->calcAll(0, 0);
  }
}

// ----------------------------------------------------------------------
void::modulesAnalysis::anaAll() {
  for (auto mm : fCompunds) {
    if (mm->getModulePosition() < 0) {
      cout << "filling -999 for missing module " << mm->getName() << endl;
      // -- avoid "zero" as entry for missing modules
      fProfiles["prfAngleChips"]->Fill(mm->getHistIndex(), -0.01);
      fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), 21.4);
      fProfiles["prfdiffXChips"]->Fill(mm->getHistIndex(), -0.1);
      fProfiles["prfdiffYChips"]->Fill(mm->getHistIndex(), 37.);
      fProfiles["prfscaleFactor"]->Fill(mm->getHistIndex(), 0.00678);
      continue;  
    }

    for (int ichip = 0; ichip < 4; ichip++) {
      if (!mm->chipWellMeasured(ichip)) {
        cout << "**************************** skipping chip " << ichip << " of " << mm->getName() << endl;
        continue;
      }
      fHists["chipWidth"]->Fill(mm->getChipWidth(ichip));
    }

    double sf = mm->getSF();
    if (mm->markersWellMeasured()) {
      fHists["markerDistanceX"]->Fill(mm->getMarkerDistance("x"));
      fHists["markerDistanceY"]->Fill(mm->getMarkerDistance("y"));
      fHists["scaleFactor"]->Fill(sf);
      fHists["scaleFactorInkScape"]->Fill(sf);
      fHists["alpha"]->Fill(mm->getAlpha());
      fHists["orthogonality"]->Fill(mm->getOrthogonality());
    }

    for (int ichip = 0; ichip < 4; ichip++) {
      if (mm->chipWellMeasured(ichip)) {
        fHists["chip" + to_string(ichip) + "0"]->Fill(sf * mm->getROCsPrime(2*ichip).X());
        fHists["chip" + to_string(ichip) + "1"]->Fill(sf * mm->getROCsPrime(2*ichip + 1).X());
      }
    }

    if (mm->chipWellMeasured(0) && mm->chipWellMeasured(3)) {
      fHists["diffXChips"]->Fill(sf * (mm->getROCsPrime(0).X() - mm->getROCsPrime(7).X()));
      fHists["diffYChips"]->Fill(sf * (mm->getROCsPrime(0).Y() - mm->getROCsPrime(7).Y()));
    }

    vector<double> angles; 
    if (mm->chipWellMeasured(0) && mm->chipWellMeasured(3)) {
      angles.push_back(mm->getAngleChipMarkerHDIyAxis(1, 6));
    }
    if (mm->chipWellMeasured(1) && mm->chipWellMeasured(2)) {
      angles.push_back(mm->getAngleChipMarkerHDIyAxis(2, 5));
      angles.push_back(mm->getAngleChipMarkerHDIyAxis(3, 4));
    }

    if (angles.size() > 0) {
      double aveAngle = 0.;
      for (auto angle : angles) {
        aveAngle += angle;
      }
      aveAngle /= angles.size();
      fHists["aveAngleChips"]->Fill(aveAngle);
    }
    double angle = mm->getAngleChipMarkerHDIyAxis(1, 6);
    fHists["angleChips"]->Fill(angle);


    fProfiles["prfAngleChips"]->Fill(mm->getHistIndex(), angle);
    if (mm->chipWellMeasured(0)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(0));
    if (mm->chipWellMeasured(1)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(1));
    if (mm->chipWellMeasured(2)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(2));
    if (mm->chipWellMeasured(3)) fProfiles["prfChipWidth"]->Fill(mm->getHistIndex(), mm->getChipWidth(3));

    if (mm->chipWellMeasured(0) && mm->chipWellMeasured(3)) {
      fProfiles["prfdiffXChips"]->Fill(mm->getHistIndex(), sf * (mm->getROCsPrime(0).X() - mm->getROCsPrime(7).X()));
      fProfiles["prfdiffYChips"]->Fill(mm->getHistIndex(), sf * (mm->getROCsPrime(0).Y() - mm->getROCsPrime(7).Y()));
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
