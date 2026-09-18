#pragma once

#include <cmath>
#include <complex>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "gradyx/core/dtype.h"
#include "gradyx/core/result.h"

namespace gradyx {
namespace detail {

template <typename T>
struct IsSupportedComplex : std::false_type {};

template <>
struct IsSupportedComplex<std::complex<float>> : std::true_type {};

template <>
struct IsSupportedComplex<std::complex<double>> : std::true_type {};

template <typename T>
inline constexpr bool kIsSupportedComplex =
    IsSupportedComplex<std::remove_cv_t<T>>::value;

template <std::integral T>
[[nodiscard]] Result<T> floating_to_integer(long double value) {
  if (!std::isfinite(value)) {
    return Status::out_of_range(
        "cannot convert a non-finite floating-point value to an integer");
  }
  if (std::trunc(value) != value) {
    return Status::invalid_argument(
        "cannot convert a fractional floating-point value to an integer");
  }

  const long double upper_bound = std::ldexp(1.0L, std::numeric_limits<T>::digits);
  if constexpr (std::is_signed_v<T>) {
    if (value < -upper_bound || value >= upper_bound) {
      return Status::out_of_range("scalar conversion is out of range");
    }
  } else {
    if (value < 0.0L || value >= upper_bound) {
      return Status::out_of_range("scalar conversion is out of range");
    }
  }
  return static_cast<T>(value);
}

template <std::floating_point T>
[[nodiscard]] Result<T> checked_floating_cast(long double value) {
  const auto maximum = static_cast<long double>(std::numeric_limits<T>::max());
  if (std::isfinite(value) && (value < -maximum || value > maximum)) {
    return Status::out_of_range("scalar conversion is out of range");
  }
  return static_cast<T>(value);
}

}  // namespace detail

class Scalar final {
 public:
  explicit constexpr Scalar(bool value) noexcept
      : kind_(Kind::kBoolean), value_(value) {}

  template <std::signed_integral T>
    requires(!std::same_as<std::remove_cv_t<T>, bool>)
  explicit constexpr Scalar(T value) noexcept
      : kind_(Kind::kSignedInteger), value_(static_cast<std::int64_t>(value)) {}

  template <std::unsigned_integral T>
    requires(!std::same_as<std::remove_cv_t<T>, bool>)
  explicit constexpr Scalar(T value) noexcept
      : kind_(Kind::kUnsignedInteger), value_(static_cast<std::uint64_t>(value)) {}

  explicit constexpr Scalar(float value) noexcept
      : kind_(Kind::kFloating), value_(static_cast<double>(value)) {}
  explicit constexpr Scalar(double value) noexcept
      : kind_(Kind::kFloating), value_(value) {}

  explicit constexpr Scalar(std::complex<float> value) noexcept
      : kind_(Kind::kComplex),
        value_(ComplexValue{static_cast<double>(value.real()),
                            static_cast<double>(value.imag())}) {}
  explicit constexpr Scalar(std::complex<double> value) noexcept
      : kind_(Kind::kComplex), value_(ComplexValue{value.real(), value.imag()}) {}

  [[nodiscard]] constexpr DType dtype() const noexcept {
    switch (kind_) {
      case Kind::kBoolean:
        return DType::kBool;
      case Kind::kSignedInteger:
        return DType::kInt64;
      case Kind::kUnsignedInteger:
        return DType::kUInt64;
      case Kind::kFloating:
        return DType::kFloat64;
      case Kind::kComplex:
        return DType::kComplex128;
    }
    return DType::kFloat64;
  }

  template <typename T>
  [[nodiscard]] Result<T> to() const {
    using Target = std::remove_cv_t<T>;
    static_assert(std::same_as<T, Target>,
                  "Scalar::to<T>() requires an unqualified value type");
    static_assert(std::is_arithmetic_v<Target> || detail::kIsSupportedComplex<Target>,
                  "Scalar::to<T>() does not support this type");

    if constexpr (std::same_as<Target, bool>) {
      return to_bool();
    } else if constexpr (std::integral<Target>) {
      return to_integer<Target>();
    } else if constexpr (std::floating_point<Target>) {
      auto real = to_real();
      if (!real) {
        return real.status();
      }
      return detail::checked_floating_cast<Target>(*real);
    } else {
      using Component = typename Target::value_type;
      const auto parts = complex_parts();
      auto real = detail::checked_floating_cast<Component>(parts.real);
      if (!real) {
        return real.status();
      }
      auto imaginary = detail::checked_floating_cast<Component>(parts.imaginary);
      if (!imaginary) {
        return imaginary.status();
      }
      return Target{*real, *imaginary};
    }
  }

