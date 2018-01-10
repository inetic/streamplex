#pragma once

#include <streamplex/detail/namespaces.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/system/error_code.hpp>

namespace streamplex {
namespace detail {

template<class Token, class Ret>
using handler_t
    = typename asio::handler_type<Token, void(sys::error_code, Ret)>::type;

template<class Token, class Ret>
using result_t
    = typename asio::async_result<handler_t<Token, Ret>>;

} // detail namespace
} // streamplex namespace
