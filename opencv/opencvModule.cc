#include "opencvModule.hh"

#include <string>
#include <iostream>

using namespace cv;
using namespace std;

void moduleAna1(Mat &img) {
  imshow("Original", img);
  int k = waitKey(0); // Wait for a keystroke in the window
  if (k == 'q') {
    return;
  } 

  cout << "hallo, continuing" << endl;
  
  Mat src_gray;
  int thresh = 100;
  RNG rng(12345);
  
  Mat canny_output;
  Canny(img, canny_output, thresh, thresh*2);
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  cout << "Found " << contours.size() << " contours" << endl;
  for (size_t i = 0; i< contours.size(); i++ ) {
      Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
      drawContours( drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
    }
  imshow( "Contours", drawing );
  
  if (k == 'q') {
    return; 
  }
}


