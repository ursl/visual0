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
//   ./bin/ocvMarkersFromJPG -f image.JPG -n 7   // narrow pads in corner signature (holistic finder)
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
std::vector<cv::Point> findChipMatches(const cv::Mat &result, const vector<cv::Vec3f> &hdiMarkers,
                                       const vector<double> &xVec, bool isLHS,
                                       double threshold, double minDist2) {
    struct Match {
        cv::Point pt;
        double score;
    };
    std::vector<Match> candidates;
    const int localRad = 4;
    for (int y = localRad; y < result.rows - localRad; ++y) {
        for (int x = localRad; x < result.cols - localRad; ++x) {
            float score = result.at<float>(y, x);
            if (score < threshold) {
                continue;
            }
            bool isPeak = true;
            for (int dy = -localRad; dy <= localRad && isPeak; ++dy) {
                for (int dx = -localRad; dx <= localRad; ++dx) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }
                    if (result.at<float>(y + dy, x + dx) > score) {
                        isPeak = false;
                        break;
                    }
                }
            }
            if (!isPeak) {
                continue;
            }
            if (isLHS && x > 5000) {
                continue;
            }
            if (!isLHS && x < 2500) {
                continue;
            }
            if (!xVec.empty() && !isInRange(x, xVec)) {
                continue;
            }
            candidates.push_back({cv::Point(x, y), score});
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Match& a, const Match& b) { return a.score > b.score; });

    double y0m = hdiMarkers.size() > 0 ? hdiMarkers[0][1] : 0.0;
    double y1m = hdiMarkers.size() > 1 ? hdiMarkers[1][1] : 0.0;
    double yMin = std::min(y0m, y1m);
    double yMax = std::max(y0m, y1m);

    std::vector<cv::Point> matches;
    for (const auto& cand : candidates) {
        if (hdiMarkers.size() > 1 && cand.pt.y > yMin && cand.pt.y < yMax) {
            continue;
        }
        bool tooClose = false;
        for (const auto& kept : matches) {
            if (cv::norm(cand.pt - kept) < minDist2) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            matches.push_back(cand.pt);
        }
    }

    std::cout << (isLHS ? "LHS" : "RHS") << " found " << matches.size()
              << " matches (after NMS from " << candidates.size()
              << " local-max candidates)" << std::endl;

    return matches;
}

// ----------------------------------------------------------------------
// helper to construct the composite chip-marker template
// if mirrorRight is true, the template is horizontally flipped (RHS version)
cv::Mat makeChipTemplate(bool mirrorRight = false, int nAdjacent = 3) {
    // Create a composite template: 3 rectangles (9x12 each), widely separated,
    // then n rectangles nearly adjacent (1 px apart)
    if (nAdjacent < 1) {
        nAdjacent = 1;
    }
    int rectW = 9; // 70um according to measurement by WE
    int rectH = 12;
    int centerSpacing = 25; // 200um according to RD53 drawing
    int gap = centerSpacing - rectW; // "big" gap between widely spaced rectangles
    int gap2 = 32;
    int adjacentGap = 1;
    
    int leftPadding = rectW;
    int widelySpacedW = 3 * rectW + 2 * gap;
    int adjacentW = nAdjacent * rectW + (nAdjacent - 1) * adjacentGap;
    int templateW = leftPadding + widelySpacedW + gap2 + adjacentW;
    int templateH = rectH;
    
    cv::Mat templateRect = cv::Mat(templateH, templateW, CV_8UC3, cv::Scalar(143, 168, 202));  // B, G, R
    
    // -- three widely spaced rectangles
    int x = leftPadding;
    for (int i = 0; i < 3; ++i) {
        cv::rectangle(templateRect, cv::Rect(x, 0, rectW, rectH), cv::Scalar(206, 200, 195), -1);
        x += centerSpacing;
    }
    
    // -- n nearly adjacent rectangles
    x = leftPadding + widelySpacedW + gap2;
    for (int i = 0; i < nAdjacent; ++i) {
        cv::rectangle(templateRect, cv::Rect(x, 0, rectW, rectH), cv::Scalar(206, 200, 195), -1);
        x += rectW + adjacentGap;
    }
    
    // Mirror horizontally for RHS pattern if requested
    if (mirrorRight) {
        cv::Mat flipped;
        cv::flip(templateRect, flipped, 1); // flip around y-axis
        return flipped;
    }
    
    return templateRect;
}

