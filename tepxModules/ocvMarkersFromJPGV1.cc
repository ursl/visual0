#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <ostream>
#include <vector>
#include <cmath>
#include <string>
#include <map>

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
    
    void determineTransformation() {
        TVector2 ySvg = pMarkers[0] - pMarkers[1];
        ySvg /= ySvg.Mod();
        
        TVector2 xSvg = pMarkers[2] - pMarkers[1];
        xSvg /= xSvg.Mod();
        offset = pMarkers[1];
        
        // -- rotation angles determined from  x- and y-axes
        double theta0 = TMath::ACos(xSvg * TVector2(1., 0.));
        double theta1 = TMath::ACos(ySvg * TVector2(0., 1.));
        // -- average rotation angle - this implies that neither M0 nor M2 
        //    will be *perfectly* aligned in HDI CS
        alpha = 0.5 * (theta0 + theta1);
        
        // -- make sure rotation is towards y-axis
        TVector2 xOld = TVector2(1, 0);
        double   rsgn = xOld.X() * xSvg.Y() - xOld.Y() * xSvg.X();
        alpha -= TMath::Pi();
        alpha = -TMath::Pi() + (rsgn < 0 ? alpha : -1. * alpha);

        // -- determine scale factor
        double dm0m1 = 23.0; // mm
        double dm1m2 = 40.0; // mm
      
        double pxDiff1 = (pMarkers[0] - pMarkers[1]).Mod();
        double pxDiff2 = (pMarkers[1] - pMarkers[2]).Mod();
      
        sf = 0.5 * (dm0m1 / pxDiff1 + dm1m2 / pxDiff2);
    }
    
    // Forward: image pixel p -> HDI primed p' = R(alpha) * (p - offset),  offset = M1.
    TVector2 transform(TVector2 r, double theta, TVector2 t) {
        TVector2 rp = r - t;
        double   xnew = TMath::Cos(theta) * rp.X() - TMath::Sin(theta) * rp.Y();
        double   ynew = TMath::Sin(theta) * rp.X() + TMath::Cos(theta) * rp.Y();
        return TVector2(xnew, ynew);
    }

    // Inverse: p = R(-alpha) * p' + offset.  NOT R(-alpha)*(p' - offset) 
    TVector2 fromHDIFrame(TVector2 primed) {
        const double c = TMath::Cos(-alpha);
        const double s = TMath::Sin(-alpha);
        return TVector2(c * primed.X() - s * primed.Y() + offset.X(),
                        s * primed.X() + c * primed.Y() + offset.Y());
    }
    
    std::vector<TVector2> transformToHDIFrame(std::vector<TVector2> &orig) {
        std::vector<TVector2> trsf(orig.size());
        for (unsigned it = 0; it < orig.size(); ++it) {
            trsf[it] = transform(orig[it], alpha, offset);
        }
        return trsf;
    }

    TVector2 transformToHDIFrame(TVector2 orig) {
        std::vector<TVector2> one = {orig};
        return transformToHDIFrame(one)[0];
    }

    // HDI primed (pixel units) or mm/sf -> image pixel coordinates.
    std::vector<TVector2> transformToSVGFrame(std::vector<TVector2> &orig) {
        std::vector<TVector2> trsf(orig.size());
        for (unsigned it = 0; it < orig.size(); ++it) {
            trsf[it] = fromHDIFrame(orig[it]);
        }
        return trsf;
    }

    TVector2 transformToSVGFrame(TVector2 orig) {
        return fromHDIFrame(orig);
    }

    // HDI primed position in mm -> image pixel coordinates.
    TVector2 hdiMmToPixel(TVector2 hdiMm) {
        return fromHDIFrame(TVector2(hdiMm.X() / sf, hdiMm.Y() / sf));
    }

};


// ----------------------------------------------------------------------
// helper to construct the composite chip-marker template
// if mirrorRight is true, the template is horizontally flipped (RHS version)
cv::Mat makeChipTemplate(bool mirrorRight = false) {
    // Create a composite template: 3 rectangles (9x14 each), widely separated,
    // then three rectangles nearly adjacent
    int rectW = 8; // 70um according to measurement by WE
    int rectH = 12;
    int centerSpacing = 23; // 200um according to RD53 drawing
    int gap = centerSpacing - rectW; // "big" gap between rectangles
    int gap2 = 32;  
    
    // Template size: enough to fit all three rectangles
    // Total width: left padding + rect1 + gap + rect2 + gap + rect3 + right padding
    int leftPadding = 0; //rectW;
    int templateW = leftPadding + rectW + gap + rectW + gap + rectW;
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
    
    // int x3 = x2  + gap2;
    // cv::Rect r3(x3, 0, rectW, rectH);
    // cv::rectangle(templateRect, r3, cv::Scalar(206, 200, 195), -1);
    
    // int x4 = x3 + rectW + 1;
    // cv::Rect r4(x4, 0, rectW, rectH);
    // cv::rectangle(templateRect, r4, cv::Scalar(206, 200, 195), -1);
    
    // int x5 = x4 + rectW + 1;
    // cv::Rect r5(x5, 0, rectW, rectH);
    // cv::rectangle(templateRect, r5, cv::Scalar(206, 200, 195), -1);
    
    // Mirror horizontally for RHS pattern if requested
    if (mirrorRight) {
        cv::Mat flipped;
        cv::flip(templateRect, flipped, 1); // flip around y-axis
        return flipped;
    }
    
    return templateRect;
}

