#include <iostream>
#include <string>

#include "opencvTests.hh"
#include "opencvModule.hh"

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------
// -- Usage examples
// 
// bin/opencvRun -f test.jpeg -t patterns/HDI-markers.png 
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string file("fixme"), mode("tests"), tmpl("template");
  int algo(0);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-a"))   {algo = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-f"))   {file = argv[++i];}
    if (!strcmp(argv[i], "-t"))   {tmpl = argv[++i];}
    if (!strcmp(argv[i], "-m"))   {mode = argv[++i];}
  }

  std::string image_path = samples::findFile(file);
  Mat img = imread(image_path, IMREAD_COLOR);
  if (img.empty()) {
    std::cout << "Could not read the image: " << image_path << std::endl;
    return 1;
  }

  Mat templ;
  if (tmpl != "template") {
    std::string tmpl_path = samples::findFile(tmpl);
    templ = imread(tmpl_path, IMREAD_COLOR);
    if (templ.empty()) {
      std::cout << "Could not read the image: " << tmpl_path << std::endl;
      return 1;
    }
  }

  if (string::npos != mode.find("test0")) {
    opencvTest0(img);
  }

  if (string::npos != mode.find("test1")) {
    opencvTest1(img);
  }

  if (string::npos != mode.find("test2")) {
    opencvTest2(img, templ, algo);
  }

  if (string::npos != mode.find("mana1")) {
    moduleAna1(img);
  }

  if (string::npos != mode.find("airex1")) {
    airex1(img, templ, algo);
  }

  if (string::npos != mode.find("hdicutout")) {
    HDICutout(img, algo);
  }

  return 0;
}


