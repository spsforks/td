//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2024
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/DialogId.h"
#include "td/telegram/files/FileSourceId.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"
#include "td/telegram/UserId.h"

#include "td/actor/actor.h"

#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/Promise.h"
#include "td/utils/Status.h"

namespace td {

class Td;

class StarManager final : public Actor {
 public:
  StarManager(Td *td, ActorShared<> parent);

  void get_star_payment_options(Promise<td_api::object_ptr<td_api::starPaymentOptions>> &&promise);

  void get_star_gift_payment_options(UserId user_id, Promise<td_api::object_ptr<td_api::starPaymentOptions>> &&promise);

  void get_star_transactions(td_api::object_ptr<td_api::MessageSender> owner_id, const string &offset, int32 limit,
                             td_api::object_ptr<td_api::StarTransactionDirection> &&direction,
                             Promise<td_api::object_ptr<td_api::starTransactions>> &&promise);

  void refund_star_payment(UserId user_id, const string &telegram_payment_charge_id, Promise<Unit> &&promise);

  void get_star_revenue_statistics(const td_api::object_ptr<td_api::MessageSender> &owner_id, bool is_dark,
                                   Promise<td_api::object_ptr<td_api::starRevenueStatistics>> &&promise);

  void get_star_withdrawal_url(const td_api::object_ptr<td_api::MessageSender> &owner_id, int64 star_count,
                               const string &password, Promise<string> &&promise);

  void get_star_ad_account_url(const td_api::object_ptr<td_api::MessageSender> &owner_id, Promise<string> &&promise);

  void reload_star_transaction(DialogId dialog_id, const string &transaction_id, bool is_refund,
                               Promise<Unit> &&promise);

  void on_update_stars_revenue_status(telegram_api::object_ptr<telegram_api::updateStarsRevenueStatus> &&update);

  FileSourceId get_star_transaction_file_source_id(DialogId dialog_id, const string &transaction_id, bool is_refund);

  static int64 get_star_count(int64 amount, bool allow_negative = false);

  static int32 get_months_by_star_count(int64 star_count);

 private:
  void tear_down() final;

  Status can_manage_stars(DialogId dialog_id, bool allow_self = false) const;

  void do_get_star_transactions(DialogId dialog_id, const string &offset, int32 limit,
                                td_api::object_ptr<td_api::StarTransactionDirection> &&direction,
                                Promise<td_api::object_ptr<td_api::starTransactions>> &&promise);

  void send_get_star_withdrawal_url_query(
      DialogId dialog_id, int64 star_count,
      telegram_api::object_ptr<telegram_api::InputCheckPasswordSRP> input_check_password, Promise<string> &&promise);

  Td *td_;
  ActorShared<> parent_;

  FlatHashMap<DialogId, FlatHashMap<string, FileSourceId>, DialogIdHash> star_transaction_file_source_ids_[2];
};

}  // namespace td
