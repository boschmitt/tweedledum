/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../Operators/Meta.h"
#include "../Utils/Matrix.h"
#include "OperatorTraits.h"

#include <memory>
#include <optional>
#include <string_view>

namespace tweedledum {

class Instruction;

// Type-erased operator class
//
// This class that can be initialized with any­ type that satisfies the Operator
// concept, which is defined in `struct Concept`.
//
// It takes ownership of the controlled object. That is, it moves the object
// into a region of storage where we can control its lifetime.  We do that by
// creating a copy of the original ConcreteOp object on the heap (or on the
// stack if the object is small enough).
//
class Operator {
public:
    // clang-format off
    template<typename ConcreteOp,
             std::enable_if_t<!std::is_same_v<Operator, remove_cvref_t<ConcreteOp>>, bool> = true>
    Operator(ConcreteOp&& op) noexcept
    {
        using ConcreteType = remove_cvref_t<ConcreteOp>;
        static_assert(!std::is_same_v<ConcreteType, Instruction>);
        constexpr bool is_small = sizeof(Model<ConcreteType, true>) <= small_size;
        new (&model_) Model<ConcreteType, is_small>(std::forward<ConcreteOp>(op));
        concept_ = &Model<ConcreteType, is_small>::vtable_;
    }

    template<typename ConcreteOp,
             std::enable_if_t<!std::is_same_v<Operator, remove_cvref_t<ConcreteOp>>, bool> = true>
    Operator& operator=(ConcreteOp&& op) noexcept
    {
        using ConcreteType = remove_cvref_t<ConcreteOp>;
        static_assert(!std::is_same_v<ConcreteType, Instruction>);
        constexpr bool is_small = sizeof(Model<ConcreteType, true>) <= small_size;
        concept_->dtor(&model_);
        new (&model_) Model<ConcreteType, is_small>(std::forward<ConcreteOp>(op));
        concept_ = &Model<ConcreteType, is_small>::vtable_;
        return *this;
    }
    // clang-format on

    Operator(Operator const& other) noexcept
    {
        concept_ = other.concept_;
        concept_->clone(&other.model_, &model_);
    }

    Operator& operator=(Operator const& other) noexcept
    {
        // Guard self assignment
        if (this == &other) {
            return *this;
        }
        concept_->dtor(&model_);

        concept_ = other.concept_;
        concept_->clone(&other.model_, &model_);
        return *this;
    }

    // TODO: I need to think a bit better about move!!
    Operator(Operator&& other) = delete;

    Operator& operator=(Operator&& other) = delete;

    ~Operator()
    {
        concept_->dtor(&model_);
    }

    std::optional<Operator> adjoint() const
    {
        return concept_->adjoint(&model_);
    }

    std::string_view kind() const
    {
        return concept_->kind(&model_);
    };

    std::string_view name() const
    {
        std::string_view the_kind = kind();
        auto pos = the_kind.find_first_of(".");
        if (pos == std::string_view::npos) {
            pos = 0;
        } else {
            ++pos;
        }
        return std::string_view(the_kind.data() + pos, the_kind.size() - pos);
    };

    std::optional<UMatrix> const matrix() const
    {
        return concept_->matrix(&model_);
    }

    uint32_t num_targets() const
    {
        return concept_->num_targets(&model_);
    };

    template<typename ConcreteOp>
    bool is_a() const
    {
        return ConcreteOp::kind() == kind();
    }

    template<typename... Args>
    bool is_one() const
    {
        return (... || is_a<Args>());
    }

    template<typename ConcreteOp>
    ConcreteOp const& cast() const
    {
        assert(is_a<ConcreteOp>());
        return *static_cast<ConcreteOp const*>(concept_->optor(&model_));
    }

    bool operator==(Operator const& other) const
    {
        if (kind() != other.kind()) {
            return false;
        }
        return concept_->equal(&model_, &other.model_);
    }

private:
    friend class Instruction;

    struct Concept {
        void (*dtor)(void*) noexcept;
        void (*clone)(void const*, void*) noexcept;
        bool (*equal)(void const*, void const*) noexcept;
        void const* (*optor)(void const*) noexcept;
        std::optional<Operator> (*adjoint)(void const*) noexcept;
        std::string_view (*kind)(void const*) noexcept;
        std::optional<UMatrix> const (*matrix)(void const*) noexcept;
        uint32_t (*num_targets)(void const*) noexcept;
    };

