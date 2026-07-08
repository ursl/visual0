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

int modulePosition(int moduleNumber);
string parsePosition(string module, string fileName);

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

  if (parseMode == "validation") {
    for (int i = 1000; i <= 1139; i++) {
      int position = modulePosition(i);
      if (position == -1) {
        cout << "Module number " << i << " not found in modulePositions" << endl;
        return 0;
      }
      string module2Parse = "P-" + to_string(i);
      string parsedPosition = parsePosition(module2Parse, fileName);
      string bla("");
      if (parsedPosition != to_string(position)) {
       bla += "  ***";
      }
      cout << "Module number " << i << " modulePosition: " << position 
           << " parsedPosition: " << parsedPosition 
           << " " << bla
           << " (" << module2Parse << ")"
           << endl;
    }
    return 0;
  }
  return 0;
}


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
