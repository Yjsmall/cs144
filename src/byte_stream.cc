#include "byte_stream.hh"
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : buffer_(), capacity_( capacity ), cur_num_( 0 ), pushed_num( 0 ), popped_num_( 0 )
{}

/*
 * ********************************************************
 * *                                                      *
 * *                    Writer                            *
 * *                                                      *
 * ********************************************************
 */
void Writer::push( string data )
{
  if ( is_closed() || data.empty() ) {
    return;
  }

  const auto free_space = available_capacity();

  if ( free_space <= 0 ) {
    std::cout << "There is no space to write.\n";
    return;
  }

  // Take the minimum of available space and the string length
  const auto in_size = data.size() > free_space ? free_space : data.size();
  const auto in_datas = data.substr( 0, in_size );
  buffer_.push( in_datas );

  pushed_num += in_datas.size();
  cur_num_ += in_datas.size();
}

void Writer::close()
{
  closed_ = true;
}

bool Writer::is_closed() const
{
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - cur_num_;
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_num;
}

/*
 * ********************************************************
 * *                                                      *
 * *                    Reader                            *
 * *                                                      *
 * ********************************************************
 */

string_view Reader::peek() const
{
  return buffer_.front();
}

void Reader::pop( uint64_t len )
{
  auto out_num = cur_num_ > len ? len : cur_num_;

  while ( out_num != 0 ) {
    if ( auto& first = buffer_.front(); out_num >= first.size() ) {
      cur_num_ -= first.size();
      out_num -= first.size();
      popped_num_ += first.size();
      buffer_.pop();
    } else {
      first = first.substr( out_num );
      cur_num_ -= out_num;
      popped_num_ += out_num;
      out_num = 0;
    }
  }
}

bool Reader::is_finished() const
{
  return cur_num_ == 0 && closed_;
}

uint64_t Reader::bytes_buffered() const
{
  return cur_num_;
}

uint64_t Reader::bytes_popped() const
{
  return popped_num_;
}
