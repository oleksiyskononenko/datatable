//------------------------------------------------------------------------------
// Copyright 2018 H2O.ai
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//------------------------------------------------------------------------------
#ifdef DTTEST
#include <atomic>       // std::atomic
#include <cmath>        // std::pow, std::abs
#include "parallel/api.h"
#include "parallel/atomic.h"
#include "utils/exceptions.h"
#include "ztest.h"
namespace dttest {


template <typename T>
static void test_atomic_impl() {
  size_t n = dt::get_num_threads();
  std::atomic<int> barrier { static_cast<int>(n) };

  dt::atomic<T> x { T(0.0) };
  dt::atomic<T> y { T(1.0) };
  dt::atomic<T> z { T(1.3e20) };

  dt::run_once_per_thread(
    [&](size_t i) {
      barrier--;
      while (barrier.load());

      x += static_cast<T>(i);
      y *= static_cast<T>(2.0);
      z /= static_cast<T>(7.0);
      x -= static_cast<T>(1.0);
    });

  T x_exp = static_cast<T>(n * (n - 1) / 2 - n);
  T y_exp = static_cast<T>(std::pow(2.0, n));
  T z_exp = static_cast<T>(1.3e20 / std::pow(7.0, n));
  T x_act = x.load();
  T y_act = y.load();
  T z_act = z.load();

  T eps = static_cast<T>(sizeof(T) == 4? 1e-6 : 1e-10);
  if (std::abs(x_act/x_exp - 1) > eps) {
    throw AssertionError() << "Invalid x = " << x_act << " after " << n
        << " atomic operations, expected = " << x_exp;
  }
  if (std::abs(y_act/y_exp - 1) > eps) {
    throw AssertionError() << "Invalid y = " << y_act << " after " << n
        << " atomic operations, expected = " << y_exp;
  }
  if (std::abs(z_act/z_exp - 1) > eps) {
    throw AssertionError() << "Invalid z = " << z_act << " after " << n
        << " atomic operations, expected = " << z_exp;
  }
}


void test_atomic() {
  test_atomic_impl<float>();
  test_atomic_impl<double>();
}



} // namespace dt
#endif