// ----------------------------------------------------------------------
// Corner finder: detect the unique pad-spacing signature along the bond row.
// LHS geometry (left to right):  [circle] [3 wide pads @25px] [gap] [n narrow pads @10px]
// RHS geometry (left to right):  [n narrow @10px] [gap] [3 wide @25px] [circle]
struct ChipCornerHit {
    Point mark;
    vector<Point> padCenters;
    double score;
    bool isRHS;
};

namespace {

constexpr int kRectW = 9;
constexpr int kWidePitch = 25;
constexpr int kNarrowPitch = 10;
constexpr int kWideToNarrow = 41; // gap2(32) + pad width(9) between wide-3 and narrow-1 centers
constexpr int kMarkOffset = 12;

double sampleBright(const cv::Mat& bright, int x, int y) {
    if (x < 1 || y < 1 || x >= bright.cols - 1 || y >= bright.rows - 1) {
        return 0.0;
    }
    double sum = 0.0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            sum += bright.at<uchar>(y + dy, x + dx);
        }
    }
    return sum / 9.0;
}

int findBestBondRowY(const cv::Mat& bright, int yLo, int yHi) {
    int bestY = -1;
    double bestScore = 0.0;
    yLo = std::max(1, yLo);
    yHi = std::min(bright.rows - 1, yHi);
    for (int y = yLo; y < yHi; ++y) {
        const uchar* row = bright.ptr<uchar>(y);
        double score = 0.0;
        for (int x = 0; x < bright.cols; ++x) {
            score += row[x];
        }
        if (score > bestScore) {
            bestScore = score;
            bestY = y;
        }
    }
    return bestY;
}

// LHS (left → right):  [circle] [wide][wide][wide] [gap] [narrow]…[narrow]
// RHS (left → right):  [narrow]…[narrow] [gap] [wide][wide][wide] [circle]
struct CornerLayout {
    vector<int> padXs; // pad centers, always left → right along bond row
    int markX;
    bool isRHS;
};

CornerLayout makeLHSLayout(int xWide1, int nNarrow) {
    CornerLayout layout;
    layout.isRHS = false;
    layout.markX = xWide1 - kMarkOffset;
    layout.padXs = {xWide1, xWide1 + kWidePitch, xWide1 + 2 * kWidePitch};
    int narrowStart = xWide1 + 2 * kWidePitch + kWideToNarrow;
    for (int i = 0; i < nNarrow; ++i) {
        layout.padXs.push_back(narrowStart + i * kNarrowPitch);
    }
    return layout;
}

CornerLayout makeRHSLayout(int xWide3, int nNarrow) {
    CornerLayout layout;
    layout.isRHS = true;
    int xWide1 = xWide3 - 2 * kWidePitch;
    layout.markX = xWide3 + kMarkOffset;
    int narrowStart = xWide1 - kWideToNarrow - (nNarrow - 1) * kNarrowPitch;
    for (int i = 0; i < nNarrow; ++i) {
        layout.padXs.push_back(narrowStart + i * kNarrowPitch);
    }
    layout.padXs.push_back(xWide1);
    layout.padXs.push_back(xWide3 - kWidePitch);
    layout.padXs.push_back(xWide3);
    return layout;
}

bool scoreLayout(const cv::Mat& bright, int bondY, const CornerLayout& layout, int nNarrow,
                 double& score) {
    const int nWide = 3;
    const double wideMin = 15.0;
    const int wideBegin = layout.isRHS ? nNarrow : 0;
    for (int i = 0; i < nWide; ++i) {
        double v = sampleBright(bright, layout.padXs[wideBegin + i], bondY);
        if (v < wideMin) {
            return false;
        }
        score += v;
    }
    const int narrowBegin = layout.isRHS ? 0 : nWide;
    for (int i = 0; i < nNarrow; ++i) {
        double v = sampleBright(bright, layout.padXs[narrowBegin + i], bondY);
        if (v < 30.0) {
            return false;
        }
        score += v;
    }
    return true;
}

