// SPDX-License-Identifier: MIT

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "variant.hpp"

namespace sixdom {

using Bool    = bool;
using Float   = double;
using Integer = ::int32_t;
using Key     = ::std::string;
using String  = ::std::string;
using Value   = rva::variant<
	Integer,
	Float,
	String,
	Bool,
	std::vector<rva::self_t>,
	std::unordered_map<Key, rva::self_t>
>;

using List = std::vector<Value>;
using Dict = std::unordered_map<Key, Value>;

struct Error
{
	const unsigned line;
	const unsigned column;
	const std::string message;
};

enum class IOStatus : uint8_t
{
	Eof,
	Error,
};
using IOResult = std::variant<IOStatus, int>;
using IOReader = std::function<IOResult()>;

struct Delegate
{
	using Result = std::optional<const char*>;

	enum Container {
		List,
		Dict,
	};

	virtual Result value(Bool value) { return std::nullopt; };
	virtual Result value(Float value) { return std::nullopt; };
	virtual Result value(Integer value) { return std::nullopt; };
	virtual Result value(String&& value) { return std::nullopt; };
	virtual Result begin(Container kind) { return std::nullopt; };
	virtual Result end(Container kind) { return std::nullopt; };

	virtual ~Delegate() = default;
};

std::optional<Error> parse(Delegate&, IOReader&&);

struct DOMBuilder : Delegate
{
	void append(Value&& value);

	Result value(Bool value) override { append(value); return std::nullopt; }
	Result value(Float value) override { append(value); return std::nullopt; }
	Result value(Integer value) override { append(value); return std::nullopt; }
	Result value(String&& value) override { append(value); return std::nullopt; }
	Result begin(Container kind) override;
	Result end(Container kind) override;

	using StackItem = std::pair<std::optional<Key>, Value>;

	std::vector<StackItem> stack { };
	std::optional<sixdom::Dict> result { std::nullopt };

	Container topKind() const {
		if (std::holds_alternative<sixdom::List>(stack.back().second))
			return List;
		if (std::holds_alternative<sixdom::Dict>(stack.back().second))
			return Dict;
		assert(!"unreachable");
		abort();
	}

	std::optional<Key>& topKey() { return stack.back().first; }
	Value& topValue() { return stack.back().second; }
	template <typename T> T* top() { return rva::get_if<T>(&topValue()); }

	void push(Value&& value) {
		stack.emplace_back(std::make_pair(std::nullopt, std::move(value)));
	}

	Value pop() {
		auto value = std::move(topValue());
		stack.pop_back();
		return value;
	}
};

std::variant<Error, Value> parse(IOReader&&);

} // namespace sixdom
