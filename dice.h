//
// dice.h
//
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <algorithm>
#include <execution>

template<typename RANDOM_VALUE_TYPE,
         typename Engine = std::mt19937>  // std::mt19937_64 shows poorer performance
class Dice final {
private:
  using engine_type = Engine;
  using seedType = typename engine_type::result_type;

  mutable RANDOM_VALUE_TYPE sides_;
  mutable std::uniform_int_distribution<RANDOM_VALUE_TYPE> distr_;
  mutable seedType seed_;
  mutable engine_type urbg_;

  static seedType clockSeed() {
    return static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count());
  }

public:
  explicit Dice() :
      sides_ {6},
      distr_ {1, 6},
      seed_  {clockSeed()},
      urbg_  {seed_}
  {}

  static void checkNonDeterministicEntropySource() {
    std::random_device rd;
    const bool non_deterministic { (rd.entropy() > 0) };
    const bool deterministic     { (rd.entropy() == 0) };

    std::cout << "Non deterministic: " << ((non_deterministic) ? "Yes" : "No") << std::endl;
    std::cout << "Deterministic:     " << ((deterministic) ? "Yes" : "No") << std::endl << std::endl;
  }

  // generate numbers in [startFrom, (startFrom + (sides - 1)))] and seed with clock
  explicit Dice(const RANDOM_VALUE_TYPE sides, const RANDOM_VALUE_TYPE startFrom) :
      sides_ {sides},
      distr_ {startFrom, (startFrom + (sides_ - 1))},
      seed_  {clockSeed()},
      urbg_  {seed_}
  {}

  // copy ctor
  Dice(const Dice& rhs) {
    sides_ = rhs.sides_;
    distr_ = rhs.distr_;
    seed_  = rhs.seed_;
    urbg_  = rhs.urbg_;
  }

  // copy assignment operator
  Dice& operator=(const Dice& rhs) {
    sides_ = rhs.sides_;
    distr_ = rhs.distr_;
    seed_  = rhs.seed_;
    urbg_  = rhs.urbg_;
    return *this;
  }

  void doReSeed(const seedType newSeed) const noexcept {
    seed_ = newSeed;
    urbg_.seed(newSeed);
  }

  void doReSeedWithClock() const noexcept {
    seed_ = clockSeed();
    urbg_.seed(seed_);
  }

  int operator () () const noexcept {
    return distr_(urbg_);
  }

  seedType seed() const {
    return seed_;
  }
};  // class Dice


// diceRoller: class-based support for a dice roller
template <typename Container,
          typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937>  // std::mt19937_64 shows poorer performance
class diceRoller final {
private:
  using engine_type = Engine;
  using seedType = typename engine_type::result_type;

  mutable RANDOM_VALUE_TYPE sides_     {1};
  mutable std::size_t randomValues_    {};
  mutable RANDOM_VALUE_TYPE startFrom_ {};
  mutable Dice<RANDOM_VALUE_TYPE>dice_ {sides_, startFrom_};
  mutable Container rolls_             {};

public:
  diceRoller(const RANDOM_VALUE_TYPE startFrom,
             const RANDOM_VALUE_TYPE sides,
             const std::size_t randomValues = 0) :
  sides_ (sides),
  randomValues_ (randomValues),
  startFrom_ (startFrom)
  {
    // make a dice and seed with clock
    Dice<RANDOM_VALUE_TYPE>dice(sides, startFrom);
    dice_ = dice;

    // allocate the container
    Container rolls(randomValues);
    rolls_ = rolls;
  }

  void doReseed(const seedType newSeed) const {
    dice_.doReSeed(newSeed);
  }

  void doReSeedWithClock() const {
    dice_.doReSeedWithClock();
  }

  void doReSeedWithSame() const {
    dice_.doReSeed(dice_.seed());
  }

  // print the contents of a container; all of them by default if howMany is 0
  void printResults(const Container& c, std::size_t howMany = 0) const {
    howMany = ((0 == howMany) ? c.size() : howMany);
    std::cout << "diceRoller::printResults: container at " << &c << ": results ("<< howMany << "/" << c.size() << "): ";
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

  void info() const {
    std::cout << "diceRoller::info: dice at " << &dice_
              << " with " << sides_ << " sides"
              << " generates " << randomValues_ << " values"
              << " from " << startFrom_
              << " in container at " << &rolls_
              << " with size: " << rolls_.size()
              << " and capacity: " << rolls_.capacity()
              << std::endl;
  }

  Container& rollDice(const auto policyType) const {
    // roll the dice
    std::for_each(policyType, rolls_.begin(), rolls_.end(), [&] (RANDOM_VALUE_TYPE &arg) { arg = dice_(); } );

    //printResults(rolls_, 20);
    //std::cout << "diceRoller::rollDice: rolls_ at " << &rolls_ << std::endl;
    return rolls_;
  }

  // container is passed by ref from caller, resized, filled, and returned by the same argument
  void rollDice(Container& rolls, const std::size_t randomValues, const auto policyType) const {
    // allocate the room needed in the container
    rolls.reserve(randomValues);
    rolls.resize(randomValues);

    // roll the dice
    std::for_each(policyType, rolls.begin(), rolls.end(), [&] (RANDOM_VALUE_TYPE &arg) { arg = dice_(); } );

    //printResults(rolls, 20);
    //std::cout << "diceRoller::rollDice: rolls at " << &rolls << std::endl;
  }
};  // class diceRoller


// quick and dirty dice roller: container allocated here and returned
template <typename Container,
          typename RANDOM_VALUE_TYPE>
Container rollDice(const RANDOM_VALUE_TYPE sides,
                   const std::size_t randomValues,
                   const RANDOM_VALUE_TYPE startFrom,
                   const auto policyType) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE>dice(sides, startFrom);

  // allocate the container
  Container rolls(randomValues);

//  std::cout << "rollDice: BEFORE: size: " << rolls.size()
//            << " capacity: " << rolls.capacity()
//            << " at " << &rolls << std::endl;

  // roll the dice
  std::for_each(policyType, rolls.begin(), rolls.end(), [&dice] (RANDOM_VALUE_TYPE &arg) { arg = dice(); } );

//  std::cout << "rollDice: AFTER: size: " << rolls.size()
//            << " capacity: " << rolls.capacity()
//            << " at " << &rolls << std::endl;

  return rolls;
}

// quick and dirty dice roller: container is passed by ref from caller,
// resized, filled, and returned by the same argument
template <typename Container,
          typename RANDOM_VALUE_TYPE>
void rollDice(Container& rolls,
              const RANDOM_VALUE_TYPE sides,
              const std::size_t randomValues,
              const RANDOM_VALUE_TYPE startFrom,
              const auto policyType) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE>dice(sides, startFrom);

//  std::cout << "rollDice: BEFORE: size: " << rolls.size()
//            << " capacity: " << rolls.capacity()
//            << " at " << &rolls << std::endl;

  // allocate the room needed in the container
  rolls.reserve(randomValues);
  rolls.resize(randomValues);

//  std::cout << "rollDice: AFTER: size: " << rolls.size()
//            << " capacity: " << rolls.capacity()
//            << " at " << &rolls << std::endl;

  // roll the dice
  std::for_each(policyType, rolls.begin(), rolls.end(), [&dice] (RANDOM_VALUE_TYPE &arg) { arg = dice(); } );
}
