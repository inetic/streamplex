#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <queue>

namespace streamplex {

struct connect_entry {
    using handler_t = std::function<void(const sys::error_code&)>;

    handler_t handler;
    std::vector<uint8_t> message;
    asio::const_buffer buffer;

    connect_entry(handler_t h, std::vector<uint8_t> m)
        : handler(std::move(h))
        , message(std::move(m))
        , buffer(message.data(), message.size())
    {}
};

struct plex_loop_state {
    bool was_destroyed = false;
    std::queue<connect_entry> tx_queue;

    void on_write(const sys::error_code&)
    {
        // TODO: Clear depleted tx_queue entries and
        // move partially sent entries to the back of
        // the queue.
    }
};

template<class Stream>
class plex_loop {
public:
    plex_loop(Stream stream);

    asio::io_service& get_io_service();

    template<class OnAccept>
    void set_on_accept(OnAccept&&);

    template<class OnConnect>
    void set_on_connect(OnConnect&&);

    ~plex_loop();

private:
    void start_transmit_loop();

private:
    Stream _stream;
    std::shared_ptr<plex_loop_state> _state;
    std::vector<asio::const_buffer> _tx_buffers;
};

template<class Stream>
inline
plex_loop<Stream>::plex_loop(Stream stream)
    : _stream(std::move(stream))
    , _state(std::make_shared<plex_loop_state>())
{}

template<class Stream>
template<class OnAccept>
inline
void plex_loop<Stream>::set_on_accept(OnAccept&&)
{
    /* TODO */
}

template<class Stream>
template<class OnConnect>
inline
void plex_loop<Stream>::set_on_connect(OnConnect&& h)
{
    connect_entry ce(h, std::vector<uint8_t>({1}) /* TODO */);
    _state->tx_queue.push(std::move(ce));
    start_transmit_loop();
}

template<class Stream>
inline
void
plex_loop<Stream>::start_transmit_loop()
{
    _tx_buffers.resize(0);

    // TODO: Fill _tx_buffers with data.

    asio::async_write(_stream, _tx_buffers
                     , [this, s = _state](sys::error_code ec, size_t)
                       {
                           if (!ec && s->was_destroyed) {
                               ec = asio::error::operation_aborted;
                           }

                           s->on_write(ec);

                           if (ec) return;

                           start_transmit_loop();
                       });
}

template<class Stream>
inline
asio::io_service&
plex_loop<Stream>::get_io_service()
{
    return _stream.get_io_service();
}

template<class Stream>
inline
plex_loop<Stream>::~plex_loop()
{
    _state->was_destroyed = true;
}

} // streamplex namespace
