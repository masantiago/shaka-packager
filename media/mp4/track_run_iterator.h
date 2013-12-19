// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_MP4_TRACK_RUN_ITERATOR_H_
#define MEDIA_MP4_TRACK_RUN_ITERATOR_H_

#include <vector>

#include "base/memory/scoped_ptr.h"
#include "media/mp4/box_definitions.h"
#include "media/mp4/cenc.h"

namespace media {

class DecryptConfig;

namespace mp4 {

struct SampleInfo;
struct TrackRunInfo;

class TrackRunIterator {
 public:
  // Create a new TrackRunIterator. A reference to |moov| will be retained for
  // the lifetime of this object.
  explicit TrackRunIterator(const Movie* moov);
  ~TrackRunIterator();

  // For non-fragmented mp4, moov contains all the chunks information; This
  // function sets up the iterator to handle all the chunks.
  // For fragmented mp4, chunk and sample information are generally contained
  // in moof. This function is a no-op in this case. Init(moof) will be called
  // later after parsing moof.
  bool Init();

  // Sets up the iterator to handle all the runs from the current fragment.
  bool Init(const MovieFragment& moof);

  // Returns true if the properties of the current run or sample are valid.
  bool IsRunValid() const;
  bool IsSampleValid() const;

  // Advance the properties to refer to the next run or sample. Requires that
  // the current sample be valid.
  void AdvanceRun();
  void AdvanceSample();

  // Returns true if this track run has auxiliary information and has not yet
  // been cached. Only valid if IsRunValid().
  bool AuxInfoNeedsToBeCached();

  // Caches the CENC data from the given buffer. |buf| must be a buffer starting
  // at the offset given by cenc_offset(), with a |size| of at least
  // cenc_size(). Returns true on success, false on error.
  bool CacheAuxInfo(const uint8* buf, int size);

  // Returns the maximum buffer location at which no data earlier in the stream
  // will be required in order to read the current or any subsequent sample. You
  // may clear all data up to this offset before reading the current sample
  // safely. Result is in the same units as offset() (for Media Source this is
  // in bytes past the the head of the MOOF box).
  int64 GetMaxClearOffset();

  // Property of the current run. Only valid if IsRunValid().
  uint32 track_id() const;
  int64 aux_info_offset() const;
  int aux_info_size() const;
  bool is_encrypted() const;
  bool is_audio() const;
  // Only one is valid, based on the value of is_audio().
  const AudioSampleEntry& audio_description() const;
  const VideoSampleEntry& video_description() const;

  // Properties of the current sample. Only valid if IsSampleValid().
  int64 sample_offset() const;
  int sample_size() const;
  int64 dts() const;
  int64 cts() const;
  int64 duration() const;
  bool is_keyframe() const;

  // Only call when is_encrypted() is true and AuxInfoNeedsToBeCached() is
  // false. Result is owned by caller.
  scoped_ptr<DecryptConfig> GetDecryptConfig();

 private:
  void ResetRun();
  const TrackEncryption& track_encryption() const;

  const Movie* moov_;

  std::vector<TrackRunInfo> runs_;
  std::vector<TrackRunInfo>::const_iterator run_itr_;
  std::vector<SampleInfo>::const_iterator sample_itr_;

  std::vector<FrameCENCInfo> cenc_info_;

  int64 sample_dts_;
  int64 sample_offset_;

  DISALLOW_COPY_AND_ASSIGN(TrackRunIterator);
};

}  // namespace mp4
}  // namespace media

#endif  // MEDIA_MP4_TRACK_RUN_ITERATOR_H_