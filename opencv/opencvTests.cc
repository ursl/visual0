#include "opencvTests.hh"


#include <string>
#include <iostream>

using namespace cv;
using namespace std;


// ----------------------------------------------------------------------
bool waitQ() {
  int k0 = waitKey(0); // Wait for a keystroke in the window
  if(k0 == 'q') {
    return true; 
  }
  return false;
}


// ----------------------------------------------------------------------
void resizeCanvas(const Mat& src, Mat& dst, const Size& canvasSize, const Scalar& emptyColor) {
  if((canvasSize.height < src.rows) || canvasSize.width < src.cols) {
    // Canvas is smaller than source image
    return;
  }
  
  int bottom = canvasSize.height - src.rows;
  int right = canvasSize.width - src.cols;
  
  copyMakeBorder(src, dst, 0 /*top*/, bottom, 0 /*left*/, right, cv::BORDER_CONSTANT, emptyColor);
}


// ----------------------------------------------------------------------
Mat resizeCanvas(const Mat& source, Size newSize, Scalar emptyColor) {
  cv::Mat result(newSize, source.type(), emptyColor);
  
  int height = std::min(source.rows, newSize.height);
  int width = std::min(source.cols, newSize.width);
  cv::Rect roi(0, 0, width, height);
  
  auto sourceWindow = source(roi);
  auto targetWindow = result(roi);
  sourceWindow.copyTo(targetWindow);
  
  return result;
}


// ----------------------------------------------------------------------
// -- started from https://pyimagesearch.com/2015/01/26/multi-scale-template-matching-using-python-opencv/
// -- this matches the AIREX frames onto either glass pictures or HDI pictures
// -- pictures taken with CANON EOS, standard alignment
void airex1(Mat &colImg, Mat &colTempl, int match) {
  Mat result;

  /** Template matching methods 
      enum
      {
      TM_SQDIFF        =0,
      TM_SQDIFF_NORMED =1, masked
      TM_CCORR         =2,
      TM_CCORR_NORMED  =3, masked
      TM_CCOEFF        =4,
      TM_CCOEFF_NORMED =5
      };
  */

	Mat img;
  img.create(colImg.size(), colImg.type());
  cvtColor(colImg, img, COLOR_BGR2GRAY);

	Mat templ;
  templ.create(colTempl.size(), colTempl.type());
  cvtColor(colTempl, templ, COLOR_BGR2GRAY);

  int match_method(match);
  
  Mat img_display;
  img.copyTo(img_display);
  int result_cols =  img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;
  cout << "Hallo 1 img.cols   = " << img.cols   << " img.rows   = " << img.rows << endl;
  cout << "Hallo 1 templ.cols = " << templ.cols << " templ.rows = " << templ.rows << endl;
  result.create(result_rows, result_cols, CV_32FC1);

  matchTemplate(img, templ, result, match_method); 

  normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat() );

  double minVal; double maxVal; Point minLoc; Point maxLoc;
  Point matchLoc;
  minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
  
  cout << "minVal = " << minVal << " maxVal = " << maxVal << endl;
  cout << "minLoc = " << minLoc << " maxLoc = " << maxLoc << endl;

  if (match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED ) { 
    matchLoc = minLoc; 
  } else { 
    matchLoc = maxLoc; 
  }

  rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(255), 20, 8, 0);
  rectangle(result,      matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(255), 20, 8, 0);

  const char* image_window = "Source Image";
  namedWindow(image_window, WINDOW_AUTOSIZE);
  imshow( image_window, img_display );

  int k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("result.png", img_display);
  } else if (k == 'q') {
    return; 
  }

}