void tryLHSAt(const cv::Mat& bright, int bondY, int xWide1, int nNarrow, const vector<double>& xVec,
              vector<ChipCornerHit>& hits, bool restrictRange = true) {
    if (restrictRange && (xWide1 > 5000 || !isInRange(xWide1, xVec))) {
        return;
    }
    CornerLayout layout = makeLHSLayout(xWide1, nNarrow);
    if (layout.padXs.back() + kNarrowPitch >= bright.cols) {
        return;
    }
    double score = 0.0;
    if (!scoreLayout(bright, bondY, layout, nNarrow, score)) {
        return;
    }

    ChipCornerHit hit;
    hit.isRHS = false;
    hit.score = score;
    hit.mark = Point(layout.markX, bondY);
    for (int x : layout.padXs) {
        hit.padCenters.emplace_back(x, bondY);
    }
    hits.push_back(hit);
}

void tryRHSAt(const cv::Mat& bright, int bondY, int xWide3, int nNarrow, const vector<double>& xVec,
              vector<ChipCornerHit>& hits, bool restrictRange = true) {
    if (restrictRange && (xWide3 < 2500 || !isInRange(xWide3, xVec))) {
        return;
    }
    CornerLayout layout = makeRHSLayout(xWide3, nNarrow);
    if (layout.padXs.front() < 0) {
        return;
    }
    double score = 0.0;
    if (!scoreLayout(bright, bondY, layout, nNarrow, score)) {
        return;
    }

    ChipCornerHit hit;
    hit.isRHS = true;
    hit.score = score;
    hit.mark = Point(layout.markX, bondY);
    for (int x : layout.padXs) {
        hit.padCenters.emplace_back(x, bondY);
    }
    hits.push_back(hit);
}

vector<ChipCornerHit> findCornersOnRow(const cv::Mat& bright, int bondY, int nNarrow,
                                       const vector<double>& xVec) {
    vector<ChipCornerHit> hits;
    if (bondY < 0) {
        return hits;
    }
    for (int x1 = 80; x1 < bright.cols - 200; x1 += 2) {
        tryLHSAt(bright, bondY, x1, nNarrow, xVec, hits);
    }
    for (int x3 = 200; x3 < bright.cols - 80; x3 += 2) {
        tryRHSAt(bright, bondY, x3, nNarrow, xVec, hits);
    }
    std::sort(hits.begin(), hits.end(), [](const ChipCornerHit& a, const ChipCornerHit& b) {
        return a.score > b.score;
    });
    return hits;
}

vector<ChipCornerHit> nmsCornerHits(vector<ChipCornerHit> hits, double minDist) {
    vector<ChipCornerHit> kept;
    for (const auto& hit : hits) {
        bool tooClose = false;
        for (const auto& k : kept) {
            if (hit.isRHS == k.isRHS && cv::norm(hit.mark - k.mark) < minDist) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            kept.push_back(hit);
        }
    }
    return kept;
}

vector<ChipCornerHit> findChipCornersBySpacing(const cv::Mat& bright,
                                               const vector<cv::Vec3f>& hdiMarkers,
                                               const vector<double>& xVec,
                                               int nNarrow) {
    double yLo = hdiMarkers.size() > 0 ? hdiMarkers[0][1] : 0.0;
    double yHi = hdiMarkers.size() > 1 ? hdiMarkers[1][1] : bright.rows;
    double yMin = std::min(yLo, yHi);
    double yMax = std::max(yLo, yHi);

    vector<int> bondRows;
    int topRow = findBestBondRowY(bright, 40, static_cast<int>(yMin) - 40);
    int botRow = findBestBondRowY(bright, static_cast<int>(yMax) + 40, bright.rows - 40);
    if (topRow > 0) {
        bondRows.push_back(topRow);
    }
    if (botRow > 0 && botRow != topRow) {
        bondRows.push_back(botRow);
    }

    vector<ChipCornerHit> allHits;
    for (int bondY : bondRows) {
        auto rowHits = findCornersOnRow(bright, bondY, nNarrow, xVec);
        allHits.insert(allHits.end(), rowHits.begin(), rowHits.end());
    }
    return nmsCornerHits(allHits, 250.0);
}

