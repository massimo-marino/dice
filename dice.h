//
// dice.h
//
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

template<typename RANDOM_VALUE_TYPE,
         typename Engine = std::mt19937>  // std::mt19937_64 shows poorer performance 
class Dice final {
private:
  using engine_type = Engine;
  const RANDOM_VALUE_TYPE sides_;
  mutable std::uniform_int_distribution<RANDOM_VALUE_TYPE> distr_;
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

  // generate numbers in [startFrom, (startFrom + (sides - 1)))] and seed with clock
  explicit Dice(const RANDOM_VALUE_TYPE sides, const RANDOM_VALUE_TYPE startFrom) :
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
};  // class Dice


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

  // seed
  dice.reSeedWithClock();

  // roll the dice
  std::for_each(policyType, rolls.begin(), rolls.end(), [&dice] (RANDOM_VALUE_TYPE &arg) { arg = dice(); } );

  //printResults<Container>(rolls, 20);
  return rolls;
}

