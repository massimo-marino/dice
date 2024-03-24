//
// example.cpp
//
#include "dice.h"

#include <iostream>
#include <array>
#include <vector>
#include <deque>

// print the contents of a container; all of them by default if howMany is 0
template <typename Container>
static void printResults(const Container& c, std::size_t howMany = 0) {
  howMany = ((0 == howMany) ? c.size() : howMany);
  std::cout << "container at " << &c << ": results ("<< howMany << "/" << c.size() << "): ";
  std::size_t counter {0};
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

using RANDOM_VALUE_TYPE = unsigned int;

// number of sides of the dice
constexpr RANDOM_VALUE_TYPE SIDES {20};
// number of random values generated for the containers cases
constexpr std::size_t RANDOM_VALUES          {2'000'000'000};
// number of random values generated for the array case
constexpr std::size_t RANDOM_VALUES_IN_ARRAY {500'000};

static void generateRollsArrayWithExecutionPolicy(auto policyType, const std::string& prompt, auto& dice) {
  std::array<RANDOM_VALUE_TYPE, RANDOM_VALUES_IN_ARRAY> rolls;

  dice.doReSeedWithClock();
  getExecutionTime(prompt,
                   [&rolls, &dice, policyType] () mutable {
                     std::for_each(policyType, rolls.begin(), rolls.end(), [&dice] (RANDOM_VALUE_TYPE &arg) { arg = dice(); } );
                     //std::transform(policyType, rolls.begin(), rolls.end(), rolls.begin(), [&dice] ([[maybe_unused]] RANDOM_VALUE_TYPE arg) { return dice(); } );
                   } );
}

static void diceWithArrays(const RANDOM_VALUE_TYPE sides) {
  std::cout << "Test run with container: std::array" << " size: " << RANDOM_VALUES_IN_ARRAY << std::endl;

  // make a dice that starts from 0, and seed with clock
  Dice<RANDOM_VALUE_TYPE>dice(sides, 0);
  constexpr std::size_t randomValues {RANDOM_VALUES_IN_ARRAY};

  // generate sequences of dice rolls
  {
    std::array<RANDOM_VALUE_TYPE, randomValues> rolls;
    getExecutionTime("1- std::generate:            ",
	             [&rolls, &dice] () mutable {
	               std::generate(begin(rolls), end(rolls), dice);
	             } );
  }

  generateRollsArrayWithExecutionPolicy(std::execution::seq,       "2- std_execution::seq:       ", dice);
  generateRollsArrayWithExecutionPolicy(std::execution::unseq,     "3- std_execution::unseq:     ", dice);
  generateRollsArrayWithExecutionPolicy(std::execution::par,       "4- std_execution::par:       ", dice);
  generateRollsArrayWithExecutionPolicy(std::execution::par_unseq, "5- std_execution::par_unseq: ", dice);

  std::cout << std::endl;
}

template <typename Container>
static void generateRollsWithExecutionPolicy(auto policyType, const std::string& prompt, auto& dice, const std::size_t randomValues) {
  Container rolls(randomValues);

  dice.doReSeedWithClock();
  getExecutionTime(prompt,
                   [&rolls, &dice, policyType] () mutable {
                     std::for_each(policyType, rolls.begin(), rolls.end(), [&dice] (RANDOM_VALUE_TYPE &arg) { arg = dice(); } );
                     //std::transform(policyType, rolls.begin(), rolls.end(), rolls.begin(), [&dice] ([[maybe_unused]] RANDOM_VALUE_TYPE arg) { return dice(); } );
                   } );
  //printResults<Container>(rolls, 20);
}

template <typename Container>
static void diceWithContainer(const std::string containerType,
                              const RANDOM_VALUE_TYPE sides,
                              const std::size_t randomValues) {
  std::cout << "Test run with container: " << containerType << " size: " << randomValues << std::endl;

  // make a dice that starts from 0, and seed with clock
  Dice<RANDOM_VALUE_TYPE>dice(sides, 0);

  // generate sequences of dice rolls
  {
    Container rolls(randomValues);
    getExecutionTime("1- std::generate:            ",
	             [&rolls, &dice] () mutable {
	               std::generate(begin(rolls), end(rolls), dice);
	             } );
    //printResults<Container>(rolls, 20);
  }

  generateRollsWithExecutionPolicy<Container>(std::execution::seq,       "2- std_execution::seq:       ", dice, randomValues);
  generateRollsWithExecutionPolicy<Container>(std::execution::unseq,     "3- std_execution::unseq:     ", dice, randomValues);
  generateRollsWithExecutionPolicy<Container>(std::execution::par,       "4- std_execution::par:       ", dice, randomValues);
  generateRollsWithExecutionPolicy<Container>(std::execution::par_unseq, "5- std_execution::par_unseq: ", dice, randomValues);

  std::cout << std::endl;
}

static void runTestExamples() {
  std::cout << "----- runTestExamples - start -----\n";
  Dice<RANDOM_VALUE_TYPE>::checkNonDeterministicEntropySource();

  diceWithArrays(SIDES);
  diceWithArrays(SIDES);

  diceWithContainer<std::deque<RANDOM_VALUE_TYPE>>("std::deque",   SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::deque<RANDOM_VALUE_TYPE>>("std::deque",   SIDES, RANDOM_VALUES_IN_ARRAY);

  diceWithContainer<std::vector<RANDOM_VALUE_TYPE>>("std::vector", SIDES, RANDOM_VALUES_IN_ARRAY);
  diceWithContainer<std::vector<RANDOM_VALUE_TYPE>>("std::vector", SIDES, RANDOM_VALUES_IN_ARRAY);

  diceWithContainer<std::deque<RANDOM_VALUE_TYPE>>("std::deque",   SIDES, RANDOM_VALUES);
  diceWithContainer<std::deque<RANDOM_VALUE_TYPE>>("std::deque",   SIDES, RANDOM_VALUES);

  diceWithContainer<std::vector<RANDOM_VALUE_TYPE>>("std::vector", SIDES, RANDOM_VALUES);
  diceWithContainer<std::vector<RANDOM_VALUE_TYPE>>("std::vector", SIDES, RANDOM_VALUES);
  std::cout << "----- runTestExamples - end -----\n\n";
}

static void makeDiceRolls() {
  std::cout << "----- makeDiceRolls - start -----\n";
  using rndType = long;
  using cnt = std::vector<rndType>;

  constexpr rndType     diceSides       { 1000 };
  constexpr std::size_t howMany         { 20 };
  constexpr rndType     startFrom       { 55 };
  constexpr auto        executionPolicy { std::execution::par };

  const auto rolls { rollDice<cnt, rndType>(diceSides, howMany, startFrom, executionPolicy) };
  printResults<cnt>(rolls, howMany);
  std::cout << "----- makeDiceRolls - end -----\n\n";
}

static void makeDiceRolls_2() {
  std::cout << "----- makeDiceRolls_2 - start -----\n";
  using rndType = long;
  using cnt = std::vector<rndType>;
  cnt rolls(0);

  constexpr rndType     diceSides       { 1000 };
  constexpr std::size_t howMany         { 20 };
  constexpr rndType     startFrom       { 55 };
  constexpr auto        executionPolicy { std::execution::par };

  std::cout << "makeDiceRolls_2: BEFORE - size: " << rolls.size()
            << " capacity: " << rolls.capacity()
            << " at " << &rolls << std::endl;

  rollDice<cnt, rndType>(rolls, diceSides, howMany, startFrom, executionPolicy);

  std::cout << "makeDiceRolls_2: AFTER - size: " << rolls.size()
            << " capacity: " << rolls.capacity()
            << " at " << &rolls << std::endl;

  printResults<cnt>(rolls, howMany);
  std::cout << "----- makeDiceRolls_2 - end -----\n\n";
}

static void runDiceRollerExample() {
  std::cout << "----- runDiceRollerExample - start -----\n";
  using rndType = unsigned int;
  using cnt = std::vector<rndType>;

  constexpr rndType     diceSides       { 1000 };
  constexpr std::size_t howMany         { 20 };
  constexpr rndType     startFrom       { 55 };
  constexpr auto        seqExecutionPolicy      { std::execution::seq };
  constexpr auto        unseqExecutionPolicy    { std::execution::unseq };
  constexpr auto        parExecutionPolicy      { std::execution::par };
  constexpr auto        parUnseqexecutionPolicy { std::execution::par_unseq };
  constexpr auto        executionPolicy         { parExecutionPolicy };

  diceRoller<cnt, rndType> dr(startFrom, diceSides, howMany);
  dr.logInfo();

  cnt& rolls_1 { dr.rollDice(executionPolicy) };
  printResults<cnt>(rolls_1, howMany);

  // rolls_2 contents will be the same of rolls_1
  // rolls_1 and rolls_2 ref to the same object
  dr.doReSeedWithSame();
  cnt& rolls_2 { dr.rollDice(executionPolicy) };
  printResults<cnt>(rolls_2, howMany);

  // rolls_3 contents will be the same of rolls_1 and rolls_2
  dr.doReSeedWithSame();
  cnt rolls_3 { dr.rollDice(executionPolicy) };
  printResults<cnt>(rolls_3, howMany);

  // rolls_4 contents will not be the same of rolls_1, rolls_2, rools_3
  dr.doReSeedWithClock();
  cnt rolls_4 { dr.rollDice(executionPolicy) };
  printResults<cnt>(rolls_4, howMany);

  // rolls_5 is declared here and passed to the dice roller
  dr.doReSeedWithClock();
  cnt rolls_5 {};
  dr.rollDice(rolls_5, howMany, executionPolicy);
  printResults<cnt>(rolls_5, howMany);

  std::cout << "rolls_1 at: " << &rolls_1 << " must be equal to rolls_2 at " << &rolls_2 << ": " << ((&rolls_1 == &rolls_2) ? "true" : "false") << std::endl;
  std::cout << "rolls_2 at: " << &rolls_2 << " must be equal to rolls_1 at " << &rolls_1 << ": " << ((&rolls_2 == &rolls_1) ? "true" : "false") << std::endl;
  std::cout << "rolls_3 at: " << &rolls_3 << " must not be equal to rolls_1 at " << &rolls_1 << " and rolls_2 at " << &rolls_2 << ": " << (((&rolls_3 != &rolls_1) && (&rolls_3 != &rolls_2)) ? "true" : "false") << std::endl;
  std::cout << "rolls_4 at: " << &rolls_4 << std::endl;
  std::cout << "rolls_5 at: " << &rolls_5 << std::endl;
  std::cout << "----- runDiceRollerExample - end -----\n\n";
}

static void rollDiceOnceExample() {
  std::cout << "----- rollDiceOnceExample - start -----\n";
  using rndType = unsigned int;
  using cnt = std::vector<rndType>;

  constexpr rndType     diceSides { 6 };
  constexpr rndType     startFrom { 1 };
  constexpr std::size_t howMany   { 50 };
  cnt rolls {};

  for (std::size_t i { 1 }; i <= howMany; ++i) {
    rolls.push_back(rollDiceOnce<unsigned int>(diceSides, startFrom));
  }
  printResults<cnt>(rolls);
  std::cout << "----- rollDiceOnceExample - end -----\n\n";
}

static void makeDiceExample() {
  using rndType = unsigned int;

  constexpr rndType diceSides { 6 };
  constexpr rndType startFrom { 1 };

  auto d { makeDice<rndType>(diceSides, startFrom)};
  d.logInfo();
}

static void runAllExamples() {
  runTestExamples();
  makeDiceRolls();
  makeDiceRolls_2();
  runDiceRollerExample();
  rollDiceOnceExample();
  makeDiceExample();
}

int main() {
  runAllExamples();
  return 0;
}
