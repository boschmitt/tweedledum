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

template<typename ConcreteOptor>
void print(ConcreteOptor const& optor, std::ostream& os, uint32_t indent)
{
	os << fmt::format(
	    "{:>{}}{} (Default print)\n", "", indent, optor.name());
}

class Operator {
public:
	template<class ConcreteOptor>
	Operator(ConcreteOptor optor) noexcept
	    : self_(
	        std::make_shared<Model<ConcreteOptor> const>(std::move(optor)))
	{}

	std::string_view name() const
	{
		return self_->name();
	}

	friend void print(
	    Operator const& optor, std::ostream& os, uint32_t indent)
	{
		optor.self_->print_(os, indent);
	}

private:
	struct Concept {
		virtual ~Concept() = default;
		virtual std::string_view name() const = 0;
		virtual void print_(std::ostream& os, uint32_t) const = 0;
	};

	template<class ConcreteOptor>
	class Model final : public Concept {
	public:
		Model(ConcreteOptor optor) noexcept : optor_(std::move(optor))
		{}

		std::string_view name() const override
		{
			return optor_.name();
		}

		void print_(std::ostream& os, uint32_t indent) const override
		{
			print(optor_, os, indent);
		}

	private:
		ConcreteOptor optor_;
	};

	std::shared_ptr<Concept const> self_;
};

} // namespace tweedledum
