
#include "Receiver.h"
#include "AlcPacket.h"
#include <iostream>
#include <string>
#include "spdlog/spdlog.h"


LibFlute::Receiver::Receiver ( const std::string& iface, const std::string& address,
    short port, uint64_t tsi, 
    boost::asio::io_service& io_service)
    : _socket(io_service)
    , _tsi(tsi)
{
    boost::asio::ip::udp::endpoint listen_endpoint(
        boost::asio::ip::address::from_string(iface), port);
    _socket.open(listen_endpoint.protocol());
    _socket.set_option(boost::asio::ip::multicast::enable_loopback(true));
    _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    _socket.bind(listen_endpoint);

    // Join the multicast group.
    _socket.set_option(
        boost::asio::ip::multicast::join_group(
          boost::asio::ip::address::from_string(address)));

    _socket.async_receive_from(
        boost::asio::buffer(_data, max_length), _sender_endpoint,
        boost::bind(&LibFlute::Receiver::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

LibFlute::Receiver::~Receiver()
{
  //spdlog::debug("Closing flute receiver for ALC session {}", _alc_session_id);
  //fcl::set_flute_session_state(_alc_session_id, fcl::SExiting);
}

auto LibFlute::Receiver::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd) -> void
{
  if (!error)
  {
    auto alc = LibFlute::AlcPacket(_data, bytes_recvd);

    if (alc.tsi() != _tsi) {
      return;
    }

    const std::lock_guard<std::mutex> lock(_files_mutex);

    if (alc.toi() == 0 && (!_fdt || _fdt->instance_id() != alc.fdt_instance_id())) {
      if (_files.find(alc.toi()) == _files.end()) {
        FileDeliveryTable::FileEntry fe{0, "", static_cast<uint32_t>(alc.fec_oti().transfer_length), "", "", 0, alc.fec_oti()};
        _files.emplace(alc.toi(), std::make_shared<LibFlute::File>(fe));
      }
    }

    if (_files.find(alc.toi()) != _files.end() && !_files[alc.toi()]->complete()) {
      auto encoding_symbols = LibFlute::EncodingSymbol::initialize_from_payload(
          _data + alc.header_length(), 
          bytes_recvd - alc.header_length(),
          _files[alc.toi()]->fec_oti());

      for (const auto& symbol : encoding_symbols) {
        _files[alc.toi()]->put_symbol(symbol);
      }

      auto file = _files[alc.toi()].get();
      if (_files[alc.toi()]->complete()) {
        for (auto it = _files.cbegin(); it != _files.cend();)
        {
          if (it->second.get() != file && it->second->meta().content_location == file->meta().content_location)
          {
            spdlog::debug("Replacing file with TOI {}", it->first);
            it = _files.erase(it);
          }
          else
          {
            ++it;
          }
        }

        spdlog::debug("File with TOI {} completed", alc.toi());
        if (alc.toi() == 0) { // parse complete FDT
          _fdt = std::make_unique<LibFlute::FileDeliveryTable>(
              alc.fdt_instance_id(), _files[alc.toi()]->buffer(), _files[alc.toi()]->length());

          _files.erase(alc.toi());
          for (const auto& file_entry : _fdt->file_entries()) {
            // automatically receive all files in the FDT
            if (_files.find(file_entry.toi) == _files.end()) {
              spdlog::debug("Starting reception for file with TOI {}", file_entry.toi);
              _files.emplace(file_entry.toi, std::make_shared<LibFlute::File>(file_entry));
            }
          }
        }
      }
    }

    _socket.async_receive_from(
        boost::asio::buffer(_data, max_length), _sender_endpoint,
        boost::bind(&LibFlute::Receiver::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

auto LibFlute::Receiver::file_list() -> std::vector<std::shared_ptr<LibFlute::File>>
{
  std::vector<std::shared_ptr<LibFlute::File>> files;
  for (auto& f : _files) {
    files.push_back(f.second);
  }
  return files;
}

auto LibFlute::Receiver::remove_expired_files(unsigned max_age) -> void
{
  const std::lock_guard<std::mutex> lock(_files_mutex);
  for (auto it = _files.cbegin(); it != _files.cend();)
  {
    auto age = time(nullptr) - it->second->received_at();
    if ( it->second->meta().content_location != "bootstrap.multipart"  && age > max_age) {
      it = _files.erase(it);
    } else {
      ++it;
    }
  }
}
