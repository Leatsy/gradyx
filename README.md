# GradyX

GradyX 是一个面向 CPU 和 CUDA 的 C++20 深度学习框架。项目当前处于
Phase 0，正在建立基础类型、所有权模型、Runtime 边界和工程基础设施。

## 构建要求

- CMake 3.24+
- 支持 C++20 的 GCC 或 Clang
- Ninja
- Git（首次配置时获取固定版本的 GoogleTest 和 Google Benchmark）
- 可选：CUDA Toolkit 11.0+（仅 `cuda` preset 需要）

也可以安装系统版本的 GoogleTest 和 Google Benchmark，并在配置时传入
`-DGRADYX_USE_SYSTEM_DEPENDENCIES=ON`，完全禁止依赖下载。

## Debug 构建与测试

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Sanitizer

```bash
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan
```

TSan 使用独立 preset：

```bash
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan
```

也可直接使用 `tools/run_sanitizers.sh`。脚本会在当前环境不允许禁用 ASLR
时跳过 TSan 执行，并保留构建结果；此时请在允许 `setarch -R` 的主机运行 TSan。

## Release 与 Benchmark

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release

cmake --preset release-bench
cmake --build --preset release-bench
ctest --preset release-bench -L benchmark-smoke
```

## CUDA 边界

CPU-only 构建不会探测 CUDA，也不需要安装 CUDA Toolkit。`Device::parse("cuda:0")`
仍可用，而默认 Runtime Registry 查询 CUDA 会返回 `Unavailable`。

在安装 CUDA Toolkit 11.0+ 的环境中，使用独立 preset 构建 CUDA 工具边界：

```bash
cmake --preset cuda
cmake --build --preset cuda
ctest --preset cuda
```

CUDA 头文件和 `CUDA::cudart` 只链接到 `gradyx_backend_cuda`；公共 Runtime
接口不暴露 CUDA ABI 类型。

## 开发工具

```bash
tools/format.sh --check
tools/format.sh
tools/lint.sh
tools/run_sanitizers.sh
tools/run_benchmarks.sh
```

## 调试与性能

- 使用 `ctest --preset debug -R <测试名> --output-on-failure` 运行单个测试。
- CUDA 调试使用 `cuda-gdb`；运行时错误检查使用 Compute Sanitizer。
- 生成机器可读 benchmark 结果：

```bash
tools/run_benchmarks.sh
```
