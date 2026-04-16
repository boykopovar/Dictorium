#include "Dictorium/Dictorium.h"

using namespace dtr;

void print_json(const IDictionary<std::string, int>& dict);


int main() {
  PerfectHashDictionary<std::string, int> dict = {
    {"test1", 1},
    {"test2", 2},
  };

  dict.Add("qwe", 123);
  std::cout << dict << std::endl;
  std::cout << dict.ContainsKey("qwe");

  std::cout << dict["test1"] << '\n';
  std::cout << dict.Count() << '\n';

  print_json(dict);

  LinearDictionary<int, std::vector<int>> dict2 = {
    {1, {1, 2, 3}},
    {2, {4, 5, 6}}
  };
  std::cout << dict2;

}