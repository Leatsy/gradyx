#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string_view>
#include <type_traits>

#include "gradyx/core/result.h"

namespace gradyx {

struct Float16 final {
  [[nodiscard]] static constexpr Float16 from_bits(std::uint16_t bits) noexcept {
    return Float16{bits};
  }

  [[nodiscard]] constexpr std::uint16_t bits() const noexcept { return bits_; }
  friend constexpr bool operator==(Float16, Float16) = default;

 private:
  explicit constexpr Float16(std::uint16_t bits) noexcept : bits_(bits) {}

  std::uint16_t bits_;
};

struct BFloat16 final {
  [[nodiscard]] static constexpr BFloat16 from_bits(std::uint16_t bits) noexcept {
    return BFloat16{bits};
  }

  [[nodiscard]] constexpr std::uint16_t bits() const noexcept { return bits_; }
  friend constexpr bool operator==(BFloat16, BFloat16) = default;

 private:
  explicit constexpr BFloat16(std::uint16_t bits) noexcept : bits_(bits) {}

  std::uint16_t bits_;
};

enum class DType : std::uint8_t {
  kBool = 0,
  kUInt8 = 1,
  kInt8 = 2,
  kInt16 = 3,
  kInt32 = 4,
  kInt64 = 5,
  kFloat16 = 6,
  kBFloat16 = 7,
  kFloat32 = 8,
  kFloat64 = 9,
  kComplex64 = 10,
  kComplex128 = 11,
  kUInt64 = 12,
};

enum class DTypeCategory : std::uint8_t {
  kBoolean,
  kUnsignedInteger,
  kSignedInteger,
  kFloating,
  kComplex,
};

struct DTypeInfo final {
  std::string_view name;
  std::uint8_t size_bytes;
  std::uint8_t alignment;
  DTypeCategory category;

  friend constexpr bool operator==(DTypeInfo, DTypeInfo) = default;
};

[[nodiscard]] constexpr bool is_valid_dtype(DType dtype) noexcept {
  return dtype >= DType::kBool && dtype <= DType::kUInt64;
}

[[nodiscard]] Result<DTypeInfo> dtype_info(DType dtype);
[[nodiscard]] Result<std::string_view> dtype_name(DType dtype);
[[nodiscard]] Result<std::size_t> element_size(DType dtype);
[[nodiscard]] Result<DType> parse_dtype(std::string_view text);

[[nodiscard]] bool is_boolean(DType dtype) noexcept;
[[nodiscard]] bool is_integral(DType dtype) noexcept;
[[nodiscard]] bool is_floating(DType dtype) noexcept;
[[nodiscard]] bool is_complex(DType dtype) noexcept;

std::ostream& operator<<(std::ostream& output, DType dtype);

template <typename T>
struct DTypeTraits {
  static constexpr bool kIsSpecialized = false;
};

template <>
struct DTypeTraits<bool> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kBool;
};

template <>
struct DTypeTraits<std::uint8_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kUInt8;
};

template <>
struct DTypeTraits<std::int8_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kInt8;
};

template <>
struct DTypeTraits<std::int16_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kInt16;
};

template <>
struct DTypeTraits<std::int32_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kInt32;
};

template <>
struct DTypeTraits<std::int64_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kInt64;
};

template <>
struct DTypeTraits<std::uint64_t> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kUInt64;
};

template <>
struct DTypeTraits<Float16> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kFloat16;
};

template <>
struct DTypeTraits<BFloat16> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kBFloat16;
};

template <>
struct DTypeTraits<float> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kFloat32;
};

template <>
struct DTypeTraits<double> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kFloat64;
};

template <>
struct DTypeTraits<std::complex<float>> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kComplex64;
};

template <>
struct DTypeTraits<std::complex<double>> {
  static constexpr bool kIsSpecialized = true;
  static constexpr DType kDType = DType::kComplex128;
};

template <typename T>
inline constexpr bool kHasDTypeV = DTypeTraits<std::remove_cv_t<T>>::kIsSpecialized;

template <typename T>
  requires(kHasDTypeV<T>)
[[nodiscard]] constexpr DType dtype_of() noexcept {
  return DTypeTraits<std::remove_cv_t<T>>::kDType;
}

}  // namespace gradyx
