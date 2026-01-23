#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <ostream>
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
bool isInRange(double x, const vector<double> &xVec) {
  for (size_t k = 0; k < xVec.size(); ++k) {
    if (TMath::Abs(x - xVec[k]) < 250) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
map<string, cv::Point> combineChipMatches(const vector<Point> &matchesLHS, const vector<Point> &matchesRHS, const vector<double> &xVec) {
    map<string, cv::Point> chipMarkers;
    // RHS x-divider: 5000, LHS x-divider: 3000
    // -- LHS
    // Match 0 center (px): 1504, 4313 circle coordinates: 1489, 4318
    // Match 1 center (px): 3889, 214 circle coordinates: 3874, 219
    // Match 2 center (px): 3905, 4305 circle coordinates: 3890, 4310
    // Match 3 center (px): 1491, 223 circle coordinates: 1476, 228
    // -- RHS
    // Match 0 center (px): 3732, 4305 circle coordinates: 3863, 4310
    // Match 1 center (px): 3715, 215 circle coordinates: 3846, 220
    // Match 2 center (px): 6111, 208 circle coordinates: 6242, 213
    // Match 3 center (px): 6130, 4295 circle coordinates: 6261, 4300

    cout << "xVec = " << xVec[0] << ", " << xVec[1] << ", " << xVec[2] << endl;

    string chipName = "chip";
    int xDivider = 2500;
    int yDivider = 2500;
    // -- LHS
    for (size_t k = 0; k < matchesLHS.size(); ++k) {
        // -- skip obviously bad matches
        if (matchesLHS[k].x > 5000) continue;
        if (!isInRange(matchesLHS[k].x, xVec)) {
            cout << "skipping LHS match " << k << " because it is not in range" << endl;
            continue;
        }

        if (matchesLHS[k].x < xDivider) {
            if (matchesLHS[k].y < yDivider) {
                chipName = "chip00";
            } else {
                chipName = "chip31";
            }
        } else {
            if (matchesLHS[k].y < yDivider) {
                chipName = "chip10";
            } else {
                chipName = "chip21";
            }
        }
        chipMarkers[chipName] = matchesLHS[k];
    }
    
    // -- RHS
    xDivider = 5000;
    for (size_t k = 0; k < matchesRHS.size(); ++k) {
        // -- skip obviously bad matches
        if (matchesRHS[k].x < 2500) continue;
        if (!isInRange(matchesRHS[k].x, xVec)) {
            cout << "skipping RHS match " << k << " because it is not in range" << endl;
            continue;
        }

        if (matchesRHS[k].x < xDivider) {
            if (matchesRHS[k].y < yDivider) {
                chipName = "chip01";
            } else {
                chipName = "chip30";
            }
        } else {
            if (matchesRHS[k].y < yDivider) {
                chipName = "chip11";
            } else {
                chipName = "chip20";
            }
        }
        chipMarkers[chipName] = matchesRHS[k];
    }
    
    // -- fill missing entries
    vector<string> chipNames = {"chip00", "chip01", "chip10", "chip11", "chip20", "chip21", "chip30", "chip31"};
    for (const auto& chipName : chipNames) {
        if (chipMarkers.find(chipName) == chipMarkers.end()) {
            chipMarkers[chipName] = cv::Point(-1, -1);
        }
    }
    
    
    return chipMarkers;
}



// ----------------------------------------------------------------------
// find chip-marker matches in the template match result, applying
// thresholding, geometric vetoes using HDI markers, and non-maximum suppression
std::vector<cv::Point> findChipMatches(const cv::Mat &result, const vector<cv::Vec3f> &hdiMarkers, double threshold, double minDist2) {
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
    int rectW = 9; // 70um according to measurement by WE
    int rectH = 12;
    int centerSpacing = 25; // 200um according to RD53 drawing
    int gap = centerSpacing - rectW; // "big" gap between rectangles
    int gap2 = 32;  
    
    // Template size: enough to fit all three rectangles
    // Total width: left padding + rect1 + gap + rect2 + gap + rect3 + right padding
    int leftPadding = rectW;
    int templateW = leftPadding + rectW + gap + rectW + gap + rectW + gap2 + 3 * (rectW) - 6; /*I don't know why I need to subtract this?!*/
    int templateH = rectH; // add small padding top/bottom
    
    cv::Mat templateRect = cv::Mat(templateH, templateW, CV_8UC3, cv::Scalar(143, 168, 202));  // B, G, R
    
    // -- construct the pattern: three widely spaced then three nearly adjacent
    int y0 = templateH / 2;         // center y for all rectangles
    int x0 = leftPadding;           // first rectangle left x
    cv::Rect r0(x0, 0, rectW, rectH);
    cv::rectangle(templateRect, r0, cv::Scalar(206, 200, 195), -1); // filled rectangle
    
    int x1 = x0 + centerSpacing;
    cv::Rect r1(x1, 0, rectW, rectH);
    cv::rectangle(templateRect, r1, cv::Scalar(206, 200, 195), -1);
    
    int x2 = x1 + centerSpacing;
    cv::Rect r2(x2, 0, rectW, rectH);
    cv::rectangle(templateRect, r2, cv::Scalar(206, 200, 195), -1);
    
    int x3 = x2  + gap2;
    cv::Rect r3(x3, 0, rectW, rectH);
    cv::rectangle(templateRect, r3, cv::Scalar(206, 200, 195), -1);
    
    int x4 = x3 + rectW + 1;
    cv::Rect r4(x4, 0, rectW, rectH);
    cv::rectangle(templateRect, r4, cv::Scalar(206, 200, 195), -1);
    
    int x5 = x4 + rectW + 1;
    cv::Rect r5(x5, 0, rectW, rectH);
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
int modulePosition(int moduleNumber) {
    static map<int, int> modulePositions = {
      {1000, 0}, {1001, 0}, {1002, 1}, {1003, 2}, {1004, 0}, {1005, 1}, {1006, 0}, {1007, 1}, {1008, 2}, {1009, 3},
      {1010, 4}, {1011, 0}, {1012, 0}, {1013, 1}, {1014, 2}, {1015, 1}, {1016, 2}, {1017, 3}, {1018, 0}, {1019, 3},
      {1020, 4}, {1021, 5}, {1022, 4}, {1023, 4}, {1024, 0}, {1025, 1}, {1026, 2}, {1027, 1}, {1028, 2}, {1029, 3},
      {1030, 4}, {1031, 0}, {1032, 1}, {1033, 3}, {1034, 4}, {1035, 0}, {1036, 0}, {1037, 1}, {1038, 2}, {1039, 3},
      {1040, 4}, {1041, 5}, {1042, 0}, {1043, 1}, {1044, 2}, {1045, 3}, {1046, 4}, {1047, 5}, {1048, 0}, {1049, 1},
      {1050, 2}, {1051, 0}, {1052, 1}, {1053, 2}, {1054, 3}, {1055, 2}, {1056, 3}, {1057, 0}, {1058, 1}, {1059, 3},
      {1060, 4}, {1061, 5}, {1062, 4}, {1063, 4}, {1064, 0}, {1065, 1}, {1066, 2}, {1067, 3}, {1068, 4}, {1069, 5},
      {1070, 0}, {1071, 1}, {1072, 2}, {1073, 3}, {1074, 4}, {1075, 5}, {1076, 0}, {1077, 1}, {1078, 2}, {1079, 3},
      {1080, 5}, {1081, 0}, {1082, 1}, {1083, 2}, {1084, 3}, {1085, 4}, {1086, 5}, {1087, 0}, {1088, 1}, {1089, 2},
      {1090, 3}, {1091, 4}, {1092, 5}, {1093, 0}, {1094, 1}, {1095, 2}, {1096, 0}, {1097, 1}, {1098, 2}, {1099, 3},
      {1100, 4}, {1101, 5}, {1102, 0}, {1103, 1}, {1104, 2}, {1105, 3}, {1106, 4}, {1107, 0}, {1108, 1}, {1109, 2},
      {1110, 1}, {1111, 2}, {1112, 3}, {1113, 4}, {1114, 5}, {1115, 0}, {1116, 1}, {1117, 2}, {1118, 3}, {1119, 4},
      {1120, 5}, {1121, 0}, {1122, 1}, {1123, 2}, {1124, 3}, {1125, 0}, {1126, 1}, {1127, 2}, {1128, 3}, {1129, 4},
      {1130, 5}, {1131, 0}, {1132, 1}, {1133, 2}
     };
    if (modulePositions.find(moduleNumber) == modulePositions.end()) {
      cout << "Module number " << moduleNumber << " not found in modulePositions" << endl;
      return -1;
    }
    return modulePositions[moduleNumber];
  }
  


// ---------------------------------------------------------------------- 
int main(int argc, char** argv) {
    bool verbose(false);
    string directory = "json";
    string filename = "250109-M035_0255.JPG";
    string outMarksFile;  // e.g. "moduleTemplate.marks"
    bool visualize(false);    
    int moduleNumber(-1);
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "-d" && i + 1 < argc) { directory = argv[++i]; }
        if (string(argv[i]) == "-f" && i + 1 < argc) { filename = argv[++i]; }
        if (string(argv[i]) == "-o" && i + 1 < argc) { outMarksFile = argv[++i]; }
        if (string(argv[i]) == "-v") { visualize = true; verbose = true; }
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
    // -- find circles with HoughCircles
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
    
    cout << "Found " << hdiMarkers.size() << " concentric pairs of HDI markers\n";
    for (size_t k = 0; k < hdiMarkers.size(); ++k) {
        const auto& c = hdiMarkers[k];
        Scalar color(0, 0, 255 - int(80 * k));  // different reds
        putText(vis1, "M" + std::to_string(k), Point2f(c[0]-15, c[1]+10), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        cout << "HDI marker " << k << " center (px): "
        << c[0] << ", " << c[1]
        << endl;
    }
    
    
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
    
    double offsetX = 7;
    
    // -- alignment crosses
    vector<cv::Point> lhsAC, rhsAC;
    
    vector<cv::Point> matchesLHS = findChipMatches(resultLHS, hdiMarkers, threshold, minDist2);
    for (size_t k = 0; k < matchesLHS.size(); ++k) {
        const auto& m = matchesLHS[k];
        Scalar color(0, 0, 255);  // red
        
        // Draw template at match location (m is top-left corner from matchTemplate)
        cv::Rect roi(m.x, m.y, templateLHS.cols, templateLHS.rows);
        // Make sure ROI is within image bounds
        if (roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= vis1.cols && roi.y + roi.height <= vis1.rows) {
            cv::Mat roiMat = vis1(roi);
            cv::addWeighted(roiMat, 0.5, templateLHS, 0.5, 0, roiMat);
            cv::rectangle(vis1, roi, color, 1);
            
            // Draw a small circle just left of the LHS template
            cv::Point circleCenter(roi.x - offsetX, roi.y + roi.height / 2);
            int circleRadius = 6;
            cv::circle(vis1, circleCenter, circleRadius, color, 1);
            lhsAC.push_back(circleCenter);
            
            putText(vis1, "LHS" + std::to_string(k), Point2f(m.x-15, m.y+60), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
            if (verbose) {
                cout << "Match " << k << " center (px): "
                << m.x << ", " << m.y
                << " circle coordinates: " << circleCenter.x << ", " << circleCenter.y
                << endl;
            }
        }
    }    
    
    
    vector<Point> matchesRHS = findChipMatches(resultRHS, hdiMarkers, threshold, minDist2);
    for (size_t k = 0; k < matchesRHS.size(); ++k) {
        const auto& m = matchesRHS[k];
        Scalar color(255, 0, 0);  // blue
        cv::Rect roi(m.x, m.y, templateRHS.cols, templateRHS.rows);
        if (roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= vis1.cols && roi.y + roi.height <= vis1.rows) {
            cv::Mat roiMat = vis1(roi);
            cv::addWeighted(roiMat, 0.5, templateRHS, 0.5, 0, roiMat);
            cv::rectangle(vis1, roi, color, 1);
            
            // Draw a small circle just right of the RHS template
            cv::Point circleCenter(roi.x + roi.width + offsetX, roi.y + roi.height / 2);
            int circleRadius = 6;
            cv::circle(vis1, circleCenter, circleRadius, color, 1);
            rhsAC.push_back(circleCenter);
            putText(vis1, "RHS" + std::to_string(k), Point2f(m.x-15, m.y+60), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
            if (verbose) {
                cout << "Match " << k << " center (px): "
                << m.x << ", " << m.y
                << " circle coordinates: " << circleCenter.x << ", " << circleCenter.y
                << endl;
            }
        }
    }
    
    if (visualize) {
        cv::imshow("templateLHS", templateLHS);
        cv::imshow("templateRHS", templateRHS);
    }
    
    vector<double> xVec = {hdiMarkers[2][0], (0.5*(hdiMarkers[0][0] + hdiMarkers[2][0])), hdiMarkers[0][0]};
   

    map<string, cv::Point> chipMarkers = combineChipMatches(lhsAC, rhsAC, xVec);
    for (const auto& [chipName, chipPoint] : chipMarkers) {
        cout << "Chip " << chipName << " center (px): "
        << chipPoint.x << ", " << chipPoint.y
        << endl;
    }
    
    // -- write the matches to a JSON file
    std::ofstream outFile(outMarksFile);
    outFile << "{" << std::endl;
    outFile << "  \"filename\": \"" << filename << "\"," << std::endl;
    outFile << "  \"moduleNumber\": " << moduleNumber << "," << std::endl;
    outFile << "  \"modulePosition\": " << modulePosition(moduleNumber) << "," << std::endl;
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
    outFile << "  ]," << std::endl;
    outFile << "  \"chipMarkers\": [" << std::endl;
    size_t chipIndex = 0;
    for (const auto& [chipName, chipPoint] : chipMarkers) {
        outFile << "    {" << std::endl;
        outFile << "      \"name\": \"" << chipName << "\", " << std::endl;
        outFile << "      \"x\": " << chipPoint.x << ", " << std::endl;
        outFile << "      \"y\": " << chipPoint.y << endl;
        outFile << "    }";
        if (++chipIndex < chipMarkers.size()) {
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
    
    if (visualize) {
        cv::imshow("vis1", vis1);
        cv::waitKey(0);
    }
    return 0;
}