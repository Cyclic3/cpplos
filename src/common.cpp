#include <cpplos/common.hpp>

#include <charconv>

namespace cpplos {
  template<typename T>
  static T parse_num(std::string_view sv) {
    auto end = &sv.back() + 1;
    T t;
    if (auto res = std::from_chars(sv.data(), end, t); res.ec != std::errc{} || res.ptr != end)
      throw parsing_error{"Invalid number"};
    return t;
  }

#define CPPLOS_NUM(T) template<> T parse(std::string_view sv) { return parse_num<T>(sv); }

  CPPLOS_NUM(uint8_t );
  CPPLOS_NUM(uint16_t);
  CPPLOS_NUM(uint32_t);
  CPPLOS_NUM(uint64_t);
  CPPLOS_NUM( int8_t );
  CPPLOS_NUM( int16_t);
  CPPLOS_NUM( int32_t);
  CPPLOS_NUM( int64_t);
//  CPPLOS_NUM(float);
//  CPPLOS_NUM(double);

#undef CPPLOS_NUM
}
