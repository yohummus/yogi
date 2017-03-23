#ifndef YOGI_INTERNAL_PROTO_HPP
#define YOGI_INTERNAL_PROTO_HPP

#include "../result.hpp"
#include "../object.hpp"
#include "../types.hpp"
#include "async.hpp"
#include "utility.hpp"

#include <memory>
#include <vector>


namespace yogi {
namespace internal {

inline int invoke_publish_raw_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, const void* data, std::size_t size)
{
    if (size > MAX_MESSAGE_SIZE) {
        return YOGI_ERR_BUFFER_TOO_SMALL;
    }

    int res = apiFn(terminal->handle(), data, static_cast<unsigned>(size));
    return res;
}

inline bool try_publish_raw_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, const void* data, std::size_t size)
{
    int res = invoke_publish_raw_message(apiFn, terminal, data, size);
    return res == YOGI_OK;
}

inline void publish_raw_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, const void* data, std::size_t size)
{
    int res = invoke_publish_raw_message(apiFn, terminal, data, size);
    throw_on_failure(res);
}

template <typename Message>
inline int invoke_publish_proto_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, Message msg)
{
    auto size = static_cast<std::size_t>(msg.ByteSize());
    if (size > MAX_MESSAGE_SIZE) {
        return YOGI_ERR_BUFFER_TOO_SMALL;
    }

    unsigned char buffer[MAX_MESSAGE_SIZE]; // TODO: Potentially nasty
    msg.SerializeWithCachedSizesToArray(buffer);

    return invoke_publish_raw_message(apiFn, terminal, buffer, size);
}

template <typename Message>
inline bool try_publish_proto_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, Message msg)
{
    int res = invoke_publish_proto_message(apiFn, terminal, msg);
    return res == YOGI_OK;
}

template <typename Message>
inline void publish_proto_message(int (*apiFn)(void*, const void*, unsigned), const Object* terminal, Message msg)
{
    int res = invoke_publish_proto_message(apiFn, terminal, msg);
    throw_on_failure(res);
}

inline std::vector<char> get_cached_raw_message(int (*apiFn)(void*, void*, unsigned, unsigned*), const Object* terminal)
{
    unsigned char buffer[MAX_MESSAGE_SIZE]; // TODO: Potentially nasty

    unsigned bytesWritten;
    int res = apiFn(terminal->handle(), buffer, sizeof(buffer), &bytesWritten);
    throw_on_failure(res);

    std::vector<char> data(std::begin(buffer), std::begin(buffer) + bytesWritten);
    return data;
}

template <typename Message>
inline Message get_cached_proto_message(int (*apiFn)(void*, void*, unsigned, unsigned*), const Object* terminal)
{
    unsigned char buffer[MAX_MESSAGE_SIZE]; // TODO: Potentially nasty

    unsigned bytesWritten;
    int res = apiFn(terminal->handle(), buffer, sizeof(buffer), &bytesWritten);
    throw_on_failure(res);

    Message msg;
    msg.ParseFromArray(buffer, bytesWritten);
    return msg;
}

inline void async_receive_raw_message(int (*apiFn)(void*, void* buffer, unsigned, void (*)(int, unsigned, void*), void*),
    const Object* terminal, std::function<void (const Result&, std::vector<char>&&)> completionHandler)
{
    auto buffer = std::make_shared<std::vector<char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    async_call<unsigned>([=](const Result& result, unsigned bytesWritten) {
        completionHandler(result, std::vector<char>(buffer->begin(), buffer->begin() + bytesWritten));
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), buffer->data(), static_cast<unsigned>(buffer->size()), fn, userArg);
    });
}

template <typename Message>
inline void async_receive_proto_message(int (*apiFn)(void*, void* buffer, unsigned, void (*)(int, unsigned, void*), void*),
    const Object* terminal, std::function<void (const Result&, Message&&)> completionHandler)
{
    async_receive_raw_message(apiFn, terminal, [=](const Result& result, std::vector<char>&& data) {
        Message msg;
        if (result == Success()) {
            msg.ParseFromArray(data.data(), data.size());
        }

        completionHandler(result, std::move(msg));
    });
}

