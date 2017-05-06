#ifndef YOGI_TERMINALS_HPP
#define YOGI_TERMINALS_HPP

#include "types.hpp"
#include "binder.hpp"
#include "path.hpp"
#include "subscribable.hpp"
#include "signature.hpp"
#include "internal/terminal.hpp"

#include <string>
#include <functional>
#include <memory>


namespace yogi {

class Leaf;
class ProcessInterface;

class Terminal : public Object
{
private:
    Leaf&             m_leaf;
    const std::string m_name;
    const Signature   m_signature;

private:
    static std::unique_ptr<Terminal> _make_raw_terminal(Leaf& leaf, terminal_type type, std::string name, Signature signature);
    static std::unique_ptr<Terminal> _make_raw_terminal(Leaf& leaf, terminal_type type, const Path& path, Signature signature);

protected:
    Terminal(Leaf& leaf, int type, std::string name, Signature signature);
    Terminal(Leaf& leaf, int type, const Path& path, Signature signature);
    Terminal(int type, std::string name, Signature signature);
    Terminal(int type, const Path& path, Signature signature);

public:
    template <typename Name>
    static std::unique_ptr<Terminal> make_raw_terminal(Leaf& leaf, terminal_type type, Name&& name, Signature signature)
    {
        return _make_raw_terminal(leaf, type, std::forward<Name>(name), signature);
    }

    Leaf& leaf()
    {
        return m_leaf;
    }

    const std::string& name() const
    {
        return m_name;
    }

    Signature signature() const
    {
        return m_signature;
    }

    virtual terminal_type type() const =0;
};


class PrimitiveTerminal : public Terminal
{
protected:
    template <typename Name>
    PrimitiveTerminal(Leaf& leaf, int type, Name&& name, Signature signature)
    : Terminal(leaf, type, std::forward<Name>(name), signature)
    {
    }

    template <typename Name>
    PrimitiveTerminal(int type, Name&& name, Signature signature)
    : Terminal(type, std::forward<Name>(name), signature)
    {
    }
};


class ConvenienceTerminal : public Terminal
{
protected:
    template <typename Name>
    ConvenienceTerminal(Leaf& leaf, int type, Name&& name, Signature signature)
    : Terminal(leaf, type, std::forward<Name>(name), signature)
    {
    }

    template <typename Name>
    ConvenienceTerminal(int type, Name&& name, Signature signature)
    : Terminal(type, std::forward<Name>(name), signature)
    {
    }
};


template <typename ProtoDescription>
class PrimitiveTerminalT : public PrimitiveTerminal
{
protected:
    template <typename Name>
    PrimitiveTerminalT(Leaf& leaf, int type, Name&& name)
    : PrimitiveTerminal(leaf, type, std::forward<Name>(name), Signature(ProtoDescription::SIGNATURE))
    {
    }

    template <typename Name>
    PrimitiveTerminalT(int type, Name&& name)
    : PrimitiveTerminal(type, std::forward<Name>(name), Signature(ProtoDescription::SIGNATURE))
    {
    }
};


template <typename ProtoDescription>
class ConvenienceTerminalT : public ConvenienceTerminal
{
protected:
    template <typename Name>
    ConvenienceTerminalT(Leaf& leaf, int type, Name&& name)
    : ConvenienceTerminal(leaf, type, std::forward<Name>(name), Signature(ProtoDescription::SIGNATURE))
    {
    }

    template <typename Name>
    ConvenienceTerminalT(int type, Name&& name)
    : ConvenienceTerminal(type, std::forward<Name>(name), Signature(ProtoDescription::SIGNATURE))
    {
    }
};


template <typename ProtoDescription>
class DeafMuteTerminal : public PrimitiveTerminalT<ProtoDescription>
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::DEAF_MUTE
    };

public:
    template <typename Name>
    DeafMuteTerminal(Leaf& leaf, Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    {
    }

    template <typename Name>
    DeafMuteTerminal(Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    {
    }

    virtual ~DeafMuteTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "DeafMuteTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }
};


class RawDeafMuteTerminal : public PrimitiveTerminal
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::DEAF_MUTE
    };

public:
    template <typename Name>
    RawDeafMuteTerminal(Leaf& leaf, Name&& name, Signature signature)
    : PrimitiveTerminal(leaf, type(), std::forward<Name>(name), signature)
    {
    }

    template <typename Name>
    RawDeafMuteTerminal(Name&& name, Signature signature)
    : PrimitiveTerminal(type(), std::forward<Name>(name), signature)
    {
    }

    virtual ~RawDeafMuteTerminal();
    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
};


