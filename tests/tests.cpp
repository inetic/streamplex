#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "StreamPlex"
#include <boost/test/unit_test.hpp>

#include <streamplex.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>

#include <iostream>

namespace asio = streamplex::asio;
using namespace std;

BOOST_AUTO_TEST_CASE(first_test)
{
    namespace sp = streamplex;

    using stream_t = asio::ip::tcp::socket;
    using plex_t   = sp::plex<stream_t>;

    sp::asio::io_service ios;

    stream_t stream(ios);
    plex_t plex(move(stream));

    bool spawn_accept_finished  = false;
    bool spawn_connect_finished = false;

    asio::spawn(ios, [&](asio::yield_context yield) {
            auto socket = plex.async_accept(yield);

            spawn_accept_finished = true;
        });

    asio::spawn(ios, [&](asio::yield_context yield) {
            auto socket = plex.async_connect(yield);

            spawn_connect_finished = true;
        });

    ios.run();

    BOOST_CHECK(spawn_accept_finished);
    BOOST_CHECK(spawn_connect_finished);
}
