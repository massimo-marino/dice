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

template<typename Engine = std::mt19937,  // std::mt19937_64 shows poorer performance
         typename VALUE_TYPE = unsigned int> 
class Dice final {
private:
  using engine_type = Engine;
  const VALUE_TYPE sides_;
  mutable std::uniform_int_distribution<VALUE_TYPE> distr_;
  mutable engine_type urbg_;

public:
  using seedType = typename engine_type::result_type;

  Dice() = default;

  static void checkNonDeterministicEntropySource() {
    std::random_device rd;
    const bool non_deterministic { (rd.entropy() > 0) };
    const bool deterministic     { (rd.entropy() == 0) };

    std::cout << "Non deterministic: " << ((non_deterministic) ? "Yes" : "No") << std::endl;
    std::cout << "Deterministic:     " << ((deterministic) ? "Yes" : "No") << std::endl << std::endl;
  }

  // generate numbers in [1, sides]
  explicit Dice(const VALUE_TYPE sides, seedType seed) :
      sides_ {sides},
      distr_ {1, sides_},
      urbg_  {seed}
  {}

  // generate numbers in [startFrom, (startFrom + (sides - 1)))]
  explicit Dice(seedType seed, const VALUE_TYPE sides, const VALUE_TYPE startFrom) :
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

template <typename Container = std::vector<unsigned int>>
static void printResults(const Container& c, size_t howMany = 0) {
  howMany = ((0 == howMany) ? c.size() : howMany);
  std::cout << "results("<< howMany << "/" << c.size() << "): ";
  size_t counter {0};
  for (const auto item : c) {
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
  std::cout << title << dur.count() << " sec." << std::endl;
}

// ----------------------------------------------------------------------------

constexpr unsigned int SIDES {20};
constexpr std::size_t RANDOM_VALUES          {500'000'000};
constexpr std::size_t RANDOM_VALUES_IN_ARRAY {500'000};

static void diceWithArrays(const unsigned int sides) {
  // make a dice and seed with clock:
  const auto ticks { std::chrono::system_clock::now().time_since_epoch().count() };
  Dice<> dice(ticks, sides, 0);

  constexpr size_t printItems {20};

  // generate sequence of dice rolls:
  constexpr std::size_t randomValues {RANDOM_VALUES_IN_ARRAY};
  std::array<unsigned int, randomValues> rolls_1;
  std::array<unsigned int, randomValues> rolls_2;
  std::array<unsigned int, randomValues> rolls_3;
  std::array<unsigned int, randomValues> rolls_4;

  std::cout << "Test run with container: std::array" << std::endl;

  getExecutionTime("1- std::generate:             ",
                   [&rolls_1, &dice] () mutable {
                     std::generate(begin(rolls_1), end(rolls_1), dice);
                   } );

  dice.reSeedWithClock();
  getExecutionTime("2- std::execution::seq:       ",
                   [&rolls_2, &dice] () mutable {
                     std::transform(std::execution::seq, rolls_2.begin(), rolls_2.end(),
                                    rolls_2.begin(), [&dice] (int arg) { return dice(); } ); });

  dice.reSeedWithClock();
  getExecutionTime("3- std::execution::par:       ",
                   [&rolls_3, &dice] () mutable {
                     std::transform(std::execution::par, rolls_3.begin(), rolls_3.end(),
                                    rolls_3.begin(), [&dice] (int arg) { return dice(); } ); });

  dice.reSeedWithClock();
  getExecutionTime("4- std::execution::par_unseq: ",
                   [&rolls_4, &dice] () mutable {
                     std::transform(std::execution::par_unseq, rolls_4.begin(), rolls_4.end(),
                                    rolls_4.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_1, printItems);
  printResults(rolls_2, printItems);
  printResults(rolls_3, printItems);
  printResults(rolls_4, printItems);
  std::cout << std::endl << std::endl;
}

template <typename Container = std::vector<unsigned int>>
void diceWithContainer(const std::string containerType,
                       const unsigned int sides,
                       const long long randomValues) {
  // make a dice and seed with clock:
  const auto ticks { std::chrono::system_clock::now().time_since_epoch().count() };
  Dice<> dice(ticks, sides, 0);

  constexpr size_t printItems {20};

  // generate sequence of dice rolls:
  Container rolls_1(randomValues);
  Container rolls_2(randomValues);
  Container rolls_3(randomValues);
  Container rolls_4(randomValues);

  std::cout << "Test run with container: " << containerType << std::endl;

  getExecutionTime("1- std::generate:             ",
                   [&rolls_1, &dice] () mutable {
                     std::generate(begin(rolls_1), end(rolls_1), dice);
                   } );

  dice.reSeedWithClock();
  getExecutionTime("2- std::execution::seq:       ",
                   [&rolls_2, &dice] () mutable {
                     std::transform(std::execution::seq, rolls_2.begin(), rolls_2.end(),
                                    rolls_2.begin(), [&dice] (int arg) { return dice(); } ); });

  dice.reSeedWithClock();
  getExecutionTime("3- std::execution::par:       ",
                   [&rolls_3, &dice] () mutable {
                     std::transform(std::execution::par, rolls_3.begin(), rolls_3.end(),
                                    rolls_3.begin(), [&dice] (int arg) { return dice(); } ); });

  dice.reSeedWithClock();
  getExecutionTime("4- std::execution::par_unseq: ",
                   [&rolls_4, &dice] () mutable {
                     std::transform(std::execution::par_unseq, rolls_4.begin(), rolls_4.end(),
                                    rolls_4.begin(), [&dice] (int arg) { return dice(); } ); });
  // print results:
  printResults(rolls_1, printItems);
  printResults(rolls_2, printItems);
  printResults(rolls_3, printItems);
  printResults(rolls_4, printItems);
  std::cout << std::endl << std::endl;
}

int main() {
  Dice<>::checkNonDeterministicEntropySource();
  diceWithArrays(SIDES);
  diceWithArrays(SIDES);
  diceWithContainer<std::deque<unsigned int>>("std::deque",   SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::deque<unsigned int>>("std::deque",   SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::vector<unsigned int>>("std::vector", SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::vector<unsigned int>>("std::vector", SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::deque<unsigned int>>("std::deque",   SIDES, RANDOM_VALUES);
  diceWithContainer<std::deque<unsigned int>>("std::deque",   SIDES, RANDOM_VALUES);
  diceWithContainer<std::vector<unsigned int>>("std::vector", SIDES, RANDOM_VALUES);
  diceWithContainer<std::vector<unsigned int>>("std::vector", SIDES, RANDOM_VALUES);
  return 0;
}