template <typename ProtoDescription>
class PublishSubscribeTerminal : public PrimitiveTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::PUBLISH_SUBSCRIBE
    };

public:
    template <typename Name>
    PublishSubscribeTerminal(Leaf& leaf, Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    PublishSubscribeTerminal(Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~PublishSubscribeTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "PublishSubscribeTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    void publish(message_type msg)
    {
        internal::publish_proto_message(YOGI_PS_Publish, this, msg);
    }

    bool try_publish(message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_PS_Publish, this, msg);
    }

    void async_receive_message(std::function<void (const Result&, message_type&&)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_PS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_PS_CancelReceiveMessage, this);
    }
};


class RawPublishSubscribeTerminal : public PrimitiveTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::PUBLISH_SUBSCRIBE
    };

public:
    template <typename Name>
    RawPublishSubscribeTerminal(Leaf& leaf, Name&& name, Signature signature)
    : PrimitiveTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawPublishSubscribeTerminal(Name&& name, Signature signature)
    : PrimitiveTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawPublishSubscribeTerminal();
    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class CachedPublishSubscribeTerminal : public PrimitiveTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CACHED_PUBLISH_SUBSCRIBE
    };

public:
    template <typename Name>
    CachedPublishSubscribeTerminal(Leaf& leaf, Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    CachedPublishSubscribeTerminal(Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~CachedPublishSubscribeTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "CachedPublishSubscribeTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    void publish(message_type msg)
    {
        internal::publish_proto_message(YOGI_CPS_Publish, this, msg);
    }

    bool try_publish(message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_CPS_Publish, this, msg);
    }

    message_type get_cached_message()
    {
        return internal::get_cached_proto_message<message_type>(YOGI_CPS_GetCachedMessage, this);
    }

    void async_receive_message(std::function<void (const Result&, message_type&&, cached_flag)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_CPS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_CPS_CancelReceiveMessage, this);
    }
};


class RawCachedPublishSubscribeTerminal : public PrimitiveTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CACHED_PUBLISH_SUBSCRIBE
    };

