/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/remote_bitrate_estimator/inter_arrival.h"

#include <algorithm>
#include <cassert>

#include "webrtc/modules/interface/module_common_types.h"

namespace webrtc {

static const int kBurstDeltaThresholdMs  = 5;

InterArrival::InterArrival(uint32_t timestamp_group_length_ticks,
                           double timestamp_to_ms_coeff,
                           bool enable_burst_grouping)
    : kTimestampGroupLengthTicks(timestamp_group_length_ticks),
      current_timestamp_group_(),
      prev_timestamp_group_(),
      timestamp_to_ms_coeff_(timestamp_to_ms_coeff),
      burst_grouping_(enable_burst_grouping) {}

/*
2.1.2 预过滤（Pre-filtering）

预过滤的目的是处理由于通道中断造成的延迟瞬间变大的情况。在通道发生中断时，数据包会持续进入网络队列中，
而当通道恢复时，所有的数据包会在一个burst时间（5 ms）里面全部发送，而这些数据包可能原先包分布于多个数
据分组。而预过滤所要做的就是将这些在同一个burst时间里发送的数据包合为一个数据分组。

这里涉及到了WebRTC中关于数据传输的一个设计--PacedSender。Encoded数据完成RTP封装之后先是被保存在本地应
用的队列中，而不是直接发送到网络。此时可以将PacedSender视为一个数据发送的节拍器，它每隔一个burst时间
启动一次，启动之后会将队列中的RTP包全数发出。
数据包会在下面两种情况下被划分到一个数据分组：

在同一个burst时间区间内被发送的数据包序列；
一个数据包与相邻数据包的到达时间间隔小于一个burst时间，同时d(i) < 0，那么这个数据包将会被划到当前的分
组中。

链接：http://www.jianshu.com/p/9061b6d0a901
*/
bool InterArrival::ComputeDeltas(uint32_t timestamp,
                                 int64_t arrival_time_ms,
                                 size_t packet_size,
                                 uint32_t* timestamp_delta,
                                 int64_t* arrival_time_delta_ms,
                                 int* packet_size_delta) {
  assert(timestamp_delta != NULL);
  assert(arrival_time_delta_ms != NULL);
  assert(packet_size_delta != NULL);
  bool calculated_deltas = false;
  if (current_timestamp_group_.IsFirstPacket()) {
    // We don't have enough data to update the filter, so we store it until we
    // have two frames of data to process.
    current_timestamp_group_.timestamp = timestamp;
    current_timestamp_group_.first_timestamp = timestamp;
  } else if (!PacketInOrder(timestamp)) {
    return false;
  } else if (NewTimestampGroup(arrival_time_ms, timestamp)) {
    // First packet of a later frame, the previous frame sample is ready.
    if (prev_timestamp_group_.complete_time_ms >= 0) {
      *timestamp_delta = current_timestamp_group_.timestamp -
                         prev_timestamp_group_.timestamp;
      *arrival_time_delta_ms = current_timestamp_group_.complete_time_ms -
                               prev_timestamp_group_.complete_time_ms;
      assert(*arrival_time_delta_ms >= 0);
      *packet_size_delta = static_cast<int>(current_timestamp_group_.size) -
          static_cast<int>(prev_timestamp_group_.size);
      calculated_deltas = true;
    }
    prev_timestamp_group_ = current_timestamp_group_;
    // The new timestamp is now the current frame.
    current_timestamp_group_.first_timestamp = timestamp;
    current_timestamp_group_.timestamp = timestamp;
    current_timestamp_group_.size = 0;
  }
  else {
    current_timestamp_group_.timestamp = LatestTimestamp(
        current_timestamp_group_.timestamp, timestamp);
  }
  // Accumulate the frame size.
  current_timestamp_group_.size += packet_size;
  current_timestamp_group_.complete_time_ms = arrival_time_ms;

  return calculated_deltas;
}

bool InterArrival::PacketInOrder(uint32_t timestamp) {
  if (current_timestamp_group_.IsFirstPacket()) {
    return true;
  } else {
    // Assume that a diff which is bigger than half the timestamp interval
    // (32 bits) must be due to reordering. This code is almost identical to
    // that in IsNewerTimestamp() in module_common_types.h.
    uint32_t timestamp_diff = timestamp -
        current_timestamp_group_.first_timestamp;
    return timestamp_diff < 0x80000000;
  }
}

// Assumes that |timestamp| is not reordered compared to
// |current_timestamp_group_|.
bool InterArrival::NewTimestampGroup(int64_t arrival_time_ms,
                                     uint32_t timestamp) const {
  if (current_timestamp_group_.IsFirstPacket()) {
    return false;
  } else if (BelongsToBurst(arrival_time_ms, timestamp)) {
    return false;
  } else {
    uint32_t timestamp_diff = timestamp -
        current_timestamp_group_.first_timestamp;
    return timestamp_diff > kTimestampGroupLengthTicks;
  }
}

bool InterArrival::BelongsToBurst(int64_t arrival_time_ms,
                                  uint32_t timestamp) const {
  if (!burst_grouping_) {
    return false;
  }
  assert(current_timestamp_group_.complete_time_ms >= 0);
  int64_t arrival_time_delta_ms = arrival_time_ms -
      current_timestamp_group_.complete_time_ms;
  uint32_t timestamp_diff = timestamp - current_timestamp_group_.timestamp;
  int64_t ts_delta_ms = timestamp_to_ms_coeff_ * timestamp_diff + 0.5;
  if (ts_delta_ms == 0)
    return true;
  int propagation_delta_ms = arrival_time_delta_ms - ts_delta_ms;
  return propagation_delta_ms < 0 &&
      arrival_time_delta_ms <= kBurstDeltaThresholdMs;
}
}  // namespace webrtc