void drawCornerHit(cv::Mat& vis, const ChipCornerHit& hit, int idx, const Scalar& color) {
    for (size_t i = 0; i < hit.padCenters.size(); ++i) {
        const auto& p = hit.padCenters[i];
        cv::rectangle(vis, Rect(p.x - kRectW / 2, p.y - 6, kRectW, 12), color, 1);
        if (i + 1 < hit.padCenters.size()) {
            cv::line(vis, p, hit.padCenters[i + 1], color, 1);
        }
    }
    cv::circle(vis, hit.mark, 6, color, 2);
    if (!hit.padCenters.empty()) {
        const Point& edgePad = hit.isRHS ? hit.padCenters.back() : hit.padCenters.front();
        cv::line(vis, edgePad, hit.mark, color, 1);
    }
    string label = (hit.isRHS ? "RHS" : "LHS") + std::to_string(idx);
    putText(vis, label, Point2f(hit.mark.x - 20, hit.mark.y + 25), FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
}

// HDI primed-frame layout (compound.hh sketch: origin M1, x' toward M2, y' toward M0).
// Nominal chip-corner ROC positions in mm, then mapped to pixels via measured M0/M1/M2.
namespace HdiLayout {

constexpr double kDm01 = 23.0;        // mm, M0–M1 marker separation
constexpr double kDm12 = 40.0;        // mm, M1–M2 marker separation
constexpr double kHdiLength = 54.275; // mm along x' (M1 toward M2)
constexpr double kHdiWidth = 36.2;    // mm along y' (HDI midline span)
constexpr double kChipMarkerSpan = 21.68; // mm, ROC separation LHS–RHS on one chip
constexpr double kInterChipGap = 0.16;    // mm between adjacent chips along x'
constexpr double kRocBondOffset = 0.6;    // mm, ROC row just outside HDI edge
constexpr double kInsetRight = 1.70;      // mm, rightmost ROC inset from M1 (x'=0)

struct ChipCornerNominal {
    const char* name;
    bool isRhs;
    double xPrimeMm;
    double yPrimeMm;
};

inline double yTopRocMm() {
    return 0.5 * kDm01 + 0.5 * kHdiWidth + kRocBondOffset;
}

inline double yBotRocMm() {
    return 0.5 * kDm01 - 0.5 * kHdiWidth - kRocBondOffset;
}

inline void chipColumnXs(double& xFarLhs, double& xMidRhs, double& xMidLhs, double& xFarRhs) {
    xFarRhs = -kInsetRight;
    xMidLhs = xFarRhs + kChipMarkerSpan;
    xMidRhs = xMidLhs + kInterChipGap;
    xFarLhs = xMidRhs + kChipMarkerSpan;
}

inline vector<ChipCornerNominal> chipCornerNominals() {
    double xFarLhs, xMidRhs, xMidLhs, xFarRhs;
    chipColumnXs(xFarLhs, xMidRhs, xMidLhs, xFarRhs);
    const double yTop = yTopRocMm();
    const double yBot = yBotRocMm();
    return {
        {"chip00", false, xFarLhs, yTop},
        {"chip01", true, xMidRhs, yTop},
        {"chip10", false, xMidLhs, yTop},
        {"chip11", true, xFarRhs, yTop},
        {"chip20", true, xFarRhs, yBot},
        {"chip21", false, xMidLhs, yBot},
        {"chip30", true, xMidRhs, yBot},
        {"chip31", false, xFarLhs, yBot},
    };
}

// Bottom-left distinctive pattern (HDI31), nominal mm in primed frame.
inline cv::Point2d distinctivePrimedMm() {
    double xFarLhs, xMidRhs, xMidLhs, xFarRhs;
    chipColumnXs(xFarLhs, xMidRhs, xMidLhs, xFarRhs);
    (void)xMidRhs;
    (void)xMidLhs;
    (void)xFarRhs;
    return cv::Point2d(xFarLhs - 0.69, yBotRocMm() + 1.98);
}

} // namespace HdiLayout

Point2f fromPrimedPx(Point2f p, double alpha, Point2f origin) {
    double c = std::cos(alpha);
    double s = std::sin(alpha);
    return Point2f(static_cast<float>(c * p.x + s * p.y + origin.x),
                   static_cast<float>(-s * p.x + c * p.y + origin.y));
}

Point2f fromPrimedMm(cv::Point2d primedMm, double scaleMmPerPx, double alpha, Point2f origin) {
    const float xPx = static_cast<float>(primedMm.x / scaleMmPerPx);
    const float yPx = static_cast<float>(primedMm.y / scaleMmPerPx);
    return fromPrimedPx(Point2f(xPx, yPx), alpha, origin);
}

struct ModuleFrame {
    Point2f m0;
    Point2f m1;
    Point2f m2;
    double alpha;
    double scaleMmPerPx;
    double sfSpread;
    double orthogonality;
    bool valid;
};

// compound::determineTransformation + determineSF from measured HDI markers.
ModuleFrame buildModuleFrame(const vector<cv::Vec3f>& hdiMarkers) {
    ModuleFrame frame;
    frame.valid = false;
    if (hdiMarkers.size() < 3) {
        return frame;
    }

    frame.m0 = Point2f(hdiMarkers[0][0], hdiMarkers[0][1]);
    frame.m1 = Point2f(hdiMarkers[1][0], hdiMarkers[1][1]);
    frame.m2 = Point2f(hdiMarkers[2][0], hdiMarkers[2][1]);

    Point2f yAxis = frame.m0 - frame.m1;
    Point2f xAxis = frame.m2 - frame.m1;
    yAxis /= cv::norm(yAxis);
    xAxis /= cv::norm(xAxis);
    frame.orthogonality = 100.0
        * std::acos(std::max(-1.0, std::min(1.0, static_cast<double>(xAxis.dot(yAxis)))))
        / (M_PI / 2.0);

    double theta0 = std::acos(std::max(-1.0, std::min(1.0, static_cast<double>(xAxis.x))));
    double theta1 = std::acos(std::max(-1.0, std::min(1.0, static_cast<double>(yAxis.y))));
    double alpha = 0.5 * (theta0 + theta1) - M_PI;
    double rsgn = xAxis.y;
    frame.alpha = -M_PI + (rsgn < 0 ? alpha : -alpha);

    double px01 = cv::norm(frame.m0 - frame.m1);
    double px12 = cv::norm(frame.m1 - frame.m2);
    double sf1 = HdiLayout::kDm01 / px01;
    double sf2 = HdiLayout::kDm12 / px12;
    frame.scaleMmPerPx = 0.5 * (sf1 + sf2);
    frame.sfSpread = std::abs(sf1 - sf2) / frame.scaleMmPerPx;
    frame.valid = true;
    return frame;
}

Point2f predictChipCornerPx(const HdiLayout::ChipCornerNominal& corner, const ModuleFrame& frame) {
    return fromPrimedMm(cv::Point2d(corner.xPrimeMm, corner.yPrimeMm),
                        frame.scaleMmPerPx, frame.alpha, frame.m1);
}

cv::Mat makeDistinctiveTemplate() {
    cv::Mat tmpl(24, 42, CV_8UC1, cv::Scalar(0));
    cv::rectangle(tmpl, cv::Rect(2, 7, 18, 10), cv::Scalar(180), -1);
    cv::circle(tmpl, cv::Point(30, 12), 4, cv::Scalar(255), -1);
    return tmpl;
}

bool findDistinctivePattern(const cv::Mat& bright, Point2f predicted, Point& found, int searchRad = 90) {
    cv::Mat tmpl = makeDistinctiveTemplate();
    int x0 = std::max(0, static_cast<int>(predicted.x) - searchRad);
    int y0 = std::max(0, static_cast<int>(predicted.y) - searchRad);
    int x1 = std::min(bright.cols, static_cast<int>(predicted.x) + searchRad);
    int y1 = std::min(bright.rows, static_cast<int>(predicted.y) + searchRad);
    if (x1 - x0 < static_cast<int>(tmpl.cols) || y1 - y0 < static_cast<int>(tmpl.rows)) {
        return false;
    }
    cv::Mat roi = bright(cv::Rect(x0, y0, x1 - x0, y1 - y0));
    cv::Mat result;
    cv::matchTemplate(roi, tmpl, result, cv::TM_CCOEFF_NORMED);
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);
    if (maxVal < 0.40) {
        return false;
    }
    found = Point(x0 + maxLoc.x + 30, y0 + maxLoc.y + 12);
    return true;
}

