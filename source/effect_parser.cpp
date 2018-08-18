/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#include "effect_parser.hpp"
#include "effect_symbol_table.hpp"
#include <algorithm>
#include <fstream>

namespace reshadefx
{
	//void scalar_literal_cast(const literal_expression_node *from, size_t i, int &to);
	//void scalar_literal_cast(const literal_expression_node *from, size_t i, unsigned int &to);
	//void scalar_literal_cast(const literal_expression_node *from, size_t i, float &to);
	//void vector_literal_cast(const literal_expression_node *from, size_t k, literal_expression_node *to, size_t j);

	spv::Id fold_constant_expression(spv::Id expression) {
		return expression;
	}

	inline void write(std::ofstream &s, uint32_t word)
	{
		s.write((char *)&word, 4);;
	}
	inline void write(std::ofstream &s, const spv_node &ins)
	{
		// First word of an instruction:
		// The 16 low-order bits are the opcode enumerant
		// The 16 high-order bits are the word count of the instruction
		const uint32_t num_words = 1 + (ins.result_type != 0) + (ins.result != 0) + ins.operands.size();
		write(s, (num_words << 16) | ins.op);

		// Optional instruction type ID
		if (ins.result_type != 0) write(s, ins.result_type);

		// Optional instruction result ID
		if (ins.result != 0) write(s, ins.result);

		// Write out the operands
		for (size_t i = 0; i < ins.operands.size(); ++i)
			write(s, ins.operands[i]);
	}

	const std::string get_token_name(tokenid id)
	{
		switch (id)
		{
			default:
			case tokenid::unknown:
				return "unknown";
			case tokenid::end_of_file:
				return "end of file";
			case tokenid::exclaim:
				return "!";
			case tokenid::hash:
				return "#";
			case tokenid::dollar:
				return "$";
			case tokenid::percent:
				return "%";
			case tokenid::ampersand:
				return "&";
			case tokenid::parenthesis_open:
				return "(";
			case tokenid::parenthesis_close:
				return ")";
			case tokenid::star:
				return "*";
			case tokenid::plus:
				return "+";
			case tokenid::comma:
				return ",";
			case tokenid::minus:
				return "-";
			case tokenid::dot:
				return ".";
			case tokenid::slash:
				return "/";
			case tokenid::colon:
				return ":";
			case tokenid::semicolon:
				return ";";
			case tokenid::less:
				return "<";
			case tokenid::equal:
				return "=";
			case tokenid::greater:
				return ">";
			case tokenid::question:
				return "?";
			case tokenid::at:
				return "@";
			case tokenid::bracket_open:
				return "[";
			case tokenid::backslash:
				return "\\";
			case tokenid::bracket_close:
				return "]";
			case tokenid::caret:
				return "^";
			case tokenid::brace_open:
				return "{";
			case tokenid::pipe:
				return "|";
			case tokenid::brace_close:
				return "}";
			case tokenid::tilde:
				return "~";
			case tokenid::exclaim_equal:
				return "!=";
			case tokenid::percent_equal:
				return "%=";
			case tokenid::ampersand_ampersand:
				return "&&";
			case tokenid::ampersand_equal:
				return "&=";
			case tokenid::star_equal:
				return "*=";
			case tokenid::plus_plus:
				return "++";
			case tokenid::plus_equal:
				return "+=";
			case tokenid::minus_minus:
				return "--";
			case tokenid::minus_equal:
				return "-=";
			case tokenid::arrow:
				return "->";
			case tokenid::ellipsis:
				return "...";
			case tokenid::slash_equal:
				return "|=";
			case tokenid::colon_colon:
				return "::";
			case tokenid::less_less_equal:
				return "<<=";
			case tokenid::less_less:
				return "<<";
			case tokenid::less_equal:
				return "<=";
			case tokenid::equal_equal:
				return "==";
			case tokenid::greater_greater_equal:
				return ">>=";
			case tokenid::greater_greater:
				return ">>";
			case tokenid::greater_equal:
				return ">=";
			case tokenid::caret_equal:
				return "^=";
			case tokenid::pipe_equal:
				return "|=";
			case tokenid::pipe_pipe:
				return "||";
			case tokenid::identifier:
				return "identifier";
			case tokenid::reserved:
				return "reserved word";
			case tokenid::true_literal:
				return "true";
			case tokenid::false_literal:
				return "false";
			case tokenid::int_literal:
			case tokenid::uint_literal:
				return "integral literal";
			case tokenid::float_literal:
			case tokenid::double_literal:
				return "floating point literal";
			case tokenid::string_literal:
				return "string literal";
			case tokenid::namespace_:
				return "namespace";
			case tokenid::struct_:
				return "struct";
			case tokenid::technique:
				return "technique";
			case tokenid::pass:
				return "pass";
			case tokenid::for_:
				return "for";
			case tokenid::while_:
				return "while";
			case tokenid::do_:
				return "do";
			case tokenid::if_:
				return "if";
			case tokenid::else_:
				return "else";
			case tokenid::switch_:
				return "switch";
			case tokenid::case_:
				return "case";
			case tokenid::default_:
				return "default";
			case tokenid::break_:
				return "break";
			case tokenid::continue_:
				return "continue";
			case tokenid::return_:
				return "return";
			case tokenid::discard_:
				return "discard";
			case tokenid::extern_:
				return "extern";
			case tokenid::static_:
				return "static";
			case tokenid::uniform_:
				return "uniform";
			case tokenid::volatile_:
				return "volatile";
			case tokenid::precise:
				return "precise";
			case tokenid::in:
				return "in";
			case tokenid::out:
				return "out";
			case tokenid::inout:
				return "inout";
			case tokenid::const_:
				return "const";
			case tokenid::linear:
				return "linear";
			case tokenid::noperspective:
				return "noperspective";
			case tokenid::centroid:
				return "centroid";
			case tokenid::nointerpolation:
				return "nointerpolation";
			case tokenid::void_:
				return "void";
			case tokenid::bool_:
			case tokenid::bool2:
			case tokenid::bool3:
			case tokenid::bool4:
			case tokenid::bool2x2:
			case tokenid::bool3x3:
			case tokenid::bool4x4:
				return "bool type";
			case tokenid::int_:
			case tokenid::int2:
			case tokenid::int3:
			case tokenid::int4:
			case tokenid::int2x2:
			case tokenid::int3x3:
			case tokenid::int4x4:
				return "int type";
			case tokenid::uint_:
			case tokenid::uint2:
			case tokenid::uint3:
			case tokenid::uint4:
			case tokenid::uint2x2:
			case tokenid::uint3x3:
			case tokenid::uint4x4:
				return "uint type";
			case tokenid::float_:
			case tokenid::float2:
			case tokenid::float3:
			case tokenid::float4:
			case tokenid::float2x2:
			case tokenid::float3x3:
			case tokenid::float4x4:
				return "float type";
			case tokenid::vector:
				return "vector";
			case tokenid::matrix:
				return "matrix";
			case tokenid::string_:
				return "string";
			case tokenid::texture:
				return "texture";
			case tokenid::sampler:
				return "sampler";
		}
	}

	parser::parser() :
		_symbol_table(new symbol_table())
	{
	}
	parser::~parser()
	{
	}

	bool parser::run(const std::string &input)
	{
		_lexer.reset(new lexer(input));
		_lexer_backup.reset();

		consume();

		while (!peek(tokenid::end_of_file))
		{
			if (!parse_top_level())
			{
				return false;
			}
		}

		std::ofstream s("test.spv", std::ios::binary | std::ios::out);
		// Write SPIRV header info
		write(s, spv::MagicNumber);
		write(s, spv::Version);
		write(s, 0u); // Generator magic number, see https://www.khronos.org/registry/spir-v/api/spir-v.xml
		write(s, 1000u); // Maximum ID
		write(s, 0u); // Reserved for instruction schema

		// All capabilities
		
		write(s, spv_node(spv::OpCapability)
			.add(spv::CapabilityShader));

		// Optional extension instructions
		const uint32_t glsl_ext = 1;

		write(s, spv_node(spv::OpExtInstImport, glsl_ext)
			.add_string("GLSL.std.450")); // Import GLSL extension

		// Single required memory model instruction
		write(s, spv_node(spv::OpMemoryModel)
			.add(spv::AddressingModelLogical)
			.add(spv::MemoryModelGLSL450));

		// All entry point declarations
		//_out_stream << spv_node(spv::OpEntryPoint, spv::ExecutionModelVertex, 0 /* function id */,

		// All execution mode declarations
		//_out_stream << spv_node(spv::OpExecutionMode, 0 /* function id */,

		// All debug instructions
		const char *filename = "test.fx";
		const uint32_t filename_id = 2;

		write(s, spv_node(spv::OpString, filename_id)
			.add_string(filename));
		write(s, spv_node(spv::OpSource)
			.add(spv::SourceLanguageUnknown)
			.add(0) // Version
			.add(filename_id)); // Filename string ID

		// All annotation instructions

		// All type declarations
		write(s, spv_node(spv::OpTypeVoid, type_void));
		write(s, spv_node(spv::OpTypeBool, type_bool));
		write(s, spv_node(spv::OpTypeVector, type_bool2)
			.add(type_bool)
			.add(2));
		write(s, spv_node(spv::OpTypeVector, type_bool3)
			.add(type_bool)
			.add(3));
		write(s, spv_node(spv::OpTypeVector, type_bool4)
			.add(type_bool)
			.add(4));
		write(s, spv_node(spv::OpTypeMatrix, type_bool2x2)
			.add(type_bool2)
			.add(2));
		write(s, spv_node(spv::OpTypeMatrix, type_bool3x3)
			.add(type_bool3)
			.add(3));
		write(s, spv_node(spv::OpTypeMatrix, type_bool4x4)
			.add(type_bool4)
			.add(4));
		write(s, spv_node(spv::OpTypeInt, type_int)
			.add(32) // Width: Specifies how many bits wide the type 
			.add(1)); // Signedness: 1 indicates signed semantics
		write(s, spv_node(spv::OpTypeVector, type_int2)
			.add(type_int)
			.add(2));
		write(s, spv_node(spv::OpTypeVector, type_int3)
			.add(type_int)
			.add(3));
		write(s, spv_node(spv::OpTypeVector, type_int4)
			.add(type_int)
			.add(4));
		write(s, spv_node(spv::OpTypeMatrix, type_int2x2)
			.add(type_int2)
			.add(2));
		write(s, spv_node(spv::OpTypeMatrix, type_int3x3)
			.add(type_int3)
			.add(3));
		write(s, spv_node(spv::OpTypeMatrix, type_int4x4)
			.add(type_int4)
			.add(4));
		write(s, spv_node(spv::OpTypeInt, type_uint)
			.add(32) // Width: Specifies how many bits wide the type 
			.add(0)); // Signedness: 0 indicates unsigned or no signedness semantics
		write(s, spv_node(spv::OpTypeVector, type_uint2)
			.add(type_uint)
			.add(2));
		write(s, spv_node(spv::OpTypeVector, type_uint3)
			.add(type_uint)
			.add(3));
		write(s, spv_node(spv::OpTypeVector, type_uint4)
			.add(type_uint)
			.add(4));
		write(s, spv_node(spv::OpTypeMatrix, type_uint2x2)
			.add(type_uint2)
			.add(2));
		write(s, spv_node(spv::OpTypeMatrix, type_uint3x3)
			.add(type_uint3)
			.add(3));
		write(s, spv_node(spv::OpTypeMatrix, type_uint4x4)
			.add(type_uint4)
			.add(4));
		write(s, spv_node(spv::OpTypeFloat, type_float)
			.add(32)); // Width: Specifies how many bits wide the type is (as described by the IEEE 754 standard)
		write(s, spv_node(spv::OpTypeVector, type_float2)
			.add(type_float)
			.add(2));
		write(s, spv_node(spv::OpTypeVector, type_float3)
			.add(type_float)
			.add(3));
		write(s, spv_node(spv::OpTypeVector, type_float4)
			.add(type_float)
			.add(4));
		write(s, spv_node(spv::OpTypeMatrix, type_float2x2)
			.add(type_float2)
			.add(2));
		write(s, spv_node(spv::OpTypeMatrix, type_float3x3)
			.add(type_float3)
			.add(3));
		write(s, spv_node(spv::OpTypeMatrix, type_float4x4)
			.add(type_float4)
			.add(4));
		write(s, spv_node(spv::OpTypeImage, type_texture)
			.add(type_float) // Sampled type: Type of the components that result from sampling
			.add(spv::Dim2D) // Image dimension
			.add(0) // Depth: 0 indicates not a depth image
			.add(0) // Arrayed: 0 indicates non-arrayed content
			.add(0) // MS: 0 indicates single-sampled content
			.add(1) // Sampled: 1 indicates will be used with sampler
			.add(spv::ImageFormatUnknown)); // Image format
		write(s, spv_node(spv::OpTypeSampledImage, type_sampled_texture)
			.add(type_texture));

		for (const auto &node : _variables.instructions)
		{
			write(s, node);
		}

		// All function definitions

		for (const auto &node : _functions.instructions)
		{
			write(s, node);
		}

		return true;
	}

