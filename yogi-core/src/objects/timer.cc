/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "timer.h"
#include "../api/errors.h"

namespace objects {

Timer::Timer(ContextPtr context)
    : context_(context), timer_(context->IoContext()) {}

void Timer::StartAsync(std::chrono::nanoseconds timeout, HandlerFn fn) {
  timer_.expires_after(timeout);
  timer_.async_wait([=](const auto& ec) {
    YOGI_ASSERT(!ec || ec == boost::asio::error::operation_aborted);
    fn(ec ? api::Error(YOGI_ERR_CANCELED) : api::kSuccess);
  });
}

bool Timer::Cancel() {
  return timer_.cancel() != 0;
}

}  // namespace objects