public:
    template <typename Name>
    RawCachedPublishSubscribeTerminal(Leaf& leaf, Name&& name, Signature signature)
    : PrimitiveTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawCachedPublishSubscribeTerminal(Name&& name, Signature signature)
    : PrimitiveTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawCachedPublishSubscribeTerminal();
    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    std::vector<char> get_cached_message();
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class ScatterGatherTerminal : public PrimitiveTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::ScatterMessage scatter_message_type;
    typedef typename ProtoDescription::GatherMessage  gather_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::SCATTER_GATHER
    };

    class Operation
    {
        friend class ScatterGatherTerminal;

        template <typename Operation, typename Terminal, typename ScatterMessage, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_proto(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, ScatterMessage scatterMsg, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        ScatterGatherTerminal* m_terminal;
        raw_operation_id       m_operationId;

    protected:
        Operation(ScatterGatherTerminal& terminal, raw_operation_id operationId = 0)
        : m_terminal(&terminal)
        , m_operationId(operationId)
        {
        }

        Operation(const Operation&) = delete;
        Operation& operator= (const Operation&) = delete;

    public:
        Operation(Operation&& other)
        {
            *this = std::move(other);
        }

        Operation& operator= (Operation&& other)
        {
            m_terminal    = other.m_terminal;
            m_operationId = other.m_operationId;
            return *this;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        operator bool() const
        {
            return m_operationId != 0;
        }

        ScatterGatherTerminal& terminal()
        {
            return *m_terminal;
        }

        const ScatterGatherTerminal& terminal() const
        {
            return *m_terminal;
        }

        void cancel()
        {
            internal::cancel_operation(YOGI_SG_CancelScatterGather, m_terminal, m_operationId);
            m_operationId = 0;
        }
    };

    class GatheredMessage
    {
        template <typename Operation, typename Terminal, typename ScatterMessage, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_proto(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, ScatterMessage scatterMsg, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    public:
        typedef gather_message_type message_type;

    private:
        ScatterGatherTerminal& m_terminal;
        int                    m_operationId;
        gather_flags           m_flags;
        message_type           m_msg;

    protected:
        GatheredMessage(ScatterGatherTerminal& terminal, raw_operation_id operationId, gather_flags flags, message_type&& msg)
        : m_terminal(terminal)
        , m_operationId(operationId)
        , m_flags(flags)
        , m_msg(std::move(msg))
        {
        }

    public:
        ScatterGatherTerminal& terminal()
        {
            return m_terminal;
        }

        const ScatterGatherTerminal& terminal() const
        {
            return m_terminal;
        }

        gather_flags flags() const
        {
            return m_flags;
        }

        const message_type& message() const
        {
            return m_msg;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }
    };

    class ScatteredMessage
    {
        template <typename Terminal, typename ScatteredMessage>
        friend void internal::async_receive_scattered_proto_message(
            int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
            Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler);

    public:
        typedef scatter_message_type message_type;

    private:
        ScatterGatherTerminal* m_terminal;
        raw_operation_id       m_operationId;
        message_type           m_msg;

    protected:
        ScatteredMessage(ScatterGatherTerminal& terminal, raw_operation_id operationId, message_type&& msg)
        : m_terminal(&terminal)
        , m_operationId(operationId)
        , m_msg(std::move(msg))
        {
        }

        ScatteredMessage(const ScatteredMessage&) = delete;
        ScatteredMessage& operator= (const ScatteredMessage&) = delete;

    public:
        ScatteredMessage(ScatteredMessage&& other)
        {
            *this = std::move(other);
        }

        ScatteredMessage& operator= (ScatteredMessage&& other)
        {
            m_terminal    = other.m_terminal;
            m_operationId = other.m_operationId;
            m_msg         = std::move(other.m_msg);

            other.m_terminal = nullptr;

            return *this;
        }

        ScatterGatherTerminal& terminal()
        {
            return *m_terminal;
        }

        const ScatterGatherTerminal& terminal() const
        {
            return *m_terminal;
        }

        const message_type& message() const
        {
            return m_msg;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        void respond(gather_message_type msg)
        {
            internal::respond_to_scattered_proto_message(YOGI_SG_RespondToScatteredMessage, m_terminal, m_operationId, msg);
            m_operationId = 0;
        }

        bool try_respond(gather_message_type msg)
        {
            int res = internal::try_respond_to_scattered_proto_message(YOGI_SG_RespondToScatteredMessage, m_terminal, m_operationId, msg);
            if (res == YOGI_OK) {
                m_operationId = 0;
            }

            return res == YOGI_OK;
        }

        void ignore()
        {
            internal::ignore_scattered_message(YOGI_SG_IgnoreScatteredMessage, m_terminal, m_operationId);
            m_operationId = 0;
        }

        bool try_ignore()
        {
            int res = internal::try_ignore_scattered_message(YOGI_SG_IgnoreScatteredMessage, m_terminal, m_operationId);
            if (res == YOGI_OK) {
                m_operationId = 0;
            }

            return res == YOGI_OK;
        }
    };

public:
    template <typename Name>
    ScatterGatherTerminal(Leaf& leaf, Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    ScatterGatherTerminal(Name&& name)
    : PrimitiveTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~ScatterGatherTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "ScatterGatherTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static scatter_message_type make_scatter_message()
    {
        return scatter_message_type();
    }

    static gather_message_type make_gather_message()
    {
        return gather_message_type();
    }

    Operation async_scatter_gather(scatter_message_type msg,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
    {
        return internal::async_scatter_gather_proto<Operation>(YOGI_SG_AsyncScatterGather, this, msg, completionHandler);
    }

    Operation try_async_scatter_gather(scatter_message_type msg,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
    {
        try {
            return internal::async_scatter_gather_proto<Operation>(YOGI_SG_AsyncScatterGather, this, msg, completionHandler);
        }
        catch (...) {
            return Operation(*this);
        }
    }

    void async_receive_scattered_message(std::function<void (const Result&, ScatteredMessage&&)> completionHandler)
    {
        internal::async_receive_scattered_proto_message(YOGI_SG_AsyncReceiveScatteredMessage, this, completionHandler);
    }

    void cancel_receive_scattered_message()
    {
        internal::cancel(YOGI_SG_CancelReceiveScatteredMessage, this);
    }
};


class RawScatterGatherTerminal : public PrimitiveTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::SCATTER_GATHER
    };

public:
    class Operation
    {
        friend class RawScatterGatherTerminal;

        template <typename Operation, typename Terminal, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_raw(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, const void* scatterData, std::size_t scatterSize,
            std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        RawScatterGatherTerminal* m_terminal;
        raw_operation_id          m_operationId;

    protected:
        Operation(RawScatterGatherTerminal& terminal, raw_operation_id operationId = 0);

        Operation(const Operation&) = delete;
        Operation& operator= (const Operation&) = delete;

    public:
        Operation(Operation&& other);

        Operation& operator= (Operation&& other);

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        operator bool() const
        {
            return m_operationId != 0;
        }

        RawScatterGatherTerminal& terminal()
        {
            return *m_terminal;
        }

        const RawScatterGatherTerminal& terminal() const
        {
            return *m_terminal;
        }

        void cancel();
    };

    class GatheredMessage
    {
        template <typename Operation, typename Terminal, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_raw(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, const void* scatterData, std::size_t scatterSize,
            std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        RawScatterGatherTerminal& m_terminal;
        raw_operation_id          m_operationId;
        gather_flags              m_flags;
        std::vector<char>         m_data;

    protected:
        GatheredMessage(RawScatterGatherTerminal& terminal, raw_operation_id operationId, gather_flags flags, std::vector<char>&& data);

    public:
        RawScatterGatherTerminal& terminal()
        {
            return m_terminal;
        }

        const RawScatterGatherTerminal& terminal() const
        {
            return m_terminal;
        }

        gather_flags flags() const
        {
            return m_flags;
        }

        const std::vector<char>& data() const
        {
            return m_data;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }
    };

    class ScatteredMessage
    {
        template <typename Terminal, typename ScatteredMessage>
        friend void internal::async_receive_scattered_raw_message(
            int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
            Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler);

    private:
        RawScatterGatherTerminal* m_terminal;
        raw_operation_id          m_operationId;
        std::vector<char>         m_data;

    protected:
        ScatteredMessage(RawScatterGatherTerminal& terminal, raw_operation_id operationId, std::vector<char>&& data);

        ScatteredMessage(const ScatteredMessage&) = delete;
        ScatteredMessage& operator= (const ScatteredMessage&) = delete;

    public:
        ScatteredMessage(ScatteredMessage&& other)
        {
            *this = std::move(other);
        }

        ScatteredMessage& operator= (ScatteredMessage&& other);

        RawScatterGatherTerminal& terminal()
        {
            return *m_terminal;
        }

        const RawScatterGatherTerminal& terminal() const
        {
            return *m_terminal;
        }

        const std::vector<char> data() const
        {
            return m_data;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        void respond(const void* data, std::size_t size);
        bool try_respond(const void* data, std::size_t size);
        void respond(const std::vector<char>& data);
        bool try_respond(const std::vector<char>& data);
        void ignore();
        bool try_ignore();
    };

public:
    template <typename Name>
    RawScatterGatherTerminal(Leaf& leaf, Name&& name, Signature signature)
    : PrimitiveTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawScatterGatherTerminal(Name&& name, Signature signature)
    : PrimitiveTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawScatterGatherTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;

    Operation async_scatter_gather(const void* scatterData, std::size_t scatterSize,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler);

    Operation async_scatter_gather(const std::vector<char>& data,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler);

    Operation try_async_scatter_gather(const void* scatterData, std::size_t scatterSize,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler);

    Operation try_async_scatter_gather(const std::vector<char>& data,
        std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler);

    void async_receive_scattered_message(std::function<void (const Result&, ScatteredMessage&&)> completionHandler);
    void cancel_receive_scattered_message();
};


template <typename ProtoDescription>
class ProducerTerminal : public ConvenienceTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::PRODUCER
    };

public:
    template <typename Name>
    ProducerTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    ProducerTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~ProducerTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "ProducerTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    void publish(message_type msg)
    {
        internal::publish_proto_message(YOGI_PC_Publish, this, msg);
    }

    bool try_publish(message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_PC_Publish, this, msg);
    }
};


class RawProducerTerminal : public ConvenienceTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::PRODUCER
    };

public:
    template <typename Name>
    RawProducerTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawProducerTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawProducerTerminal();
    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
};


template <typename ProtoDescription>
class ConsumerTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CONSUMER
    };

public:
    template <typename Name>
    ConsumerTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    template <typename Name>
    ConsumerTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    virtual ~ConsumerTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "ConsumerTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    void async_receive_message(std::function<void (const Result&, message_type&&)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_PC_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_PC_CancelReceiveMessage, this);
    }
};


