#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <ostream>
#include <vector>
#include <cmath>
#include <string>

#include "TVector2.h"
#include "TMath.h"

using namespace cv;
using namespace std;
// ----------------------------------------------------------------------
// ocvMarkersFromJPG.cc
//
// Usage:
//   ./bin/ocvMarkersFromJPG -f 250109-M035_0255.JPG -
//   ./bin/ocvMarkersFromJPG -f image.JPG -n 7   // narrow pads in corner signature (holistic finder)
//
// It:
//   - reads the JPG
//   - finds concentric-circle markers (M0, M1, M2)
//   - prints their SVG coordinates
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
struct lCompound {
    std::vector<TVector2> pMarkers;
    TVector2 offset;

    double alpha; 
    double sf;
    

    lCompound(std::vector<TVector2> pMarkers) {
        this->pMarkers = pMarkers;
        determineTransformation();
    }
    
    TVector2 transform(TVector2 r, double theta, TVector2 t) {
        TVector2 rp = r - t;
        double   xnew = TMath::Cos(theta) * rp.X() - TMath::Sin(theta) * rp.Y();
        double   ynew = TMath::Sin(theta) * rp.X() + TMath::Cos(theta) * rp.Y();
        return TVector2(xnew, ynew);
    }
    
    
    std::vector<TVector2> transformToHDIFrame(std::vector<TVector2> &orig) {
        std::vector<TVector2> trsf(orig.size());
        for (unsigned it = 0; it < orig.size(); ++it) {
            trsf[it] = transform(orig[it], alpha, offset);
        }
        return trsf;
    }
    
    
    void determineTransformation() {
        TVector2 ySvg = pMarkers[0] - pMarkers[1];
        ySvg /= ySvg.Mod();
        
        TVector2 xSvg = pMarkers[2] - pMarkers[1];
        xSvg /= xSvg.Mod();
        offset = pMarkers[1];
        
        // -- average rotation angle from  both x- and y-axes (always < pi/2)
        double theta0 = TMath::ACos(xSvg * TVector2(1., 0.));
        double theta1 = TMath::ACos(ySvg * TVector2(0., 1.));
        double alpha = 0.5 * (theta0 + theta1);
        
        // -- make sure rotation is towards y-axis
        TVector2 xOld = TVector2(1, 0);
        double   rsgn = xOld.X() * xSvg.Y() - xOld.Y() * xSvg.X();
        alpha -= TMath::Pi();
        alpha = -TMath::Pi() + (rsgn < 0 ? alpha : -1. * alpha);
    }
};



