#include "gradyx/core/dtype.h"

#include <array>
#include <cstddef>
#include <ostream>
#include <string>

namespace gradyx {
namespace {

template <typename T>
constexpr DTypeInfo make_dtype_info(std::string_view name, DTypeCategory category) {
  return {name, static_cast<std::uint8_t>(sizeof(T)),
          static_cast<std::uint8_t>(alignof(T)), category};
}

constexpr std::array<DTypeInfo, 13> kDTypeTable{{
    make_dtype_info<bool>("bool", DTypeCategory::kBoolean),
    make_dtype_info<std::uint8_t>("uint8", DTypeCategory::kUnsignedInteger),
    make_dtype_info<std::int8_t>("int8", DTypeCategory::kSignedInteger),
    make_dtype_info<std::int16_t>("int16", DTypeCategory::kSignedInteger),
    make_dtype_info<std::int32_t>("int32", DTypeCategory::kSignedInteger),
    make_dtype_info<std::int64_t>("int64", DTypeCategory::kSignedInteger),
    make_dtype_info<Float16>("float16", DTypeCategory::kFloating),
    make_dtype_info<BFloat16>("bfloat16", DTypeCategory::kFloating),
    make_dtype_info<float>("float32", DTypeCategory::kFloating),
    make_dtype_info<double>("float64", DTypeCategory::kFloating),
    make_dtype_info<std::complex<float>>("complex64", DTypeCategory::kComplex),
    make_dtype_info<std::complex<double>>("complex128", DTypeCategory::kComplex),
    make_dtype_info<std::uint64_t>("uint64", DTypeCategory::kUnsignedInteger),
}};

static_assert(kDTypeTable.size() == static_cast<std::size_t>(DType::kUInt64) + 1);

const DTypeInfo* find_dtype_info(DType dtype) noexcept {
  if (!is_valid_dtype(dtype)) {
    return nullptr;
  }
  return &kDTypeTable[static_cast<std::size_t>(dtype)];
}

}  // namespace

Result<DTypeInfo> dtype_info(DType dtype) {
  const auto* info = find_dtype_info(dtype);
  if (info == nullptr) {
    return Status::invalid_argument("invalid DType value");
  }
  return *info;
}

Result<std::string_view> dtype_name(DType dtype) {
  auto info = dtype_info(dtype);
  if (!info) {
    return info.status();
  }
  return info->name;
}

Result<std::size_t> element_size(DType dtype) {
  auto info = dtype_info(dtype);
  if (!info) {
    return info.status();
  }
  return static_cast<std::size_t>(info->size_bytes);
}

Result<DType> parse_dtype(std::string_view text) {
  for (std::size_t index = 0; index < kDTypeTable.size(); ++index) {
    if (kDTypeTable[index].name == text) {
      return static_cast<DType>(index);
    }
  }
  return Status::invalid_argument("unknown dtype: " + std::string{text});
}

bool is_boolean(DType dtype) noexcept {
  const auto* info = find_dtype_info(dtype);
  return info != nullptr && info->category == DTypeCategory::kBoolean;
}

bool is_integral(DType dtype) noexcept {
  const auto* info = find_dtype_info(dtype);
  return info != nullptr && (info->category == DTypeCategory::kUnsignedInteger ||
                             info->category == DTypeCategory::kSignedInteger);
}

bool is_floating(DType dtype) noexcept {
  const auto* info = find_dtype_info(dtype);
  return info != nullptr && info->category == DTypeCategory::kFloating;
}

bool is_complex(DType dtype) noexcept {
  const auto* info = find_dtype_info(dtype);
  return info != nullptr && info->category == DTypeCategory::kComplex;
}

std::ostream& operator<<(std::ostream& output, DType dtype) {
  const auto* info = find_dtype_info(dtype);
  if (info != nullptr) {
    return output << info->name;
  }
  return output << "invalid_dtype(" << static_cast<unsigned int>(dtype) << ')';
}

}  // namespace gradyx
