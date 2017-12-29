#pragma once

#include <array>
#include <functional>
#include <locale>
#include <string_view>
#include <unordered_map>

namespace mira::format::details
{
template <class StreamType>
using OutputFunction = void (*)(StreamType&, void const*);

template <class StreamType>
class Arg
{
public:
    template <class A>
    Arg(const A& a)
        : data_{&a}
        , out_{[](StreamType& out, void const* arg) { out << *reinterpret_cast<A const*>(arg); }}
    {}

    void doOut(StreamType& out) const
    {
        out_(out, data_);
    }

private:
    void const*                data_;
    OutputFunction<StreamType> out_;
};

template <class StreamType, class... Args>
class PosArgs
{
public:
    PosArgs(const Args&... args)
        : args_{args...}
    {}

    void doOut(StreamType& out, size_t idx) const;

private:
    std::array<Arg<StreamType>, sizeof...(Args)> args_;
};

template <class StreamType>
class PosArgs<StreamType>
{
public:
    PosArgs()
    {}

    void doOut(StreamType& out, size_t idx) const;
};

template <class StreamType, class StringView>
class NamedArgsMap
{
    using ArgMap = std::unordered_map<StringView, Arg<StreamType>>;
    using VT     = typename ArgMap::value_type;

public:
    NamedArgsMap(const std::initializer_list<VT>& args)
        : map_(args)
    {}

    void doOut(StreamType& out, StringView name) const;

private:
    ArgMap map_;
};

template <class CharT, class TraitsT = std::char_traits<CharT>, class Alloc = std::allocator<CharT>>
struct TraitsImpl
{
    using char_type          = CharT;
    using traits_type        = TraitsT;
    using allocator_type     = Alloc;
    using string_type        = std::basic_string<char_type, traits_type, allocator_type>;
    using string_view_type   = std::basic_string_view<char_type, traits_type>;
    using ostream_type       = std::basic_ostream<char_type, traits_type>;
    using ostringstream_type = std::basic_ostringstream<char_type, traits_type, allocator_type>;
    using outfunc            = void (*)(ostream_type&, void const*);

    template <class... Args>
    using PosArgs = PosArgs<ostream_type, Args...>;

    using NamedArgs = NamedArgsMap<ostream_type, string_view_type>;

private:
    using stream = ostream_type;
    using view   = string_view_type;

public:
    template <class... Args>
    static void doFormat(stream& s, view v, const NamedArgs& named, const Args&... pargs);
};

template <class Input>
struct TraitsTypeSelector;

template <class C, size_t N>
struct TraitsTypeSelector<C[N]>
{
    using type = TraitsImpl<C>;
};

template <class C, class T, class A>
struct TraitsTypeSelector<std::basic_string<C, T, A>>
{
    using type = TraitsImpl<C, T, A>;
};

template <class C, class T, class A>
struct TraitsTypeSelector<std::basic_ostringstream<C, T, A>>
{
    using type = TraitsImpl<C, T, A>;
};

template <class C, class T>
struct TraitsTypeSelector<std::basic_string_view<C, T>>
{
    using type = TraitsImpl<C, T>;
};

template <class C, class T>
struct TraitsTypeSelector<std::basic_ostream<C, T>>
{
    using type = TraitsImpl<C, T>;
};

template <class C>
struct IsStreamImpl : std::false_type
{};

template <template <class, class, class...> class I, class C, class T, class... Args>
struct IsStreamImpl<I<C, T, Args...>> : std::is_base_of<std::basic_ostream<C, T>, I<C, T, Args...>>
{};

template <class C>
using IsStream = typename IsStreamImpl<C>::type;

template <template <class, class, class...> class I, class C, class T, class... Args>
struct TraitsTypeSelector<I<C, T, Args...>>
{
    using is_string_view = std::is_base_of<std::basic_string_view<C, T>, I<C, T, Args...>>;

    using type = std::enable_if_t<std::disjunction_v<IsStream<I<C, T, Args...>>, is_string_view>, TraitsImpl<C, T>>;
};

template <template <class, class, class, class...> class I, class C, class T, class A, class... Args>
struct TraitsTypeSelector<I<C, T, A, Args...>>
{
    using is_string = std::is_base_of<std::basic_string<C, T, A>, I<C, T, A, Args...>>;

    using type = std::enable_if_t<is_string::value, TraitsImpl<C, T>>;
};

template <class T>
using Traits = typename TraitsTypeSelector<T>::type;

template <class T>
using EnableIfStream = std::enable_if_t<IsStream<T>::value, T>;

template <class T>
using EnableIfString = std::enable_if_t<!IsStream<T>::value, typename Traits<T>::string_type>;

template <class T>
using NamedArgs = typename Traits<T>::NamedArgs;

template <class T>
using StringView = typename Traits<T>::string_view_type;

} // namespace mira::format::details

