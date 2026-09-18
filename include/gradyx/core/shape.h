#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

#include "gradyx/core/result.h"

namespace gradyx {

using Dim = std::int64_t;
using Stride = std::int64_t;
using Strides = std::vector<Stride>;

class Shape final {
 public:
  Shape() = default;
  explicit Shape(std::span<const Dim> dims);
  Shape(std::initializer_list<Dim> dims);

  [[nodiscard]] static Result<Shape> from_dims(std::span<const Dim> dims);

  [[nodiscard]] std::size_t rank() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] Dim operator[](std::size_t axis) const;
  [[nodiscard]] std::span<const Dim> dims() const noexcept;
  [[nodiscard]] Result<std::uint64_t> numel() const;

  friend bool operator==(const Shape&, const Shape&) = default;

 private:
  struct ValidatedTag {};

  Shape(std::vector<Dim> dims, ValidatedTag);

  std::vector<Dim> dims_;
};

[[nodiscard]] Result<Strides> contiguous_strides(const Shape& shape);
[[nodiscard]] Result<bool> is_contiguous(const Shape& shape,
                                         std::span<const Stride> strides);

}  // namespace gradyx
