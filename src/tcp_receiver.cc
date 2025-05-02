#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    reader().set_error();
    return;
  }

  const auto& w = this->writer();
  if ( w.has_error() ) {
    return;
  }

  // it must start with the confirmation of the syn number
  if ( !isn_.has_value() ) {
    if ( !message.SYN ) {
      return;
    }
    isn_ = message.seqno;
  }

  // Don't be afraid that the indexing position is not sufficient(2^64 - 1 it's so big!!!)
  // The ByteStream is stored through the queue
  const uint64_t cp = w.bytes_pushed() + static_cast<uint64_t>( message.SYN );
  const uint64_t abs_seq = message.seqno.unwrap( isn_.value(), cp );
  const uint64_t stream_idx = abs_seq + static_cast<uint64_t>( message.SYN ) - 1;
  reassembler_.insert( stream_idx, std::move( message.payload ), message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  const auto& w = this->writer();
  const uint16_t wind_size
    = static_cast<uint16_t>( std::min( w.available_capacity(), static_cast<uint64_t>( UINT16_MAX ) ) );
  const bool reset = w.has_error();
  if ( isn_.has_value() ) {
    const uint64_t abs_seq = w.bytes_pushed() + static_cast<uint64_t>( w.is_closed() );
    Wrap32 ack_no = Wrap32::wrap( abs_seq, isn_.value() ) + 1;
    return TCPReceiverMessage( ack_no, wind_size, reset );
  }
  return TCPReceiverMessage { nullopt, wind_size, reset };
}