class RawConsumerTerminal : public ConvenienceTerminal, public Binder
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CONSUMER
    };

public:
    template <typename Name>
    RawConsumerTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    template <typename Name>
    RawConsumerTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    virtual ~RawConsumerTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class CachedProducerTerminal : public ConvenienceTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CACHED_PRODUCER
    };

public:
    template <typename Name>
    CachedProducerTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    CachedProducerTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~CachedProducerTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "CachedProducerTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    void publish(message_type msg)
    {
        internal::publish_proto_message(YOGI_CPC_Publish, this, msg);
    }

    bool try_publish(message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_CPC_Publish, this, msg);
    }
};


class RawCachedProducerTerminal : public ConvenienceTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CACHED_PRODUCER
    };

public:
    template <typename Name>
    RawCachedProducerTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawCachedProducerTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawCachedProducerTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
};


template <typename ProtoDescription>
class CachedConsumerTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder
{
public:
    typedef typename ProtoDescription::PublishMessage message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CACHED_CONSUMER
    };

public:
    template <typename Name>
    CachedConsumerTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    template <typename Name>
    CachedConsumerTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    virtual ~CachedConsumerTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "CachedConsumerTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static message_type make_message()
    {
        return message_type();
    }

    message_type get_cached_message()
    {
        return internal::get_cached_proto_message<message_type>(YOGI_CPC_GetCachedMessage, this);
    }

    void async_receive_message(std::function<void (const Result&, message_type&&, cached_flag)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_CPC_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_CPC_CancelReceiveMessage, this);
    }
};


