#include "opencvTests.hh"

#include <string>
#include <iostream>

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------
void airex1(Mat &img, Mat &tem) {

}


// ----------------------------------------------------------------------
void opencvTest0(Mat &img) {
  imshow("Original", img);

  // Transform source image to gray if it is not already
  Mat gray;
  if (img.channels() == 3)  {
    cvtColor(img, gray, COLOR_BGR2GRAY);
  } else {
    gray = img;
  }

  // Show gray image
  imshow("gray", gray);
  int k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("gray.png", gray);
  } else if (k == 'q') {
    return; 
  }
    
  Mat bw;
  //    adaptiveThreshold(~gray, bw, 255, /*ADAPTIVE_THRESH_GAUSSIAN_C*/ ADAPTIVE_THRESH_MEAN_C,
  //                      THRESH_BINARY, 15, -2);


  adaptiveThreshold(~gray, bw, 255, ADAPTIVE_THRESH_GAUSSIAN_C /*ADAPTIVE_THRESH_MEAN_C*/,
                    THRESH_BINARY_INV, 11, 2);
  // Show binary image
  imshow("binary", bw);
  k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("binary.png", bw);
  } else if (k == 'q') {
    return; 
  }

  // Create the images that will use to extract the horizontal and vertical lines
  Mat horizontal = bw.clone();
  Mat vertical = bw.clone();
  // Specify size on horizontal axis
  int horizontal_size = horizontal.cols / 50;
  // Create structure element for extracting horizontal lines through morphology operations
  Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontal_size, 1));
  // Apply morphology operations
  erode(horizontal, horizontal, horizontalStructure, Point(-1, -1));
  dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1));

  imshow("horizontal", horizontal);
  k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("horizontal.png", horizontal);
  } else if (k == 'q') {
    return; 
  }

    
  // Specify size on vertical axis
  int vertical_size = vertical.rows / 30;
  // Create structure element for extracting vertical lines through morphology operations
  Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, vertical_size));
  // Apply morphology operations
  erode(vertical, vertical, verticalStructure, Point(-1, -1));
  dilate(vertical, vertical, verticalStructure, Point(-1, -1));
  // Show extracted vertical lines
  imshow("vertical", vertical);
  k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("vertical.png", vertical);
  } else if (k == 'q') {
    return; 
  }

}


// ----------------------------------------------------------------------
void opencvTest1(Mat &img) {
  cv::Mat work;
  imshow("Display window start", img);
  int k = waitKey(0); // Wait for a keystroke in the window
  cvtColor(img, work, COLOR_BGR2GRAY);
  imshow("gray scale", work);
  k = waitKey(0); // Wait for a keystroke in the window

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      double alpha = 1.0 + i*0.2;
      double beta = 0 + j*5;
      img.convertTo(work, -1, alpha, beta);
      stringstream doc;
      doc << "alpha = " << alpha << " beta = " << beta; 
      imshow(doc.str(), work);
      k = waitKey(0); // Wait for a keystroke in the window
      if(k == 's') {
        imwrite("starry_night.png", work);
      } else if (k == 'q') {
        return; 
      }
    }
  }



  // -- gamma playground
  if (0) {
    imshow("Display window start", img);
    int k = waitKey(0); // Wait for a keystroke in the window
    Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    double gamma_(0.);
    for (int ij = 0; ij < 5; ++ij) {
      Mat res = img.clone();
      gamma_ = 0.3 + ij*0.1;
      for( int i = 0; i < 256; ++i) p[i] = saturate_cast<uchar>(pow(i/ 255.0, gamma_) * 255.0);
      
      LUT(img, lookUpTable, res);
      stringstream doc;
      doc << "gamma = " << gamma_; 
      imshow(doc.str(), res);
      k = waitKey(0); // Wait for a keystroke in the window
      if(k == 's') {
        imwrite("starry_night.png", work);
      } else if (k == 'q') {
        return; 
      }
      
    }
  }
  
  if (0) {
    imshow("Display window 0", img);
    int k = waitKey(0); // Wait for a keystroke in the window
    GaussianBlur(img, work, cv::Size(101, 101), 31, 31);  //high blur to extract background light
    img = img - 0.7*work; //adjust light level
    
    imshow("Display window sub", img);
    k = waitKey(0); // Wait for a keystroke in the window
    
    normalize(img, img, 0, 255, cv::NORM_MINMAX); //use whole range
    imshow("Display window normalize", img);
    k = waitKey(0); // Wait for a keystroke in the window
    
    if(k == 's') {
      imwrite("starry_night.png", img);
    }
  }
}