// ----------------------------------------------------------------------
// -- this matches the HDI rounded cutouts onto HDI pictures
// -- pictures taken with CANON EOS, standard alignment
void HDICutout(Mat &colImg, int match) {
  Mat result;

  cout << "HDICutout" << endl;

  /** Template matching methods 
      enum
      {
      TM_SQDIFF        =0,
      TM_SQDIFF_NORMED =1, masked
      TM_CCORR         =2,
      TM_CCORR_NORMED  =3, masked
      TM_CCOEFF        =4,
      TM_CCOEFF_NORMED =5
      };
  */

	Mat img;
  img.create(colImg.size(), colImg.type());
  cvtColor(colImg, img, COLOR_BGR2GRAY);

  std::string tmpl_path = samples::findFile("patterns/hdi-incut1.png");
  Mat colTempl = imread(tmpl_path, IMREAD_COLOR);
  if (colTempl.empty()) {
    std::cout << "Could not read the image: " << tmpl_path << std::endl;
    return;
  }

  // -- first overlay the two to get the approximate size corect
	Mat templ;
  templ.create(colTempl.size(), colTempl.type());
  cvtColor(colTempl, templ, COLOR_BGR2GRAY);



  cout << " templ.size() = " << templ.size << endl;
  cout << " img.size()   = " << img.size << endl;

  Mat resi;
  resize(templ, resi, Size(0.11*templ.cols, 0.11*templ.rows), INTER_LINEAR);

  imshow("frame_templ", templ);
  imshow("frame_img", img);
  imshow("frame_resi", resi);
  
  if (waitQ()) return;
  
  Mat newFrame;

  Mat resized;
  cv::cvtColor(resized, resized, COLOR_BGR2BGRA);

  resizeCanvas(templ, resized, img.size(), Scalar(10,10,10,10));

  Mat resized2 = resizeCanvas(templ, img.size(), Scalar(10,10,10,10));

  double alpha(0.4), gamma(0);
  addWeighted(resized, alpha, img, 1 - alpha, gamma, newFrame);

  //  bitwise_and(resized, newFrame, resized);
  imshow("frame", resized);


  int match_method(match);
  
  Mat img_display;
  img.copyTo(img_display);
  int result_cols =  img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;
  result.create(result_rows, result_cols, CV_32FC1);

  matchTemplate(img, templ, result, match_method); 

  normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat() );

  double minVal; double maxVal; Point minLoc; Point maxLoc;
  Point matchLoc;
  minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
  
  cout << "minVal = " << minVal << " maxVal = " << maxVal << endl;
  cout << "minLoc = " << minLoc << " maxLoc = " << maxLoc << endl;

  //bool method_accepts_mask = (CV_TM_SQDIFF == match_method || match_method == CV_TM_CCORR_NORMED);

  if (match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED ) { 
    matchLoc = minLoc; 
  } else { 
    matchLoc = maxLoc; 
  }

  rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(255), 20, 8, 0);

  const char* image_window = "Source Image";
  namedWindow(image_window, WINDOW_AUTOSIZE);
  imshow(image_window, img_display);
  imshow(image_window, templ);

  int k = waitKey(0); // Wait for a keystroke in the window
  if(k == 's') {
    imwrite("hdicutout.png", result);
  } else if (k == 'q') {
    return; 
  }

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


// ----------------------------------------------------------------------
void opencvTest2(Mat &colImg, Mat &colTempl, int match) {

  Mat result;

  /** Template matching methods 
      enum
      {
      TM_SQDIFF        =0,
      TM_SQDIFF_NORMED =1, masked
      TM_CCORR         =2,
      TM_CCORR_NORMED  =3, masked
      TM_CCOEFF        =4,
      TM_CCOEFF_NORMED =5
      };
  */

	Mat img;
  img.create(colImg.size(), colImg.type());
  cvtColor(colImg, img, COLOR_BGR2GRAY);

	Mat templ;
  templ.create(colTempl.size(), colTempl.type());
//  cvtColor(colTempl, templ, COLOR_BGR2GRAY);

  cv::Mat insetImage(img, cv::Rect(70, 70, 20, 20));
  templ.copyTo(insetImage);
  
  cv::imshow("Overlay Image", img);

  int k = waitKey(0); // Wait for a keystroke in the window

}