class RawCachedConsumerTerminal : public ConvenienceTerminal, public Binder
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CACHED_CONSUMER
    };

public:
    template <typename Name>
    RawCachedConsumerTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    template <typename Name>
    RawCachedConsumerTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    virtual ~RawCachedConsumerTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    std::vector<char> get_cached_message();
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class MasterTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder, public Subscribable
{
public:
    typedef typename ProtoDescription::MasterMessage master_message_type;
    typedef typename ProtoDescription::SlaveMessage  slave_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::MASTER
    };

public:
    template <typename Name>
    MasterTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    MasterTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~MasterTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "MasterTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static master_message_type make_message()
    {
        return master_message_type();
    }

    void publish(master_message_type msg)
    {
        internal::publish_proto_message(YOGI_MS_Publish, this, msg);
    }

    bool try_publish(master_message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_MS_Publish, this, msg);
    }

    void async_receive_message(std::function<void (const Result&, slave_message_type&&)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_MS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_MS_CancelReceiveMessage, this);
    }
};


class RawMasterTerminal : public ConvenienceTerminal, public Binder, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::MASTER
    };

public:
    template <typename Name>
    RawMasterTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawMasterTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~RawMasterTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class SlaveTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder, public Subscribable
{
public:
    typedef typename ProtoDescription::MasterMessage master_message_type;
    typedef typename ProtoDescription::SlaveMessage  slave_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::SLAVE
    };

public:
    template <typename Name>
    SlaveTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    SlaveTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~SlaveTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "SlaveTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static slave_message_type make_message()
    {
        return slave_message_type();
    }

    void publish(slave_message_type msg)
    {
        internal::publish_proto_message(YOGI_MS_Publish, this, msg);
    }

    bool try_publish(slave_message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_MS_Publish, this, msg);
    }

    void async_receive_message(std::function<void (const Result&, master_message_type&&)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_MS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_MS_CancelReceiveMessage, this);
    }
};


class RawSlaveTerminal : public ConvenienceTerminal, public Binder, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::SLAVE
    };

public:
    template <typename Name>
    RawSlaveTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawSlaveTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~RawSlaveTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class CachedMasterTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder, public Subscribable
{
public:
    typedef typename ProtoDescription::MasterMessage master_message_type;
    typedef typename ProtoDescription::SlaveMessage  slave_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CACHED_MASTER
    };

public:
    template <typename Name>
    CachedMasterTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    CachedMasterTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~CachedMasterTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "CachedMasterTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static master_message_type make_message()
    {
        return master_message_type();
    }

    void publish(master_message_type msg)
    {
        internal::publish_proto_message(YOGI_CMS_Publish, this, msg);
    }

    bool try_publish(master_message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_CMS_Publish, this, msg);
    }

    slave_message_type get_cached_message()
    {
        return internal::get_cached_proto_message<slave_message_type>(YOGI_CMS_GetCachedMessage, this);
    }

    void async_receive_message(std::function<void (const Result&, slave_message_type&&, cached_flag)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_CMS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_CMS_CancelReceiveMessage, this);
    }
};


class RawCachedMasterTerminal : public ConvenienceTerminal, public Binder, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CACHED_MASTER
    };

