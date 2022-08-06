//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2022
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/BackgroundId.h"
#include "td/telegram/ChannelId.h"
#include "td/telegram/ChatId.h"
#include "td/telegram/files/FileId.h"
#include "td/telegram/files/FileSourceId.h"
#include "td/telegram/FullMessageId.h"
#include "td/telegram/PhotoSizeSource.h"
#include "td/telegram/SetWithPosition.h"
#include "td/telegram/td_api.h"
#include "td/telegram/UserId.h"

#include "td/actor/actor.h"

#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/logging.h"
#include "td/utils/Promise.h"
#include "td/utils/Slice.h"
#include "td/utils/Status.h"
#include "td/utils/Variant.h"
#include "td/utils/WaitFreeVector.h"

namespace td {

class Td;

extern int VERBOSITY_NAME(file_references);

class FileReferenceManager final : public Actor {
 public:
  FileReferenceManager() = default;
  FileReferenceManager(const FileReferenceManager &) = delete;
  FileReferenceManager &operator=(const FileReferenceManager &) = delete;
  FileReferenceManager(FileReferenceManager &&) = delete;
  FileReferenceManager &operator=(FileReferenceManager &&) = delete;
  ~FileReferenceManager() final;

  static bool is_file_reference_error(const Status &error);
  static size_t get_file_reference_error_pos(const Status &error);

  FileSourceId create_message_file_source(FullMessageId full_message_id);
  FileSourceId create_user_photo_file_source(UserId user_id, int64 photo_id);
  // file reference aren't used for chat/channel photo download and the photos can't be reused
  // FileSourceId create_chat_photo_file_source(ChatId chat_id);
  // FileSourceId create_channel_photo_file_source(ChannelId channel_id);
  // FileSourceId create_wallpapers_file_source();  old wallpapers can't be repaired
  FileSourceId create_web_page_file_source(string url);
  FileSourceId create_saved_animations_file_source();
  FileSourceId create_recent_stickers_file_source(bool is_attached);
  FileSourceId create_favorite_stickers_file_source();
  FileSourceId create_background_file_source(BackgroundId background_id, int64 access_hash);
  FileSourceId create_user_full_file_source(UserId user_id);
  FileSourceId create_chat_full_file_source(ChatId chat_id);
  FileSourceId create_channel_full_file_source(ChannelId channel_id);
  FileSourceId create_app_config_file_source();
  FileSourceId create_saved_ringtones_file_source();

  using NodeId = FileId;
  void repair_file_reference(NodeId node_id, Promise<> promise);

  void get_file_search_text(FileSourceId file_source_id, string unique_file_id, Promise<string> promise);

  td_api::object_ptr<td_api::message> get_message_object(FileSourceId file_source_id) const;

  static void reload_photo(PhotoSizeSource source, Promise<Unit> promise);

  bool add_file_source(NodeId node_id, FileSourceId file_source_id);

  vector<FileSourceId> get_some_file_sources(NodeId node_id);

  vector<FullMessageId> get_some_message_file_sources(NodeId node_id);

  bool remove_file_source(NodeId node_id, FileSourceId file_source_id);

  void merge(NodeId to_node_id, NodeId from_node_id);

  template <class StorerT>
  void store_file_source(FileSourceId file_source_id, StorerT &storer) const;

  template <class ParserT>
  FileSourceId parse_file_source(Td *td, ParserT &parser);

 private:
  struct Destination {
    NodeId node_id;
    int64 generation{0};

    Destination() = default;
    Destination(NodeId node_id, int64 generation) : node_id(node_id), generation(generation) {
    }
    bool is_empty() const {
      return node_id.empty();
    }
  };
  struct Query {
    std::vector<Promise<>> promises;
    int32 active_queries{0};
    Destination proxy;
    int64 generation{0};
  };

  struct Node {
    SetWithPosition<FileSourceId> file_source_ids;
    unique_ptr<Query> query;
    double last_successful_repair_time = -1e10;
  };

  struct FileSourceMessage {
    FullMessageId full_message_id;
  };
  struct FileSourceUserPhoto {
    int64 photo_id;
    UserId user_id;
  };
  struct FileSourceChatPhoto {
    ChatId chat_id;
  };
  struct FileSourceChannelPhoto {
    ChannelId channel_id;
  };
  struct FileSourceWallpapers {
    // empty
  };
  struct FileSourceWebPage {
    string url;
  };
  struct FileSourceSavedAnimations {
    // empty
  };
  struct FileSourceRecentStickers {
    bool is_attached;
  };
  struct FileSourceFavoriteStickers {
    // empty
  };
  struct FileSourceBackground {
    BackgroundId background_id;
    int64 access_hash;
  };
  struct FileSourceChatFull {
    ChatId chat_id;
  };
  struct FileSourceChannelFull {
    ChannelId channel_id;
  };
  struct FileSourceAppConfig {
    // empty
  };
  struct FileSourceSavedRingtones {
    // empty
  };
  struct FileSourceUserFull {
    UserId user_id;
  };

  // append only
  using FileSource =
      Variant<FileSourceMessage, FileSourceUserPhoto, FileSourceChatPhoto, FileSourceChannelPhoto, FileSourceWallpapers,
              FileSourceWebPage, FileSourceSavedAnimations, FileSourceRecentStickers, FileSourceFavoriteStickers,
              FileSourceBackground, FileSourceChatFull, FileSourceChannelFull, FileSourceAppConfig,
              FileSourceSavedRingtones, FileSourceUserFull>;
  WaitFreeVector<FileSource> file_sources_;

  int64 query_generation_{0};

  FlatHashMap<NodeId, Node, FileIdHash> nodes_;

  void run_node(NodeId node);
  void send_query(Destination dest, FileSourceId file_source_id);
  Destination on_query_result(Destination dest, FileSourceId file_source_id, Status status, int32 sub = 0);

  template <class T>
  FileSourceId add_file_source_id(T source, Slice source_str);

  FileSourceId get_current_file_source_id() const;
};

}  // namespace td
