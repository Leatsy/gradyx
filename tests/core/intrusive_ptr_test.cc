#include "gradyx/core/intrusive_ptr.h"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

namespace gradyx {
namespace {

class Tracked final : public IntrusivePtrTarget {
 public:
  explicit Tracked(std::atomic<std::size_t>* destructions) noexcept
      : destructions_(destructions) {}
  ~Tracked() noexcept override { destructions_->fetch_add(1); }

 private:
  std::atomic<std::size_t>* destructions_;
};

class ThrowingTracked final : public IntrusivePtrTarget {
 public:
  ThrowingTracked() { throw std::runtime_error("construction failure"); }
};

TEST(IntrusivePtrTest, CopiesMovesAndResets) {
  std::atomic<std::size_t> destructions{0};
  auto first = make_intrusive<Tracked>(&destructions);
  EXPECT_EQ(first.use_count(), 1U);

  auto second = first;
  EXPECT_EQ(first.use_count(), 2U);

  auto third = std::move(second);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_FALSE(second);
  EXPECT_EQ(third.use_count(), 2U);

  second = third;
  EXPECT_EQ(third.use_count(), 3U);
  second.reset();
  third.reset();
  EXPECT_EQ(first.use_count(), 1U);
  first.reset();

  EXPECT_EQ(destructions.load(), 1U);
}

TEST(IntrusivePtrTest, WeakLockCannotReviveDestroyedTarget) {
  std::atomic<std::size_t> destructions{0};
  WeakIntrusivePtr<Tracked> weak;

  {
    const auto strong = make_intrusive<Tracked>(&destructions);
    weak = WeakIntrusivePtr<Tracked>{strong};
    const auto locked = weak.lock();
    ASSERT_TRUE(locked);
    EXPECT_EQ(locked.use_count(), 2U);
  }

  EXPECT_EQ(destructions.load(), 1U);
  EXPECT_TRUE(weak.expired());
  EXPECT_FALSE(weak.lock());
}

TEST(IntrusivePtrTest, CopiesAndMovesWeakReferences) {
  std::atomic<std::size_t> destructions{0};
  const auto strong = make_intrusive<Tracked>(&destructions);
  WeakIntrusivePtr<Tracked> first{strong};
  WeakIntrusivePtr<Tracked> second{first};
  WeakIntrusivePtr<Tracked> third{std::move(second)};

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(second.expired());
  const auto locked = third.lock();
  ASSERT_TRUE(locked);
  EXPECT_EQ(locked.use_count(), 2U);
  first.reset();
  third.reset();
}

TEST(IntrusivePtrTest, HandlesConcurrentStrongReferenceTraffic) {
  std::atomic<std::size_t> destructions{0};
  const auto target = make_intrusive<Tracked>(&destructions);

  constexpr std::size_t thread_count = 8;
  constexpr std::size_t iterations = 10000;
  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (std::size_t index = 0; index < thread_count; ++index) {
    threads.emplace_back([target] {
      for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
        auto copy = target;
        auto moved = std::move(copy);
        EXPECT_TRUE(moved);
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(target.use_count(), 1U);
  EXPECT_EQ(destructions.load(), 0U);
}

TEST(IntrusivePtrTest, ReleasesBaseWhenConstructionThrows) {
  EXPECT_THROW(static_cast<void>(make_intrusive<ThrowingTracked>()),
               std::runtime_error);
}

static_assert(!std::is_copy_constructible_v<IntrusivePtr<Tracked>> ||
              std::is_nothrow_copy_constructible_v<IntrusivePtr<Tracked>>);
static_assert(std::is_nothrow_move_constructible_v<IntrusivePtr<Tracked>>);

}  // namespace
}  // namespace gradyx
