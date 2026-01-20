#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "TMath.h"
using namespace cv;
using namespace std;
// ----------------------------------------------------------------------
// hdiMarkersFromJPG.cc
//
// Usage:
//   ./hdiMarkersFromJPG -f 250109-M035_0255.JPG -o moduleTemplate.marks
//
// It:
//   - reads the JPG
//   - finds concentric-circle markers (M0, M1, M2)
//   - prints their pixel coordinates
//   - optionally writes a 2-line CSV "marks" file compatible with compound::parseSvgFile
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
struct MarkerCandidate {
    cv::Point2f center;
    float       rInner;
    float       rOuter;
};

// ---------------------------------------------------------------------- 
int main(int argc, char** argv) {
    std::string filename = "250109-M035_0255.JPG";
    std::string outMarksFile;  // e.g. "moduleTemplate.marks"
    
    // Simple CLI parsing
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            filename = argv[++i];
        } else if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outMarksFile = argv[++i];
        }
    }
    
    cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Cannot read image: " << filename << std::endl;
        return 1;
    }
    
    cout << "img.size() = " << img.size() << endl;
    
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);
    
    // -- find circles with HoughCircles
    std::vector<cv::Vec3f> circles;
    double dp        = 1.5;    // inverse accumulator ratio
    double minDist   = 500.0;  // min distance between centers (px)
    double param1    = 200.0;  // Canny high threshold
    double param2    = 50.0;   // accumulator threshold
    int    minRadius = 25;     // px
    int    maxRadius = 30;    // px
    
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, minRadius, maxRadius);
    
    std::cout << "Found " << circles.size() << " raw circles\n";
    
    // -- draw result for visual debugging
    cv::Mat vis1 = img.clone();
    for (size_t k = 0; k < circles.size(); ++k) {
        const auto& c = circles[k];
        cv::Scalar color(0, 0, 255 - int(80 * k));  // different reds
        cv::circle(vis1, cv::Point2f(c[0], c[1]), int(c[2]), color, 2);
        cv::putText(vis1, "C" + std::to_string(k),
        cv::Point2f(c[0], c[1]) + cv::Point2f(10, -10),
        cv::FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "C" << k << " center (px): "
        << c[0] << ", " << c[1]
        << "  r=" << c[2] << endl;
    }
    
    
    // -- try to find concentric circles with fine-tuned radii
    minRadius = 55;
    maxRadius = 65;
    param1    = 130;  
    param2    = 50;
    vector<cv::Vec3f> circles2;
    
    cv::HoughCircles(gray, circles2, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, minRadius, maxRadius);
    std::cout << "Found " << circles2.size() << " raw circles\n";
    
    
    std::vector<cv::Vec3f> hdiMarkers;
    double epsilon = 10.0; // px
    for (size_t k = 0; k < circles2.size(); ++k) {
        const auto& c = circles2[k];
        cv::Scalar color(0, 255, 0);  // different reds
        cv::circle(vis1, cv::Point2f(c[0], c[1]), int(c[2]), color, 2);
        cv::putText(vis1, "D" + std::to_string(k),
        cv::Point2f(c[0], c[1]) + cv::Point2f(10, -10),
        cv::FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "D" << k << " center (px): "
        << c[0] << ", " << c[1]
        << "  r=" << c[2] << endl;
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
        
    // ------------------------------------------------------------------
    cout << "Found " << hdiMarkers.size() << " concentric pairs of HDI markers\n";
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        const auto& c = hdiMarkers[k];
        cout << "HDI marker " << k << " center (px): "
        << c[0] << ", " << c[1]
        << endl;
    }
    
    cv::imshow("circles", vis1);
    cv::waitKey(0);
    
    return 0;
}
