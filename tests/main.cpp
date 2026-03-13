#include "Dictorium/Dictorium.h"

using namespace dtr;

void print_json(const IDictionary<std::string, int>& dict);

int main() {
  LinearDictionary<std::string, int> dict = {
    {"тест", 1},
    {"тест1", 2},
  };
  dict["тест2"] = 4;
  dict.Add("qwe", 123);
  print(dict.ContainsKey("qwe"));

  std::cout << dict["тест"] << '\n';
  std::cout << dict.Count() << '\n';

  print_json(dict);

  const LinearDictionary<int, std::vector<int>> dict2 = {
    {1, {1, 2, 3}},
    {2, {4, 5, 6}}
  };
  std::cout << dict2;
}