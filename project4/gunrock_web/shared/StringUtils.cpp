#include "StringUtils.h"


using namespace std;

vector<string> StringUtils::splitWithDelimiter(string str, char delimiter) {
  vector<string> result;

  int start = 0;
  int idx;
  for (idx = 0; idx < (int) str.length(); idx++) {
    // if we're at the delimiter
    if (str[idx] == delimiter) {
      // if there is a string before add it
      if (idx != start) {
        result.push_back(str.substr(start, idx-start));
      }

      string s;
      s.push_back(delimiter);
      result.push_back(s);
      start = idx + 1;
    }
  }

  if (idx != start) {
    result.push_back(str.substr(start, idx-start));
  }

  return result;
}

vector<string> StringUtils::split(string str, char delimiter) {
  vector<string> result;
  vector<string> initial = splitWithDelimiter(str, delimiter);
  string d;
  d.push_back(delimiter);

  // filter out the delimiter tokens
  for (int idx = 0; idx < (int) initial.size(); idx++) {
    if (initial[idx] != d) {
      result.push_back(initial[idx]);
    }
  }

  return result;
}