// ---------------------------------------------------------------------- 
int main(int argc, char** argv) {
    bool verbose(true);
    string directory = "json";
    string filename = "modules/p1137.jpg";
    string outMarksFile = "json/p1137.json";  // e.g. "moduleTemplate.marks"
    int moduleNumber(-1);
    int nAdjacentRects(7);
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "-d" && i + 1 < argc) { directory = argv[++i]; }
        if (string(argv[i]) == "-f" && i + 1 < argc) { filename = argv[++i]; }
        if (string(argv[i]) == "-o" && i + 1 < argc) { outMarksFile = argv[++i]; }
        if (string(argv[i]) == "-n" && i + 1 < argc) { nAdjacentRects = stoi(argv[++i]); }
        if (string(argv[i]) == "-v") { verbose = true; }
    }
    
    if (string::npos != filename.find("p")) {
        moduleNumber = stoi(filename.substr(filename.find("p") + 1, 4));
    }
    cout << "moduleNumber = " << moduleNumber << endl;
    
    if (outMarksFile.empty()) {
        outMarksFile = filename.substr(filename.rfind('/') + 1);
        outMarksFile = directory + "/" + outMarksFile.substr(0, outMarksFile.rfind('.')) + ".json";
    }
    
    // -- read image
    cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Cannot read image: " << filename << std::endl;
        return 1;
    }
    
    cout << "img.size() = " << img.size() << endl;
    
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);
    
    // ------------------------------------------------------------------
    // -- find HDI marker concentric circles with HoughCircles
    std::vector<cv::Vec3f> circles;
    double dp        = 1.5;    // inverse accumulator ratio
    double minDist   = 500.0;  // min distance between centers (px)
    double param1    = 200.0;  // Canny high threshold
    double param2    = 50.0;   // accumulator threshold
    int    minRadius = 26;     // This is quite finely tuned - in principle should try to find the small circles 
    // after the big ones to confirm the scale
    int    maxRadius = 28;     // px
    
    if (1) cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, minRadius, maxRadius);
    
    // -- draw result for visual debugging
    cout << "circles.size() = " << circles.size() << endl;
    cv::Mat vis1 = img.clone();
    for (size_t k = 0; k < circles.size(); ++k) {
        const auto& c = circles[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        circle(vis1, cv::Point2f(c[0], c[1]), int(c[2]), color, 2);
        putText(vis1, "C" + std::to_string(k), Point2f(c[0], c[1]) + cv::Point2f(-30, -30), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        
        if (verbose) {  
            cout << "C" << k << " center (px): "
            << c[0] << ", " << c[1]
            << "  r=" << c[2] << endl;
        }
    }
    
    
    // -- try to find concentric circles with fine-tuned radii
    minRadius = 56;
    maxRadius = 65;
    param1    = 90;  
    param2    = 50;
    vector<cv::Vec3f> circles2;
    
    cv::HoughCircles(gray, circles2, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, minRadius, maxRadius);  
    
    std::vector<cv::Vec3f> hdiMarkers;
    double epsilon = 10.0; // px
    for (size_t k = 0; k < circles2.size(); ++k) {
        const auto& c = circles2[k];
        cv::Scalar color(0, 255, 0);  // different reds
        cv::circle(vis1, cv::Point2f(c[0], c[1]), int(c[2]), color, 2);
        putText(vis1, "D" + std::to_string(k), Point2f(c[0], c[1]) + cv::Point2f(60, -60),FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        if (verbose) {
            cout << "D" << k << " center (px): "
            << c[0] << ", " << c[1]
            << "  r=" << c[2] << endl;
        }
        for (size_t l = 0; l < circles.size(); ++l) {
            const auto& c2 = circles[l];
            if (TMath::Abs(c2[0] - c[0]) < epsilon  && TMath::Abs(c2[1] - c[1]) < epsilon) {
                Vec3f v(0.5*(c[0] + c2[0]), 0.5*(c[1] + c2[1]), 0.5*(c[2] + c2[2]));
                hdiMarkers.push_back(v);
                break;
            }
        }
    }
    
    // -- reorder hdiMarkers to be in order of increasing y and then x
    int idxMinY = 0;
    int idxMinX = 0;
    cout << "hdiMarkers.size() = " << hdiMarkers.size() << endl;
    
    // -- find index of smallest y and smallest x
    for (int i = 1; i < 3; ++i) {
        if (hdiMarkers[i][1] < hdiMarkers[idxMinY][1]) {
            idxMinY = i;
        }
        if (hdiMarkers[i][0] < hdiMarkers[idxMinX][0]) {
            idxMinX = i;
        }
    }
    
    // -- handle the (rare) case where the same marker is both min x and min y
    int idxMiddle;
    if (idxMinY != idxMinX) {
        // the remaining index is 0+1+2 - idxMinY - idxMinX
        idxMiddle = 0 + 1 + 2 - idxMinY - idxMinX;
    } else {
        // if the same point is both min x and min y, choose middle arbitrarily
        // here: next-by-x as "middle" and keep idxMinY as 0, idxMinX as 2
        int other1 = (idxMinX + 1) % 3;
        int other2 = (idxMinX + 2) % 3;
        idxMiddle = (hdiMarkers[other1][0] < hdiMarkers[other2][0]) ? other1 : other2;
        idxMinX   = 0 + 1 + 2 - idxMinY - idxMiddle;
    }
    
    // -- build the reordered vector
    std::vector<cv::Vec3f> ordered(3);
    ordered[0] = hdiMarkers[idxMinY]; // smallest y
    ordered[1] = hdiMarkers[idxMiddle];
    ordered[2] = hdiMarkers[idxMinX]; // smallest x
    
    hdiMarkers.swap(ordered);

    std::vector<TVector2> tvhdiMarkers(3);
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        tvhdiMarkers[k] = TVector2(hdiMarkers[k][0], hdiMarkers[k][1]);
    }

    lCompound lc(tvhdiMarkers);
    lc.determineTransformation();
    std::vector<TVector2> hdiMarkersPrime = lc.transformToHDIFrame(tvhdiMarkers);

    cout << "Found " << hdiMarkers.size() << " concentric pairs of HDI markers\n";
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        const auto& c = hdiMarkers[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        putText(vis1, "M" + std::to_string(k), Point2f(c[0]-15, c[1]+10), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "HDI marker " << k << " center (px): "
        << c[0] << ", " << c[1]
        << " tv marker: " << tvhdiMarkers[k].X() << ", " << tvhdiMarkers[k].Y()
        << "  transformed to HDI frame: " << hdiMarkersPrime[k].X() << ", " << hdiMarkersPrime[k].Y()
        << endl;
    }
    


    
    // -- write the matches to a JSON file
    std::ofstream outFile(outMarksFile);
    outFile << "{" << std::endl;
    outFile << "  \"filename\": \"" << filename << "\"," << std::endl;
    outFile << "  \"moduleNumber\": " << moduleNumber << "," << std::endl;
    outFile << "  \"hdiMarkers\": [" << std::endl;
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        const auto& m = hdiMarkers[k];
        outFile << "    {" << std::endl;
        outFile << "      \"x\": " << m[0] << ", " << std::endl;
        outFile << "      \"y\": " << m[1] << endl;
        outFile << "    }";
        if (k + 1 < hdiMarkers.size()) {
            outFile << ",";
        }
        outFile << std::endl;
    }
    outFile << "  ]" << std::endl; 
    outFile << "}" << std::endl;
    outFile.close();
    
    // Save visualization image (always, not just when visualizing)
    string visFilename = filename.substr(filename.rfind('/') + 1);
    visFilename = "png/" + visFilename.substr(0, visFilename.rfind('.')) + ".png";
    
    // Save with PNG compression (compression level 3: good balance of size vs speed)
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(3);  // 0-9: 0=no compression, 9=max compression
    
    cv::imwrite(visFilename, vis1, compression_params);
    cout << "Saved visualization to: " << visFilename << endl;
    
    return 0;
}