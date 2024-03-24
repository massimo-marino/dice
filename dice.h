//
// dice.h
//
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <algorithm>
#include <execution>

template <typename Engine = std::mt19937_64>  // std::mt19937
class DiceBase {
protected:
  using engine_type = Engine;
  using seedType = typename engine_type::result_type;

  mutable seedType seed_;
  mutable engine_type urbg_;

public:
  explicit DiceBase() :
      seed_ {clockSeed()},
      urbg_ {seed_}
  {}

  static void checkNonDeterministicEntropySource() noexcept {
    std::random_device rd;
    const bool non_deterministic { (rd.entropy() > 0) };
    const bool deterministic     { (rd.entropy() == 0) };

    std::cout << "Non deterministic: " << ((non_deterministic) ? "Yes" : "No") << std::endl
              << "Deterministic:     " << ((deterministic) ? "Yes" : "No") << std::endl << std::endl;
  }

  static seedType clockSeed() noexcept {
    return static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count());
  }

  void doReSeed(const seedType newSeed) const noexcept {
    seed_ = newSeed;
    urbg_.seed(newSeed);
  }

  void doReSeedWithClock() const noexcept {
    seed_ = clockSeed();
    urbg_.seed(seed_);
  }

  seedType seed() const noexcept {
    return seed_;
  }
};  // class DiceBase

// Dice that generates integers has an integer number of sides
template<typename INTEGER_RANDOM_VALUE_TYPE,
         typename Engine = std::mt19937_64>  // std::mt19937
class Dice final : public DiceBase<Engine> {
private:
  mutable INTEGER_RANDOM_VALUE_TYPE sides_;
  mutable std::uniform_int_distribution<INTEGER_RANDOM_VALUE_TYPE> distr_;

public:
  explicit Dice() :
      DiceBase<Engine>(),
      sides_ {6},
      distr_ {1, 6}
  {}

  // generate integer numbers in [startFrom, (startFrom + (sides - 1)))] and seed with clock
  explicit Dice(const INTEGER_RANDOM_VALUE_TYPE sides, const INTEGER_RANDOM_VALUE_TYPE startFrom) :
      DiceBase<Engine>(),
      sides_ {sides},
      distr_ {startFrom, (startFrom + (sides_ - 1))}
  {}

  // copy ctor
  Dice(const Dice& rhs) {
    sides_ = rhs.sides_;
    distr_ = rhs.distr_;
    DiceBase<Engine>::seed_ = rhs.seed_;
    DiceBase<Engine>::urbg_ = rhs.urbg_;
  }

  // copy assignment operator
  Dice& operator=(const Dice& rhs) {
    sides_ = rhs.sides_;
    distr_ = rhs.distr_;
    DiceBase<Engine>::seed_ = rhs.seed_;
    DiceBase<Engine>::urbg_ = rhs.urbg_;
    return *this;
  }

  void logInfo() const noexcept {
    std::cout << "Dice::logInfo: dice at " << this
              << " with " << sides_ << " sides, seed " << DiceBase<Engine>::seed_
              << " generates from " << distr_.min() << " to " << distr_.max()
              << std::endl;
  }

  int operator () () const noexcept {
    return distr_(DiceBase<Engine>::urbg_);
  }
};  // class Dice


// DiceRoller: class-based support for a dice roller
template <typename Container,
          typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937_64>  // std::mt19937
class DiceRoller final {
private:
  using engine_type = Engine;
  using seedType = typename engine_type::result_type;

  mutable RANDOM_VALUE_TYPE sides_     {1};
  mutable std::size_t randomValues_    {};
  mutable RANDOM_VALUE_TYPE startFrom_ {};
  mutable Dice<RANDOM_VALUE_TYPE>dice_ {sides_, startFrom_};
  mutable Container rolls_             {};

public:
  DiceRoller(const RANDOM_VALUE_TYPE startFrom,
             const RANDOM_VALUE_TYPE sides,
             const std::size_t randomValues = 0) :
  sides_ (sides),
  randomValues_ (randomValues),
  startFrom_ (startFrom)
  {
    // make a dice and seed with clock
    Dice<RANDOM_VALUE_TYPE, engine_type>dice(sides, startFrom);
    dice_ = dice;

    // allocate the container
    Container rolls(randomValues);
    rolls_ = rolls;
  }

