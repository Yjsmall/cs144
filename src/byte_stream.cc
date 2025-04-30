#include "byte_stream.hh"
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), cur_num_( 0 ), write_cnt_( 0 ), read_cnt_( 0 )
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
  if (is_closed() || data.empty()) {
    return;
  }

  const auto available_num = available_capacity();

  if ( available_num > 0 ) {
    std::cout << "There is no space to write.\n";
    return;
  }
  const auto in_datas = data.substr( available_num );
  buffer_.push( in_datas  );

  write_cnt_ += in_datas.size();
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
  return write_cnt_;
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
  auto out_num = cur_num_;
  if (out_num > len) {
    out_num = len;
  }

  while (out_num != 0) {
    auto& first = buffer_.front();
    if (out_num >= first.size()) {
      buffer_.pop();
      out_num -= first.size();
      read_cnt_ -= first.size();
    } else {
      first = first.substr( out_num );
      out_num = 0;
      cur_num_ -= out_num;
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
  return read_cnt_;
}

