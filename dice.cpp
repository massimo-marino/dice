//
// dice.cpp
//
#include <iostream>
#include <array>
#include <vector>
#include <deque>
#include <cmath>
#include <execution>
#include <random>
#include <algorithm>
#include <chrono>

template<typename Engine = std::mt19937>  // std::mt19937_64 shows poorer performance
class Dice final {
private:
  using engine_type = Engine;
  const int sides_;
  mutable std::uniform_int_distribution<int> distr_;
  mutable engine_type urbg_;

public:
  using seedType = typename engine_type::result_type;

  Dice() = default;

  // generate integers in [1, sides]
  explicit Dice(const int sides, seedType seed) :
      sides_ {sides},
      distr_ {1, sides_},
      urbg_  {seed}
  {}

  // generate integers in [startFrom, (startFrom + (sides - 1)))]
  explicit Dice(seedType seed, const int sides, const int startFrom) :
      sides_ {sides},
      distr_ {startFrom, (startFrom + (sides_ - 1))},
      urbg_  {seed}
  {}

  void reSeed(seedType newSeed) const noexcept {
    urbg_.seed(newSeed);
  }

  void reSeedWithClock() const noexcept {
    urbg_.seed(std::chrono::system_clock::now().time_since_epoch().count());
  }

  int operator () () const noexcept {
    return distr_(urbg_);
  }
};

template <typename Container = std::vector<int>>
static void printResults(const Container& c, size_t howMany = 0) {
  howMany = ((0 == howMany) ? c.size() : howMany);
  std::cout << "results("<< howMany << "/" << c.size() << "): ";
  size_t counter {0};
  for (const int item : c) {
    std:: cout << item << ' ';
    ++counter;
    if (counter == howMany) {
      std::cout << "...";
      break;
    }
  }
  std::cout << '\n';
}

template <typename Func>
void getExecutionTime(const std::string& title, Func func) {
  const auto start {std::chrono::steady_clock::now()};
  func();
  const std::chrono::duration<double> dur {std::chrono::steady_clock::now() - start};
  std::cout << title << ": " << dur.count() << " sec. " << std::endl;
}

static void checkNonDeterministicEntropySource() {
  std::random_device rd;
  const bool non_deterministic { (rd.entropy() > 0) };
  const bool deterministic     { (rd.entropy() == 0) };

  std::cout << "Non deterministic: " << ((non_deterministic) ? "Yes" : "No") << std::endl;
  std::cout << "Deterministic:     " << ((deterministic) ? "Yes" : "No") << std::endl << std::endl;
}

static void diceWithArrays() {
  constexpr int sides {20};
  // make a dice and seed with clock:
  const auto ticks { std::chrono::system_clock::now().time_since_epoch().count() };
  Dice<> dice(ticks, sides, 0);

  // generate sequence of dice rolls:
  constexpr size_t printItems {20};
  constexpr std::size_t randomValues {523'000};
  std::array<int, randomValues> rolls_1;
  std::array<int, randomValues> rolls_2;
  std::array<int, randomValues> rolls_3;
  std::array<int, randomValues> rolls_4;

  std::cout << "Test run with container: std::array" << std::endl;

  getExecutionTime("1- std::generate",
                   [&rolls_1, &dice] () mutable {
                     std::generate(begin(rolls_1), end(rolls_1), dice);
                   } );
  // print results:
  printResults(rolls_1, printItems);

  dice.reSeedWithClock();
  getExecutionTime("2- std::execution::seq",
                   [&rolls_2, &dice] () mutable {
                     std::transform(std::execution::seq, rolls_2.begin(), rolls_2.end(),
                                    rolls_2.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_2, printItems);

  dice.reSeedWithClock();
  getExecutionTime("3- std::execution::par",
                   [&rolls_3, &dice] () mutable {
                     std::transform(std::execution::par, rolls_3.begin(), rolls_3.end(),
                                    rolls_3.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_3, printItems);

  dice.reSeedWithClock();
  getExecutionTime("4- std::execution::par_unseq",
                   [&rolls_4, &dice] () mutable {
                     std::transform(std::execution::par_unseq, rolls_4.begin(), rolls_4.end(),
                                    rolls_4.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_4, printItems);
  std::cout << std::endl << std::endl;
}

template <typename Container = std::vector<int>>
void diceWithContainer(const std::string containerType) {
  constexpr int sides {20};
  // make a dice and seed with clock:
  const auto ticks { std::chrono::system_clock::now().time_since_epoch().count() };
  Dice<> dice(ticks, sides, 0);

  // generate sequence of dice rolls:
  constexpr size_t printItems {20};
  constexpr long long randomValues {500'000'000};
  Container rolls_1(randomValues);
  Container rolls_2(randomValues);
  Container rolls_3(randomValues);
  Container rolls_4(randomValues);

  std::cout << "Test run with container: " << containerType << std::endl;

  getExecutionTime("1- std::generate",
                   [&rolls_1, &dice] () mutable {
                     std::generate(begin(rolls_1), end(rolls_1), dice);
                   } );
  // print results:
  printResults(rolls_1, printItems);

  dice.reSeedWithClock();
  getExecutionTime("2- std::execution::seq",
                   [&rolls_2, &dice] () mutable {
                     std::transform(std::execution::seq, rolls_2.begin(), rolls_2.end(),
                                    rolls_2.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_2, printItems);

  dice.reSeedWithClock();
  getExecutionTime("3- std::execution::par",
                   [&rolls_3, &dice] () mutable {
                     std::transform(std::execution::par, rolls_3.begin(), rolls_3.end(),
                                    rolls_3.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_3, printItems);

  dice.reSeedWithClock();
  getExecutionTime("4- std::execution::par_unseq",
                   [&rolls_4, &dice] () mutable {
                     std::transform(std::execution::par_unseq, rolls_4.begin(), rolls_4.end(),
                                    rolls_4.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_4, printItems);
  std::cout << std::endl << std::endl;
}

int main() {
  checkNonDeterministicEntropySource();
  diceWithArrays();
  diceWithContainer<std::deque<int>>("std::deque");
  diceWithContainer<std::vector<int>>("std::vector");
  return 0;
}

