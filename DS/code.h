#include <string>
using namespace std;
class Code {
  string code_snippet;
  string error_message;
  string language;
  string context;

public:
  Code();
  void add(string code_snippet, string error_message, string language,
           string context);
};
