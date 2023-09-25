#include <iostream>
#include <string>

#include "opencvTests.hh"

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string file("fixme"), mode("tests");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-f"))   {file = argv[++i];}
    if (!strcmp(argv[i], "-m"))   {mode = argv[++i];}
  }

  std::string image_path = samples::findFile(file);
  Mat img = imread(image_path, IMREAD_COLOR);
  if (img.empty()) {
    std::cout << "Could not read the image: " << image_path << std::endl;
    return 1;
  }

  if (!string::npos == mode.find("test0")) {
    opencvTest0(img);
  }

  return 0;
}