	// Error handling
	void parser::error(const location &location, unsigned int code, const std::string &message)
	{
		_errors += location.source + '(' + std::to_string(location.line) + ", " + std::to_string(location.column) + ')' + ": ";

		if (code == 0)
		{
			_errors += "error: ";
		}
		else
		{
			_errors += "error X" + std::to_string(code) + ": ";
		}

		_errors += message + '\n';
	}
	void parser::warning(const location &location, unsigned int code, const std::string &message)
	{
		_errors += location.source + '(' + std::to_string(location.line) + ", " + std::to_string(location.column) + ')' + ": ";

		if (code == 0)
		{
			_errors += "warning: ";
		}
		else
		{
			_errors += "warning X" + std::to_string(code) + ": ";
		}

		_errors += message + '\n';
	}

	// Input management
	void parser::backup()
	{
		_lexer.swap(_lexer_backup);
		_lexer.reset(new lexer(*_lexer_backup));
		_token_backup = _token_next;
	}
	void parser::restore()
	{
		_lexer.swap(_lexer_backup);
		_token_next = _token_backup;
	}

	bool parser::peek(tokenid tokid) const
	{
		return _token_next.id == tokid;
	}
	void parser::consume()
	{
		_token = _token_next;
		_token_next = _lexer->lex();
	}
	void parser::consume_until(tokenid tokid)
	{
		while (!accept(tokid) && !peek(tokenid::end_of_file))
		{
			consume();
		}
	}
	bool parser::accept(tokenid tokid)
	{
		if (peek(tokid))
		{
			consume();

			return true;
		}

		return false;
	}
	bool parser::expect(tokenid tokid)
	{
		if (!accept(tokid))
		{
			error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "', expected '" + get_token_name(tokid) + "'");

			return false;
		}