inline void async_receive_raw_message(int (*apiFn)(void*, void* buffer, unsigned, void (*)(int, unsigned, int, void*), void*),
    const Object* terminal, std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler)
{
    auto buffer = std::make_shared<std::vector<char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    async_call<unsigned, int>([=](const Result& res, unsigned size, int cached) {
        completionHandler(res, std::vector<char>(buffer->begin(), buffer->begin() + size), !!cached);
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), buffer->data(), static_cast<unsigned>(buffer->size()), fn, userArg);
    });
}

template <typename Message>
inline void async_receive_proto_message(int (*apiFn)(void*, void* buffer, unsigned, void (*)(int, unsigned, int, void*), void*),
    const Object* terminal, std::function<void (const Result&, Message&&, cached_flag)> completionHandler)
{
    async_receive_raw_message(apiFn, terminal, [=](const Result& result, std::vector<char>&& data, cached_flag cached) {
        Message msg;
        if (result == Success()) {
            msg.ParseFromArray(data.data(), data.size());
        }

        completionHandler(result, std::move(msg), cached);
    });
}

inline void cancel(int (*apiFn)(void*), const Object* terminal)
{
    int res = apiFn(terminal->handle());
    throw_on_failure(res);
}

template <typename Operation, typename Terminal, typename GatheredMessage>
inline Operation async_scatter_gather_raw(int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
    Terminal* terminal, const void* scatterData, std::size_t scatterSize, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler)
{
    if (scatterSize > MAX_MESSAGE_SIZE) {
        throw Failure(YOGI_ERR_BUFFER_TOO_SMALL);
    }

    auto gatherBuffer = std::make_shared<std::vector<char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    Success res = async_call<int, int, unsigned>([=](const Result& res, int operationId, int flags, unsigned size) {
        auto flow = completion_handler(res, GatheredMessage(*terminal, operationId, static_cast<gather_flags>(flags), std::vector<char>(gatherBuffer->begin(), gatherBuffer->begin() + size)));
        return flow == CONTINUE ? YOGI_DO_CONTINUE : YOGI_DO_STOP;
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), scatterData, static_cast<unsigned>(scatterSize), gatherBuffer->data(),
            static_cast<unsigned>(gatherBuffer->size()), fn, userArg);
    });

    return Operation(*terminal, res.value());
}

template <typename Operation, typename Terminal, typename ScatterMessage, typename GatheredMessage>
inline Operation async_scatter_gather_proto(int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
    Terminal* terminal, ScatterMessage scatterMsg, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler)
{
    unsigned scatterSize = static_cast<unsigned>(scatterMsg.ByteSize());
    if (scatterSize > MAX_MESSAGE_SIZE) {
        throw Failure(YOGI_ERR_BUFFER_TOO_SMALL);
    }

    unsigned char scatterBuffer[MAX_MESSAGE_SIZE];
    scatterMsg.SerializeWithCachedSizesToArray(scatterBuffer);

    auto gatherBuffer = std::make_shared<std::vector<unsigned char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    Success res = async_call<int, int, unsigned>([=](const Result& res, int operationId, int flags, unsigned size) {
        typename GatheredMessage::message_type msg;
        if (res == Success()) {
            msg.ParseFromArray(gatherBuffer->data(), size);
        }

        auto flow = completion_handler(res, GatheredMessage(*terminal, operationId, static_cast<gather_flags>(flags), std::move(msg)));
        return flow == CONTINUE ? YOGI_DO_CONTINUE : YOGI_DO_STOP;
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), scatterBuffer, scatterSize, gatherBuffer->data(),
            static_cast<unsigned>(gatherBuffer->size()), fn, userArg);
    });

    return Operation(*terminal, res.value());
}

