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
}


