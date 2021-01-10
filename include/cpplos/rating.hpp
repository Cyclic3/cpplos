#pragma once

#include <chrono>
#include <ios>

namespace cpplos {
  enum class result {
    Win,
    Loss,
    Draw
  };
  constexpr result invert_result(result r) {
    switch (r) {
      case result::Win: return result::Loss;
      case result::Loss: return result::Win;
      case result::Draw: return result::Draw;
    }
  }

  class rating {
  private:
    constexpr static float K = 24;
    constexpr static float adjust = 400;
    constexpr static float calculate_value(result r) {
      switch (r) { case result::Win: return 1; case result::Loss: return 0; case result::Draw: return 0.5; }
    }

  public:
    uint16_t value;

  public:
    void adjust_rating(rating& opponent, result res);
    float calculate_prob(rating const& opponent) const;
  };

  std::ostream& operator<<(std::ostream&, rating const&);
}
