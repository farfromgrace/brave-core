/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_federated/data_store_service.h"

#include "base/memory/weak_ptr.h"
#include "base/task/thread_pool.h"
#include "base/threading/sequence_bound.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "brave/components/brave_federated/data_stores/ad_notification_timing_data_store.h"

namespace {
constexpr char kAdNotificationTaskName[] =
    "ad_notification_timing_federated_task";
constexpr int kAdNotificationTaskId = 0;
constexpr int kMaxNumberOfRecords = 50;
constexpr int kMaxRetentionDays = 30;
}  // namespace

namespace brave {

namespace federated {

DataStoreService::DataStoreService(const base::FilePath& database_path)
    : db_path_(database_path),
      ad_notification_timing_data_store_(
          base::ThreadPool::CreateSequencedTaskRunner(
              {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
               base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN}),
          db_path_),
      weak_factory_(this) {}

DataStoreService::~DataStoreService() {
  EnforceRetentionPolicies();
}

void DataStoreService::OnInitComplete(bool success) {
  if (success) {
    EnforceRetentionPolicies();
  }
}

void DataStoreService::Init() {
  ad_notification_timing_data_store_
      .AsyncCall(&AdNotificationTimingDataStore::Init)
      .WithArgs(kAdNotificationTaskId, kAdNotificationTaskName,
                kMaxNumberOfRecords, kMaxRetentionDays)
      .Then(base::BindOnce(&DataStoreService::OnInitComplete,
                           weak_factory_.GetWeakPtr()));
}

base::SequenceBound<AdNotificationTimingDataStore>*
DataStoreService::getAdNotificationTimingDataStore() {
  return &ad_notification_timing_data_store_;
}

bool DataStoreService::DeleteDatabase() {
  return sql::Database::Delete(db_path_);
}

void DataStoreService::EnforceRetentionPolicies() {
  ad_notification_timing_data_store_.AsyncCall(
      &AdNotificationTimingDataStore::EnforceRetentionPolicy);
}

}  // namespace federated

}  // namespace brave
