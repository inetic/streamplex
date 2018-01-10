#pragma once

#include <streamplex/detail/namespaces.hpp>
#include <streamplex/detail/async_result.hpp>
#include <streamplex/socket.hpp>

#include <boost/asio/io_service.hpp>

namespace streamplex {

template<class Stream>
class plex {
    using socket_t = socket<Stream>;

public:
    plex(Stream&& stream);

    asio::io_service& get_io_service();

    template<class Token>
    typename detail::result_t<Token, socket_t>::type
    async_accept(Token&&);

    template<class Token>
    typename detail::result_t<Token, socket_t>::type
    async_connect(Token&&);

private:
    Stream _stream;
};

template<class Stream>
plex<Stream>::plex(Stream&& stream)
    : _stream(std::move(stream))
{}

template<class Stream>
asio::io_service& plex<Stream>::get_io_service()
{
    return _stream.get_io_service();
}

template<class Stream>
template<class Token>
typename detail::result_t<Token, socket<Stream>>::type
plex<Stream>::async_accept(Token&& token)
{
    detail::handler_t<Token, socket_t> handler(std::forward<Token>(token));
    detail::result_t<Token, socket_t> result(handler);

    get_io_service().post([h = std::move(handler)] () mutable {
            h(sys::error_code(), socket<Stream>());
        });

    return result.get();
}

template<class Stream>
template<class Token>
typename detail::result_t<Token, socket<Stream>>::type
plex<Stream>::async_connect(Token&& token)
{
    detail::handler_t<Token, socket_t> handler(std::forward<Token>(token));
    detail::result_t<Token, socket_t> result(handler);

    get_io_service().post([h = std::move(handler)] () mutable {
            h(sys::error_code(), socket<Stream>());
        });

    return result.get();
}

} // streamplex namespace
