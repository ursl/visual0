#include "modulesAnalysis.hh"
#include "moduleMeasurement.hh"
#include "util.hh"

#include "TH1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TCanvas.h"

#include <TVersionCheck.h>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
modulesAnalysis::modulesAnalysis(int mode) {
  if (1 == mode) {
    // -- manually setup list of modules
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1000.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1001.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1002.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1003.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1004.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1005.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1006.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1007.svg2", 0));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1008.svg2", 1));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1009.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1010.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1011.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1012.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1013.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1014.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1015.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1016.svg2", 2));
    fModules.push_back(new moduleMeasurement("/Users/ursl/inkscape/tepx-modules/P1017.svg2", 2));
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
  fFile->Write();
  fFile->Close();
}

// ---------------------------------------------------------------------- 
void modulesAnalysis::bookHistograms() {
  fFile = new TFile("modulesAnalysis.root", "RECREATE");

  // -- calibration plots
  TH1D *h = new TH1D("chipWidth", "chip width", 100, 21.4, 21.6);
  setTitles(h, "chip width [mm]", "entries");
  fHists.insert({"chipWidth", h});
  h = new TH1D("markerDistanceX", " ", 100, 39.95, 40.05);
  setTitles(h, "marker distance HDI X axis [mm]", "entries");
  fHists.insert({"markerDistanceX", h});
  h = new TH1D("markerDistanceY", " ", 100, 22.95, 23.05);
  setTitles(h, "marker distance Y [mm]", "entries");
  fHists.insert({"markerDistanceY", h});
  h = new TH1D("scaleFactor", "", 100, 0.00678, 0.00682);
  setTitles(h, "scale factor [mm/pixel]", "entries");
  fHists.insert({"scaleFactor", h}); 
  h = new TH1D("alpha", "", 100, -0.1, 0.1);
  setTitles(h, "PI - alpha(svg, HDI) [rad]", "entries");
  fHists.insert({"alpha", h});
  h = new TH1D("orthogonality", "", 100, 99.8, 100.2);
  setTitles(h, "orthogonality [100. * acos(xHDI * yHDI) / (PI/2)]", "entries");
  fHists.insert({"orthogonality", h});

  // -- distance plots
  h = new TH1D("chip00", "chip0 position top left (absolute)", 100, 41.0, 41.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip00", h});
  h = new TH1D("chip31", "chip3 position bottom left (absolute)", 100, 41.0, 41.5);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip31", h});
  h = new TH1D("chip00HDI", "chip0 position top left (wrt HDI marker)", 100, 0.2, 0.6);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip00HDI", h});
  h = new TH1D("chip31HDI", "chip3 position bottom left (wrt HDI marker)", 100, 0.2, 0.6);
  setTitles(h, "x position [mm]", "entries");
  fHists.insert({"chip31HDI", h});

  // -- angle plots
  h = new TH1D("angleChips", " Pi/2 - angle(HDI, chips) [rad]", 100, -0.01, 0.01);
  setTitles(h, " Pi/2 - angle(HDI, chips) [rad]", "entries");
  fHists.insert({"angleChips", h});
  h = new TH1D("angleChipsHDI", " Pi/2 - angle(chips, HDI markers) [rad]", 100, -0.01, 0.01);
  setTitles(h, " Pi/2 - angle(chips, HDI markers) [rad]", "entries");
  fHists.insert({"angleChipsHDI", h});

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
         << endl;
  }
}

// ----------------------------------------------------------------------
void modulesAnalysis::calcAll() {
  for (auto mm : fModules) {
    mm->calcAll(0, 0);
  }
}

// ----------------------------------------------------------------------
void::modulesAnalysis::anaAll() {
  for (auto mm : fModules) {
    for (int ichip = 0; ichip < 4; ichip++) {
      fHists["chipWidth"]->Fill(mm->getChipWidth(ichip));
    }
    double sf = mm->getCompound().getSF();
    fHists["markerDistanceX"]->Fill(mm->getMarkerDistance("x"));
    fHists["markerDistanceY"]->Fill(mm->getMarkerDistance("y"));
    fHists["scaleFactor"]->Fill(sf);
    fHists["alpha"]->Fill(TMath::Pi() - mm->getCompound().getAlpha());
    fHists["orthogonality"]->Fill(mm->getCompound().getOrthogonality());

    fHists["chip00"]->Fill(sf * mm->getCompound().getROCsPrime(0).X());
    fHists["chip31"]->Fill(sf * mm->getCompound().getROCsPrime(7).X());

    fHists["chip00HDI"]->Fill(sf * (mm->getCompound().getROCsPrime(0).X() - mm->getCompound().getHDIPrime(0).X()));
    fHists["chip31HDI"]->Fill(sf * (mm->getCompound().getROCsPrime(7).X() - mm->getCompound().getHDIPrime(7).X()));

    TVector2 rocEdge = mm->getCompound().getROCsPrime(0) - mm->getCompound().getROCsPrime(7);
    double angle = TMath::PiOver2() - TMath::ATan2(rocEdge.Y(), rocEdge.X());
    fHists["angleChips"]->Fill(angle);

    TVector2 hdiEdge = mm->getCompound().getHDIPrime(0) - mm->getCompound().getHDIPrime(7);
    double angle2 = TMath::ACos(hdiEdge * rocEdge);
    fHists["angleChipsHDI"]->Fill(angle2);
  }
}

// ----------------------------------------------------------------------
void modulesAnalysis::plotAll() {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  shrinkPad(0.15, 0.15, 0.1, 0.1);
  for (auto h : fHists) {
    h.second->Draw();
    c1->SaveAs((h.first + ".pdf").c_str());
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
  c1->SaveAs("glueTests.pdf");

}