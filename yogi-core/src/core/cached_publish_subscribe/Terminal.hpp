#ifndef CHIRP_CORE_CACHED_PUBLISH_SUBSCRIBE_TERMINAL_HPP
#define CHIRP_CORE_CACHED_PUBLISH_SUBSCRIBE_TERMINAL_HPP

#include "../../config.h"
#include "../../base/Buffer.hpp"
#include "../common/PublishSubscribeTerminalBaseT.hpp"
#include "logic_types.hpp"


namespace chirp {
namespace core {

class Leaf;

namespace cached_publish_subscribe {

template <typename TTypes>
class LeafLogic;

/***************************************************************************//**
 * Implements a cached publish-subscribe terminal
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public common::PublishSubscribeTerminalBaseT<TTypes, true>
{
    typedef common::PublishSubscribeTerminalBaseT<TTypes, true> super;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : super(leaf, identifier)
    {
    }

    std::pair<bool, std::size_t> get_cache(
        boost::asio::mutable_buffers_1 buffer)
    {
        auto& leafLogic = static_cast<typename TTypes::leaf_logic_type&>(
            static_cast<Leaf&>(super::leaf()));

        auto cache = leafLogic.get_cache(
            static_cast<typename TTypes::terminal_type&>(*this));
        if (!cache.second) {
            // not initialised
            return std::make_pair(false, 0);
        }

        auto n = boost::asio::buffer_copy(buffer, boost::asio::buffer(
            cache.first.data(), cache.first.size()));
        if (boost::asio::buffer_size(buffer) < cache.first.size()) {
            throw api::ExceptionT<CHIRP_ERR_BUFFER_TOO_SMALL>{};
        }

        return std::make_pair(true, n);
    }
};

} // namespace cached_publish_subscribe
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_CACHED_PUBLISH_SUBSCRIBE_TERMINAL_HPP
