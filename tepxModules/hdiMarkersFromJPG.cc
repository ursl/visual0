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
    
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            filename = argv[++i];
        } else if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outMarksFile = argv[++i];
        }
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
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        circle(vis1, cv::Point2f(c[0], c[1]), int(c[2]), color, 2);
        putText(vis1, "C" + std::to_string(k), Point2f(c[0], c[1]) + cv::Point2f(-30, -30), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
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
        putText(vis1, "D" + std::to_string(k), Point2f(c[0], c[1]) + cv::Point2f(60, -60),FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
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
    
    cout << "Found " << hdiMarkers.size() << " concentric pairs of HDI markers\n";
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        const auto& c = hdiMarkers[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        putText(vis1, "M" + std::to_string(k), Point2f(c[0]-15, c[1]+10), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "HDI marker " << k << " center (px): "
        << c[0] << ", " << c[1]
        << endl;
    }
    
    //cv::waitKey(0);
    
    
    // ------------------------------------------------------------------
    // -- find chip markers
       
    // Create a composite template: 3 rectangles (9x14 each), centers 27 px apart
    int rectW = 9;
    int rectH = 12;
    int centerSpacing = 27;
    
    // Template size: enough to fit all three rectangles
    // Total width: left padding + rect1 + gap + rect2 + gap + rect3 + right padding
    int templateW = rectW + centerSpacing + rectW + centerSpacing + 15; // = 9 + 27 + 9 + 27 + 9 = 81
    int templateH = rectH + 2; // add small padding top/bottom
    
    //cv::Mat templateRect = cv::Mat::zeros(templateH, templateW, CV_8UC1);
    cv::Mat templateRect = cv::Mat(templateH, templateW, CV_8UC1, cv::Scalar(90));
    
    // Draw three rectangles: Center positions within template (assuming we center the whole pattern)
    int x0 = rectW / 2;  // first rectangle center x
    int y0 = templateH / 2;  // center y for all rectangles
    
    // Rectangle 1
    int grayValue = 128;
    cv::Rect r1(x0 + 5, y0 - rectH/2, rectW, rectH);
    cv::rectangle(templateRect, r1, cv::Scalar(grayValue), -1); // filled white rectangle
    
    // Rectangle 2 (center at x0 + 27)
    int x1 = x0 + centerSpacing;
    cv::Rect r2(x1, y0 - rectH/2, rectW, rectH);
    cv::rectangle(templateRect, r2, cv::Scalar(grayValue), -1);
    
    // Rectangle 3 (center at x0 + 54)
    int x2 = x0 + 2 * centerSpacing;
    cv::Rect r3(x2, y0 - rectH/2, rectW, rectH);
    cv::rectangle(templateRect, r3, cv::Scalar(grayValue), -1);
    
    // Optional: blur slightly to make matching more robust
    //cv::GaussianBlur(templateRect, templateRect, cv::Size(3, 3), 0.5);
    
    cv::Mat result;
    cv::matchTemplate(gray, templateRect, result, cv::TM_CCOEFF_NORMED);
    
    // Find peaks (local maxima) with non-maximum suppression
    double threshold = 0.5; // tune: how strong a match you need
    double minDist2 = 20.0;  // minimum distance between matches (px)
    
    // Get all candidate matches with their scores
    struct Match {
        cv::Point pt;
        double score;
    };
    std::vector<Match> candidates;
    for (int y = 0; y < result.rows; ++y) {
        for (int x = 0; x < result.cols; ++x) {
            float score = result.at<float>(y, x);
            if (score >= threshold) {
                candidates.push_back({cv::Point(x, y), score});
            }
        }
    }
    
    // Sort by score (highest first)
    std::sort(candidates.begin(), candidates.end(),
              [](const Match& a, const Match& b) { return a.score > b.score; });
    
    // Non-maximum suppression: keep only matches far enough from already-kept ones
    std::vector<cv::Point> matches;
    for (const auto& cand : candidates) {
        bool tooClose = false;
        for (const auto& kept : matches) {
            double dist = cv::norm(cand.pt - kept);
            if (dist < minDist2) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            matches.push_back(cand.pt);
        }
    }
    
    cout << "Found " << matches.size() << " matches (after NMS from " << candidates.size() << " candidates)\n";
    for (size_t k = 0; k < matches.size(); ++k) {
        const auto& m = matches[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        
        // Draw template at match location (m is top-left corner from matchTemplate)
        cv::Rect roi(m.x, m.y, templateRect.cols, templateRect.rows);
        // Make sure ROI is within image bounds
        if (roi.x >= 0 && roi.y >= 0 && 
            roi.x + roi.width <= vis1.cols && 
            roi.y + roi.height <= vis1.rows) {
            // Convert template to BGR for visualization (if vis1 is color)
            cv::Mat templateBGR;
            cv::cvtColor(templateRect, templateBGR, cv::COLOR_GRAY2BGR);
            // Draw template with colored border
            cv::Mat roiMat = vis1(roi);
            cv::addWeighted(roiMat, 0.5, templateBGR, 0.5, 0, roiMat);
            cv::rectangle(vis1, roi, color, 2);
        }
        
        putText(vis1, "P" + std::to_string(k), Point2f(m.x-15, m.y+60), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "Match " << k << " center (px): "
        << m.x << ", " << m.y
        << endl;
    }
    
    cv::imshow("templateRect", templateRect);


    cv::imshow("circles", vis1);

    cv::waitKey(0);
    
    return 0;
}