public:
    template <typename Name>
    RawCachedMasterTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawCachedMasterTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~RawCachedMasterTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    std::vector<char> get_cached_message();
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class CachedSlaveTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder, public Subscribable
{
public:
    typedef typename ProtoDescription::MasterMessage master_message_type;
    typedef typename ProtoDescription::SlaveMessage  slave_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CACHED_SLAVE
    };

public:
    template <typename Name>
    CachedSlaveTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    CachedSlaveTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~CachedSlaveTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "CachedSlaveTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static slave_message_type make_message()
    {
        return slave_message_type();
    }

    void publish(slave_message_type msg)
    {
        internal::publish_proto_message(YOGI_CMS_Publish, this, msg);
    }

    bool try_publish(slave_message_type msg)
    {
        return internal::try_publish_proto_message(YOGI_CMS_Publish, this, msg);
    }

    master_message_type get_cached_message()
    {
        return internal::get_cached_proto_message<master_message_type>(YOGI_CMS_GetCachedMessage, this);
    }

    void async_receive_message(std::function<void (const Result&, master_message_type&&, cached_flag)> completionHandler)
    {
        internal::async_receive_proto_message(YOGI_CMS_AsyncReceiveMessage, this, completionHandler);
    }

    void cancel_receive_message()
    {
        internal::cancel(YOGI_CMS_CancelReceiveMessage, this);
    }
};


class RawCachedSlaveTerminal : public ConvenienceTerminal, public Binder, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CACHED_SLAVE
    };

public:
    template <typename Name>
    RawCachedSlaveTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawCachedSlaveTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    , Subscribable(this)
    {
    }

    virtual ~RawCachedSlaveTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;
    void publish(const void* data, std::size_t size);
    void publish(const std::vector<char>& data);
    bool try_publish(const void* data, std::size_t size);
    bool try_publish(const std::vector<char>& data);
    std::vector<char> get_cached_message();
    void async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler);
    void cancel_receive_message();
};


template <typename ProtoDescription>
class ServiceTerminal : public ConvenienceTerminalT<ProtoDescription>, public Binder
{
public:
    typedef typename ProtoDescription::ScatterMessage request_message_type;
    typedef typename ProtoDescription::GatherMessage response_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::SERVICE
    };

    class Request
    {
        template <typename Terminal, typename ScatteredMessage>
        friend void internal::async_receive_scattered_proto_message(
            int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
            Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler);

    public:
        typedef request_message_type message_type;

    private:
        ServiceTerminal* m_terminal;
        raw_operation_id m_operationId;
        message_type     m_msg;

    protected:
        Request(ServiceTerminal& terminal, raw_operation_id operationId, message_type&& msg)
        : m_terminal(&terminal)
        , m_operationId(operationId)
        , m_msg(std::move(msg))
        {
        }

        Request(const Request&) = delete;
        Request& operator= (const Request&) = delete;

    public:
        Request(Request&& other)
        {
            *this = std::move(other);
        }

        Request& operator= (Request&& other)
        {
            m_terminal    = other.m_terminal;
            m_operationId = other.m_operationId;
            m_msg         = std::move(other.m_msg);

            other.m_terminal = nullptr;

            return *this;
        }

        ServiceTerminal& terminal()
        {
            return *m_terminal;
        }

        const ServiceTerminal& terminal() const
        {
            return *m_terminal;
        }

        const message_type& message() const
        {
            return m_msg;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        void respond(response_message_type msg)
        {
            internal::respond_to_scattered_proto_message(YOGI_SC_RespondToRequest, m_terminal, m_operationId, msg);
            m_operationId = 0;
        }

        bool try_respond(response_message_type msg)
        {
            int res = internal::try_respond_to_scattered_proto_message(YOGI_SC_RespondToRequest, m_terminal, m_operationId, msg);
            if (res == YOGI_OK) {
                m_operationId = 0;
            }

            return res == YOGI_OK;
        }

        void ignore()
        {
            internal::ignore_scattered_message(YOGI_SC_IgnoreRequest, m_terminal, m_operationId);
            m_operationId = 0;
        }

        bool try_ignore()
        {
            int res = internal::try_ignore_scattered_message(YOGI_SC_IgnoreRequest, m_terminal, m_operationId);
            if (res == YOGI_OK) {
                m_operationId = 0;
            }

            return res == YOGI_OK;
        }
    };

public:
    template <typename Name>
    ServiceTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    template <typename Name>
    ServiceTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Binder(this)
    {
    }

    virtual ~ServiceTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "ServiceTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static request_message_type make_request_message()
    {
        return request_message_type();
    }

    static response_message_type make_response_message()
    {
        return response_message_type();
    }

    void async_receive_request(std::function<void (const Result&, Request&&)> completionHandler)
    {
        internal::async_receive_scattered_proto_message(YOGI_SC_AsyncReceiveRequest, this, completionHandler);
    }

    void cancel_receive_request()
    {
        internal::cancel(YOGI_SC_CancelReceiveRequest, this);
    }
};


