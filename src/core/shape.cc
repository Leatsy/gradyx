#include "gradyx/core/shape.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "gradyx/core/checked_math.h"

namespace gradyx {
namespace {

Status validate_dims(std::span<const Dim> dims) {
  if (std::ranges::any_of(dims, [](Dim dim) { return dim < 0; })) {
    return Status::invalid_argument("shape dimensions must be non-negative");
  }
  return {};
}

}  // namespace

Shape::Shape(std::span<const Dim> dims) : dims_(dims.begin(), dims.end()) {
  const auto status = validate_dims(dims);
  if (!status) {
    throw std::invalid_argument(std::string{status.message()});
  }
}

Shape::Shape(std::initializer_list<Dim> dims)
    : Shape(std::span<const Dim>{dims.begin(), dims.size()}) {}

Shape::Shape(std::vector<Dim> dims, ValidatedTag) : dims_(std::move(dims)) {}

Result<Shape> Shape::from_dims(std::span<const Dim> dims) {
  const auto status = validate_dims(dims);
  if (!status) {
    return status;
  }
  return Shape{std::vector<Dim>{dims.begin(), dims.end()}, ValidatedTag{}};
}

std::size_t Shape::rank() const noexcept { return dims_.size(); }

bool Shape::empty() const noexcept { return dims_.empty(); }

Dim Shape::operator[](std::size_t axis) const { return dims_.at(axis); }

std::span<const Dim> Shape::dims() const noexcept { return dims_; }

Result<std::uint64_t> Shape::numel() const {
  if (std::ranges::find(dims_, Dim{0}) != dims_.end()) {
    return std::uint64_t{0};
  }

  std::uint64_t count = 1;
  for (const Dim dim : dims_) {
    auto next = checked_multiply(count, static_cast<std::uint64_t>(dim));
    if (!next) {
      return next.status();
    }
    count = *next;
  }
  return count;
}

Result<Strides> contiguous_strides(const Shape& shape) {
  Strides strides(shape.rank(), Stride{1});
  Stride next_stride = 1;

  for (std::size_t axis = shape.rank(); axis > 0; --axis) {
    const auto index = axis - 1;
    strides[index] = next_stride;
    const Dim extent = std::max(shape[index], Dim{1});
    auto next = checked_multiply(next_stride, extent);
    if (!next) {
      return next.status();
    }
    next_stride = *next;
  }
  return strides;
}

Result<bool> is_contiguous(const Shape& shape, std::span<const Stride> strides) {
  if (shape.rank() != strides.size()) {
    return Status::invalid_argument("shape and strides must have the same rank");
  }

  auto expected = contiguous_strides(shape);
  if (!expected) {
    return expected.status();
  }
  return std::ranges::equal(*expected, strides);
}

}  // namespace gradyx
