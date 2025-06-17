// SPDX-License-Identifier: MIT

#include "sixdom.hh"
#include <cassert>
#include <cinttypes>
#include <cstring>

extern "C" {
#include "sixpack.h"
}

using namespace sixdom;

struct Context
{
	Delegate& delegate;
	IOReader read;
};

static int
readerGetchar(struct sixpack *sp)
{
	auto& context = *static_cast<Context*>(sp->userdata);
	IOResult result = context.read();
	if (auto* status = std::get_if<IOStatus>(&result)) {
		switch (*status) {
		case IOStatus::Eof:
			return SIXPACK_IO_EOF;
		case IOStatus::Error:
			return SIXPACK_IO_ERROR;
		default:
			assert(!"unreachable");
			abort();
		}
	}
	return std::get<int>(result);
}

static void*
delegateProcess(struct sixpack *sp, enum sixpack_type type, const char *data, unsigned data_size)
{
	auto& context = *static_cast<Context*>(sp->userdata);

	Delegate::Result result;
	switch (type) {
		case SIXPACK_INTEGER: {
			auto value = ::strtoll(data, nullptr, 0);
			if (value < INT32_MIN || value > INT32_MAX)
				return (void*) "integer out of bounds";
			result = context.delegate.value(Integer(value));
			break;
		}
		case SIXPACK_FLOAT:
			result = context.delegate.value(Float(::strtod(data, nullptr)));
			break;
		case SIXPACK_BOOL:
			result = context.delegate.value(Bool(data_size));
			break;
		case SIXPACK_STRING:
			result = context.delegate.value(String(data, data_size));
			break;
		case SIXPACK_LIST:
			if (data == SIXPACK_BEGIN)
				result = context.delegate.begin(Delegate::List);
			else
				result = context.delegate.end(Delegate::List);
			break;
		case SIXPACK_DICT:
			if (data == SIXPACK_BEGIN)
				result = context.delegate.begin(Delegate::Dict);
			else
				result = context.delegate.end(Delegate::Dict);
		break;
	}

	return result ? (void*) result.value() : SIXPACK_CONTINUE;
}

std::optional<Error>
sixdom::parse(Delegate& delegate, IOReader&& reader)
{
	Context context = { delegate, std::move(reader) };
	struct sixpack sp = {
		&context,
		readerGetchar,
		delegateProcess,
	};

	if (sixpack_parse(&sp))
		return std::nullopt;

	return Error { sp.error_line, sp.error_column, { sp.error_message } };
}

Delegate::Result
DOMBuilder::begin(Container kind)
{
	if (kind == Container::List) {
		push(sixdom::List {});
	} else if (kind == Container::Dict) {
		push(sixdom::Dict {});
	} else {
		assert(!"unreachable");
		return "internal error: non list/dictionary container?";
	}
	return std::nullopt;
}

void
DOMBuilder::append(Value&& value)
{
	if (auto* list = top<sixdom::List>()) {
		list->emplace_back(std::move(value));
	} else if (auto* dict = top<sixdom::Dict>()) {
		auto& key = topKey();
		if (key) {
			dict->emplace(std::make_pair(std::move(*key), std::move(value)));
			key.reset();
		} else {
			assert(std::holds_alternative<Key>(value));
			assert(!key.has_value());
			key = std::move(std::get<Key>(value));
		}
	} else {
		assert(!"unreachable");
	}
}

Delegate::Result
DOMBuilder::end(Container kind)
{
	if (!stack.size())
		return "internal error: cannot pop empty stack";

	assert(topKind() == kind);

	auto value = pop();
	if (stack.size() == 0) {
		assert(kind == Container::Dict);
		assert(std::holds_alternative<sixdom::Dict>(value));
		result = std::move(rva::get<sixdom::Dict>(value));
	} else {
		append(std::move(value));
	}

	return std::nullopt;
}

std::variant<Error, Value>
sixdom::parse(IOReader&& reader)
{
	DOMBuilder builder;
	if (auto err = parse(builder, std::move(reader)))
		return err.value();
	return std::move(*builder.result);
}

#ifdef SIXDOM_MAIN
template <typename... Ts> struct Overload : Ts... { using Ts::operator()...; };
template <typename... Ts> Overload(Ts...) -> Overload<Ts...>;

static void
dump(const Value& value, FILE* stream = stderr)
{
	rva::visit(Overload {
		[stream](const Integer& value) {
			fprintf(stream, "Integer<%" PRIi32 ">", value);
		},
		[stream](const Float& value) {
			fprintf(stream, "Float<%f>", value);
		},
		[stream](const Bool& value) {
			fprintf(stream, "Bool<%s>", value ? "true" : "false");
		},
		[stream](const String& value) {
			fprintf(stream, "String<%*s%s>", std::min<int>(value.size(), 20),
					value.c_str(), (value.size() > 20) ? "..." : "");
		},
		[stream](const List& value) {
			fputs("List[ ", stream);
			for (const auto& item : value) {
				dump(item, stream);
				fputc(' ', stream);
			}
			fputc(']', stream);
		},
		[stream](const Dict& value) {
			fputs("Dict{ ", stream);
			for (const auto& item : value) {
				fputs(item.first.c_str(), stream);
				fputc(':', stream);
				dump(item.second);
				fputc(' ', stream);
			}
			fputc('}', stream);
		},
	}, value);
}


int
main(int argc, char *argv[])
{
	if ((argc > 1) && !(stdin = freopen(argv[1], "rb", stdin))) {
		fprintf(stderr, "cannot open '%s': %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	auto reader = []() -> IOResult {
		int c = getchar();
		if (c == EOF) {
			if (ferror(stdin))
				return IOStatus::Error;
			return IOStatus::Eof;
		}
		return c;
	};

	auto result = sixdom::parse(std::move(reader));
	if (auto* err = std::get_if<Error>(&result)) {
		fprintf(stderr, "line %d, column %d: %s\n", err->line, err->column, err->message.c_str());
		return EXIT_FAILURE;
	}

	dump(std::get<Value>(result));
	fputc('\n', stderr);
}
#endif // SIXDOM_MAIN
