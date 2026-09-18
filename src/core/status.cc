#include "gradyx/core/status.h"

#include <sstream>
#include <utility>

namespace gradyx {

std::string_view error_code_name(ErrorCode code) noexcept {
  switch (code) {
    case ErrorCode::kOk:
      return "Ok";
    case ErrorCode::kInvalidArgument:
      return "InvalidArgument";
    case ErrorCode::kOutOfRange:
      return "OutOfRange";
    case ErrorCode::kOverflow:
      return "Overflow";
    case ErrorCode::kNotFound:
      return "NotFound";
    case ErrorCode::kAlreadyExists:
      return "AlreadyExists";
    case ErrorCode::kUnavailable:
      return "Unavailable";
    case ErrorCode::kResourceExhausted:
      return "ResourceExhausted";
    case ErrorCode::kFailedPrecondition:
      return "FailedPrecondition";
    case ErrorCode::kInternal:
      return "Internal";
    case ErrorCode::kNotImplemented:
      return "NotImplemented";
  }
  return "Unknown";
}

const Status& ok_status() noexcept {
  static const Status status;
  return status;
}

Status::Status(ErrorCode code, std::string message, std::source_location location)
    : code_(code), message_(std::move(message)), location_(location) {}

Status Status::error(ErrorCode code, std::string message,
                     std::source_location location) {
  if (code == ErrorCode::kOk) {
    return Status{ErrorCode::kInternal, "an error Status cannot use ErrorCode::kOk",
                  location};
  }
  return Status{code, std::move(message), location};
}

Status Status::invalid_argument(std::string message, std::source_location location) {
  return error(ErrorCode::kInvalidArgument, std::move(message), location);
}

Status Status::out_of_range(std::string message, std::source_location location) {
  return error(ErrorCode::kOutOfRange, std::move(message), location);
}

Status Status::overflow(std::string message, std::source_location location) {
  return error(ErrorCode::kOverflow, std::move(message), location);
}

Status Status::unavailable(std::string message, std::source_location location) {
  return error(ErrorCode::kUnavailable, std::move(message), location);
}

Status Status::internal(std::string message, std::source_location location) {
  return error(ErrorCode::kInternal, std::move(message), location);
}

bool Status::ok() const noexcept { return code_ == ErrorCode::kOk; }

Status::operator bool() const noexcept { return ok(); }

ErrorCode Status::code() const noexcept { return code_; }

std::string_view Status::message() const noexcept { return message_; }

const std::source_location& Status::location() const noexcept { return location_; }

std::string Status::to_string() const {
  if (ok()) {
    return "Ok";
  }

  std::ostringstream output;
  output << error_code_name(code_) << ": " << message_ << " [" << location_.file_name()
         << ':' << location_.line() << ']';
  return output.str();
}

}  // namespace gradyx
