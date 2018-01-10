#pragma once

#include <boost/asio/io_service.hpp>

namespace streamplex {

template<class Stream>
class socket {
public:
    asio::io_service& get_io_service();

    template< class MutableBufferSequence
            , class Token >
    detail::result_t<Token, size_t>
    async_read_some(const MutableBufferSequence&, Token&&);

    template< class ConstBufferSequence
            , class Token >
    detail::result_t<Token, size_t>
    async_write_some(const ConstBufferSequence&, Token&&);
};

} // streamplex namespace
