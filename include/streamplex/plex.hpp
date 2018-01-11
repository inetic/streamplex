#pragma once

#include <streamplex/detail/namespaces.hpp>
#include <streamplex/detail/async_result.hpp>
#include <streamplex/detail/plex_loop.hpp>
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
    std::shared_ptr<plex_loop<Stream>> _loop;
};

template<class Stream>
plex<Stream>::plex(Stream&& stream)
    : _loop(std::make_shared<plex_loop<Stream>>(std::move(stream)))
{
}

template<class Stream>
asio::io_service& plex<Stream>::get_io_service()
{
    return _loop->get_io_service();
}

template<class Stream>
template<class Token>
typename detail::result_t<Token, socket<Stream>>::type
plex<Stream>::async_accept(Token&& token)
{
    detail::handler_t<Token, socket_t> handler(std::forward<Token>(token));
    detail::result_t<Token, socket_t> result(handler);

    auto h = [ w = asio::io_service::work(get_io_service())
             , h = std::move(handler)]
             (const sys::error_code& ec) mutable
             {
                 h(ec, socket_t());
             };

    _loop->set_on_accept(std::move(h));

    return result.get();
}

template<class Stream>
template<class Token>
typename detail::result_t<Token, socket<Stream>>::type
plex<Stream>::async_connect(Token&& token)
{
    detail::handler_t<Token, socket_t> handler(std::forward<Token>(token));
    detail::result_t<Token, socket_t> result(handler);

    auto h = [ w = asio::io_service::work(get_io_service())
             , h = std::move(handler)]
             (const sys::error_code& ec) mutable
             {
                 h(ec, socket_t());
             };

    _loop->set_on_connect(std::move(h));

    return result.get();
}

} // streamplex namespace
