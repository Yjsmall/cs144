#include "reassembler.hh"
#include "debug.hh"
#include <algorithm>
#include <ranges>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );
  auto& writer = output_.writer();

  const std::pair<uint64_t, uint64_t> window( next_idx_, next_idx_ + writer.available_capacity() );
  std::pair<uint64_t, uint64_t> elem( first_index, first_index + data.size() );

  if ( is_last_substring ) {
    EOF_idx_ = elem.second;
  }

  // wind: [xxxx]
  // data:         [datas]
  if ( elem.first >= window.second ) {
    return;
  }

  // wind:[xxxxxx*????       [xxxxx*
  // data:    *xxxxx]                 *xxxx]
  elem.first = std::max( window.first, elem.first );
  elem.second = std::min( window.second, elem.second );

  // wind: [xxxxxxxxxx]
  // data:   [datas]
  if ( elem.first >= elem.second ) {
    if ( next_idx_ == EOF_idx_ ) {
      writer.close();
    }
    return;
  }

  const uint64_t len = elem.second - elem.first;
  reasseembler_vec_.emplace_back( data.substr( elem.first - first_index, len ), elem.first );

  merge_interval();
  if ( const auto it = reasseembler_vec_.begin(); it->start_ == next_idx_ ) {
    writer.push( it->data_ );
    next_idx_ = it->end_;
    reasseembler_vec_.erase( it );
  }

  if ( next_idx_ == EOF_idx_ ) {
    writer.close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t stored_num = 0;
  for ( const auto& val : reasseembler_vec_ ) {
    stored_num += val.data_.size();
  }
  return stored_num;
}

void Reassembler::merge_interval()
{
  std::ranges::sort( reasseembler_vec_,
                     []( const interval_& val1, const interval_& val2 ) { return val1.start_ < val2.start_; } );

  vector<interval_> merged;
  merged.push_back( reasseembler_vec_[0] );

  for ( size_t i = 1; i < reasseembler_vec_.size(); ++i ) {
    auto& back = merged.back();
    auto& val = reasseembler_vec_[i];
    if ( back.end_ >= val.start_ ) {
      // handle overlapping interval
      if ( val.end_ > back.end_ ) {
        const auto len = val.end_ - back.end_;
        back.data_ += val.data_.substr( val.data_.size() - len );
        back.end_ = back.start_ + back.data_.length();
      }
    } else {
      merged.push_back( val );
    }
  }

  reasseembler_vec_.swap( merged );
}
