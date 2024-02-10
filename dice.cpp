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

  // generate numbers in [startFrom, (startFrom + (sides - 1)))]
  // and seed with clock
  explicit Dice(const VALUE_TYPE sides, const VALUE_TYPE startFrom) :
      sides_ {sides},
      distr_ {startFrom, (startFrom + (sides_ - 1))},
      urbg_  {static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count())}
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
    if (counter == c.size()) {
      break;
    } else if (counter == howMany) {
      std::cout << "...";
      break;
    }
  }
  std::cout << '\n';
}

template <typename Func>
static void getExecutionTime(const std::string& title, Func func) {
  const auto startTime { std::chrono::high_resolution_clock::now() };

  func();

  const auto endTime { std::chrono::high_resolution_clock::now() };
  const auto takenTimeNsec { std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() };
  const std::chrono::duration<double> takenTimeSec { endTime - startTime };
  std::cout << title << takenTimeSec.count() << " sec. - " << takenTimeNsec << " nsec.\n";
}

// ----------------------------------------------------------------------------

constexpr unsigned int SIDES {20};
constexpr std::size_t RANDOM_VALUES          {500'000'000};
constexpr std::size_t RANDOM_VALUES_IN_ARRAY {500'000};

static void generateRollsArray(auto policyType, const std::string& prompt, auto& dice) {
  std::array<unsigned int, RANDOM_VALUES_IN_ARRAY> rolls;

  dice.reSeedWithClock();
  getExecutionTime(prompt,
                   [&rolls, &dice, policyType] () mutable {
                     std::for_each(policyType, rolls.begin(), rolls.end(), [&dice](unsigned int &arg) { arg = dice(); });
                     //std::transform(policyType, rolls.begin(), rolls.end(), rolls.begin(), [&dice] ([[maybe_unused]] unsigned int arg) { return dice(); } );
                     });
}

static void diceWithArrays(const unsigned int sides) {
  // make a dice and seed with clock
  Dice<>dice(sides, 0);

  // generate sequence of dice rolls
  constexpr std::size_t randomValues {RANDOM_VALUES_IN_ARRAY};

  std::cout << "Test run with container: std::array" << " size: " << randomValues << std::endl;

  {
    std::array<unsigned int, randomValues> rolls;
    getExecutionTime("1- std::generate:            ",
	             [&rolls, &dice] () mutable {
	               std::generate(begin(rolls), end(rolls), dice);
	             } );
  }

  generateRollsArray(std::execution::seq,       "2- std_execution::seq:       ", dice);
  generateRollsArray(std::execution::unseq,     "3- std_execution::unseq:     ", dice);
  generateRollsArray(std::execution::par,       "4- std_execution::par:       ", dice);
  generateRollsArray(std::execution::par_unseq, "5- std_execution::par_unseq: ", dice);

  std::cout << std::endl;
}

template <typename Container = std::vector<unsigned int>>
static void generateRolls(auto policyType, const std::string& prompt, auto& dice, const std::size_t randomValues) {
  Container rolls(randomValues);

  dice.reSeedWithClock();
  getExecutionTime(prompt,
                   [&rolls, &dice, policyType] () mutable {
                     std::for_each(policyType, rolls.begin(), rolls.end(), [&dice](unsigned int &arg) { arg = dice(); });
                     //std::transform(policyType, rolls.begin(), rolls.end(), rolls.begin(), [&dice] ([[maybe_unused]] unsigned int arg) { return dice(); } );
                     });
  //printResults(rolls, 20);
}

template <typename Container = std::vector<unsigned int>>
static void diceWithContainer(const std::string containerType,
                              const unsigned int sides,
                              const long long randomValues) {
  // make a dice and seed with clock
  Dice<>dice(sides, 0);

  // generate sequence of dice rolls
  std::cout << "Test run with container: " << containerType << " size: " << randomValues << std::endl;

  {
    Container rolls(randomValues);
    getExecutionTime("1- std::generate:            ",
	             [&rolls, &dice] () mutable {
	               std::generate(begin(rolls), end(rolls), dice);
	             } );
    //printResults(rolls, 20);
  }

  generateRolls(std::execution::seq,       "2- std_execution::seq:       ", dice, randomValues);
  generateRolls(std::execution::unseq,     "3- std_execution::unseq:     ", dice, randomValues);
  generateRolls(std::execution::par,       "4- std_execution::par:       ", dice, randomValues);
  generateRolls(std::execution::par_unseq, "5- std_execution::par_unseq: ", dice, randomValues);

  std::cout << std::endl;
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