    template<class ConcreteOp, bool IsSmall>
    struct Model;

    static constexpr size_t small_size = sizeof(void*) * 4;
    Concept const* concept_;
    std::aligned_storage_t<small_size> model_;
};

// Stack
template<class ConcreteOp>
struct Operator::Model<ConcreteOp, true> {
    Model(ConcreteOp&& op) noexcept
        : operator_(std::forward<ConcreteOp>(op))
    {}

    Model(ConcreteOp const& op) noexcept
        : operator_(op)
    {}

    static void dtor(void* self) noexcept
    {
        static_cast<Model*>(self)->~Model();
    }

    static void clone(void const* self, void* other) noexcept
    {
        new (other) Model<remove_cvref_t<ConcreteOp>, true>(
          static_cast<Model const*>(self)->operator_);
    }

    static bool equal(void const* self, void const* other) noexcept
    {
        if constexpr (!supports<std::equal_to<>(ConcreteOp, ConcreteOp)>::value)
        {
            return true;
        } else {
            return static_cast<Model const*>(self)->operator_
                == static_cast<Model const*>(other)->operator_;
        }
    }

    static void const* optor(void const* self) noexcept
    {
        return &static_cast<Model const*>(self)->operator_;
    }

    static std::optional<Operator> adjoint(void const* self) noexcept
    {
        if constexpr (has_adjoint_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_.adjoint();
        } else {
            return std::nullopt;
        }
    }

    static std::string_view kind(void const* self) noexcept
    {
        return static_cast<Model const*>(self)->operator_.kind();
    }

    static std::optional<UMatrix> const matrix(void const* self) noexcept
    {
        if constexpr (has_matrix_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_.matrix();
        } else {
            return std::nullopt;
        }
    }

    static uint32_t num_targets(void const* self) noexcept
    {
        if constexpr (has_num_targets_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_.num_targets();
        } else {
            return 1u;
        }
    }

    static constexpr Concept vtable_{
      dtor, clone, equal, optor, adjoint, kind, matrix, num_targets};

    ConcreteOp operator_;
};

// Heap
template<class ConcreteOp>
struct Operator::Model<ConcreteOp, false> {
    Model(ConcreteOp&& op) noexcept
        : operator_(std::make_unique<ConcreteOp>(std::forward<ConcreteOp>(op)))
    {}

    Model(ConcreteOp const& op) noexcept
        : operator_(std::make_unique<ConcreteOp>(op))
    {}

    static void dtor(void* self) noexcept
    {
        static_cast<Model*>(self)->operator_.reset();
    }

    static void clone(void const* self, void* other) noexcept
    {
        new (other)
          Model<ConcreteOp, false>(*static_cast<Model const*>(self)->operator_);
    }

    static bool equal(void const* self, void const* other) noexcept
    {
        if constexpr (!supports<std::equal_to<>(ConcreteOp, ConcreteOp)>::value)
        {
            return true;
        } else {
            return static_cast<Model const*>(self)->operator_
                == static_cast<Model const*>(other)->operator_;
        }
    }

    static void const* optor(void const* self) noexcept
    {
        return static_cast<Model const*>(self)->operator_.get();
    }

    static std::optional<Operator> adjoint(void const* self) noexcept
    {
        if constexpr (has_adjoint_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_->adjoint();
        } else {
            return std::nullopt;
        }
    }

    static std::string_view kind(void const* self) noexcept
    {
        return static_cast<Model const*>(self)->operator_->kind();
    }

    static std::optional<UMatrix> const matrix(void const* self) noexcept
    {
        if constexpr (has_matrix_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_->matrix();
        } else {
            return std::nullopt;
        }
    }

    static uint32_t num_targets(void const* self) noexcept
    {
        if constexpr (has_num_targets_v<ConcreteOp>) {
            return static_cast<Model const*>(self)->operator_->num_targets();
        } else {
            return 1u;
        }
    }

    static constexpr Concept vtable_{
      dtor, clone, equal, optor, adjoint, kind, matrix, num_targets};

    std::unique_ptr<ConcreteOp> operator_;
};

} // namespace tweedledum
