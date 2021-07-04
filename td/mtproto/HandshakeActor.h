//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/mtproto/Handshake.h"
#include "td/mtproto/HandshakeConnection.h"
#include "td/mtproto/RawConnection.h"

#include "td/actor/actor.h"
#include "td/actor/PromiseFuture.h"

#include "td/utils/Status.h"

namespace td {
namespace mtproto {

// Has Raw connection. Generates new auth key. And returns it and raw_connection. Or error...
class HandshakeActor final : public Actor {
 public:
  HandshakeActor(unique_ptr<AuthKeyHandshake> handshake, unique_ptr<RawConnection> raw_connection,
                 unique_ptr<AuthKeyHandshakeContext> context, double timeout,
                 Promise<unique_ptr<RawConnection>> raw_connection_promise,
                 Promise<unique_ptr<AuthKeyHandshake>> handshake_promise);
  void close();

 private:
  unique_ptr<AuthKeyHandshake> handshake_;
  unique_ptr<HandshakeConnection> connection_;
  double timeout_;

  Promise<unique_ptr<RawConnection>> raw_connection_promise_;
  Promise<unique_ptr<AuthKeyHandshake>> handshake_promise_;

  void start_up() final;
  void tear_down() final {
    finish(Status::OK());
  }
  void hangup() final {
    finish(Status::Error(1, "Canceled"));
    stop();
  }
  void timeout_expired() final {
    finish(Status::Error("Timeout expired"));
    stop();
  }
  void loop() final;

  void finish(Status status) {
    // NB: order may be important for parent
    return_connection(std::move(status));
    return_handshake();
  }

  void return_connection(Status status);
  void return_handshake();
};

}  // namespace mtproto
}  // namespace td
