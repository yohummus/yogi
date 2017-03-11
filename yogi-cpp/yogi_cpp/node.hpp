#ifndef YOGICPP_NODE_HPP
#define YOGICPP_NODE_HPP

#include "endpoint.hpp"
#include "types.hpp"
#include "result.hpp"

#include <vector>
#include <functional>


namespace yogi {

class Scheduler;

class Node : public Endpoint
{
private:
    std::vector<char> m_knownTerminalsBuffer;

public:
    Node(Scheduler& scheduler);
    virtual ~Node();

    virtual const std::string& class_name() const override;

    std::vector<terminal_info> get_known_terminals();
    void async_await_known_terminals_change(std::function<void (const Result&, terminal_info&&, change_type)> completionHandler);
    void cancel_await_known_terminals_change();
};

} // namespace yogi

#endif // YOGICPP_NODE_HPP