  void doReseed(const seedType newSeed) const noexcept {
    dice_.doReSeed(newSeed);
  }

  void doReSeedWithClock() const noexcept {
    dice_.doReSeedWithClock();
  }

  void doReSeedWithSame() const noexcept {
    dice_.doReSeed(dice_.seed());
  }

  // print the contents of a container; all of them by default if howMany is 0
  void printResults(const Container& c, std::size_t howMany = 0) const noexcept {
    howMany = ((0 == howMany) ? c.size() : howMany);
    std::cout << "DiceRoller::printResults: container at " << &c << ": results ("<< howMany << "/" << c.size() << "): ";
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

  void logInfo() const noexcept {
    std::cout << "DiceRoller::logInfo: dice at " << &dice_
              << " with " << sides_ << " sides"
              << " generates " << randomValues_ << " values"
              << " from " << startFrom_
              << " in container at " << &rolls_
              << " with size: " << rolls_.size()
              << " and capacity: " << rolls_.capacity()
              << std::endl;
  }

  Container& rollDice(const auto policyType) const noexcept {
    // roll the dice
    std::for_each(policyType, rolls_.begin(), rolls_.end(), [&] (RANDOM_VALUE_TYPE &arg) { arg = dice_(); } );

    //printResults(rolls_, 20);
    //std::cout << "DiceRoller::rollDice: rolls_ at " << &rolls_ << std::endl;
    return rolls_;
  }

  // container is passed by ref from caller, resized, filled, and returned by the same argument
  void rollDice(Container& rolls, const std::size_t randomValues, const auto policyType) const noexcept {
    // allocate the room needed in the container
    rolls.reserve(randomValues);
    rolls.resize(randomValues);

    // roll the dice
    std::for_each(policyType, rolls.begin(), rolls.end(), [&] (RANDOM_VALUE_TYPE &arg) { arg = dice_(); } );

    //printResults(rolls, 20);
    //std::cout << "DiceRoller::rollDice: rolls at " << &rolls << std::endl;
  }
};  // class diceRoller


// quick and dirty dice roller: container allocated here and returned
template <typename Container,
          typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937_64>  // std::mt19937
Container rollDice(const RANDOM_VALUE_TYPE sides,
                   const std::size_t randomValues,
                   const RANDOM_VALUE_TYPE startFrom,
                   const auto policyType) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE, Engine>dice(sides, startFrom);

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
          typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937_64>  // std::mt19937
void rollDice(Container& rolls,
              const RANDOM_VALUE_TYPE sides,
              const std::size_t randomValues,
              const RANDOM_VALUE_TYPE startFrom,
              const auto policyType) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE, Engine>dice(sides, startFrom);

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

template <typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937_64>  // std::mt19937
RANDOM_VALUE_TYPE rollDiceOnce(const RANDOM_VALUE_TYPE sides,
                               const RANDOM_VALUE_TYPE startFrom) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE, Engine>dice(sides, startFrom);

  // roll the dice and return the generated random value
  return dice();
}

template <typename RANDOM_VALUE_TYPE,
          typename Engine = std::mt19937_64>  // std::mt19937
Dice<RANDOM_VALUE_TYPE, Engine> makeDice(const RANDOM_VALUE_TYPE diceSides,
                                         const RANDOM_VALUE_TYPE startFrom) {
  // make a dice and seed with clock
  Dice<RANDOM_VALUE_TYPE, Engine> dice(diceSides, startFrom);

  // return the dice
  return dice;
}
