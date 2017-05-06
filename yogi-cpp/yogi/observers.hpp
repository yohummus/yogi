#ifndef YOGI_OBSERVERS_HPP
#define YOGI_OBSERVERS_HPP

#include "types.hpp"
#include "result.hpp"
#include "terminals.hpp"
#include "logging.hpp"
#include "internal/observing.hpp"

#include <vector>
#include <functional>
#include <mutex>
#include <exception>


namespace yogi {

class Binder;
class Subscribable;
class ProcessInterface;


class Observer
{
public:
    virtual void start() =0;
    virtual void stop() =0;
};

class BadCallbackId : public std::runtime_error
{
public:
    BadCallbackId();
};


class BindingObserver final
: public internal::StateObserver<Observer, binding_state, BadCallbackId>
{
public:
    using internal::StateObserver<Observer, binding_state, BadCallbackId>::CallbackId;

private:
    Binder& m_binder;

protected:
    virtual void _async_get_state(std::function<void (const Result&, binding_state)> completionHandler) override;
    virtual void _async_await_state_change(std::function<void (const Result&, binding_state)> completionHandler) override;
    virtual void _cancel_await_state() override;

public:
    BindingObserver(Binder& binder);
    virtual ~BindingObserver();

    const Binder& binder() const
    {
        return m_binder;
    }

    Binder& binder()
    {
        return m_binder;
    }
};


class SubscriptionObserver final
: public internal::StateObserver<Observer, subscription_state, BadCallbackId>
{
public:
    using internal::StateObserver<Observer, subscription_state, BadCallbackId>::CallbackId;

private:
    Subscribable& m_subscribable;

protected:
    virtual void _async_get_state(std::function<void (const Result&, subscription_state)> completionHandler) override;
    virtual void _async_await_state_change(std::function<void (const Result&, subscription_state)> completionHandler) override;
    virtual void _cancel_await_state() override;

public:
    SubscriptionObserver(Subscribable& subscribable);
    virtual ~SubscriptionObserver();

    const Subscribable& subscribable() const
    {
        return m_subscribable;
    }

    Subscribable& subscribable()
    {
        return m_subscribable;
    }
};


template <typename Terminal>
class MessageObserver;


template <typename ProtoDescription>
class MessageObserver<PublishSubscribeTerminal<ProtoDescription>> final
: public internal::PublishMessageObserver<Observer, PublishSubscribeTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::PublishMessageObserver<Observer, PublishSubscribeTerminal<ProtoDescription>, BadCallbackId>::PublishMessageObserver;
};


template <>
class MessageObserver<RawPublishSubscribeTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawPublishSubscribeTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawPublishSubscribeTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<CachedPublishSubscribeTerminal<ProtoDescription>> final
: public internal::PublishMessageObserver<Observer, CachedPublishSubscribeTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::PublishMessageObserver<Observer, CachedPublishSubscribeTerminal<ProtoDescription>, BadCallbackId>::PublishMessageObserver;
};


template <>
class MessageObserver<RawCachedPublishSubscribeTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawCachedPublishSubscribeTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawCachedPublishSubscribeTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<ScatterGatherTerminal<ProtoDescription>> final
: public internal::ScatterMessageObserver<Observer, ScatterGatherTerminal<ProtoDescription>>
{
public:
    using internal::ScatterMessageObserver<Observer, ScatterGatherTerminal<ProtoDescription>>::ScatterMessageObserver;
};


template <>
class MessageObserver<RawScatterGatherTerminal> final
: public internal::ScatterMessageObserver<Observer, RawScatterGatherTerminal>
{
public:
    using internal::ScatterMessageObserver<Observer, RawScatterGatherTerminal>::ScatterMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<ServiceTerminal<ProtoDescription>> final
: public internal::RequestObserver<Observer, ServiceTerminal<ProtoDescription>>
{
public:
    using internal::RequestObserver<Observer, ServiceTerminal<ProtoDescription>>::RequestObserver;
};


template <>
class MessageObserver<RawServiceTerminal> final
: public internal::RequestObserver<Observer, RawServiceTerminal>
{
public:
    using internal::RequestObserver<Observer, RawServiceTerminal>::RequestObserver;
};


template <typename ProtoDescription>
class MessageObserver<ConsumerTerminal<ProtoDescription>> final
: public internal::PublishMessageObserver<Observer, ConsumerTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::PublishMessageObserver<Observer, ConsumerTerminal<ProtoDescription>, BadCallbackId>::PublishMessageObserver;
};


template <>
class MessageObserver<RawConsumerTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawConsumerTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawConsumerTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<CachedConsumerTerminal<ProtoDescription>> final
: public internal::PublishMessageObserver<Observer, CachedConsumerTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::PublishMessageObserver<Observer, CachedConsumerTerminal<ProtoDescription>, BadCallbackId>::PublishMessageObserver;
};


template <>
class MessageObserver<RawCachedConsumerTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawCachedConsumerTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawCachedConsumerTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<MasterTerminal<ProtoDescription>> final
: public internal::SlaveMessageObserver<Observer, MasterTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::SlaveMessageObserver<Observer, MasterTerminal<ProtoDescription>, BadCallbackId>::SlaveMessageObserver;
};


template <>
class MessageObserver<RawMasterTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawMasterTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawMasterTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<SlaveTerminal<ProtoDescription>> final
: public internal::MasterMessageObserver<Observer, SlaveTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::MasterMessageObserver<Observer, SlaveTerminal<ProtoDescription>, BadCallbackId>::MasterMessageObserver;
};


template <>
class MessageObserver<RawSlaveTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawSlaveTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawSlaveTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<CachedMasterTerminal<ProtoDescription>> final
: public internal::SlaveMessageObserver<Observer, CachedMasterTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::SlaveMessageObserver<Observer, CachedMasterTerminal<ProtoDescription>, BadCallbackId>::SlaveMessageObserver;
};


template <>
class MessageObserver<RawCachedMasterTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawCachedMasterTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawCachedMasterTerminal, BadCallbackId>::RawPublishMessageObserver;
};


template <typename ProtoDescription>
class MessageObserver<CachedSlaveTerminal<ProtoDescription>> final
: public internal::MasterMessageObserver<Observer, CachedSlaveTerminal<ProtoDescription>, BadCallbackId>
{
public:
    using internal::MasterMessageObserver<Observer, CachedSlaveTerminal<ProtoDescription>, BadCallbackId>::MasterMessageObserver;
};


template <>
class MessageObserver<RawCachedSlaveTerminal> final
: public internal::RawPublishMessageObserver<Observer, RawCachedSlaveTerminal, BadCallbackId>
{
public:
    using internal::RawPublishMessageObserver<Observer, RawCachedSlaveTerminal, BadCallbackId>::RawPublishMessageObserver;
};


class OperationalObserver final
: public internal::StateObserver<Observer, operational_flag, BadCallbackId>
{
protected:
    virtual void _async_get_state(std::function<void (const Result&, operational_flag)> completionHandler) override;
    virtual void _async_await_state_change(std::function<void (const Result&, operational_flag)> completionHandler) override;
    virtual void _cancel_await_state() override;

public:
    OperationalObserver();
    virtual ~OperationalObserver();
};

} // namespace yogi

#endif // YOGI_OBSERVERS_HPP