 private:
  enum class Kind : std::uint8_t {
    kBoolean,
    kSignedInteger,
    kUnsignedInteger,
    kFloating,
    kComplex,
  };

  struct ComplexValue {
    double real;
    double imaginary;
  };

  union Value {
    explicit constexpr Value(bool input) noexcept : boolean(input) {}
    explicit constexpr Value(std::int64_t input) noexcept : signed_integer(input) {}
    explicit constexpr Value(std::uint64_t input) noexcept : unsigned_integer(input) {}
    explicit constexpr Value(double input) noexcept : floating(input) {}
    explicit constexpr Value(ComplexValue input) noexcept : complex(input) {}

    bool boolean;
    std::int64_t signed_integer;
    std::uint64_t unsigned_integer;
    double floating;
    ComplexValue complex;
  };

  [[nodiscard]] Result<bool> to_bool() const {
    switch (kind_) {
      case Kind::kBoolean:
        return value_.boolean;
      case Kind::kSignedInteger:
        if (value_.signed_integer == 0 || value_.signed_integer == 1) {
          return value_.signed_integer == 1;
        }
        break;
      case Kind::kUnsignedInteger:
        if (value_.unsigned_integer == 0 || value_.unsigned_integer == 1) {
          return value_.unsigned_integer == 1;
        }
        break;
      case Kind::kFloating:
        if (value_.floating == 0.0 || value_.floating == 1.0) {
          return value_.floating == 1.0;
        }
        break;
      case Kind::kComplex:
        if (value_.complex.imaginary == 0.0 &&
            (value_.complex.real == 0.0 || value_.complex.real == 1.0)) {
          return value_.complex.real == 1.0;
        }
        break;
    }
    return Status::invalid_argument(
        "only scalar values 0 and 1 can be converted to bool");
  }

  template <std::integral T>
  [[nodiscard]] Result<T> to_integer() const {
    switch (kind_) {
      case Kind::kBoolean:
        return static_cast<T>(value_.boolean);
      case Kind::kSignedInteger:
        if (!std::in_range<T>(value_.signed_integer)) {
          return Status::out_of_range("scalar conversion is out of range");
        }
        return static_cast<T>(value_.signed_integer);
      case Kind::kUnsignedInteger:
        if (!std::in_range<T>(value_.unsigned_integer)) {
          return Status::out_of_range("scalar conversion is out of range");
        }
        return static_cast<T>(value_.unsigned_integer);
      case Kind::kFloating:
        return detail::floating_to_integer<T>(
            static_cast<long double>(value_.floating));
      case Kind::kComplex:
        if (value_.complex.imaginary != 0.0) {
          return Status::invalid_argument(
              "cannot convert a scalar with a non-zero imaginary part");
        }
        return detail::floating_to_integer<T>(
            static_cast<long double>(value_.complex.real));
    }
    return Status::internal("invalid Scalar kind");
  }

  [[nodiscard]] Result<long double> to_real() const {
    switch (kind_) {
      case Kind::kBoolean:
        return value_.boolean ? 1.0L : 0.0L;
      case Kind::kSignedInteger:
        return static_cast<long double>(value_.signed_integer);
      case Kind::kUnsignedInteger:
        return static_cast<long double>(value_.unsigned_integer);
      case Kind::kFloating:
        return static_cast<long double>(value_.floating);
      case Kind::kComplex:
        if (value_.complex.imaginary != 0.0) {
          return Status::invalid_argument(
              "cannot convert a scalar with a non-zero imaginary part");
        }
        return static_cast<long double>(value_.complex.real);
    }
    return Status::internal("invalid Scalar kind");
  }

  [[nodiscard]] constexpr ComplexValue complex_parts() const noexcept {
    switch (kind_) {
      case Kind::kBoolean:
        return {value_.boolean ? 1.0 : 0.0, 0.0};
      case Kind::kSignedInteger:
        return {static_cast<double>(value_.signed_integer), 0.0};
      case Kind::kUnsignedInteger:
        return {static_cast<double>(value_.unsigned_integer), 0.0};
      case Kind::kFloating:
        return {value_.floating, 0.0};
      case Kind::kComplex:
        return value_.complex;
    }
    return {0.0, 0.0};
  }

  Kind kind_;
  Value value_;
};

}  // namespace gradyx