class RawServiceTerminal : public ConvenienceTerminal, public Binder
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::SERVICE
    };

public:
    class Request
    {
        template <typename Terminal, typename ScatteredMessage>
        friend void internal::async_receive_scattered_raw_message(
            int (*apiFn)(void*, void*, unsigned, void (*)(int, int, unsigned, void*), void*),
            Terminal* terminal, std::function<void (const Result&, ScatteredMessage&&)> completionHandler);

    private:
        RawServiceTerminal* m_terminal;
        raw_operation_id    m_operationId;
        std::vector<char>   m_data;

    protected:
        Request(RawServiceTerminal& terminal, raw_operation_id operationId, std::vector<char>&& data);

        Request(const Request&) = delete;
        Request& operator= (const Request&) = delete;

    public:
        Request(Request&& other);

        Request& operator= (Request&& other);

        RawServiceTerminal& terminal()
        {
            return *m_terminal;
        }

        const RawServiceTerminal& terminal() const
        {
            return *m_terminal;
        }

        const std::vector<char> data() const
        {
            return m_data;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        void respond(const void* data, std::size_t size);
        bool try_respond(const void* data, std::size_t size);
        void respond(const std::vector<char>& data);
        bool try_respond(const std::vector<char>& data);
        void ignore();
        bool try_ignore();
    };

public:
    template <typename Name>
    RawServiceTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    template <typename Name>
    RawServiceTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Binder(this)
    {
    }

    virtual ~RawServiceTerminal();
    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;

    void async_receive_request(std::function<void (const Result&, Request&&)> completionHandler);
    void cancel_receive_request();
};


template <typename ProtoDescription>
class ClientTerminal : public ConvenienceTerminalT<ProtoDescription>, public Subscribable
{
public:
    typedef typename ProtoDescription::ScatterMessage request_message_type;
    typedef typename ProtoDescription::GatherMessage response_message_type;

    enum {
        TERMINAL_TYPE = terminal_type::CLIENT
    };

    class Operation
    {
        friend class ClientTerminal;

