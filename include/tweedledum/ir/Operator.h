/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <complex>
#include <fmt/format.h>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>

namespace tweedledum {

template<class Op, class = void>
struct has_matrix : std::false_type {
};

template<class Op>
struct has_matrix<Op, std::void_t<decltype(std::declval<Op>().matrix())>>
    : std::true_type {
};

template<class Op>
inline constexpr bool has_matrix_v = has_matrix<Op>::value;

template<typename ConcreteOp>
void print(ConcreteOp const& optor, std::ostream& os, uint32_t indent)
{
	os << fmt::format(
	    "{:>{}}{} (Default print)\n", "", indent, optor.kind());
}

template<typename ConcreteOp>
std::array<std::complex<double>, 4> to_matrix(ConcreteOp const& optor)
{
	 if constexpr (has_matrix_v<ConcreteOp>) {
		 return optor.matrix();
	 } else {
		 assert(0);
		 return {{{1., 0.}, {0., 0.}, {0., 0.}, {1., 0.}}};
	}
}

class Operator {
	using Matrix = std::array<std::complex<double>, 4>;
public:
	template<class ConcreteOp>
	Operator(ConcreteOp optor) noexcept
	    : self_(std::make_shared<Model<ConcreteOp> const>(std::move(optor)))
	{}

	std::string_view kind() const
	{
		return self_->kind();
	}

	Matrix matrix() const
	{
		return self_->matrix();
	}

	template<typename ConcreteOp>
	bool is() const
	{
		return ConcreteOp::kind() == kind();
	}

	template<typename ConcreteOp>
	ConcreteOp const& cast() const
	{
		assert(is<ConcreteOp>());
		auto model = std::static_pointer_cast<Model<ConcreteOp> const>(self_);
		return model->optor();
	}

	friend void print(
	    Operator const& optor, std::ostream& os, uint32_t indent)
	{
		optor.self_->print_(os, indent);
	}

private:
	struct Concept {
		virtual ~Concept() = default;
		virtual std::string_view kind() const = 0;
		virtual Matrix matrix() const = 0;
		virtual void print_(std::ostream& os, uint32_t) const = 0;
	};

	template<class ConcreteOp>
	class Model final : public Concept {
	public:
		Model(ConcreteOp optor) noexcept : optor_(std::move(optor)) {}

		std::string_view kind() const override
		{
			return optor_.kind();
		}

		ConcreteOp const& optor() const
		{
			return optor_;
		}

		Matrix matrix() const override
		{
			return to_matrix(optor_);
		}

		void print_(std::ostream& os, uint32_t indent) const override
		{
			print(optor_, os, indent);
		}

	private:
		ConcreteOp optor_;
	};

	std::shared_ptr<Concept const> self_;
};

} // namespace tweedledum
