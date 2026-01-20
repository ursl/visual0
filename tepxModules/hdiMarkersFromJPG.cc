#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

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
   }

   cv::imshow("circles", vis1);
   cv::waitKey(0);
   


    // --- 2) Group circles into concentric pairs (same center within tolerance)
    const float centerTol = 5.0f; // px
    std::vector<MarkerCandidate> markers;

    std::vector<bool> used(circles.size(), false);
    for (size_t i = 0; i < circles.size(); ++i) {
        if (used[i]) continue;
        cv::Point2f ci(circles[i][0], circles[i][1]);
        float ri = circles[i][2];

        // Find other circles with nearly same center
        float rInner = ri;
        float rOuter = ri;
        for (size_t j = i + 1; j < circles.size(); ++j) {
            if (used[j]) continue;
            cv::Point2f cj(circles[j][0], circles[j][1]);
            float rj = circles[j][2];

            if (cv::norm(ci - cj) < centerTol) {
                // mark as used and update inner/outer radii
                used[j] = true;
                rInner = std::min(rInner, rj);
                rOuter = std::max(rOuter, rj);
            }
        }

        // Keep only “real” concentric pairs (outer radius significantly larger)
        if (rOuter > rInner * 1.2f) {
            MarkerCandidate mc;
            mc.center = ci;
            mc.rInner = rInner;
            mc.rOuter = rOuter;
            markers.push_back(mc);
        }
    }

    std::cout << "Concentric marker candidates: " << markers.size() << std::endl;

    // --- 3) For now: keep up to the three best (largest) markers
    std::sort(markers.begin(), markers.end(),
              [](const MarkerCandidate& a, const MarkerCandidate& b) {
                  return a.rOuter > b.rOuter;
              });

    if (markers.size() > 3) markers.resize(3);

    // OPTIONAL: order by x/y to label M0, M1, M2 in a consistent way.
    // For a first step we just print them in this order.

    // --- 4) Draw result for visual debugging
    cv::Mat vis = img.clone();
    for (size_t k = 0; k < markers.size(); ++k) {
        const auto& m = markers[k];
        cv::Scalar color(0, 0, 255 - int(80 * k));  // different reds
        cv::circle(vis, m.center, int(m.rOuter), color, 2);
        cv::circle(vis, m.center, int(m.rInner), color, 2);
        cv::putText(vis, "M" + std::to_string(k),
                    m.center + cv::Point2f(10, -10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        std::cout << "M" << k << " center (px): "
                  << m.center.x << ", " << m.center.y
                  << "  rInner=" << m.rInner
                  << "  rOuter=" << m.rOuter << "\n";
    }

    cv::imshow("markers", vis);
    cv::waitKey(0);

    // --- 5) Optionally write in compound::parseSvgFile .marks format
    if (!outMarksFile.empty() && markers.size() == 3) {
        std::ofstream ofs(outMarksFile);
        if (!ofs.good()) {
            std::cerr << "Cannot open output file " << outMarksFile << std::endl;
            return 1;
        }
        // Line 1: X coordinates, comma-separated
        ofs << markers[0].center.x << ","
            << markers[1].center.x << ","
            << markers[2].center.x << "\n";
        // Line 2: Y coordinates, comma-separated
        ofs << markers[0].center.y << ","
            << markers[1].center.y << ","
            << markers[2].center.y << "\n";
        ofs.close();
        std::cout << "Wrote marker CSV to " << outMarksFile << std::endl;
    }

    return 0;
}