inline void cancel_operation(int (*apiFn)(void*, int), const Object* terminal, int operationId)
{
    int res = apiFn(terminal->handle(), operationId);
    throw_on_failure(res);
}

template <typename Terminal, typename ScatteredMessage>
inline void async_receive_scattered_raw_message(int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
    Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler)
{
    auto buffer = std::make_shared<std::vector<char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    async_call<int, unsigned>([=](const Result& res, int operationId, unsigned size) {
        completionHandler(res, ScatteredMessage(*terminal, operationId, std::vector<char>(buffer->begin(), buffer->begin() + size)));
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), buffer->data(), static_cast<unsigned>(buffer->size()), fn, userArg);
    });
}

template <typename Terminal, typename ScatteredMessage>
inline void async_receive_scattered_proto_message(int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
    Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler)
{
    auto buffer = std::make_shared<std::vector<unsigned char>>(MAX_MESSAGE_SIZE); // TODO: Potentially nasty
    async_call<int, unsigned>([=](const Result& res, int operationId, unsigned size) {
        typename ScatteredMessage::message_type msg;
        if (res == Success()) {
            msg.ParseFromArray(buffer->data(), size);
        }

        completionHandler(res, ScatteredMessage(*terminal, operationId, std::move(msg)));
    }, [&](auto fn, void* userArg) {
        return apiFn(terminal->handle(), buffer->data(), static_cast<unsigned>(buffer->size()), fn, userArg);
    });
}

inline void respond_to_scattered_raw_message(int (*apiFn)(void*, int, const void*, unsigned), const Object* terminal,
    int operationId, const void* data, std::size_t size)
{
    int res = apiFn(terminal->handle(), operationId, data, static_cast<unsigned>(size));
    throw_on_failure(res);
}

inline bool try_respond_to_scattered_raw_message(int (*apiFn)(void*, int, const void*, unsigned), const Object* terminal,
    int operationId, const void* data, std::size_t size)
{
    int res = apiFn(terminal->handle(), operationId, data, static_cast<unsigned>(size));
    return res == YOGI_OK;
}

template <typename GatherMessage>
inline int respond_to_scattered_proto_message_impl(int (*apiFn)(void*, int, const void*, unsigned), const Object* terminal,
    int operationId, GatherMessage gatherMsg)
{
    unsigned gatherSize = static_cast<unsigned>(gatherMsg.ByteSize());
    if (gatherSize > MAX_MESSAGE_SIZE) {
        throw Failure(YOGI_ERR_BUFFER_TOO_SMALL);
    }

    unsigned char gatherBuffer[MAX_MESSAGE_SIZE];
    gatherMsg.SerializeWithCachedSizesToArray(gatherBuffer);

    int res = apiFn(terminal->handle(), operationId, gatherBuffer, gatherSize);
    return res;
}

template <typename GatherMessage>
inline void respond_to_scattered_proto_message(int (*apiFn)(void*, int, const void*, unsigned), const Object* terminal,
    int operationId, GatherMessage gatherMsg)
{
    int res = respond_to_scattered_proto_message_impl(apiFn, terminal, operationId, std::move(gatherMsg));
    throw_on_failure(res);
}

template <typename GatherMessage>
inline bool try_respond_to_scattered_proto_message(int (*apiFn)(void*, int, const void*, unsigned), const Object* terminal,
    int operationId, GatherMessage gatherMsg)
{
    int res = respond_to_scattered_proto_message_impl(apiFn, terminal, operationId, std::move(gatherMsg));
    return res == YOGI_OK;
}

inline void ignore_scattered_message(int (*apiFn)(void*, int), const Object* terminal, int operationId)
{
    int res = apiFn(terminal->handle(), operationId);
    throw_on_failure(res);
}

inline bool try_ignore_scattered_message(int (*apiFn)(void*, int), const Object* terminal, int operationId)
{
    int res = apiFn(terminal->handle(), operationId);
    return res == YOGI_OK;
}

} // namespace internal
} // namespace yogi

#endif // YOGI_INTERNAL_PROTO_HPP
