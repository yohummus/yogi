/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
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

#include "../../common.h"
#include "../../../src/objects/web/detail/worker_pool.h"
using namespace objects;
using namespace objects::web::detail;

class WorkerPoolTest : public TestFixture {
 protected:
  ContextPtr ctx_1_ = std::make_shared<Context>();
  ContextPtr ctx_2_ = std::make_shared<Context>();
  ContextPtr ctx_3_ = std::make_shared<Context>();
  WorkerPool pool_{ctx_1_};
};

TEST_F(WorkerPoolTest, AddSameWorkerTwice) {
  EXPECT_NO_THROW(pool_.AddWorker(ctx_1_));
  EXPECT_THROW_ERROR(pool_.AddWorker(ctx_1_), YOGI_ERR_WORKER_ALREADY_ADDED);

  EXPECT_NO_THROW(pool_.AddWorker(ctx_2_));
  EXPECT_THROW_ERROR(pool_.AddWorker(ctx_2_), YOGI_ERR_WORKER_ALREADY_ADDED);
}

TEST_F(WorkerPoolTest, Fallback) {
  auto worker = pool_.AcquireWorker();
  EXPECT_EQ(worker.Context().lock(), ctx_1_);
  EXPECT_TRUE(worker.IsFallback());

  pool_.AddWorker(ctx_2_);
  worker = pool_.AcquireWorker();
  EXPECT_EQ(worker.Context().lock(), ctx_2_);
  EXPECT_FALSE(worker.IsFallback());
  worker = pool_.AcquireWorker();
  EXPECT_EQ(worker.Context().lock(), ctx_2_);
  EXPECT_FALSE(worker.IsFallback());
  worker = pool_.AcquireWorker();
  EXPECT_EQ(worker.Context().lock(), ctx_2_);
  EXPECT_FALSE(worker.IsFallback());

  pool_.AddWorker(ctx_1_);
  worker = pool_.AcquireWorker();
  EXPECT_EQ(worker.Context().lock(), ctx_1_);
  EXPECT_TRUE(worker.IsFallback());
}

TEST_F(WorkerPoolTest, TaskDistribution) {
  pool_.AddWorker(ctx_2_);
  pool_.AddWorker(ctx_3_);

  auto worker_1 = std::make_unique<Worker>(pool_.AcquireWorker());
  auto worker_2 = std::make_unique<Worker>(pool_.AcquireWorker());
  EXPECT_NE(worker_1->Context().lock(), ctx_1_);
  EXPECT_NE(worker_2->Context().lock(), ctx_1_);
  EXPECT_NE(worker_2->Context().lock(), worker_1->Context().lock());

  auto worker_3 = std::make_unique<Worker>(pool_.AcquireWorker());
  EXPECT_NE(worker_3->Context().lock(), ctx_1_);

  worker_1.reset();
  worker_2.reset();
  auto worker_4 = std::make_unique<Worker>(pool_.AcquireWorker());
  EXPECT_NE(worker_4->Context().lock(), ctx_1_);
  EXPECT_NE(worker_4->Context().lock(), worker_3->Context().lock());
}