struct CornerSearchRoi {
    cv::Rect rect;
    std::string chipName;
    Point2f predicted;
    bool verified;
};

cv::Rect makeCornerSearchRoi(Point2f predicted, bool isRhs, int nNarrow, int winX, int winY) {
    (void)isRhs;
    int patternW = 2 * kWidePitch + kWideToNarrow + nNarrow * kNarrowPitch;
    // Symmetric about the predicted circle mark.  (Asymmetric ROI made LHS look
    // too far right and RHS too far left in SVG coordinates.)
    int halfX = winX + kMarkOffset + patternW / 2;
    int xLo = static_cast<int>(predicted.x) - halfX;
    int xHi = static_cast<int>(predicted.x) + halfX;
    int yLo = static_cast<int>(predicted.y) - winY;
    int yHi = static_cast<int>(predicted.y) + winY;
    return cv::Rect(xLo, yLo, xHi - xLo, yHi - yLo);
}

void drawCornerSearchRois(cv::Mat& vis, const vector<CornerSearchRoi>& rois) {
    cv::Rect bounds(0, 0, vis.cols, vis.rows);
    for (const auto& roi : rois) {
        cv::Rect clipped = roi.rect & bounds;
        if (clipped.width <= 0 || clipped.height <= 0) {
            continue;
        }
        Scalar color = roi.verified ? Scalar(0, 200, 0) : Scalar(0, 180, 255);
        cv::rectangle(vis, clipped, color, 1);
        cv::drawMarker(vis, roi.predicted, color, cv::MARKER_CROSS, 10, 1);
        putText(vis, roi.chipName, Point2f(clipped.x, clipped.y - 4),
                FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    }
}

bool verifyCornerAt(const cv::Mat& bright, Point2f predicted, bool isRhs, int nNarrow,
                    int winX, int winY, ChipCornerHit& bestHit) {
    int yLo = std::max(1, static_cast<int>(predicted.y) - winY);
    int yHi = std::min(bright.rows - 1, static_cast<int>(predicted.y) + winY);
    int bondY = findBestBondRowY(bright, yLo, yHi);
    if (bondY < 0) {
        return false;
    }

    int xAnchor = isRhs ? static_cast<int>(predicted.x + kMarkOffset)
                        : static_cast<int>(predicted.x - kMarkOffset);
    vector<ChipCornerHit> hits;
    double bestScore = -1.0;
    bool found = false;
    vector<double> noFilter;
    for (int x = xAnchor - winX; x <= xAnchor + winX; x += 2) {
        hits.clear();
        if (isRhs) {
            tryRHSAt(bright, bondY, x, nNarrow, noFilter, hits, false);
        } else {
            tryLHSAt(bright, bondY, x, nNarrow, noFilter, hits, false);
        }
        if (!hits.empty() && hits[0].score > bestScore) {
            bestScore = hits[0].score;
            bestHit = hits[0];
            found = true;
        }
    }
    return found && bestScore >= 180.0;
}

struct HolisticResult {
    map<string, Point> chipMarkers;
    vector<ChipCornerHit> hits;
    vector<CornerSearchRoi> searchRois;
    Point distinctive;
    Point2f distinctivePredicted;
    bool distinctiveFound;
    ModuleFrame frame;
    int nVerified;
};

HolisticResult findChipCornersHolistic(const cv::Mat& bright, const vector<cv::Vec3f>& hdiMarkers,
                                       int nNarrow, int roiWinX = 50, int roiWinY = 12) {
    HolisticResult result;
    result.frame = buildModuleFrame(hdiMarkers);
    if (!result.frame.valid) {
        cout << "Holistic: failed to build module frame from HDI markers" << endl;
        return result;
    }

    cout << "Holistic frame: SF=" << result.frame.scaleMmPerPx << " mm/px"
         << " spread=" << result.frame.sfSpread
         << " orthogonality=" << result.frame.orthogonality
         << " alpha=" << result.frame.alpha << endl;

    const cv::Point2d distinctiveMm = HdiLayout::distinctivePrimedMm();
    Point2f predDistinctive = fromPrimedMm(distinctiveMm, result.frame.scaleMmPerPx,
                                           result.frame.alpha, result.frame.m1);
    result.distinctivePredicted = predDistinctive;
    result.distinctiveFound = findDistinctivePattern(bright, predDistinctive, result.distinctive);
    if (result.distinctiveFound) {
        cout << "Holistic: distinctive pattern at " << result.distinctive.x << ", "
             << result.distinctive.y << " (pred "
             << predDistinctive.x << ", " << predDistinctive.y << ")" << endl;
    }

    vector<string> allNames = {"chip00", "chip01", "chip10", "chip11",
                               "chip20", "chip21", "chip30", "chip31"};
    for (const auto& name : allNames) {
        result.chipMarkers[name] = Point(-1, -1);
    }

    for (const auto& corner : HdiLayout::chipCornerNominals()) {
        Point2f pred = predictChipCornerPx(corner, result.frame);
        CornerSearchRoi roi;
        roi.chipName = corner.name;
        roi.predicted = pred;
        roi.rect = makeCornerSearchRoi(pred, corner.isRhs, nNarrow, roiWinX, roiWinY);
        roi.verified = false;

        ChipCornerHit hit;
        if (verifyCornerAt(bright, pred, corner.isRhs, nNarrow, roiWinX, roiWinY, hit)) {
            result.chipMarkers[corner.name] = hit.mark;
            result.hits.push_back(hit);
            result.nVerified++;
            roi.verified = true;
        }
        result.searchRois.push_back(roi);
    }

    cout << "Holistic: verified " << result.nVerified << "/8 chip corners" << endl;
    return result;
}

} // namespace


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
      {1130, 5}, {1131, 0}, {1132, 1}, {1133, 2}, {1134, 0}, {1135, 1}, {1136, 2}, {1137, 3}, {1138, 4}, {1139, 0}
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
    int nAdjacentRects(7);
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "-d" && i + 1 < argc) { directory = argv[++i]; }
        if (string(argv[i]) == "-f" && i + 1 < argc) { filename = argv[++i]; }
        if (string(argv[i]) == "-o" && i + 1 < argc) { outMarksFile = argv[++i]; }
        if (string(argv[i]) == "-n" && i + 1 < argc) { nAdjacentRects = stoi(argv[++i]); }
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
    // -- holistic chip-corner finder: HDI frame -> predicted ROIs -> local verify
    cv::Mat imgGray;
    cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);
    cv::Mat brightSpots;
    cv::Mat morphKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
    cv::morphologyEx(imgGray, brightSpots, cv::MORPH_TOPHAT, morphKernel);

    HolisticResult holistic = findChipCornersHolistic(brightSpots, hdiMarkers, nAdjacentRects);
    map<string, cv::Point> chipMarkers = holistic.chipMarkers;

    drawCornerSearchRois(vis1, holistic.searchRois);
    if (holistic.frame.valid) {
        int blRad = 90;
        cv::rectangle(vis1,
                      Rect(static_cast<int>(holistic.distinctivePredicted.x) - blRad,
                           static_cast<int>(holistic.distinctivePredicted.y) - blRad,
                           2 * blRad, 2 * blRad),
                      Scalar(255, 200, 0), 1);
    }

    for (size_t k = 0; k < holistic.hits.size(); ++k) {
        const auto& hit = holistic.hits[k];
        Scalar color = hit.isRHS ? Scalar(255, 0, 0) : Scalar(0, 0, 255);
        drawCornerHit(vis1, hit, static_cast<int>(k), color);
        if (verbose) {
            cout << (hit.isRHS ? "RHS" : "LHS") << " verified corner " << k
                 << " mark (px): " << hit.mark.x << ", " << hit.mark.y
                 << " score=" << hit.score << endl;
        }
    }
    if (holistic.distinctiveFound) {
        cv::rectangle(vis1, Rect(holistic.distinctive.x - 12, holistic.distinctive.y - 8, 24, 16),
                      Scalar(0, 255, 255), 2);
        cv::circle(vis1, holistic.distinctive, 5, Scalar(0, 255, 255), 2);
        putText(vis1, "BL", Point2f(holistic.distinctive.x + 15, holistic.distinctive.y),
                FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
    }

    if (visualize) {
        cv::imshow("brightSpots", brightSpots);
    }

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