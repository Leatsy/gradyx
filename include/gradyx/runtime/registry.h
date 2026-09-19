#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "gradyx/core/device.h"
#include "gradyx/core/result.h"
#include "gradyx/core/status.h"
#include "gradyx/runtime/runtime.h"

namespace gradyx {

class RuntimeRegistry final {
 public:
  RuntimeRegistry() = default;

  RuntimeRegistry(const RuntimeRegistry&) = delete;
  RuntimeRegistry& operator=(const RuntimeRegistry&) = delete;

  [[nodiscard]] Status register_runtime(std::shared_ptr<Runtime> runtime);
  [[nodiscard]] Result<std::shared_ptr<Runtime>> get(DeviceType type) const;

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<DeviceType, std::shared_ptr<Runtime>> runtimes_;
};

}  // namespace gradyx
