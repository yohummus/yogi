#include "node.hpp"
#include "internal/async.hpp"

#include <yogi_core.h>


namespace yogi {
namespace {

std::vector<char>::iterator extract_terminal_info(terminal_info* info, std::vector<char>::iterator bufferIt)
{
    info->type = static_cast<terminal_type>(*bufferIt);
    bufferIt += 1;
    info->signature = Signature(*reinterpret_cast<unsigned*>(&*bufferIt));
    bufferIt += 4;
    info->name = &*bufferIt;
    bufferIt += info->name.size() + 1;
    return bufferIt;
}

} // anonymous namespace

Node::Node(Scheduler& scheduler)
: Endpoint(YOGI_CreateNode, scheduler)
, m_knownTerminalsBuffer(1024)
{
}

Node::~Node()
{
    this->_destroy();
}

const std::string& Node::class_name() const
{
    static std::string s = "Node";
    return s;
}

std::vector<terminal_info> Node::get_known_terminals()
{
    std::vector<char> buffer(1024);
    unsigned numTerminals;
    int res;
    while (true) {
        res = YOGI_GetKnownTerminals(this->handle(), buffer.data(), static_cast<unsigned>(buffer.size()), &numTerminals);
        if (res == YOGI_ERR_BUFFER_TOO_SMALL) {
            buffer.resize(buffer.size() * 2);
            continue;
        }

        internal::throw_on_failure(res);
        break;
    }

    std::vector<terminal_info> terminals(numTerminals);
    auto it = buffer.begin();
    for (unsigned i = 0; i < numTerminals; ++i) {

        it = extract_terminal_info(&terminals[i], it);
    }

    return terminals;
}

void Node::async_await_known_terminals_change(std::function<void (const Result&, terminal_info&&, change_type)> completionHandler)
{
    internal::async_call([=](const Result& res) {
        terminal_info info;
        change_type   change = REMOVED;
        if (res) {
            auto it = this->m_knownTerminalsBuffer.begin();
            change = *it ? ADDED : REMOVED;
            extract_terminal_info(&info, it + 1);
        }

        completionHandler(res, std::move(info), change);
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncAwaitKnownTerminalsChange(this->handle(), this->m_knownTerminalsBuffer.data(),
            static_cast<unsigned>(this->m_knownTerminalsBuffer.size()), fn, userArg);
    });
}

void Node::cancel_await_known_terminals_change()
{
    int res = YOGI_CancelAwaitKnownTerminalsChange(this->handle());
    internal::throw_on_failure(res);
}

} // namespace yogi
