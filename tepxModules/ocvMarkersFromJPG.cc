#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "TMath.h"
using namespace cv;
using namespace std;
// ----------------------------------------------------------------------
// ocvMarkersFromJPG.cc
//
// Usage:
//   ./bin/ocvMarkersFromJPG -f 250109-M035_0255.JPG -o moduleTemplate.marks
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
// find chip-marker matches in the template match result, applying
// thresholding, geometric vetoes using HDI markers, and non-maximum suppression
std::vector<cv::Point> findChipMatches(const cv::Mat &result,
    const std::vector<cv::Vec3f> &hdiMarkers,
    double threshold,
    double minDist2) {
        // Get all candidate matches with their scores
        struct Match {
            cv::Point pt;
            double score;
        };
        std::vector<Match> candidates;
        for (int y = 0; y < result.rows; ++y) {
            for (int x = 0; x < result.cols; ++x) {
                float score = result.at<float>(y, x);
                if (score > threshold) {
                    candidates.push_back({cv::Point(x, y), score});
                }
            }
        }
        
        // Sort by score (highest first)
        std::sort(candidates.begin(), candidates.end(), [](const Match& a, const Match& b) { return a.score > b.score; });
        
        // Compute y/x bands between M0 and M1 to veto matches in that strip
        double y0m = hdiMarkers.size() > 0 ? hdiMarkers[0][1] : 0.0;
        double y1m = hdiMarkers.size() > 1 ? hdiMarkers[1][1] : 0.0;
        double yMin = std::min(y0m, y1m);
        double yMax = std::max(y0m, y1m);
               
        // Apply geometric vetoes and spatial non-maximum suppression
        std::vector<cv::Point> matches;
        for (const auto& cand : candidates) {
            // Skip matches whose y lies strictly between the two marker y positions
            if (hdiMarkers.size() > 1 && cand.pt.y > yMin && cand.pt.y < yMax) {
                continue;
            }
                       
            // Non-maximum suppression in space: enforce a minimum distance between matches
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
        
        std::cout << "Found " << matches.size()
        << " matches (after NMS from " << candidates.size()
        << " candidates)" << std::endl;
        
        return matches;
}
    
// ----------------------------------------------------------------------
// helper to construct the composite chip-marker template
// if mirrorRight is true, the template is horizontally flipped (RHS version)
cv::Mat makeChipTemplate(bool mirrorRight = false) {
    // Create a composite template: 3 rectangles (9x14 each), widely separated,
    // then three rectangles nearly adjacent
    int rectW = 9;
    int rectH = 10;
    int centerSpacing = rectH + rectW;
    int gap = centerSpacing - rectW; // "big" gap between rectangles
    
    // Template size: enough to fit all three rectangles
    // Total width: left padding + rect1 + gap + rect2 + gap + rect3 + right padding
    int leftPadding = 10;
    int templateW = leftPadding + rectW + gap + rectW + gap + rectW + 2 * gap + 3 * (rectW + 4);
    int templateH = rectH; // add small padding top/bottom
    
    cv::Mat templateRect = cv::Mat(templateH, templateW, CV_8UC3, cv::Scalar(143, 168, 202));  // B, G, R
    
    // -- construct the pattern: three widely spaced then three nearly adjacent
    int y0 = templateH / 2;         // center y for all rectangles
    int x0 = leftPadding;           // first rectangle left x
    cv::Rect r0(x0, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r0, cv::Scalar(206, 200, 195), -1); // filled rectangle
    
    int x1 = x0 + centerSpacing;
    cv::Rect r1(x1, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r1, cv::Scalar(206, 200, 195), -1);
    
    int x2 = x1 + centerSpacing;
    cv::Rect r2(x2, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r2, cv::Scalar(206, 200, 195), -1);
    
    int x3 = x2 + 2*centerSpacing;
    cv::Rect r3(x3, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r3, cv::Scalar(206, 200, 195), -1);
    
    int x4 = x3 + rectW + 1;
    cv::Rect r4(x4, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r4, cv::Scalar(206, 200, 195), -1);
    
    int x5 = x4 + rectW + 1;
    cv::Rect r5(x5, y0 - rectH / 2, rectW, rectH);
    cv::rectangle(templateRect, r5, cv::Scalar(206, 200, 195), -1);
    
    // Mirror horizontally for RHS pattern if requested
    if (mirrorRight) {
        cv::Mat flipped;
        cv::flip(templateRect, flipped, 1); // flip around y-axis
        return flipped;
    }
    
    return templateRect;
}
    
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
    // -- find chip markers using a composite templates
    cv::Mat templateLHS = makeChipTemplate(false);
    cv::Mat resultLHS;
    cv::matchTemplate(img, templateLHS, resultLHS, cv::TM_CCOEFF_NORMED);
    
    cv::Mat templateRHS = makeChipTemplate(true);
    cv::Mat resultRHS;
    cv::matchTemplate(img, templateRHS, resultRHS, cv::TM_CCOEFF_NORMED);
    
    // Find peaks (local maxima) with non-maximum suppression
    double threshold = 0.52; // tune: how strong a match you need
    double minDist2 = 100.0;  // minimum distance between matches (px)
    
    double offsetX = 15;
    double offsetY = 1;
    
    std::vector<cv::Point> matchesLHS = findChipMatches(resultLHS, hdiMarkers, threshold, minDist2);
    for (size_t k = 0; k < matchesLHS.size(); ++k) {
        const auto& m = matchesLHS[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        
        // Draw template at match location (m is top-left corner from matchTemplate)
        cv::Rect roi(m.x, m.y, templateLHS.cols, templateLHS.rows);
        // Make sure ROI is within image bounds
        if (roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= vis1.cols && roi.y + roi.height <= vis1.rows) {
            cv::Mat roiMat = vis1(roi);
            cv::addWeighted(roiMat, 0.5, templateLHS, 0.5, 0, roiMat);
            cv::rectangle(vis1, roi, color, 2);

             // Draw a small circle just left of the LHS template
             cv::Point circleCenter(roi.x - offsetX, roi.y + roi.height / 2);
             int circleRadius = 6;
             cv::circle(vis1, circleCenter, circleRadius, color, 2);

            putText(vis1, "P" + std::to_string(k), Point2f(m.x-15, m.y+60), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
             cout << "Match " << k << " center (px): "
             << m.x << ", " << m.y
             << " circle coordinates: " << circleCenter.x << ", " << circleCenter.y
            << endl;
        }
    }    


     std::vector<cv::Point> matchesRHS = findChipMatches(resultRHS, hdiMarkers, threshold, minDist2);
     for (size_t k = 0; k < matchesRHS.size(); ++k) {
         const auto& m = matchesRHS[k];
         Scalar color(0, 0, 255 - int(80 * k));  // different reds
         cv::Rect roi(m.x, m.y, templateRHS.cols, templateRHS.rows);
         if (roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= vis1.cols && roi.y + roi.height <= vis1.rows) {
             cv::Mat roiMat = vis1(roi);
             cv::addWeighted(roiMat, 0.5, templateRHS, 0.5, 0, roiMat);
             cv::rectangle(vis1, roi, color, 2);

              // Draw a small circle just right of the RHS template
              cv::Point circleCenter(roi.x + roi.width + offsetX,
                                     roi.y + roi.height / 2);
              int circleRadius = 6;
              cv::circle(vis1, circleCenter, circleRadius, color, 2);
            putText(vis1, "P" + std::to_string(k), Point2f(m.x-15, m.y+60), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
             cout << "Match " << k << " center (px): "
             << m.x << ", " << m.y
             << " circle coordinates: " << circleCenter.x << ", " << circleCenter.y
            << endl;
        }
    }

    if (0) {
        cv::imshow("templateLHS", templateLHS);
        cv::imshow("templateRHS", templateRHS);
    }
    cv::imshow("vis1", vis1);
    cv::waitKey(0);
    return 0;
}