		return true;
	}

	// Types
	bool parser::accept_type_class(type_node &type)
	{
		type.definition = 0;
		type.array_length = 0;

		if (peek(tokenid::identifier))
		{
			type.rows = type.cols = 0;
			type.basetype = type_node::datatype_struct;

			const auto symbol = _symbol_table->find(_token_next.literal_as_string);

			if (symbol && lookup_id(symbol).op == spv::OpTypeStruct)
			{
				type.definition = symbol;

				consume();
			}
			else
			{
				return false;
			}
		}
		else if (accept(tokenid::vector))
		{
			type.rows = 4, type.cols = 1;
			type.basetype = type_node::datatype_float;

			if (accept('<'))
			{
				if (!accept_type_class(type))
				{
					error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "', expected vector element type");

					return false;
				}

				if (!type.is_scalar())
				{
					error(_token.location, 3122, "vector element type must be a scalar type");

					return false;
				}

				if (!(expect(',') && expect(tokenid::int_literal)))
				{
					return false;
				}

				if (_token.literal_as_int < 1 || _token.literal_as_int > 4)
				{
					error(_token.location, 3052, "vector dimension must be between 1 and 4");

					return false;
				}

				type.rows = _token.literal_as_int;

				if (!expect('>'))
				{
					return false;
				}
			}
		}
		else if (accept(tokenid::matrix))
		{
			type.rows = 4, type.cols = 4;
			type.basetype = type_node::datatype_float;

			if (accept('<'))
			{
				if (!accept_type_class(type))
				{
					error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "', expected matrix element type");

					return false;
				}

				if (!type.is_scalar())
				{
					error(_token.location, 3123, "matrix element type must be a scalar type");

					return false;
				}

				if (!(expect(',') && expect(tokenid::int_literal)))
				{
					return false;
				}

				if (_token.literal_as_int < 1 || _token.literal_as_int > 4)
				{
					error(_token.location, 3053, "matrix dimensions must be between 1 and 4");

					return false;
				}

				type.rows = _token.literal_as_int;

				if (!(expect(',') && expect(tokenid::int_literal)))
				{
					return false;
				}

				if (_token.literal_as_int < 1 || _token.literal_as_int > 4)
				{
					error(_token.location, 3053, "matrix dimensions must be between 1 and 4");

					return false;
				}

				type.cols = _token.literal_as_int;

				if (!expect('>'))
				{
					return false;
				}
			}
		}
		else
		{
			type.rows = type.cols = 0;

			switch (_token_next.id)
			{
				case tokenid::void_:
					type.basetype = type_node::datatype_void;
					break;
				case tokenid::bool_:
					type.rows = 1, type.cols = 1;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool2:
					type.rows = 2, type.cols = 1;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool2x2:
					type.rows = 2, type.cols = 2;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool3:
					type.rows = 3, type.cols = 1;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool3x3:
					type.rows = 3, type.cols = 3;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool4:
					type.rows = 4, type.cols = 1;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::bool4x4:
					type.rows = 4, type.cols = 4;
					type.basetype = type_node::datatype_bool;
					break;
				case tokenid::int_:
					type.rows = 1, type.cols = 1;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int2:
					type.rows = 2, type.cols = 1;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int2x2:
					type.rows = 2, type.cols = 2;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int3:
					type.rows = 3, type.cols = 1;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int3x3:
					type.rows = 3, type.cols = 3;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int4:
					type.rows = 4, type.cols = 1;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::int4x4:
					type.rows = 4, type.cols = 4;
					type.basetype = type_node::datatype_int;
					break;
				case tokenid::uint_:
					type.rows = 1, type.cols = 1;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint2:
					type.rows = 2, type.cols = 1;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint2x2:
					type.rows = 2, type.cols = 2;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint3:
					type.rows = 3, type.cols = 1;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint3x3:
					type.rows = 3, type.cols = 3;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint4:
					type.rows = 4, type.cols = 1;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::uint4x4:
					type.rows = 4, type.cols = 4;
					type.basetype = type_node::datatype_uint;
					break;
				case tokenid::float_:
					type.rows = 1, type.cols = 1;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float2:
					type.rows = 2, type.cols = 1;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float2x2:
					type.rows = 2, type.cols = 2;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float3:
					type.rows = 3, type.cols = 1;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float3x3:
					type.rows = 3, type.cols = 3;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float4:
					type.rows = 4, type.cols = 1;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::float4x4:
					type.rows = 4, type.cols = 4;
					type.basetype = type_node::datatype_float;
					break;
				case tokenid::string_:
					type.basetype = type_node::datatype_string;
					break;
				case tokenid::texture:
					type.basetype = type_node::datatype_texture;
					break;
				case tokenid::sampler:
					type.basetype = type_node::datatype_sampler;
					break;
				default:
					return false;
			}

			consume();
		}

		return true;
	}
	bool parser::accept_type_qualifiers(type_node &type)
	{
		unsigned int qualifiers = 0;

		// Storage
		if (accept(tokenid::extern_))
		{
			qualifiers |= type_node::qualifier_extern;
		}
		if (accept(tokenid::static_))
		{
			qualifiers |= type_node::qualifier_static;
		}
		if (accept(tokenid::uniform_))
		{
			qualifiers |= type_node::qualifier_uniform;
		}
		if (accept(tokenid::volatile_))
		{
			qualifiers |= type_node::qualifier_volatile;
		}
		if (accept(tokenid::precise))
		{
			qualifiers |= type_node::qualifier_precise;
		}

		if (accept(tokenid::in))
		{
			qualifiers |= type_node::qualifier_in;
		}
		if (accept(tokenid::out))
		{
			qualifiers |= type_node::qualifier_out;
		}
		if (accept(tokenid::inout))
		{
			qualifiers |= type_node::qualifier_inout;
		}

		// Modifiers
		if (accept(tokenid::const_))
		{
			qualifiers |= type_node::qualifier_const;
		}

		// Interpolation
		if (accept(tokenid::linear))
		{
			qualifiers |= type_node::qualifier_linear;
		}
		if (accept(tokenid::noperspective))
		{
			qualifiers |= type_node::qualifier_noperspective;
		}
		if (accept(tokenid::centroid))
		{
			qualifiers |= type_node::qualifier_centroid;
		}
		if (accept(tokenid::nointerpolation))
		{
			qualifiers |= type_node::qualifier_nointerpolation;
		}

		if (qualifiers == 0)
		{
			return false;
		}
		if ((type.qualifiers & qualifiers) == qualifiers)
		{
			warning(_token.location, 3048, "duplicate usages specified");
		}

		type.qualifiers |= qualifiers;

		accept_type_qualifiers(type);

		return true;
	}
	bool parser::parse_type(type_node &type)
	{
		type.qualifiers = 0;

		accept_type_qualifiers(type);

		const auto location = _token_next.location;

		if (!accept_type_class(type))
		{
			return false;
		}

		if (type.is_integral() && (type.has_qualifier(type_node::qualifier_centroid) || type.has_qualifier(type_node::qualifier_noperspective)))
		{
			error(location, 4576, "signature specifies invalid interpolation mode for integer component type");

			return false;
		}

		if (type.has_qualifier(type_node::qualifier_centroid) && !type.has_qualifier(type_node::qualifier_noperspective))
		{
			type.qualifiers |= type_node::qualifier_linear;
		}

		return true;
	}

	// Expressions
	bool parser::accept_unary_op(spv::Op &op)
	{
		switch (_token_next.id)
		{
		case tokenid::exclaim:
			op = spv::OpLogicalNot;
			break;
		case tokenid::plus:
			op = spv::OpNop;
			break;
		case tokenid::minus:
			op = spv::OpFNegate;
			break;
		case tokenid::tilde:
			op = spv::OpNot;
			break;
		case tokenid::plus_plus:
			op = spv::OpFAdd;
			break;
		case tokenid::minus_minus:
			op = spv::OpFSub;
			break;
		default:
			return false;
		}

		consume();

		return true;
	}
	bool parser::accept_postfix_op(spv::Op &op)
	{
		switch (_token_next.id)
		{
		case tokenid::plus_plus:
			op = spv::OpFAdd;
			break;
		case tokenid::minus_minus:
			op = spv::OpFSub;
			break;
		default:
			return false;
		}

		consume();

		return true;
	}
	bool parser::peek_multary_op(spv::Op &op, unsigned int &precedence) const
	{
		switch (_token_next.id)
		{
		case tokenid::percent:
			op = spv::OpFMod;
			precedence = 11;
			break;
		case tokenid::ampersand:
			op = spv::OpBitwiseAnd;
			precedence = 6;
			break;
		case tokenid::star:
			op = spv::OpFMul;
			precedence = 11;
			break;
		case tokenid::plus:
			op = spv::OpFAdd;
			precedence = 10;
			break;
		case tokenid::minus:
			op = spv::OpFSub;
			precedence = 10;
			break;
		case tokenid::slash:
			op = spv::OpFDiv;
			precedence = 11;
			break;
		case tokenid::less:
			op = spv::OpFOrdLessThan;
			precedence = 8;
			break;
		case tokenid::greater:
			op = spv::OpFOrdGreaterThan;
			precedence = 8;
			break;
		case tokenid::question:
			op = spv::OpSelect;
			precedence = 1;
			break;
		case tokenid::caret:
			op = spv::OpBitwiseXor;
			precedence = 5;
			break;
		case tokenid::pipe:
			op = spv::OpBitwiseOr;
			precedence = 4;
			break;
		case tokenid::exclaim_equal:
			op = spv::OpLogicalNotEqual;
			precedence = 7;
			break;
		case tokenid::ampersand_ampersand:
			op = spv::OpLogicalAnd;
			precedence = 3;
			break;
		case tokenid::less_less:
			op = spv::OpShiftLeftLogical;
			precedence = 9;
			break;
		case tokenid::less_equal:
			op = spv::OpFOrdLessThanEqual;
			precedence = 8;
			break;
		case tokenid::equal_equal:
			op = spv::OpLogicalEqual;
			precedence = 7;
			break;
		case tokenid::greater_greater:
			op = spv::OpShiftRightLogical;
			precedence = 9;
			break;
		case tokenid::greater_equal:
			op = spv::OpFOrdGreaterThanEqual;
			precedence = 8;
			break;
		case tokenid::pipe_pipe:
			op = spv::OpLogicalOr;
			precedence = 2;
			break;
		default:
			return false;
		}

		return true;
	}
	bool parser::accept_assignment_op(spv::Op &op)
	{
		switch (_token_next.id)
		{
		case tokenid::equal:
			op = spv::OpNop;
			break;
		case tokenid::percent_equal:
			op = spv::OpFMod;
			break;
		case tokenid::ampersand_equal:
			op = spv::OpBitwiseAnd;
			break;
		case tokenid::star_equal:
			op = spv::OpFMul;
			break;
		case tokenid::plus_equal:
			op = spv::OpFAdd;
			break;
		case tokenid::minus_equal:
			op = spv::OpFSub;
			break;
		case tokenid::slash_equal:
			op = spv::OpFDiv;
			break;
		case tokenid::less_less_equal:
			op = spv::OpShiftLeftLogical;
			break;
		case tokenid::greater_greater_equal:
			op = spv::OpShiftRightLogical;
			break;
		case tokenid::caret_equal:
			op = spv::OpBitwiseXor;
			break;
		case tokenid::pipe_equal:
			op = spv::OpBitwiseOr;
			break;
		default:
			return false;
		}

		consume();

		return true;
	}

	bool parser::parse_expression(spv_section &section, spv::Id &node_id)
	{
		if (!parse_expression_assignment(section, node_id))
			return false;

		// Continue parsing if an expression sequence is next
		// The last expression is the result, so keep on passing in 'node_id'
		while (accept(','))
			if (!parse_expression_assignment(section, node_id))
				return false;

		return true;
	}
	bool parser::parse_expression_unary(spv_section &section, spv::Id &node_id)
	{
		type_node type;
		spv::Op op;
		auto location = _token_next.location;

		#pragma region Prefix
		if (accept_unary_op(op))
		{
			if (!parse_expression_unary(section, node_id))
			{
				return false;
			}

			const auto &node = lookup_id(node_id);

			if (!node.type.is_scalar() && !node.type.is_vector() && !node.type.is_matrix())
			{
				error(node.location, 3022, "scalar, vector, or matrix expected");
				return false;
			}

			if (op != spv::OpNop)
			{
				spv::Id right = 0;

				if (op == spv::OpNot && !node.type.is_integral())
				{
					error(node.location, 3082, "int or unsigned int type required");

					return false;
				}
				else if ((op == spv::OpFAdd || op == spv::OpFSub) && (node.type.has_qualifier(type_node::qualifier_const) || node.type.has_qualifier(type_node::qualifier_uniform)))
				{
					right = literal_1_float; // TODO
					error(node.location, 3025, "l-value specifies const object");
					return false;
				}

				auto &newexpression = add_node(section, location, op, node.result_type);
				newexpression.add(node_id);
				newexpression.add(right);

				newexpression.type = lookup_id(node_id).type;

				node_id = fold_constant_expression(newexpression.result);
			}

			type = lookup_id(node_id).type;
		}
		else if (accept('('))
		{
			backup();

			if (accept_type_class(type))
			{
				if (peek('('))
				{
					restore();
				}
				else if (expect(')'))
				{
					if (!parse_expression_unary(section, node_id))
					{
						return false;
					}

					const auto &node = lookup_id(node_id);

					if (node.type.basetype == type.basetype && (node.type.rows == type.rows && node.type.cols == type.cols) && !(node.type.is_array() || type.is_array()))
					{
						return true;
					}
					else if (node.type.is_numeric() && type.is_numeric())
					{
						if ((node.type.rows < type.rows || node.type.cols < type.cols) && !node.type.is_scalar())
						{
							error(location, 3017, "cannot convert these vector types");
							return false;
						}

						if (node.type.rows > type.rows || node.type.cols > type.cols)
						{
							warning(location, 3206, "implicit truncation of vector type");
						}

						//auto &castexpression = _ast.make_node<unary_expression_node>(location);
						//type.qualifiers = type_node::qualifier_const;
						//castexpression->type = type;
						//castexpression->op = unary_expression_node::cast;
						//castexpression->operand = node;

						//node = fold_constant_expression(_ast, castexpression.result);

						//return true;
						// TODO
						error(location, 0, "CAST NOT IMPLEMENTED");

						return false;
					}
					else
					{
						error(location, 3017, "cannot convert non-numeric types");

						return false;
					}
				}
				else
				{
					return false;
				}
			}

			if (!parse_expression(section, node_id))
				return false;

			if (!expect(')'))
				return false;

			type = lookup_id(node_id).type;
		}
		else if (accept(tokenid::true_literal))
		{
			auto &literal = add_node(_variables, location, spv::OpConstantTrue, type_bool);

			literal.type.basetype = type_node::datatype_bool;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 1, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept(tokenid::false_literal))
		{
			auto &literal = add_node(_variables, location, spv::OpConstantFalse, type_bool);

			literal.type.basetype = type_node::datatype_bool;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 1, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept(tokenid::int_literal))
		{
			auto &literal = add_node(_variables, location, spv::OpConstant, type_int);
			literal.add(_token.literal_as_int);

			literal.type.basetype = type_node::datatype_int;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 1, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept(tokenid::uint_literal))
		{
			auto &literal = add_node(_variables, location, spv::OpConstant, type_uint);
			literal.add(_token.literal_as_uint);

			literal.type.basetype = type_node::datatype_uint;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 1, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept(tokenid::float_literal))
		{
			auto &literal = add_node(_variables, location, spv::OpConstant, type_float);
			literal.add(_token.literal_as_uint); // Interpret float bit pattern as int

			literal.type.basetype = type_node::datatype_float;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 1, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept(tokenid::string_literal))
		{
			std::string value = _token.literal_as_string;

			while (accept(tokenid::string_literal))
			{
				value += _token.literal_as_string;
			}

			if (value.size() / 4 >= 5)
			{
				error(location, 0, "STRING TOO BIG");
				return false;
			}

			auto &literal = add_node(_strings, location, spv::OpString);
			literal.add_string(value.c_str());

			literal.type.basetype = type_node::datatype_string;
			literal.type.qualifiers = type_node::qualifier_const;
			literal.type.rows = literal.type.cols = 0, literal.type.array_length = 0;

			node_id = literal.result;
			type = literal.type;
		}
		else if (accept_type_class(type))
		{
			if (!expect('('))
			{
				return false;
			}

			if (!type.is_numeric())
			{
				error(location, 3037, "constructors only defined for numeric base types");

				return false;
			}

			if (accept(')'))
			{
				error(location, 3014, "incorrect number of arguments to numeric-type constructor");

				return false;
			}

			auto &constructor = add_node(section, location, spv::OpCompositeConstruct, 0xFF); // TODO
			error(location, 0, "NOT IMPLEMENTED");
			return false;
#if 0
			constructor.type = type;
			constructor.type.qualifiers = type_node::qualifier_const;

			unsigned int num_elements = 0;
			unsigned int num_arguments = 0;

			while (!peek(')'))
			{
				if (num_arguments != 0 && !expect(','))
				{
					return false;
				}

				spv::Id argument = 0;

				if (!parse_expression_assignment(argument))
				{
					return false;
				}

				if (!lookup_id(argument).type.is_numeric())
				{
					error(lookup_id(argument).location, 3017, "cannot convert non-numeric types");

					return false;
				}

				num_elements += lookup_id(argument).type.rows * lookup_id(argument).type.cols;

				constructor.operands[num_arguments++] = argument;
			}

			if (!expect(')'))
			{
				return false;
			}

			if (num_elements != type.rows * type.cols)
			{
				error(location, 3014, "incorrect number of arguments to numeric-type constructor");

				return false;
			}

			if (num_arguments > 1)
			{
				node_id = constructor.result;
				type = constructor.type;
			}
			else
			{
				// TODO
				// OpConvertFToU OpConvertFToS OpConvertSToF OpConvertUToF OpUConvert OpSConvert OpFConvert

				//const auto castexpression = _ast.make_node<unary_expression_node>(constructor->location);
				//castexpression->type = type;
				//castexpression->op = unary_expression_node::cast;
				//castexpression->operand = constructor->arguments[0];

				//node = castexpression;

				error(location, 0, "CAST NOT IMPLEMENTED");
				return false;
			}

			node_id = fold_constant_expression(_ast, node_id);
#endif
		}
		else
		{
			scope scope;
			bool exclusive;
			std::string identifier;

			if (accept(tokenid::colon_colon))
			{
				scope.name = "::";
				scope.namespace_level = scope.level = 0;
				exclusive = true;
			}
			else
			{
				scope = _symbol_table->current_scope();
				exclusive = false;
			}

			if (exclusive ? expect(tokenid::identifier) : accept(tokenid::identifier))
			{
				identifier = _token.literal_as_string;
			}
			else
			{
				return false;
			}

			while (accept(tokenid::colon_colon))
			{
				if (!expect(tokenid::identifier))
				{
					return false;
				}

				identifier += "::" + _token.literal_as_string;
			}

			symbol symbol = _symbol_table->find(identifier, scope, exclusive);

			if (accept('('))
			{
				if (symbol && lookup_id(symbol).op != spv::OpFunction)
				{
					error(location, 3005, "identifier '" + identifier + "' represents a variable, not a function");

					return false;
				}

				std::vector<spv::Id> arguments;
				std::vector<type_node> argument_types;

				while (!peek(')'))
				{
					if (!arguments.empty() && !expect(','))
					{
						return false;
					}

					spv::Id argument = 0;

					if (!parse_expression_assignment(section, argument))
					{
						return false;
					}

					arguments.push_back(std::move(argument));
					argument_types.push_back(lookup_id(argument).type);
				}

				if (!expect(')'))
				{
					return false;
				}

				spv::Id return_type = 0;
				bool undeclared = !!symbol, ambiguous = false;

				if (!_symbol_table->resolve_call(identifier, argument_types, scope, ambiguous, op, symbol, return_type))
				{
					if (undeclared && op == spv::OpFunctionCall)
					{
						error(location, 3004, "undeclared identifier '" + identifier + "'");
					}
					else if (ambiguous)
					{
						error(location, 3067, "ambiguous function call to '" + identifier + "'");
					}
					else
					{
						error(location, 3013, "no matching function overload for '" + identifier + "'");
					}

					return false;
				}

				if (op != spv::OpFunctionCall)
				{
					auto &newexpression = add_node(section, location, op, return_type);

					if (op == spv::OpExtInst)
					{
						newexpression.add(1); // GLSL extended instruction set
						newexpression.add(symbol);
					}

					for (size_t i = 0; i < arguments.size(); ++i)
					{
						newexpression.add(arguments[i]);
					}

					node_id = fold_constant_expression(newexpression.result);
				}
				else
				{
					const auto parent = _symbol_table->current_parent();

					if (parent == symbol)
					{
						error(location, 3500, "recursive function calls are not allowed");

						return false;
					}

					auto &callexpression = add_node(section, location, spv::OpFunctionCall, return_type);
					callexpression.add(symbol);

					for (size_t i = 0; i < arguments.size(); ++i)
					{
						callexpression.add(arguments[i]);
					}

					node_id = callexpression.result;
				}

				type = lookup_id(node_id).type;

				// TODO
				//for (size_t i = 0; i < arguments.size(); ++i)
				//{
				//	const spv::Id argument = arguments[i];
				//	const auto parameter = callexpression->callee->parameter_list[i];

				//	if (argument->type.rows > parameter->type.rows || argument->type.cols > parameter->type.cols)
				//	{
				//		warning(argument->location, 3206, "implicit truncation of vector type");
				//	}
				//}
			}
			else
			{
				if (!symbol)
				{
					error(location, 3004, "undeclared identifier '" + identifier + "'");

					return false;
				}

				if (lookup_id(symbol).op != spv::OpVariable)
				{
					error(location, 3005, "identifier '" + identifier + "' represents a function, not a variable");

					return false;
				}

				// TODO Result
				auto &newexpression = add_node(section, location, spv::OpLoad, 0xFF);
				newexpression.add(symbol);

				//newexpression->reference = static_cast<const variable_declaration_node *>(symbol);
				//newexpression->type = newexpression->reference->type;

				node_id = fold_constant_expression(newexpression.result);
				type = lookup_id(node_id).type;
			}
		}
		#pragma endregion

		#pragma region Postfix
		while (!peek(tokenid::end_of_file))
		{
			location = _token_next.location;

			if (accept_postfix_op(op))
			{
				const auto &node = lookup_id(node_id);

				if (!type.is_scalar() && !type.is_vector() && !type.is_matrix())
				{
					error(node.location, 3022, "scalar, vector, or matrix expected");

					return false;
				}

				if (type.has_qualifier(type_node::qualifier_const) || type.has_qualifier(type_node::qualifier_uniform))
				{
					error(node.location, 3025, "l-value specifies const object");

					return false;
				}

				auto &newexpression = add_node(section, location, op, node.result_type);
				newexpression.add(node_id);
				newexpression.add(literal_1_float); // TODO

				newexpression.type = type;
				newexpression.type.qualifiers |= type_node::qualifier_const;

				node_id = newexpression.result;
				type = newexpression.type;
			}
			/*else if (accept('.'))
			{
				if (!expect(tokenid::identifier))
				{
					return false;
				}

				location = _token.location;
				const auto subscript = _token.literal_as_string;

				if (accept('('))
				{
					if (!type.is_struct() || type.is_array())
					{
						error(location, 3087, "object does not have methods");
					}
					else
					{
						error(location, 3088, "structures do not have methods");
					}

					return false;
				}
				else if (type.is_array())
				{
					error(location, 3018, "invalid subscript on array");

					return false;
				}
				else if (type.is_vector())
				{
					const size_t length = subscript.size();

					if (length > 4)
					{
						error(location, 3018, "invalid subscript '" + subscript + "', swizzle too long");

						return false;
					}

					bool constant = false;
					signed char offsets[4] = { -1, -1, -1, -1 };
					enum { xyzw, rgba, stpq } set[4];

					for (size_t i = 0; i < length; ++i)
					{
						switch (subscript[i])
						{
							case 'x': offsets[i] = 0, set[i] = xyzw; break;
							case 'y': offsets[i] = 1, set[i] = xyzw; break;
							case 'z': offsets[i] = 2, set[i] = xyzw; break;
							case 'w': offsets[i] = 3, set[i] = xyzw; break;
							case 'r': offsets[i] = 0, set[i] = rgba; break;
							case 'g': offsets[i] = 1, set[i] = rgba; break;
							case 'b': offsets[i] = 2, set[i] = rgba; break;
							case 'a': offsets[i] = 3, set[i] = rgba; break;
							case 's': offsets[i] = 0, set[i] = stpq; break;
							case 't': offsets[i] = 1, set[i] = stpq; break;
							case 'p': offsets[i] = 2, set[i] = stpq; break;
							case 'q': offsets[i] = 3, set[i] = stpq; break;
							default:
								error(location, 3018, "invalid subscript '" + subscript + "'");
								return false;
						}

						if (i > 0 && (set[i] != set[i - 1]))
						{
							error(location, 3018, "invalid subscript '" + subscript + "', mixed swizzle sets");

							return false;
						}
						if (static_cast<unsigned int>(offsets[i]) >= type.rows)
						{
							error(location, 3018, "invalid subscript '" + subscript + "', swizzle out of range");

							return false;
						}

						for (size_t k = 0; k < i; ++k)
						{
							if (offsets[k] == offsets[i])
							{
								constant = true;
								break;
							}
						}
					}

					const auto newexpression = _ast.make_node<swizzle_expression_node>(location);
					newexpression->type = type;
					newexpression->type.rows = static_cast<unsigned int>(length);
					newexpression->operand = node;
					newexpression->mask[0] = offsets[0];
					newexpression->mask[1] = offsets[1];
					newexpression->mask[2] = offsets[2];
					newexpression->mask[3] = offsets[3];

					if (constant || type.has_qualifier(type_node::qualifier_uniform))
					{
						newexpression->type.qualifiers |= type_node::qualifier_const;
						newexpression->type.qualifiers &= ~type_node::qualifier_uniform;
					}

					node = fold_constant_expression(_ast, newexpression);
					type = node->type;
				}
				else if (type.is_matrix())
				{
					const size_t length = subscript.size();

					if (length < 3)
					{
						error(location, 3018, "invalid subscript '" + subscript + "'");

						return false;
					}

					bool constant = false;
					signed char offsets[4] = { -1, -1, -1, -1 };
					const unsigned int set = subscript[1] == 'm';
					const char coefficient = static_cast<char>(!set);

					for (size_t i = 0, j = 0; i < length; i += 3 + set, ++j)
					{
						if (subscript[i] != '_' || subscript[i + set + 1] < '0' + coefficient || subscript[i + set + 1] > '3' + coefficient || subscript[i + set + 2] < '0' + coefficient || subscript[i + set + 2] > '3' + coefficient)
						{
							error(location, 3018, "invalid subscript '" + subscript + "'");

							return false;
						}
						if (set && subscript[i + 1] != 'm')
						{
							error(location, 3018, "invalid subscript '" + subscript + "', mixed swizzle sets");

							return false;
						}

						const unsigned int row = subscript[i + set + 1] - '0' - coefficient;
						const unsigned int col = subscript[i + set + 2] - '0' - coefficient;

						if ((row >= type.rows || col >= type.cols) || j > 3)
						{
							error(location, 3018, "invalid subscript '" + subscript + "', swizzle out of range");

							return false;
						}

						offsets[j] = static_cast<unsigned char>(row * 4 + col);

						for (size_t k = 0; k < j; ++k)
						{
							if (offsets[k] == offsets[j])
							{
								constant = true;
								break;
							}
						}
					}

					const auto newexpression = _ast.make_node<swizzle_expression_node>(_token.location);
					newexpression->type = type;
					newexpression->type.rows = static_cast<unsigned int>(length / (3 + set));
					newexpression->type.cols = 1;
					newexpression->operand = node;
					newexpression->mask[0] = offsets[0];
					newexpression->mask[1] = offsets[1];
					newexpression->mask[2] = offsets[2];
					newexpression->mask[3] = offsets[3];

					if (constant || type.has_qualifier(type_node::qualifier_uniform))
					{
						newexpression->type.qualifiers |= type_node::qualifier_const;
						newexpression->type.qualifiers &= ~type_node::qualifier_uniform;
					}

					node = fold_constant_expression(_ast, newexpression);
					type = node->type;
				}
				else if (type.is_struct())
				{
					variable_declaration_node *field = nullptr;

					for (auto currfield : type.definition->field_list)
					{
						if (currfield->name == subscript)
						{
							field = currfield;
							break;
						}
					}

					if (field == nullptr)
					{
						error(location, 3018, "invalid subscript '" + subscript + "'");

						return false;
					}

					const auto newexpression = _ast.make_node<field_expression_node>(location);
					newexpression->type = field->type;
					newexpression->operand = node;
					newexpression->field_reference = field;

					if (type.has_qualifier(type_node::qualifier_uniform))
					{
						newexpression->type.qualifiers |= type_node::qualifier_const;
						newexpression->type.qualifiers &= ~type_node::qualifier_uniform;
					}

					node = newexpression;
					type = node->type;
				}
				else if (type.is_scalar())
				{
					signed char offsets[4] = { -1, -1, -1, -1 };
					const size_t length = subscript.size();

					for (size_t i = 0; i < length; ++i)
					{
						if ((subscript[i] != 'x' && subscript[i] != 'r' && subscript[i] != 's') || i > 3)
						{
							error(location, 3018, "invalid subscript '" + subscript + "'");

							return false;
						}

						offsets[i] = 0;
					}

					const auto newexpression = _ast.make_node<swizzle_expression_node>(location);
					newexpression->type = type;
					newexpression->type.qualifiers |= type_node::qualifier_const;
					newexpression->type.rows = static_cast<unsigned int>(length);
					newexpression->operand = node;
					newexpression->mask[0] = offsets[0];
					newexpression->mask[1] = offsets[1];
					newexpression->mask[2] = offsets[2];
					newexpression->mask[3] = offsets[3];

					node = newexpression;
					type = node->type;
				}
				else
				{
					error(location, 3018, "invalid subscript '" + subscript + "'");

					return false;
				}
			}
			else if (accept('['))
			{
				if (!type.is_array() && !type.is_vector() && !type.is_matrix())
				{
					error(node->location, 3121, "array, matrix, vector, or indexable object type expected in index expression");

					return false;
				}

				const auto newexpression = _ast.make_node<binary_expression_node>(location);
				newexpression->type = type;
				newexpression->op = binary_expression_node::element_extract;
				newexpression->operands[0] = node;

				if (!parse_expression(newexpression->operands[1]))
				{
					return false;
				}

				if (!newexpression->operands[1]->type.is_scalar())
				{
					error(newexpression->operands[1]->location, 3120, "invalid type for index - index must be a scalar");

					return false;
				}

				if (type.is_array())
				{
					newexpression->type.array_length = 0;
				}
				else if (type.is_matrix())
				{
					newexpression->type.rows = newexpression->type.cols;
					newexpression->type.cols = 1;
				}
				else if (type.is_vector())
				{
					newexpression->type.rows = 1;
				}

				node = fold_constant_expression(_ast, newexpression);
				type = node->type;

				if (!expect(']'))
				{
					return false;
				}
			}*/
			else
			{
				break;
			}
		}
		#pragma endregion

		return true;
	}
	bool parser::parse_expression_multary(spv_section &section, spv::Id &left_id, unsigned int left_precedence)
	{
		if (!parse_expression_unary(section, left_id))
		{
			return false;
		}

		spv::Op op;
		unsigned int right_precedence;

		while (peek_multary_op(op, right_precedence))
		{
			if (right_precedence <= left_precedence)
			{
				break;
			}

			consume();

			bool boolean = false;
			spv::Id right_id = 0, right2_id = 0;

			if (op != spv::OpSelect)
			{
				if (!parse_expression_multary(section, right_id, right_precedence))
				{
					return false;
				}

				const auto &left = lookup_id(left_id);
				const auto &right = lookup_id(right_id);

				if (op == spv::OpFOrdEqual || op == spv::OpFOrdNotEqual)
				{
					boolean = true;

					if (left.type.is_array() || right.type.is_array() || left.type.definition != right.type.definition)
					{
						error(right.location, 3020, "type mismatch");
						return false;
					}
				}
				else if (op == spv::OpBitwiseAnd || op == spv::OpBitwiseOr || op == spv::OpBitwiseXor)
				{
					if (!left.type.is_integral())
					{
						error(left.location, 3082, "int or unsigned int type required");
						return false;
					}
					if (!right.type.is_integral())
					{
						error(right.location, 3082, "int or unsigned int type required");
						return false;
					}
				}
				else
				{
					boolean = op == spv::OpLogicalAnd || op == spv::OpLogicalOr ||
						op == spv::OpFOrdLessThan || op == spv::OpFOrdGreaterThan ||
						op == spv::OpFOrdLessThanEqual || op == spv::OpFOrdGreaterThanEqual;

					if (!left.type.is_scalar() && !left.type.is_vector() && !left.type.is_matrix())
					{
						error(left.location, 3022, "scalar, vector, or matrix expected");
						return false;
					}
					if (!right.type.is_scalar() && !right.type.is_vector() && !right.type.is_matrix())
					{
						error(right.location, 3022, "scalar, vector, or matrix expected");
						return false;
					}
				}

				auto &newexpression = add_node(section, left.location, op, 0); // TODO Result Type
				newexpression.add(left_id);
				newexpression.add(right_id);

				right2_id = right_id;
				right_id = left_id;
				left_id = newexpression.result;
			}
			else
			{
				const auto &left = lookup_id(left_id);

				if (!left.type.is_scalar() && !left.type.is_vector())
				{
					error(left.location, 3022, "boolean or vector expression expected");
					return false;
				}

				if (!(parse_expression(section, right_id) && expect(':') && parse_expression_assignment(section, right2_id)))
				{
					return false;
				}

				const auto &right1 = lookup_id(right_id);
				const auto &right2 = lookup_id(right2_id);

				if (right1.type.is_array() || right2.type.is_array() || right1.type.definition != right2.type.definition)
				{
					error(right1.location, 3020, "type mismatch between conditional values");
					return false;
				}

				auto &newexpression = add_node(section, lookup_id(left_id).location, spv::OpSelect, 0); // TODO Result Type
				newexpression.add(left_id);
				newexpression.add(right_id);
				newexpression.add(right2_id);

				left_id = newexpression.result;
			}

			auto &result = lookup_id(left_id);
			const auto &right1 = lookup_id(right_id);
			const auto &right2 = lookup_id(right2_id);

			result.type.basetype = boolean ? type_node::datatype_bool : std::max(right1.type.basetype, right2.type.basetype);

			if ((right1.type.rows == 1 && right1.type.cols == 1) || (right2.type.rows == 1 && right2.type.cols == 1))
			{
				result.type.rows = std::max(right1.type.rows, right2.type.rows);
				result.type.cols = std::max(right1.type.cols, right2.type.cols);
			}
			else
			{
				result.type.rows = std::min(right1.type.rows, right2.type.rows);
				result.type.cols = std::min(right1.type.cols, right2.type.cols);

				if (right1.type.rows > right2.type.rows || right1.type.cols > right2.type.cols)
				{
					warning(right1.location, 3206, "implicit truncation of vector type");
				}
				if (right2.type.rows > right1.type.rows || right2.type.cols > right1.type.cols)
				{
					warning(right2.location, 3206, "implicit truncation of vector type");
				}
			}

			result.result_type = 0; // TODO

			left_id = fold_constant_expression(left_id);
		}

		return true;
	}
	bool parser::parse_expression_assignment(spv_section &section, spv::Id &left_id)
	{
		if (!parse_expression_multary(section, left_id))
		{
			return false;
		}

		spv::Op op;

		if (accept_assignment_op(op))
		{
			spv::Id right_id = 0;

			if (!parse_expression_multary(section, right_id))
			{
				return false;
			}

			const auto &left = lookup_id(left_id);
			const auto &right = lookup_id(right_id);

			if (left.type.has_qualifier(type_node::qualifier_const) || left.type.has_qualifier(type_node::qualifier_uniform))
			{
				error(left.location, 3025, "l-value specifies const object");
				return false;
			}

			if (left.type.is_array() || right.type.is_array() || !type_node::rank(left.type, right.type))
			{
				error(right.location, 3020, "cannot convert these types");
				return false;
			}

			if (right.type.rows > left.type.rows || right.type.cols > left.type.cols)
			{
				warning(right.location, 3206, "implicit truncation of vector type");
			}

			if (op != spv::OpNop)
			{
				auto &newexpression = add_node(section, left.location, op, left.result_type);
				newexpression.add(left_id);
				newexpression.add(right_id);

				newexpression.type = lookup_id(left_id).type;

				left_id = newexpression.result;
			}

			auto &assignment = add_node(section, left.location, spv::OpStore, left.result_type);
			assignment.add(left_id);
			assignment.add(right_id);

			assignment.type = lookup_id(left_id).type;

			left_id = assignment.result;
		}

		return true;
	}

	// Statements
	bool parser::parse_statement(spv_section &section, bool scoped)
	{
		unsigned int loop_control = spv::LoopControlMaskNone;
		unsigned int selection_control = spv::SelectionControlMaskNone;

		// Attributes
		while (accept('['))
		{
			if (expect(tokenid::identifier))
			{
				const auto attribute = _token.literal_as_string;

				if (expect(']'))
				{
					if (attribute == "unroll")
					{
						loop_control |= spv::LoopControlUnrollMask;
					}
					else if (attribute == "flatten")
					{
						selection_control |= spv::SelectionControlFlattenMask;
					}
					else
					{
						warning(_token.location, 0, "unknown attribute");
					}
				}
			}
			else
			{
				accept(']');
			}
		}

		if (peek('{'))
		{
			spv::Id label = 0;

			return parse_statement_block(section, label, scoped);
		}

		if (accept(';'))
		{
			return true;
		}

		#pragma region If
		if (accept(tokenid::if_))
		{
			auto &merge = add_node_without_result(section, _token.location, spv::OpSelectionMerge);
			merge.add(selection_control);
			size_t merge_index = merge.index;

			add_node_without_result(section, _token.location, spv::OpBranchConditional);

			spv::Id condition = 0;

			if (!(expect('(') && parse_expression(section, condition) && expect(')')))
			{
				return false;
			}

			if (!lookup_id(condition).type.is_scalar())
			{
				error(lookup_id(condition).location, 3019, "if statement conditional expressions must evaluate to a scalar");
				return false;
			}

			section.instructions[merge_index].add(condition); // Condition

			spv::Id true_label = add_node(section, _token.location, spv::OpLabel).result;

			section.instructions[merge_index].add(true_label); // True Label

			if (!parse_statement(section))
			{
				return false;
			}

			spv::Id false_label = add_node(section, _token.location, spv::OpLabel).result;

			section.instructions[merge_index].add(false_label); // False Label

			if (accept(tokenid::else_))
			{
				if (!parse_statement(section))
				{
					return false;
				}
			}

			size_t final_branch_index = add_node_without_result(section, _token.location, spv::OpBranch).index;

			spv::Id continue_label = add_node(section, _token.location, spv::OpLabel).result;

			section.instructions[final_branch_index].add(continue_label); // Target Label

			return true;
		}
		#pragma endregion

		#pragma region Switch
#if 0
		if (accept(tokenid::switch_))
		{
			auto &merge = create_node(spv::OpSelectionMerge, _token.location);
			merge.operands[0] = selection_control;

			auto &branch = create_node_without_result(spv::OpSwitch, _token.location);

			spv::Id selector = 0;

			if (!(expect('(') && parse_expression(selector) && expect(')')))
			{
				return false;
			}

			if (!lookup_node(selector).type.is_scalar())
			{
				error(lookup_node(selector).location, 3019, "switch statement expression must evaluate to a scalar");

				return false;
			}

			if (!expect('{'))
			{
				return false;
			}

			while (!peek('}') && !peek(tokenid::end_of_file))
			{
				const auto casenode = _ast.make_node<case_statement_node>(_token_next.location);

				while (accept(tokenid::case_) || accept(tokenid::default_))
				{
					expression_node *label = nullptr;

					if (_token.id == tokenid::case_)
					{
						if (!parse_expression(label))
						{
							return false;
						}

						if (label->id != nodeid::literal_expression || !label->type.is_numeric())
						{
							error(label->location, 3020, "non-numeric case expression");

							return false;
						}
					}

					if (!expect(':'))
					{
						return false;
					}

					casenode->labels.push_back(static_cast<literal_expression_node *>(label));
				}

				if (casenode->labels.empty())
				{
					error(casenode->location, 3000, "a case body can only contain a single statement");

					return false;
				}

				if (!parse_statement(casenode->statement_list))
				{
					return false;
				}

				newstatement->case_list.push_back(casenode);
			}

			//if (newstatement->case_list.empty())
			//{
			//	warning(newstatement->location, 5002, "switch statement contains no 'case' or 'default' labels");

			//	statement = nullptr;
			//}
			//else
			//{
			//	statement = newstatement;
			//}

			return expect('}');
		}
#endif
		#pragma endregion

#if 0
		#pragma region For
		if (accept(tokenid::for_))
		{
			const auto newstatement = _ast.make_node<for_statement_node>(_token.location);
			newstatement->attributes = attributes;

			if (!expect('('))
			{
				return false;
			}

			_symbol_table->enter_scope();

			if (!parse_statement_declarator_list(newstatement->init_statement))
			{
				expression_node *expression = nullptr;

				if (parse_expression(expression))
				{
					const auto initialization = _ast.make_node<expression_statement_node>(expression->location);
					initialization->expression = expression;

					newstatement->init_statement = initialization;
				}
			}

			if (!expect(';'))
			{
				_symbol_table->leave_scope();

				return false;
			}

			parse_expression(newstatement->condition);

			if (!expect(';'))
			{
				_symbol_table->leave_scope();

				return false;
			}

			parse_expression(newstatement->increment_expression);

			if (!expect(')'))
			{
				_symbol_table->leave_scope();

				return false;
			}

			if (!newstatement->condition->type.is_scalar())
			{
				error(newstatement->condition->location, 3019, "scalar value expected");

				return false;
			}

			if (!parse_statement(newstatement->statement_list, false))
			{
				_symbol_table->leave_scope();

				return false;
			}

			_symbol_table->leave_scope();

			statement = newstatement;

			return true;
		}
		#pragma endregion

		#pragma region While
		if (accept(tokenid::while_))
		{
			const auto newstatement = _ast.make_node<while_statement_node>(_token.location);
			newstatement->attributes = attributes;
			newstatement->is_do_while = false;

			_symbol_table->enter_scope();

			if (!(expect('(') && parse_expression(newstatement->condition) && expect(')')))
			{
				_symbol_table->leave_scope();

				return false;
			}

			if (!newstatement->condition->type.is_scalar())
			{
				error(newstatement->condition->location, 3019, "scalar value expected");

				_symbol_table->leave_scope();

				return false;
			}

			if (!parse_statement(newstatement->statement_list, false))
			{
				_symbol_table->leave_scope();

				return false;
			}

			_symbol_table->leave_scope();

			statement = newstatement;

			return true;
		}
		#pragma endregion

		#pragma region DoWhile
		if (accept(tokenid::do_))
		{
			const auto newstatement = _ast.make_node<while_statement_node>(_token.location);
			newstatement->attributes = attributes;
			newstatement->is_do_while = true;

			if (!(parse_statement(newstatement->statement_list) && expect(tokenid::while_) && expect('(') && parse_expression(newstatement->condition) && expect(')') && expect(';')))
			{
				return false;
			}

			if (!newstatement->condition->type.is_scalar())
			{
				error(newstatement->condition->location, 3019, "scalar value expected");

				return false;
			}

			statement = newstatement;

			return true;
		}
		#pragma endregion

		#pragma region Break
		if (accept(tokenid::break_))
		{
			const auto newstatement = _ast.make_node<jump_statement_node>(_token.location);
			newstatement->attributes = attributes;
			newstatement->is_break = true;

			statement = newstatement;

			return expect(';');
		}
		#pragma endregion

		#pragma region Continue
		if (accept(tokenid::continue_))
		{
			const auto newstatement = _ast.make_node<jump_statement_node>(_token.location);
			newstatement->attributes = attributes;
			newstatement->is_continue = true;

			statement = newstatement;

			return expect(';');
		}
		#pragma endregion
#endif

		#pragma region Return
		if (accept(tokenid::return_))
		{
			const auto parent = _symbol_table->current_parent();
			const auto location = _token.location;

			if (!peek(';'))
			{
				spv::Id return_value = 0;

				if (!parse_expression(section, return_value))
				{
					return false;
				}

#if 0
				if (parent->return_type.is_void())
				{
					error(location, 3079, "void functions cannot return a value");

					accept(';');

					return false;
				}

				if (!type_node::rank(lookup_id(return_value).type, parent->return_type))
				{
					error(location, 3017, "expression does not match function return type");

					return false;
				}

				if (lookup_id(return_value).type.rows > parent->return_type.rows || lookup_id(return_value).type.cols > parent->return_type.cols)
				{
					warning(location, 3206, "implicit truncation of vector type");
				}
#endif

				auto &node = add_node_without_result(section, location, spv::OpReturnValue);
				node.add(return_value);
			}
#if 0
			else if (!parent->return_type.is_void())
			{
				error(location, 3080, "function must return a value");

				accept(';');

				return false;
			}
#endif
			else
			{
				add_node_without_result(section, location, spv::OpReturn);
			}

			return expect(';');
		}
		#pragma endregion

		#pragma region Discard
		if (accept(tokenid::discard_))
		{
			add_node_without_result(section, _token.location, spv::OpKill);

			return expect(';');
		}
		#pragma endregion

		#pragma region Declaration
		const auto location = _token_next.location;

		if (type_node type; parse_type(type))
		{
			unsigned int count = 0;

			do
			{
				if (count++ > 0 && !expect(','))
				{
					return false;
				}

				if (!expect(tokenid::identifier))
				{
					return false;
				}

				spv::Id variable_id = 0;

				if (!parse_variable_declaration(section, type, _token.literal_as_string, variable_id))
				{
					return false;
				}
			}
			while (!peek(';'));

			return expect(';');
		}
		#pragma endregion

		#pragma region Expression
		
		if (spv::Id expression_id; parse_expression(section, expression_id))
		{
			return expect(';');
		}
		#pragma endregion

		error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "'");

		consume_until(';');

		return false;
	}
	bool parser::parse_statement_block(spv_section &section, spv::Id &label, bool scoped)
	{
		if (!expect('{'))
		{
			return false;
		}

		if (scoped)
		{
			_symbol_table->enter_scope();
		}

		label = add_node(section, _token.location, spv::OpLabel).result;

		while (!peek('}') && !peek(tokenid::end_of_file))
		{
			if (!parse_statement(section))
			{
				if (scoped)
				{
					_symbol_table->leave_scope();
				}

				unsigned level = 0;

				while (!peek(tokenid::end_of_file))
				{
					if (accept('{'))
					{
						++level;
					}
					else if (accept('}'))
					{
						if (level-- == 0)
						{
							break;
						}
					}
					else
					{
						consume();
					}
				}

				return false;
			}
		}

		if (scoped)
		{
			_symbol_table->leave_scope();
		}

		return expect('}');
	}

	// Declarations
	bool parser::parse_top_level()
	{
		type_node type = { type_node::datatype_void };

		if (peek(tokenid::namespace_))
		{
			return parse_namespace();
		}
		else if (peek(tokenid::struct_))
		{
			spv::Id type_id = 0;

			if (!parse_struct(type_id))
			{
				return false;
			}

			if (!expect(';'))
			{
				return false;
			}
		}
		else if (peek(tokenid::technique))
		{
			technique_properties technique;

			if (!parse_technique(technique))
			{
				return false;
			}

			techniques.push_back(std::move(technique));
		}
		else if (parse_type(type))
		{
			if (!expect(tokenid::identifier))
			{
				return false;
			}

			if (peek('('))
			{
				spv::Id function_id = 0;

				if (!parse_function_declaration(type, _token.literal_as_string, function_id))
				{
					return false;
				}
			}
			else
			{
				unsigned int count = 0;

				do
				{
					if (count++ > 0 && !(expect(',') && expect(tokenid::identifier)))
					{
						return false;
					}

					spv::Id variable_id = 0;

					if (!parse_variable_declaration(_variables, type, _token.literal_as_string, variable_id, true))
					{
						consume_until(';');

						return false;
					}
				}
				while (!peek(';'));

				if (!expect(';'))
				{
					return false;
				}
			}
		}
		else if (!accept(';'))
		{
			consume();

			error(_token.location, 3000, "syntax error: unexpected '" + get_token_name(_token.id) + "'");

			return false;
		}

		return true;
	}
	bool parser::parse_namespace()
	{
		if (!accept(tokenid::namespace_))
		{
			return false;
		}

		if (!expect(tokenid::identifier))
		{
			return false;
		}

		const auto name = _token.literal_as_string;

		if (!expect('{'))
		{
			return false;
		}

		_symbol_table->enter_namespace(name);

		bool success = true;

		while (!peek('}'))
		{
			if (!parse_top_level())
			{
				success = false;
				break;
			}
		}

		_symbol_table->leave_namespace();

		return success && expect('}');
	}

	bool parser::parse_array(int &size)
	{
		size = 0;

#if 0
		if (accept('['))
		{
			spv::Id expression_id = 0;

			if (accept(']'))
			{
				size = -1;
			}
			else if (parse_expression(expression_id) && expect(']'))
			{
				if (expression->id != nodeid::literal_expression || !(expression->type.is_scalar() && expression->type.is_integral()))
				{
					error(expression->location, 3058, "array dimensions must be literal scalar expressions");

					return false;
				}

				size = static_cast<literal_expression_node *>(expression)->value_int[0];

				if (size < 1 || size > 65536)
				{
					error(expression->location, 3059, "array dimension must be between 1 and 65536");

					return false;
				}
			}
			else
			{
				return false;
			}
		}
#endif

		return true;
	}
	bool parser::parse_annotations(std::unordered_map<std::string, reshade::variant> &annotations)
	{
		if (!accept('<'))
		{
			return true;
		}

		while (!peek('>'))
		{
			type_node type;

			if (accept_type_class(type))
			{
				warning(_token.location, 4717, "type prefixes for annotations are deprecated");
			}

			if (!expect(tokenid::identifier))
			{
				return false;
			}

			const auto name = _token.literal_as_string;
			spv::Id expression_id = 0;

			if (!(expect('=') && parse_expression_unary(_temporary, expression_id) && expect(';')))
			{
				return false;
			}

			const auto &expression = lookup_id(expression_id);

			if (expression.op != spv::OpConstant)
			{
				error(expression.location, 3011, "value must be a literal expression");

				continue;
			}

			//switch (expression.type.basetype)
			//{
			//	case type_node::datatype_int:
			//		annotations[name] = expression->value_int;
			//		break;
			//	case type_node::datatype_bool:
			//	case type_node::datatype_uint:
			//		annotations[name] = expression->value_uint;
			//		break;
			//	case type_node::datatype_float:
			//		annotations[name] = expression->value_float;
			//		break;
			//	case type_node::datatype_string:
			//		annotations[name] = expression->value_string;
			//		break;
			//}
		}

		return expect('>');
	}

	bool parser::parse_struct(spv::Id &type_id)
	{
		if (!accept(tokenid::struct_))
		{
			return false;
		}

		const auto location = _token.location;

		std::string name;

		if (accept(tokenid::identifier))
		{
			name = _token.literal_as_string;
		}
		else
		{
			name = "__anonymous_struct_" + std::to_string(location.line) + '_' + std::to_string(location.column);
		}

		//structure->unique_name = 'S' + _symbol_table->current_scope().name + structure->name;
		//std::replace(structure->unique_name.begin(), structure->unique_name.end(), ':', '_');

		if (!expect('{'))
		{
			return false;
		}

		std::vector<spv::Id> field_list;

		while (!peek('}'))
		{
			type_node type;

			if (!parse_type(type))
			{
				error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "', expected struct member type");

				consume_until('}');

				return false;
			}

			if (type.is_void())
			{
				error(_token_next.location, 3038, "struct members cannot be void");

				consume_until('}');

				return false;
			}
			if (type.has_qualifier(type_node::qualifier_in) || type.has_qualifier(type_node::qualifier_out))
			{
				error(_token_next.location, 3055, "struct members cannot be declared 'in' or 'out'");

				consume_until('}');

				return false;
			}

			unsigned int count = 0;

			do
			{
				if (count++ > 0 && !expect(','))
				{
					consume_until('}');

					return false;
				}

				if (!expect(tokenid::identifier))
				{
					consume_until('}');

					return false;
				}

				spv::Id field = 0; // TODO
				//auto &field = add_node(_variables, _token.location, spv::OpType
				//const auto field = _ast.make_node<variable_declaration_node>(_token.location);
				//field->unique_name = field->name = _token.literal_as_string;
				//field->type = type;

				//if (!parse_array(field->type.array_length))
				//{
				//	return false;
				//}

				//if (accept(':'))
				//{
				//	if (!expect(tokenid::identifier))
				//	{
				//		consume_until('}');

				//		return false;
				//	}

				//	field->semantic = _token.literal_as_string;
				//	std::transform(field->semantic.begin(), field->semantic.end(), field->semantic.begin(), ::toupper);
				//}

				field_list.push_back(std::move(field));
			}
			while (!peek(';'));

			if (!expect(';'))
			{
				consume_until('}');

				return false;
			}
		}

		if (field_list.empty())
		{
			warning(location, 5001, "struct has no members");

			auto &structure = add_node(_variables, _token.location, spv::OpTypeOpaque);

			type_id = structure.result;
		}
		else
		{
			auto &structure = add_node(_variables, _token.location, spv::OpTypeStruct);


			//structure.operands

			type_id = structure.result;
		}

		if (!_symbol_table->insert(name, type_id, spv::OpTypeStruct, nullptr, true))
		{
			error(_token.location, 3003, "redefinition of '" + name + "'");

			return false;
		}

		return expect('}');
	}

	bool parser::parse_function_declaration(type_node &type, std::string name, spv::Id &function_id)
	{
		const auto location = _token.location;

		if (!expect('('))
		{
			return false;
		}

		if (type.qualifiers != 0)
		{
			error(location, 3047, "function return type cannot have any qualifiers");

			return false;
		}

		auto &function = add_node(_functions, location, spv::OpFunction, lookup_type(type)); // TODO
		function.add(spv::FunctionControlInlineMask); // Function Control
		function.add(-1); // Function Type

		function_id = function.result;

		function.type = type;
		function.type.qualifiers = type_node::qualifier_const;
		//function->name = name;

		//function->unique_name = 'F' + _symbol_table->current_scope().name + function->name;
		//std::replace(function->unique_name.begin(), function->unique_name.end(), ':', '_');

		auto props = new function_properties(); // TODO LEEAAK

		_symbol_table->insert(name, function_id, spv::OpTypeFunction, props, true);

		_symbol_table->enter_scope(function_id);

		unsigned int num_params = 0;

		while (!peek(')'))
		{
			if (num_params != 0 && !expect(','))
			{
				_symbol_table->leave_scope();

				return false;
			}

			type_node param_type;

			if (!parse_type(param_type))
			{
				_symbol_table->leave_scope();

				error(_token_next.location, 3000, "syntax error: unexpected '" + get_token_name(_token_next.id) + "', expected parameter type");

				return false;
			}

			if (!expect(tokenid::identifier))
			{
				_symbol_table->leave_scope();

				return false;
			}

			std::string param_name = _token.literal_as_string;

			props->parameter_list.push_back(param_type);

			auto &param = add_node(_functions, _token.location, spv::OpFunctionParameter, lookup_type(param_type));

			param.type = param_type;

			//parameter->unique_name = parameter->name = _token.literal_as_string;

			if (param.type.is_void())
			{
				error(param.location, 3038, "function parameters cannot be void");

				_symbol_table->leave_scope();

				return false;
			}
			if (param.type.has_qualifier(type_node::qualifier_extern))
			{
				error(param.location, 3006, "function parameters cannot be declared 'extern'");

				_symbol_table->leave_scope();

				return false;
			}
			if (param.type.has_qualifier(type_node::qualifier_static))
			{
				error(param.location, 3007, "function parameters cannot be declared 'static'");

				_symbol_table->leave_scope();

				return false;
			}
			if (param.type.has_qualifier(type_node::qualifier_uniform))
			{
				error(param.location, 3047, "function parameters cannot be declared 'uniform', consider placing in global scope instead");

				_symbol_table->leave_scope();

				return false;
			}

			if (param.type.has_qualifier(type_node::qualifier_out))
			{
				if (param_type.has_qualifier(type_node::qualifier_const))
				{
					error(param.location, 3046, "output parameters cannot be declared 'const'");

					_symbol_table->leave_scope();

					return false;
				}
			}
			else
			{
				param.type.qualifiers |= type_node::qualifier_in;
			}

			if (!parse_array(param.type.array_length))
			{
				return false;
			}

			if (!_symbol_table->insert(param_name, param.result, spv::OpVariable, nullptr))
			{
				error(param.location, 3003, "redefinition of '" + param_name + "'");

				_symbol_table->leave_scope();

				return false;
			}

			if (accept(':'))
			{
				if (!expect(tokenid::identifier))
				{
					_symbol_table->leave_scope();

					return false;
				}

				// TODO
				//parameter->semantic = _token.literal_as_string;
				//std::transform(parameter->semantic.begin(), parameter->semantic.end(), parameter->semantic.begin(), ::toupper);
			}

			num_params++;
		}

		if (!expect(')'))
		{
			_symbol_table->leave_scope();

			return false;
		}

		if (accept(':'))
		{
			if (!expect(tokenid::identifier))
			{
				_symbol_table->leave_scope();

				return false;
			}

			// TODO
			//function->return_semantic = _token.literal_as_string;
			//std::transform(function->return_semantic.begin(), function->return_semantic.end(), function->return_semantic.begin(), ::toupper);

			if (type.is_void())
			{
				error(_token.location, 3076, "void function cannot have a semantic");

				return false;
			}
		}

		spv::Id definition = 0;

		if (!parse_statement_block(_functions, definition, false))
		{
			_symbol_table->leave_scope();

			return false;
		}

		_symbol_table->leave_scope();

		return true;
	}

	bool parser::parse_variable_declaration(spv_section &section, type_node &type, std::string name, spv::Id &variable_id, bool global)
	{
		auto location = _token.location;

		if (type.is_void())
		{
			error(location, 3038, "variables cannot be void");

			return false;
		}
		if (type.has_qualifier(type_node::qualifier_in) || type.has_qualifier(type_node::qualifier_out))
		{
			error(location, 3055, "variables cannot be declared 'in' or 'out'");

			return false;
		}

		const auto parent = _symbol_table->current_parent();

		if (!parent)
		{
			if (!type.has_qualifier(type_node::qualifier_static))
			{
				if (!type.has_qualifier(type_node::qualifier_uniform) && !(type.is_texture() || type.is_sampler()))
				{
					warning(location, 5000, "global variables are considered 'uniform' by default");
				}

				if (type.has_qualifier(type_node::qualifier_const))
				{
					error(location, 3035, "variables which are 'uniform' cannot be declared 'const'");

					return false;
				}

				type.qualifiers |= type_node::qualifier_extern | type_node::qualifier_uniform;
			}
		}
		else
		{
			if (type.has_qualifier(type_node::qualifier_extern))
			{
				error(location, 3006, "local variables cannot be declared 'extern'");

				return false;
			}
			if (type.has_qualifier(type_node::qualifier_uniform))
			{
				error(location, 3047, "local variables cannot be declared 'uniform'");

				return false;
			}

			if (type.is_texture() || type.is_sampler())
			{
				error(location, 3038, "local variables cannot be textures or samplers");

				return false;
			}
		}

		if (!parse_array(type.array_length))
		{
			return false;
		}

		spv::StorageClass storage = spv::StorageClassGeneric;

		if (global)
		{
			//variable->unique_name = (type.has_qualifier(type_node::qualifier_uniform) ? 'U' : 'V') + _symbol_table->current_scope().name + variable->name;
			//std::replace(variable->unique_name.begin(), variable->unique_name.end(), ':', '_');
		}
		else
		{
			//variable->unique_name = variable->name;
			storage = spv::StorageClassFunction;
		}

		if (accept(':'))
		{
			if (!expect(tokenid::identifier))
			{
				return false;
			}

			// TODO
			//variable->semantic = _token.literal_as_string;
			//std::transform(variable->semantic.begin(), variable->semantic.end(), variable->semantic.begin(), ::toupper);

			return true;
		}

		variable_properties props; // TODO
		spv::Id initializer_id = 0;

		if (global && !parse_annotations(props.annotation_list))
		{
			return false;
		}

		if (accept('='))
		{
			location = _token.location;

			if (!parse_variable_assignment(section, initializer_id))
			{
				return false;
			}

			if (!parent && lookup_id(initializer_id).op != spv::OpConstant)
			{
				error(location, 3011, "initial value must be a literal expression");

				return false;
			}

#if 0 // TODO
			if (variable->initializer_expression->id == nodeid::initializer_list && type.is_numeric())
			{
				const auto nullval = _ast.make_node<literal_expression_node>(location);
				nullval->type.basetype = type.basetype;
				nullval->type.qualifiers = type_node::qualifier_const;
				nullval->type.rows = type.rows, nullval->type.cols = type.cols, nullval->type.array_length = 0;

				const auto initializerlist = static_cast<initializer_list_node *>(variable->initializer_expression);

				while (initializerlist->type.array_length < type.array_length)
				{
					initializerlist->type.array_length++;
					initializerlist->values.push_back(nullval);
				}
			}
#endif

			if (!type_node::rank(lookup_id(initializer_id).type, type))
			{
				error(location, 3017, "initial value does not match variable type");

				return false;
			}
			if ((lookup_id(initializer_id).type.rows < type.rows || lookup_id(initializer_id).type.cols < type.cols) && !lookup_id(initializer_id).type.is_scalar())
			{
				error(location, 3017, "cannot implicitly convert these vector types");

				return false;
			}

			if (lookup_id(initializer_id).type.rows > type.rows || lookup_id(initializer_id).type.cols > type.cols)
			{
				warning(location, 3206, "implicit truncation of vector type");
			}
		}
		else if (type.is_numeric())
		{
			if (type.has_qualifier(type_node::qualifier_const))
			{
				error(location, 3012, "missing initial value for '" + name + "'");

				return false;
			}
			else if (!type.has_qualifier(type_node::qualifier_uniform) && !type.is_array())
			{
				initializer_id = literal_0_float; // TODO
			}
		}
		else if (peek('{'))
		{
			if (!parse_variable_properties(props))
			{
				return false;
			}
		}

		if (type.is_sampler() && !props.texture)
		{
			error(location, 3012, "missing 'Texture' property for '" + name + "'");

			return false;
		}

		auto &variable = add_node(_variables, location, spv::OpVariable, lookup_type(type)); // TODO
		variable.add(storage);
		if (initializer_id)
			variable.add(initializer_id);

		variable_id = variable.result;

		if (!_symbol_table->insert(name, variable_id, spv::OpVariable, nullptr, global))
		{
			error(location, 3003, "redefinition of '" + name + "'");

			return false;
		}

		variable.type = type;

		return true;
	}

	bool parser::parse_variable_assignment(spv_section &section, spv::Id &expression)
	{
#if 0 // TODO
		if (accept('{'))
		{
			const auto initializerlist = _ast.make_node<initializer_list_node>(_token.location);

			while (!peek('}'))
			{
				if (!initializerlist->values.empty() && !expect(','))
				{
					return false;
				}

				if (peek('}'))
				{
					break;
				}

				if (!parse_variable_assignment(expression))
				{
					consume_until('}');

					return false;
				}

				if (expression->id == nodeid::initializer_list && static_cast<initializer_list_node *>(expression)->values.empty())
				{
					continue;
				}

				initializerlist->values.push_back(expression);
			}

			if (!initializerlist->values.empty())
			{
				initializerlist->type = initializerlist->values[0]->type;
				initializerlist->type.array_length = static_cast<int>(initializerlist->values.size());
			}

			expression = initializerlist;

			return expect('}');
		}
#endif

		return parse_expression_assignment(section, expression);
	}

	bool parser::parse_variable_properties(variable_properties &props)
	{
		if (!expect('{'))
		{
			return false;
		}

		while (!peek('}'))
		{
			if (!expect(tokenid::identifier))
			{
				return false;
			}

			const auto name = _token.literal_as_string;
			const auto location = _token.location;

			spv::Id value_id = 0;

			if (!(expect('=') && parse_variable_properties_expression(value_id) && expect(';')))
			{
				return false;
			}

			if (name == "Texture")
			{
				if (lookup_id(value_id).op != spv::OpImage)
				{
					error(location, 3020, "type mismatch, expected texture name");

					return false;
				}

				props.texture = value_id;
			}
			else
			{
				if (lookup_id(value_id).op != spv::OpConstant)
				{
					error(location, 3011, "value must be a literal expression");

					return false;
				}

				const auto value_literal = lookup_id(value_id).operands[0]; // TODO

				if (name == "Width")
				{
					//scalar_literal_cast(value_literal, 0, props.width);
				}
				else if (name == "Height")
				{
					//scalar_literal_cast(value_literal, 0, props.height);
				}
				else if (name == "Depth")
				{
					//scalar_literal_cast(value_literal, 0, props.depth);
				}
				else if (name == "MipLevels")
				{
					//scalar_literal_cast(value_literal, 0, props.levels);

					if (props.levels == 0)
					{
						warning(location, 0, "a texture cannot have 0 mipmap levels, changed it to 1");

						props.levels = 1;
					}
				}
				else if (name == "Format")
				{
					unsigned int format = value_literal;
					//scalar_literal_cast(value_literal, 0, format);
					props.format = static_cast<reshade::texture_format>(format);
				}
				else if (name == "SRGBTexture" || name == "SRGBReadEnable")
				{
					props.srgb_texture = value_literal != 0;
				}
				else if (name == "AddressU")
				{
					unsigned address_mode = value_literal;
					//scalar_literal_cast(value_literal, 0, address_mode);
					props.address_u = static_cast<reshade::texture_address_mode>(address_mode);
				}
				else if (name == "AddressV")
				{
					unsigned address_mode = value_literal;
					//scalar_literal_cast(value_literal, 0, address_mode);
					props.address_v = static_cast<reshade::texture_address_mode>(address_mode);
				}
				else if (name == "AddressW")
				{
					unsigned address_mode = value_literal;
					//scalar_literal_cast(value_literal, 0, address_mode);
					props.address_w = static_cast<reshade::texture_address_mode>(address_mode);
				}
				else if (name == "MinFilter")
				{
					unsigned int a = static_cast<unsigned int>(props.filter), b = value_literal;
					//scalar_literal_cast(value_literal, 0, b);

					b = (a & 0x0F) | ((b << 4) & 0x30);
					props.filter = static_cast<reshade::texture_filter>(b);
				}
				else if (name == "MagFilter")
				{
					unsigned int a = static_cast<unsigned int>(props.filter), b = value_literal;
					//scalar_literal_cast(value_literal, 0, b);

					b = (a & 0x33) | ((b << 2) & 0x0C);
					props.filter = static_cast<reshade::texture_filter>(b);
				}
				else if (name == "MipFilter")
				{
					unsigned int a = static_cast<unsigned int>(props.filter), b = value_literal;
					//scalar_literal_cast(value_literal, 0, b);

					b = (a & 0x3C) | (b & 0x03);
					props.filter = static_cast<reshade::texture_filter>(b);
				}
				else if (name == "MinLOD" || name == "MaxMipLevel")
				{
					//scalar_literal_cast(value_literal, 0, props.min_lod);
				}
				else if (name == "MaxLOD")
				{
					//scalar_literal_cast(value_literal, 0, props.max_lod);
				}
				else if (name == "MipLODBias" || name == "MipMapLodBias")
				{
					//scalar_literal_cast(value_literal, 0, props.lod_bias);
				}
				else
				{
					error(location, 3004, "unrecognized property '" + name + "'");

					return false;
				}
			}
		}

		if (!expect('}'))
		{
			return false;
		}

		return true;
	}

	bool parser::parse_technique(technique_properties &technique)
	{
		if (!accept(tokenid::technique))
		{
			return false;
		}

		technique.location = _token.location;

		if (!expect(tokenid::identifier))
		{
			return false;
		}

		technique.name = _token.literal_as_string;
		technique.unique_name = 'T' + _symbol_table->current_scope().name + technique.name;
		std::replace(technique.unique_name.begin(), technique.unique_name.end(), ':', '_');

		if (!parse_annotations(technique.annotation_list))
		{
			return false;
		}

		if (!expect('{'))
		{
			return false;
		}

		while (!peek('}'))
		{
			pass_properties pass;

			if (!parse_technique_pass(pass))
			{
				return false;
			}

			technique.pass_list.push_back(std::move(pass));
		}

		return expect('}');
	}
	bool parser::parse_technique_pass(pass_properties &pass)
	{
		if (!expect(tokenid::pass))
		{
			return false;
		}

		pass.location = _token.location;

		if (accept(tokenid::identifier))
		{
			pass.name = _token.literal_as_string;
		}

		if (!expect('{'))
		{
			return false;
		}

		while (!peek('}'))
		{
			if (!expect(tokenid::identifier))
			{
				return false;
			}

			const auto passstate = _token.literal_as_string;
			const auto location = _token.location;

			spv::Id value_id = 0;

			if (!(expect('=') && parse_technique_pass_expression(value_id) && expect(';')))
			{
				return false;
			}

			if (passstate == "VertexShader" || passstate == "PixelShader")
			{
				if (lookup_id(value_id).op != spv::OpFunction)
				{
					error(location, 3020, "type mismatch, expected function name");

					return false;
				}

				if (passstate[0] != 'V')
					pass.pixel_shader = value_id;
				else
					pass.vertex_shader = value_id;
			}
			else if (passstate.compare(0, 12, "RenderTarget") == 0 && (passstate == "RenderTarget" || (passstate[12] >= '0' && passstate[12] < '8')))
			{
				size_t index = 0;

				if (passstate.size() == 13)
				{
					index = passstate[12] - '0';
				}

				if (lookup_id(value_id).op != spv::OpImage)
				{
					error(location, 3020, "type mismatch, expected texture name");

					return false;
				}

				pass.render_targets[index] = value_id;
			}
			else
			{
				if (lookup_id(value_id).op != spv::OpConstant)
				{
					error(location, 3011, "pass state value must be a literal expression");

					return false;
				}

				const auto value_literal = lookup_id(value_id).operands[0];

				if (passstate == "SRGBWriteEnable")
				{
					pass.srgb_write_enable = value_literal != 0;
				}
				else if (passstate == "BlendEnable")
				{
					pass.blend_enable = value_literal != 0;
				}
				else if (passstate == "StencilEnable")
				{
					pass.stencil_enable = value_literal != 0;
				}
				else if (passstate == "ClearRenderTargets")
				{
					pass.clear_render_targets = value_literal != 0;
				}
				else if (passstate == "RenderTargetWriteMask" || passstate == "ColorWriteMask")
				{
					unsigned int mask = value_literal; // TODO
					//scalar_literal_cast(value_literal, 0, mask);

					pass.color_write_mask = mask & 0xFF;
				}
				else if (passstate == "StencilReadMask" || passstate == "StencilMask")
				{
					unsigned int mask = value_literal; // TODO
					//scalar_literal_cast(value_literal, 0, mask);

					pass.stencil_read_mask = mask & 0xFF;
				}
				else if (passstate == "StencilWriteMask")
				{
					unsigned int mask = value_literal; // TODO
					//scalar_literal_cast(value_literal, 0, mask);

					pass.stencil_write_mask = mask & 0xFF;
				}
				else if (passstate == "BlendOp")
				{
					//scalar_literal_cast(value_literal, 0, pass.blend_op);
				}
				else if (passstate == "BlendOpAlpha")
				{
					//scalar_literal_cast(value_literal, 0, pass.blend_op_alpha);
				}
				else if (passstate == "SrcBlend")
				{
					//scalar_literal_cast(value_literal, 0, pass.src_blend);
				}
				else if (passstate == "SrcBlendAlpha")
				{
					//scalar_literal_cast(value_literal, 0, pass.src_blend_alpha);
				}
				else if (passstate == "DestBlend")
				{
					//scalar_literal_cast(value_literal, 0, pass.dest_blend);
				}
				else if (passstate == "DestBlend")
				{
					//scalar_literal_cast(value_literal, 0, pass.dest_blend_alpha);
				}
				else if (passstate == "StencilFunc")
				{
					//scalar_literal_cast(value_literal, 0, pass.stencil_comparison_func);
				}
				else if (passstate == "StencilRef")
				{
					//scalar_literal_cast(value_literal, 0, pass.stencil_reference_value);
				}
				else if (passstate == "StencilPass" || passstate == "StencilPassOp")
				{
					//scalar_literal_cast(value_literal, 0, pass.stencil_op_pass);
				}
				else if (passstate == "StencilFail" || passstate == "StencilFailOp")
				{
					//scalar_literal_cast(value_literal, 0, pass.stencil_op_fail);
				}
				else if (passstate == "StencilZFail" || passstate == "StencilDepthFail" || passstate == "StencilDepthFailOp")
				{
					//scalar_literal_cast(value_literal, 0, pass.stencil_op_depth_fail);
				}
				else
				{
					error(location, 3004, "unrecognized pass state '" + passstate + "'");

					return false;
				}
			}
		}

		return expect('}');
	}

	bool parser::parse_variable_properties_expression(spv::Id &expression)
	{
		backup();

		if (accept(tokenid::identifier))
		{
			const std::pair<const char *, unsigned int> s_values[] = {
				{ "NONE", 0 },
				{ "POINT", 0 },
				{ "LINEAR", 1 },
				{ "ANISOTROPIC", 3 },
				{ "CLAMP", static_cast<unsigned int>(reshade::texture_address_mode::clamp) },
				{ "WRAP", static_cast<unsigned int>(reshade::texture_address_mode::wrap) },
				{ "REPEAT", static_cast<unsigned int>(reshade::texture_address_mode::wrap) },
				{ "MIRROR", static_cast<unsigned int>(reshade::texture_address_mode::mirror) },
				{ "BORDER", static_cast<unsigned int>(reshade::texture_address_mode::border) },
				{ "R8", static_cast<unsigned int>(reshade::texture_format::r8) },
				{ "R16F", static_cast<unsigned int>(reshade::texture_format::r16f) },
				{ "R32F", static_cast<unsigned int>(reshade::texture_format::r32f) },
				{ "RG8", static_cast<unsigned int>(reshade::texture_format::rg8) },
				{ "R8G8", static_cast<unsigned int>(reshade::texture_format::rg8) },
				{ "RG16", static_cast<unsigned int>(reshade::texture_format::rg16) },
				{ "R16G16", static_cast<unsigned int>(reshade::texture_format::rg16) },
				{ "RG16F", static_cast<unsigned int>(reshade::texture_format::rg16f) },
				{ "R16G16F", static_cast<unsigned int>(reshade::texture_format::rg16f) },
				{ "RG32F", static_cast<unsigned int>(reshade::texture_format::rg32f) },
				{ "R32G32F", static_cast<unsigned int>(reshade::texture_format::rg32f) },
				{ "RGBA8", static_cast<unsigned int>(reshade::texture_format::rgba8) },
				{ "R8G8B8A8", static_cast<unsigned int>(reshade::texture_format::rgba8) },
				{ "RGBA16", static_cast<unsigned int>(reshade::texture_format::rgba16) },
				{ "R16G16B16A16", static_cast<unsigned int>(reshade::texture_format::rgba16) },
				{ "RGBA16F", static_cast<unsigned int>(reshade::texture_format::rgba16f) },
				{ "R16G16B16A16F", static_cast<unsigned int>(reshade::texture_format::rgba16f) },
				{ "RGBA32F", static_cast<unsigned int>(reshade::texture_format::rgba32f) },
				{ "R32G32B32A32F", static_cast<unsigned int>(reshade::texture_format::rgba32f) },
				{ "DXT1", static_cast<unsigned int>(reshade::texture_format::dxt1) },
				{ "DXT3", static_cast<unsigned int>(reshade::texture_format::dxt3) },
				{ "DXT4", static_cast<unsigned int>(reshade::texture_format::dxt5) },
				{ "LATC1", static_cast<unsigned int>(reshade::texture_format::latc1) },
				{ "LATC2", static_cast<unsigned int>(reshade::texture_format::latc2) },
			};

			const auto location = _token.location;
			std::transform(_token.literal_as_string.begin(), _token.literal_as_string.end(), _token.literal_as_string.begin(), ::toupper);

			for (const auto &value : s_values)
			{
				if (value.first == _token.literal_as_string)
				{
					auto &newexpression = add_node(_temporary, location, spv::OpConstant, type_uint);
					newexpression.add(value.second);

					newexpression.type.basetype = type_node::datatype_uint;
					newexpression.type.rows = newexpression.type.cols = 1, newexpression.type.array_length = 0;

					expression = newexpression.result;

					return true;
				}
			}

			restore();
		}

		return parse_expression_multary(_temporary, expression);
	}
	bool parser::parse_technique_pass_expression(spv::Id &expression)
	{
		scope scope;
		bool exclusive;

		if (accept(tokenid::colon_colon))
		{
			scope.namespace_level = scope.level = 0;
			exclusive = true;
		}
		else
		{
			scope = _symbol_table->current_scope();
			exclusive = false;
		}

		if (exclusive ? expect(tokenid::identifier) : accept(tokenid::identifier))
		{
			const std::pair<const char *, unsigned int> s_enums[] = {
				{ "NONE", pass_properties::NONE },
				{ "ZERO", pass_properties::ZERO },
				{ "ONE", pass_properties::ONE },
				{ "SRCCOLOR", pass_properties::SRCCOLOR },
				{ "SRCALPHA", pass_properties::SRCALPHA },
				{ "INVSRCCOLOR", pass_properties::INVSRCCOLOR },
				{ "INVSRCALPHA", pass_properties::INVSRCALPHA },
				{ "DESTCOLOR", pass_properties::DESTCOLOR },
				{ "DESTALPHA", pass_properties::DESTALPHA },
				{ "INVDESTCOLOR", pass_properties::INVDESTCOLOR },
				{ "INVDESTALPHA", pass_properties::INVDESTALPHA },
				{ "ADD", pass_properties::ADD },
				{ "SUBTRACT", pass_properties::SUBTRACT },
				{ "REVSUBTRACT", pass_properties::REVSUBTRACT },
				{ "MIN", pass_properties::MIN },
				{ "MAX", pass_properties::MAX },
				{ "KEEP", pass_properties::KEEP },
				{ "REPLACE", pass_properties::REPLACE },
				{ "INVERT", pass_properties::INVERT },
				{ "INCR", pass_properties::INCR },
				{ "INCRSAT", pass_properties::INCRSAT },
				{ "DECR", pass_properties::DECR },
				{ "DECRSAT", pass_properties::DECRSAT },
				{ "NEVER", pass_properties::NEVER },
				{ "ALWAYS", pass_properties::ALWAYS },
				{ "LESS", pass_properties::LESS },
				{ "GREATER", pass_properties::GREATER },
				{ "LEQUAL", pass_properties::LESSEQUAL },
				{ "LESSEQUAL", pass_properties::LESSEQUAL },
				{ "GEQUAL", pass_properties::GREATEREQUAL },
				{ "GREATEREQUAL", pass_properties::GREATEREQUAL },
				{ "EQUAL", pass_properties::EQUAL },
				{ "NEQUAL", pass_properties::NOTEQUAL },
				{ "NOTEQUAL", pass_properties::NOTEQUAL },
			};

			auto identifier = _token.literal_as_string;
			const auto location = _token.location;
			std::transform(_token.literal_as_string.begin(), _token.literal_as_string.end(), _token.literal_as_string.begin(), ::toupper);

			for (const auto &value : s_enums)
			{
				if (value.first == _token.literal_as_string)
				{
					auto &newexpression = add_node(_temporary, location, spv::OpConstant, type_uint);
					newexpression.add(value.second);

					newexpression.type.basetype = type_node::datatype_uint;
					newexpression.type.rows = newexpression.type.cols = 1, newexpression.type.array_length = 0;

					expression = newexpression.result;

					return true;
				}
			}

			while (accept(tokenid::colon_colon) && expect(tokenid::identifier))
			{
				identifier += "::" + _token.literal_as_string;
			}

			const auto symbol = _symbol_table->find(identifier, scope, exclusive);

			if (!symbol)
			{
				error(location, 3004, "undeclared identifier '" + identifier + "'");

				return false;
			}

			expression = symbol;

			return true;
		}

		return parse_expression_multary(_temporary, expression);
	}
}
