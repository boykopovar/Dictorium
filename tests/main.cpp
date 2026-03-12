#include "Dictorium/Dictorium.h"

using namespace dtr;

void print_dict(IDictionary<std::string, int>& dict);

int main() {
  LinearDictionary<std::string, int> dict = {
    {"тест", 1},
    {"тест1", 2},
  };
  dict["тест2"] = 4;
  dict.Add("qwe", 123);
  print(dict.ContainsKey("qwe"));

  std::cout << dict["тест"] << std::endl;
  std::cout << dict.Count() << std::endl;

  print_dict(dict);
}