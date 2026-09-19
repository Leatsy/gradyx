#include "gradyx/runtime/registry.h"

#include <mutex>
#include <utility>

namespace gradyx {

Status RuntimeRegistry::register_runtime(std::shared_ptr<Runtime> runtime) {
  if (runtime == nullptr) {
    return Status::invalid_argument("cannot register a null runtime");
  }

  const DeviceType type = runtime->device_type();
  if (!is_valid_device_type(type)) {
    return Status::invalid_argument("runtime reports an invalid device type");
  }

  std::unique_lock lock{mutex_};
  if (runtimes_.contains(type)) {
    return Status::error(ErrorCode::kAlreadyExists,
                         "a runtime is already registered for this device type");
  }
  runtimes_.emplace(type, std::move(runtime));
  return {};
}

Result<std::shared_ptr<Runtime>> RuntimeRegistry::get(DeviceType type) const {
  if (!is_valid_device_type(type)) {
    return Status::invalid_argument("cannot query an invalid device type");
  }

  std::shared_lock lock{mutex_};
  const auto found = runtimes_.find(type);
  if (found == runtimes_.end()) {
    return Status::unavailable("no runtime is registered for this device type");
  }
  return found->second;
}

}  // namespace gradyx
