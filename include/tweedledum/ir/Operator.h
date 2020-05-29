/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <fmt/format.h>
#include <memory>
#include <ostream>
#include <string>

namespace tweedledum {

template<typename ConcreteOp>
void print(ConcreteOp const& optor, std::ostream& os, uint32_t indent)
{
	os << fmt::format(
	    "{:>{}}{} (Default print)\n", "", indent, optor.kind());
}

class Operator {
public:
	template<class ConcreteOp>
	Operator(ConcreteOp optor) noexcept
	    : self_(std::make_shared<Model<ConcreteOp> const>(std::move(optor)))
	{}

	std::string_view kind() const
	{
		return self_->kind();
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
