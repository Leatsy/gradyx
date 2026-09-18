# GradyX

GradyX 是一个面向 CPU 和 CUDA 的 C++20 深度学习框架。项目当前处于
Phase 0，正在建立基础类型、所有权模型、Runtime 边界和工程基础设施。

## 构建要求

- CMake 3.24+
- 支持 C++20 的 GCC 或 Clang
- Ninja
- Git（首次配置时获取固定版本的 GoogleTest 和 Google Benchmark）

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

## Release 与 Benchmark

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release

cmake --preset release-bench
cmake --build --preset release-bench
ctest --preset release-bench -L benchmark-smoke
```

## 开发工具

```bash
tools/format.sh --check
tools/format.sh
tools/lint.sh
tools/run_sanitizers.sh
```
