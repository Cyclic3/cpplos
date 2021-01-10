#include <cpplos/rating.hpp>

#include <cpplos/common.hpp>

#include <algorithm>
#include <cmath>
#include <charconv>
#include <iostream>

namespace cpplos {
  float rating::calculate_prob(rating const& opponent) const {
    const auto transformed_rating = std::pow(10, value / adjust);
    const auto other_transformed_rating = std::pow(10, opponent.value / adjust);
    return transformed_rating / (transformed_rating + other_transformed_rating);
  }

  void rating::adjust_rating(rating& opponent, result res) {
    const auto expected = calculate_prob(opponent);
    const auto other_expected = 1 - expected;

    auto score = calculate_value(res);
    auto other_score = calculate_value(invert_result(res));

    this->value += K * (score - expected);
    opponent.value += K * (other_score - other_expected);
  }

  std::ostream& operator<<(std::ostream& os, rating const& r) {
    return os << r.value;
  }

  template<>
  rating parse(std::string_view sv) {
    rating ret;
    ret.value = parse<decltype(ret.value)>(sv);
    return ret;
  }
}