namespace ChipPattern {
constexpr int kRectW = 8;
constexpr int kRectH = 12;
constexpr int kCenterSpacing = 23;
constexpr int kPadGap = kCenterSpacing - kRectW; // edge-to-edge gap between wide pads
constexpr int kMarkerVisRadius = 10;
} // namespace ChipPattern

// LHS:  +__[]__[]__[]     marker one pad-gap left of the first wide pad
// RHS:  []__[]__[]__+     marker one pad-gap right of the third wide pad
cv::Point chipMarkerFromMatch(const cv::Point& matchPt, bool isRhs) {
    using namespace ChipPattern;
    const int bondY = matchPt.y + kRectH / 2;
    if (isRhs) {
        const double scale = 1.1;
        const int thirdPadRight = matchPt.x + 2 * kCenterSpacing + kRectW;
        return cv::Point(thirdPadRight + scale*kPadGap, bondY);
    }
    const double scale = 1.2;
    return cv::Point(matchPt.x - scale*kPadGap, bondY);
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

// Marker on RHS (+ right of wide pads) vs LHS (+ left). Matches HdiLayout::chipCornerNominals.
bool isRhsRoc(const std::string& rocName) {
    const int chipRow = rocName[3] - '0';
    const int chipCol = rocName[4] - '0';
    return (chipRow >= 2) ? (chipCol == 0) : (chipCol == 1);
}

struct RocRoi {
    std::string name;
    std::string chipName;
    bool isRhs;
    cv::Rect rect;
};

bool searchMagicPatternInRoi(const cv::Mat& img, const RocRoi& roi,
                             const cv::Mat& templateLHS, const cv::Mat& templateRHS,
                             double threshold, cv::Point& matchPt, double& score) {
    const cv::Rect bounds(0, 0, img.cols, img.rows);
    const cv::Rect clipped = roi.rect & bounds;
    const cv::Mat& tmpl = roi.isRhs ? templateRHS : templateLHS;
    if (clipped.width < tmpl.cols || clipped.height < tmpl.rows) {
        return false;
    }
    const cv::Mat patch = img(clipped);
    cv::Mat result;
    cv::matchTemplate(patch, tmpl, result, cv::TM_CCOEFF_NORMED);
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &score, nullptr, &maxLoc);
    if (score < threshold) {
        return false;
    }
    matchPt = clipped.tl() + maxLoc;
    return true;
}




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
    
    // -- test new lCompound
    lCompound lc(tvhdiMarkers);
    lc.determineTransformation();
    std::vector<TVector2> hdiMarkersPrime = lc.transformToHDIFrame(tvhdiMarkers);
    std::vector<TVector2> svgMarkersPrime = lc.transformToSVGFrame(hdiMarkersPrime);
    
    if (verbose) {
        cout << "Found " << hdiMarkers.size() << " concentric pairs of HDI markers" << endl;
        cout << "Scale factor: " << lc.sf << endl;
        cout << "Rotation: " << lc.alpha << endl;
        for (size_t k = 0; k < hdiMarkers.size(); ++k) {
            const auto& c = hdiMarkers[k];
            Scalar color(0, 0, 255 - int(80 * k));  // different reds
            putText(vis1, "M" + std::to_string(k), Point2f(c[0]-15, c[1]+10), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
            cout << "HDI marker " << k << " center (px): "
            << c[0] << ", " << c[1]
            << " tv marker: " << tvhdiMarkers[k].X() << ", " << tvhdiMarkers[k].Y()
            << "  transformed to HDI frame: " << hdiMarkersPrime[k].X() << ", " << hdiMarkersPrime[k].Y()
            << "  back to SVG frame: " << svgMarkersPrime[k].X() << ", " << svgMarkersPrime[k].Y()
            << endl;
        }
    }
    
    // -- determine and display ROIs
    const int boxSizeX = 120;
    const int boxSizeY = 40;
    const double yTopRocMm = 11.5 + 18.8;
    const double yBottomRocMm = yTopRocMm - 37.25;
    struct HdiRocSpec {
        std::string name;
        double xMm;
        double yMm;
    };
    const std::vector<HdiRocSpec> hdiRocSpecs = {
        {"ROC00", 39.3 + 2.7, yTopRocMm},
        {"ROC01", 18.5 + 2.7, yTopRocMm},
        {"ROC10", 20.0 - 0.1, yTopRocMm},
        {"ROC11", -0.6 + 0.0, yTopRocMm},
        {"ROC20", -0.6 + 0.0, yBottomRocMm},
        {"ROC21", 20.0 - 0.1, yBottomRocMm},
        {"ROC30", 18.5 + 2.7, yBottomRocMm},
        {"ROC31", 39.3 + 2.7, yBottomRocMm}
    };
    std::vector<RocRoi> rocRois;
    rocRois.reserve(hdiRocSpecs.size());
    for (const auto& spec : hdiRocSpecs) {
        TVector2 hdiROC{spec.xMm, spec.yMm};
        TVector2 svgROC = lc.hdiMmToPixel(hdiROC);
        const cv::Rect roiRect(static_cast<int>(svgROC.X()), static_cast<int>(svgROC.Y()),
                               boxSizeX, boxSizeY);
        RocRoi roi;
        roi.name = spec.name;
        roi.chipName = "chip" + spec.name.substr(3);
        roi.isRhs = isRhsRoc(spec.name);
        roi.rect = roiRect;
        rocRois.push_back(roi);
        cv::rectangle(vis1, roiRect, Scalar(0, 255, 255), 2);
        putText(vis1, spec.name, Point2f(svgROC.X(), svgROC.Y()) + cv::Point2f(-30, -30),
                FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 2);
        cout << spec.name << " (mm): " << spec.xMm << ", " << spec.yMm
             << " -> pixel: " << svgROC.X() << ", " << svgROC.Y()
             << "  sf: " << lc.sf << endl;
    }

    // -- now search for the simple pattern in ROIs
    const cv::Mat templateLHS = makeChipTemplate(false);
    //    const cv::Mat templateRHS = makeChipTemplate(true);
    const double matchThreshold = 0.40;
    struct SvgRocSpec {
        std::string name;
        cv::Point markerPt;
        double score;
    };

    std::map<std::string, SvgRocSpec> chipMarkers;
    for (const auto& spec : hdiRocSpecs) {
        const std::string chipName = "chip" + spec.name.substr(3);
        chipMarkers[chipName] = SvgRocSpec{chipName, cv::Point(-1, -1), 0.0};
    }
    int nMatched = 0;
    for (const auto& roi : rocRois) {
        SvgRocSpec hit;
        hit.name = roi.chipName;
        cv::Point matchPt;
        const bool found = searchMagicPatternInRoi(img, roi, templateLHS, templateLHS,
                                                   matchThreshold, matchPt, hit.score);
        const Scalar color = found ? Scalar(0, 200, 0) : Scalar(0, 0, 255);
        if (found) {
            hit.markerPt = chipMarkerFromMatch(matchPt, roi.isRhs);
            chipMarkers[roi.chipName] = hit;
            cv::rectangle(vis1,
                          cv::Rect(matchPt, cv::Size(templateLHS.cols, templateLHS.rows)),
                          color, 2);
            cv::circle(vis1, hit.markerPt, ChipPattern::kMarkerVisRadius, Scalar(255, 255, 0), 2);
            nMatched++;
        }
        cout << roi.name << ": " << (found ? "match" : "no match")
             << " score=" << hit.score;
        if (found) {
            cout << " marker at " << hit.markerPt.x << ", " << hit.markerPt.y;
        }
        cout << endl;
    }
    cout << "Simple pattern: matched " << nMatched << "/" << rocRois.size() << " ROIs" << endl;

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
    outFile << "  ]," << std::endl;
    outFile << "  \"chipMarkers\": [" << std::endl;
    bool firstChip = true;
    for (const auto& entry : chipMarkers) {
        if (!firstChip) {
            outFile << "," << std::endl;
        }
        firstChip = false;
        outFile << "    {" << std::endl;
        outFile << "      \"name\": \"" << entry.second.name << "\"," << std::endl;
        outFile << "      \"x\": " << entry.second.markerPt.x << "," << std::endl;
        outFile << "      \"y\": " << entry.second.markerPt.y << "," << std::endl;
        outFile << "      \"score\": " << entry.second.score << std::endl;
        outFile << "    }";
    }
    outFile << std::endl << "  ]" << std::endl;
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