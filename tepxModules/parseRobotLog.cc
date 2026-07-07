#include <iostream>
#include <fstream>
#include <ostream>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <cctype>
#include <cstring>

using namespace std;

// ---------------------------------------------------------------------
// parse the position from the robot log
string parsePosition(string module, string fileName) {
  ifstream file(fileName);
  string line;
  while (getline(file, line)) {
    if (string::npos != line.find(module)) {
      break;
    }
  }
  const string::size_type pos1 = line.find('|');
  if (pos1 == string::npos) {
    return "";
  }
  const string::size_type pos2 = line.find('|', pos1 + 1);
  if (pos2 == string::npos) {
    return "";
  }
  const string segment = line.substr(pos1 + 1, pos2 - pos1 - 1);
  const string::size_type slash = segment.rfind('/');
  if (slash == string::npos) {
    return "";
  }
  string::size_type start = slash + 1;
  while (start < segment.size() && isspace(static_cast<unsigned char>(segment[start]))) {
    ++start;
  }
  string::size_type end = start;
  while (end < segment.size() && isdigit(static_cast<unsigned char>(segment[end]))) {
    ++end;
  }
  if (start == end) {
    return "";
  }
  return segment.substr(start, end - start);
}

// ---------------------------------------------------------------------
int main(int argc, char** argv) {
  // -- command line arguments
  string fileName("/Users/ursl/pixel/robotLog/log.org"), 
        parseMode("position"), 
        module("P-1138");
  
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-f"))   {fileName = argv[++i];}
    if (!strcmp(argv[i], "-p"))   {parseMode = argv[++i];}
    if (!strcmp(argv[i], "-m"))   {module = argv[++i];}
  }

  if (parseMode == "position") {
    cout << parsePosition(module, fileName) << endl;
    return 0;
  }

  return 0;
}