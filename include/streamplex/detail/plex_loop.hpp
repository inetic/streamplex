#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <queue>

#include "namespaces.hpp"

namespace streamplex {

struct connect_entry {
    using handler_t = std::function<void(const sys::error_code&)>;

    handler_t handler;
    uint8_t data;

    connect_entry(handler_t h)
        : handler(std::move(h))
    {}

    // Returns:
    // first:  Whether it's done.
    // second: The number of bytes added to `target`.
    std::pair<bool, size_t>
    fill_buffer(size_t capacity_left, std::vector<asio::const_buffer>& target)
    {
        size_t size = sizeof(data);
        // This func is called with at capacity_left > 0, and we only need one
        // byte.
        assert(capacity_left >= size);
        //target.push_back(asio::const_buffer(data&, size));
        return std::make_pair(true, size);
    }
};

template<class Stream>
class plex_loop : public std::enable_shared_from_this<plex_loop<Stream>> {
public:
    plex_loop(Stream stream);

    asio::io_service& get_io_service();

    template<class OnAccept>
    void set_on_accept(OnAccept&&);

    template<class OnConnect>
    void set_on_connect(OnConnect&&);

    void mark_stopped();

private:
    void start_transmit_loop();

    size_t prepare_payload( const size_t max_size
                          , std::vector<asio::const_buffer>& out_buffs);

private:
    Stream _stream;
    std::vector<asio::const_buffer> _tx_buffers;
    bool _is_stopped = false;
    std::queue<connect_entry> _tx_entry_queue;
};

template<class Stream>
inline
plex_loop<Stream>::plex_loop(Stream stream)
    : _stream(std::move(stream))
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
    connect_entry ce(std::forward<OnConnect>(h));
    _tx_entry_queue.push(std::move(ce));
    start_transmit_loop();
}

template<class Stream>
inline
void
plex_loop<Stream>::start_transmit_loop()
{
    if (_tx_buffers.size()) return;

    const size_t max_size = 65536;
    const size_t size = prepare_payload(max_size, _tx_buffers);

    if (size == 0) return;

    asio::async_write(_stream, _tx_buffers
                     , [this, self = plex_loop<Stream>::shared_from_this()]
                       (sys::error_code ec, size_t)
                       {
                           if (!ec && _is_stopped) {
                               ec = asio::error::operation_aborted;
                           }

                           if (ec) return;

                           _tx_buffers.resize(0);
                           start_transmit_loop();
                       });
}

template<class Stream>
inline
size_t
plex_loop<Stream>::prepare_payload( const size_t max_size
                                  , std::vector<asio::const_buffer>& out_buffs)
{
    size_t cur_size = 0;

    for (size_t count = _tx_entry_queue.size(); count; --count) {
        auto e = std::move(_tx_entry_queue.front());
        _tx_entry_queue.pop();

        bool   is_done;
        size_t added;
        size_t remaining = max_size - cur_size;

        std::tie(is_done, added) = e.fill_buffer(remaining, out_buffs);

        cur_size += added;

        if (!is_done) {
            _tx_entry_queue.push(std::move(e));
        }

        if (cur_size >= max_size) {
            break;
        }
    }

    return cur_size;
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
void
plex_loop<Stream>::mark_stopped()
{
    _is_stopped = true;
}

} // streamplex namespace