namespace mira
{
template <class Stream>
format::details::EnableIfStream<Stream>& fmt(Stream& out, format::details::StringView<Stream> input)
{
    format::details::Traits<Stream>::doFormat(out, input, {});
    return out;
}

template <class Stream>
format::details::EnableIfStream<Stream>& fmt(Stream& out, format::details::StringView<Stream> input,
                                             const format::details::NamedArgs<Stream>& nargs)
{
    format::details::Traits<Stream>::doFormat(out, input, nargs);
    return out;
}

template <class Stream, class... Args>
format::details::EnableIfStream<Stream>& fmt(Stream& out, format::details::StringView<Stream> input,
                                             const Args&... pargs)
{
    format::details::Traits<Stream>::doFormat(out, input, {}, pargs...);
    return out;
}

template <class Stream, class... Args>
format::details::EnableIfStream<Stream>& fmt(Stream& out, format::details::StringView<Stream> input,
                                             const format::details::NamedArgs<Stream>& nargs, const Args&... pargs)
{
    format::details::Traits<Stream>::doFormat(out, input, nargs, pargs...);
    return out;
}

template <class Input>
format::details::EnableIfString<Input> fmt(const Input& input)
{
    typename format::details::Traits<Input>::ostringstream_type out;
    fmt(out, input);
    return out.str();
}

template <class Input>
format::details::EnableIfString<Input> fmt(const Input& input, const format::details::NamedArgs<Input>& nargs)
{
    typename format::details::Traits<Input>::ostringstream_type out;
    fmt(out, input, nargs);
    return out.str();
}

template <class Input, class... Args>
format::details::EnableIfString<Input> fmt(const Input& input, const Args&... pargs)
{
    typename format::details::Traits<Input>::ostringstream_type out;
    fmt(out, input, {}, pargs...);
    return out.str();
}

template <class Input, class... Args>
format::details::EnableIfString<Input> fmt(const Input& input, const format::details::NamedArgs<Input>& nargs,
                                           const Args&... pargs)
{
    typename format::details::Traits<Input>::ostringstream_type out;
    fmt(out, input, nargs, pargs...);
    return out.str();
}

} // namespace mira

namespace mira::format::details
{
template <class ChT, class TrT, class AlT>
template <class... Args>
void TraitsImpl<ChT, TrT, AlT>::doFormat(stream& s, view v, const NamedArgs& named, const Args&... pargs)
{
    PosArgs<Args...> posed{pargs...};

    constexpr char_type arg_start  = '%';
    constexpr char_type parg_brake = '\'';
    constexpr char_type narg_start = '{';
    constexpr char_type narg_stop  = '}';

    std::locale cloc("C");

    while (!v.empty()) {
        auto asp = v.find(arg_start);
        s << v.substr(0, asp);

        if (asp == v.npos) {
            break;
        }

        v = v.substr(asp);

        if (v.size() == 1) {
            throw std::invalid_argument("format string not correct. unexpected end");
        }

        if (v[1] == arg_start) {
            s << arg_start;
            v = v.substr(2);
            continue;
        }

        if (v[1] == narg_start) {
            auto nas = v.find(narg_stop);
            if (nas == v.npos) {
                throw std::invalid_argument("format string not correct. unexpected end in named argument");
            }
            auto key = v.substr(2, nas - 2);
            if (key.empty()) {
                throw std::invalid_argument("format string not correct, empty argument name");
            }
            named.doOut(s, key);
            v = (nas == v.size() - 1) ? view{} : v.substr(nas + 1);
            continue;
        }

        if (!std::isdigit(v[1], cloc)) {
            throw std::invalid_argument("format string not correct, expecting a digit");
        }

        size_t parg = 0;
        size_t idx  = 1;
        for (; idx < v.size(); ++idx) {
            if (!std::isdigit(v[idx], cloc)) {
                break;
            }
            parg = parg * 10 + (v[idx] - '0');
        }

        posed.doOut(s, parg);

        if (idx < v.size()) {
            v = (v[idx] == parg_brake) ? v.substr(idx + 1) : v.substr(idx);
        } else {
            v = view{};
        }
    }
}

template <class StreamType, class... Args>
void PosArgs<StreamType, Args...>::doOut(StreamType& out, size_t idx) const
{
    if (idx >= sizeof...(Args)) {
        throw std::out_of_range(fmt("index for positional argument too large. idx=%0, size=%1", idx, sizeof...(Args)));
    }

    args_[idx].doOut(out);
}

template <class StreamType>
void PosArgs<StreamType>::doOut(StreamType& out, size_t idx) const
{
    throw std::out_of_range(fmt("index for positional argument too large. idx=%0, size=0", idx));
}

template <class StreamType, class StringView>
void NamedArgsMap<StreamType, StringView>::doOut(StreamType& out, StringView name) const
{
    auto it = map_.find(name);

    if (it == map_.end()) {
        throw std::out_of_range(fmt("unknown named argument request: key=%0", name));
    }

    it->second.doOut(out);
}

} // namespace mira::format::details