        template <typename Operation, typename Terminal, typename ScatterMessage, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_proto(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, ScatterMessage scatterMsg, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        ClientTerminal*  m_terminal;
        raw_operation_id m_operationId;

    protected:
        Operation(ClientTerminal& terminal, raw_operation_id operationId = 0)
        : m_terminal(&terminal)
        , m_operationId(operationId)
        {
        }

        Operation(const Operation&) = delete;
        Operation& operator= (const Operation&) = delete;

    public:
        Operation(Operation&& other)
        {
            *this = std::move(other);
        }

        Operation& operator= (Operation&& other)
        {
            m_terminal    = other.m_terminal;
            m_operationId = other.m_operationId;
            return *this;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        operator bool() const
        {
            return m_operationId != 0;
        }

        ClientTerminal& terminal()
        {
            return *m_terminal;
        }

        const ClientTerminal& terminal() const
        {
            return *m_terminal;
        }

        void cancel()
        {
            internal::cancel_operation(YOGI_SC_CancelRequest, m_terminal, m_operationId);
            m_operationId = 0;
        }
    };

    class Response
    {
        template <typename Operation, typename Terminal, typename ScatterMessage, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_proto(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, ScatterMessage scatterMsg, std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    public:
        typedef response_message_type message_type;

    private:
        ClientTerminal&  m_terminal;
        raw_operation_id m_operationId;
        gather_flags     m_flags;
        message_type     m_msg;

    protected:
        Response(ClientTerminal& terminal, raw_operation_id operationId, gather_flags flags, message_type&& msg)
        : m_terminal(terminal)
        , m_operationId(operationId)
        , m_flags(flags)
        , m_msg(std::move(msg))
        {
        }

    public:
        ClientTerminal& terminal()
        {
            return m_terminal;
        }

        const ClientTerminal& terminal() const
        {
            return m_terminal;
        }

        gather_flags flags() const
        {
            return m_flags;
        }

        const message_type& message() const
        {
            return m_msg;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }
    };

public:
    template <typename Name>
    ClientTerminal(Leaf& leaf, Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(leaf, type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    template <typename Name>
    ClientTerminal(Name&& name)
    : ConvenienceTerminalT<ProtoDescription>(type(), std::forward<Name>(name))
    , Subscribable(this)
    {
    }

    virtual ~ClientTerminal()
    {
        this->_destroy();
    }

    virtual const std::string& class_name() const override
    {
        static std::string s = "ClientTerminal";
        return s;
    }

    virtual terminal_type type() const override
    {
        return static_cast<terminal_type>(TERMINAL_TYPE);
    }

    static request_message_type make_request_message()
    {
        return request_message_type();
    }

    static response_message_type make_response_message()
    {
        return response_message_type();
    }

    Operation async_request(request_message_type msg,
        std::function<control_flow (const Result&, Response&&)> completionHandler)
    {
        return internal::async_scatter_gather_proto<Operation>(YOGI_SC_AsyncRequest, this, msg, completionHandler);
    }

    Operation try_async_request(request_message_type msg,
        std::function<control_flow (const Result&, Response&&)> completionHandler)
    {
        try {
            return internal::async_scatter_gather_proto<Operation>(YOGI_SC_AsyncRequest, this, msg, completionHandler);
        }
        catch (...) {
            return Operation(*this);
        }
    }
};


class RawClientTerminal : public ConvenienceTerminal, public Subscribable
{
public:
    enum {
        TERMINAL_TYPE = terminal_type::CLIENT
    };

public:
    class Operation
    {
        friend class RawClientTerminal;

        template <typename Operation, typename Terminal, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_raw(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, const void* scatterData, std::size_t scatterSize,
            std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        RawClientTerminal* m_terminal;
        raw_operation_id   m_operationId;

    protected:
        Operation(RawClientTerminal& terminal, raw_operation_id operationId = 0);

        Operation(const Operation&) = delete;
        Operation& operator= (const Operation&) = delete;

    public:
        Operation(Operation&& other);
        Operation& operator= (Operation&& other);

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }

        operator bool() const
        {
            return m_operationId != 0;
        }

        RawClientTerminal& terminal()
        {
            return *m_terminal;
        }

        const RawClientTerminal& terminal() const
        {
            return *m_terminal;
        }

        void cancel();
    };

    class Response
    {
        template <typename Operation, typename Terminal, typename GatheredMessage>
        friend Operation internal::async_scatter_gather_raw(
            int (*apiFn)(void*, const void*, unsigned, void*, unsigned, int (*)(int, int, int, unsigned, void*), void*),
            Terminal* terminal, const void* scatterData, std::size_t scatterSize,
            std::function<control_flow (const Result&, GatheredMessage&&)> completion_handler);

    private:
        RawClientTerminal& m_terminal;
        raw_operation_id   m_operationId;
        gather_flags       m_flags;
        std::vector<char>  m_data;

    protected:
        Response(RawClientTerminal& terminal, raw_operation_id operationId, gather_flags flags, std::vector<char>&& data);

    public:
        RawClientTerminal& terminal()
        {
            return m_terminal;
        }

        const RawClientTerminal& terminal() const
        {
            return m_terminal;
        }

        gather_flags flags() const
        {
            return m_flags;
        }

        const std::vector<char>& data() const
        {
            return m_data;
        }

        raw_operation_id operation_id() const
        {
            return m_operationId;
        }
    };

public:
    template <typename Name>
    RawClientTerminal(Leaf& leaf, Name&& name, Signature signature)
    : ConvenienceTerminal(leaf, type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    template <typename Name>
    RawClientTerminal(Name&& name, Signature signature)
    : ConvenienceTerminal(type(), std::forward<Name>(name), signature)
    , Subscribable(this)
    {
    }

    virtual ~RawClientTerminal();

    virtual const std::string& class_name() const override;
    virtual terminal_type type() const override;

    Operation async_request(const void* scatterData, std::size_t scatterSize,
        std::function<control_flow (const Result&, Response&&)> completionHandler);

    Operation async_request(const std::vector<char>& data,
        std::function<control_flow (const Result&, Response&&)> completionHandler);

    Operation try_async_request(const void* scatterData, std::size_t scatterSize,
        std::function<control_flow (const Result&, Response&&)> completionHandler);

    Operation try_async_request(const std::vector<char>& data,
        std::function<control_flow (const Result&, Response&&)> completionHandler);
};

} // namespace yogi

#endif // YOGI_TERMINALS_HPP
