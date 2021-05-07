/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace tweedledum::qasm {

/*! \brief Represent a single lexed token. */
class Token {
public:
    enum class Kinds : uint8_t
    {
#define TOKEN(X) X,
#include "Tokens.def"
    };

    Token()
        : kind_(Token::Kinds::unknown)
        , location_(0u)
        , length_(0u)
        , content_ptr_(nullptr)
    {}

    Token(Kinds const k, uint32_t const loc, uint32_t const len,
      char const* content)
        : kind_(k)
        , location_(loc)
        , length_(len)
        , content_ptr_(content)
    {}

    // Token classification.
    bool is(Kinds const k) const
    {
        return kind_ == k;
    }

    template<typename... Ts>
    bool is_one_of(Kinds const k1, Kinds const k2, Ts... ks) const
    {
        return is(k1) || is_one_of(k2, ks...);
    }

    Kinds kind() const
    {
        return kind_;
    }

    uint32_t location() const
    {
        return location_;
    }

    uint32_t length() const
    {
        return length_;
    }

    std::string_view spelling() const
    {
        return std::string_view(content_ptr_, length_);
    }

    // TODO: check this further, feel like a really ugly hack.
    operator double()
    {
        return std::stod(std::string(content_ptr_, length_));
    }

    operator std::string_view()
    {
        return std::string_view(content_ptr_, length_);
    }

    operator uint32_t()
    {
        return std::stoi(std::string(content_ptr_, length_));
    }

    operator int32_t()
    {
        return std::stoi(std::string(content_ptr_, length_));
    }

private:
    Kinds kind_;
    uint32_t location_;
    uint32_t length_;
    char const* content_ptr_;
};

static std::unordered_map<std::string, Token::Kinds> const pp_tokens = {
#define TOKEN(X)
#define PPKEYWORD(X) {#X, Token::Kinds::pp_##X},
#include "Tokens.def"
};

static std::unordered_map<std::string, Token::Kinds> const kw_tokens = {
#define TOKEN(X)
#define KEYWORD(X, Y) {Y, Token::Kinds::kw_##X},
#define UOPERATOR(X, Y) {Y, Token::Kinds::kw_uop_##X},
#include "Tokens.def"
};

static std::string const token_names[] = {
#define TOKEN(X) #X,
#include "Tokens.def"
};

static std::string_view token_name(Token::Kinds k)
{
    uint32_t k_idx = static_cast<uint32_t>(k);
    return token_names[k_idx];
}

} // namespace tweedledum::qasm
