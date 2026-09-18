#pragma once

#include <cstdint>
#include <source_location>
#include <string>
#include <string_view>

namespace gradyx {

enum class ErrorCode : std::uint8_t {
  kOk = 0,
  kInvalidArgument,
  kOutOfRange,
  kOverflow,
  kNotFound,
  kAlreadyExists,
  kUnavailable,
  kResourceExhausted,
  kFailedPrecondition,
  kInternal,
  kNotImplemented,
};

class Status;

[[nodiscard]] std::string_view error_code_name(ErrorCode code) noexcept;
[[nodiscard]] const Status& ok_status() noexcept;

class [[nodiscard]] Status final {
 public:
  Status() noexcept = default;

  [[nodiscard]] static Status error(
      ErrorCode code, std::string message,
      std::source_location location = std::source_location::current());

  [[nodiscard]] static Status invalid_argument(
      std::string message,
      std::source_location location = std::source_location::current());
  [[nodiscard]] static Status out_of_range(
      std::string message,
      std::source_location location = std::source_location::current());
  [[nodiscard]] static Status overflow(
      std::string message,
      std::source_location location = std::source_location::current());
  [[nodiscard]] static Status unavailable(
      std::string message,
      std::source_location location = std::source_location::current());
  [[nodiscard]] static Status internal(
      std::string message,
      std::source_location location = std::source_location::current());

  [[nodiscard]] bool ok() const noexcept;
  [[nodiscard]] explicit operator bool() const noexcept;
  [[nodiscard]] ErrorCode code() const noexcept;
  [[nodiscard]] std::string_view message() const noexcept;
  [[nodiscard]] const std::source_location& location() const noexcept;
  [[nodiscard]] std::string to_string() const;

 private:
  Status(ErrorCode code, std::string message, std::source_location location);

  ErrorCode code_{ErrorCode::kOk};
  std::string message_;
  std::source_location location_ = std::source_location::current();
};

}  // namespace gradyx
