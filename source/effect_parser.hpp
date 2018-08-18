/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#pragma once

#include <memory>
#include "effect_lexer.hpp"
#include "variant.hpp"
#include "source_location.hpp"
#include "runtime_objects.hpp"
#include <spirv.hpp>

namespace reshadefx
{
	enum type_id
	{
		type_void = 20,
		type_bool,
		type_bool2,
		type_bool3,
		type_bool4,
		type_bool2x2,
		type_bool3x3,
		type_bool4x4,
		type_int,
		type_int2,
		type_int3,
		type_int4,
		type_int2x2,
		type_int3x3,
		type_int4x4,
		type_uint,
		type_uint2,
		type_uint3,
		type_uint4,
		type_uint2x2,
		type_uint3x3,
		type_uint4x4,
		type_float,
		type_float2,
		type_float3,
		type_float4,
		type_float2x2,
		type_float3x3,
		type_float4x4,
		type_string,
		type_sampled_texture,
		type_texture,

		literal_0_int, // OpConstantNull
		literal_0_float,
		literal_1_int,
		literal_1_float,
	};

	struct type_node
	{
		enum datatype
		{
			datatype_void,
			datatype_bool,
			datatype_int,
			datatype_uint,
			datatype_float,
			datatype_string,
			datatype_sampler,
			datatype_texture,
			datatype_struct,
		};
		enum qualifier : unsigned int
		{
			// Storage
			qualifier_extern = 1 << 0,
			qualifier_static = 1 << 1,
			qualifier_uniform = 1 << 2,
			qualifier_volatile = 1 << 3,
			qualifier_precise = 1 << 4,
			qualifier_in = 1 << 5,
			qualifier_out = 1 << 6,
			qualifier_inout = qualifier_in | qualifier_out,

			// Modifier
			qualifier_const = 1 << 8,

			// Interpolation
			qualifier_linear = 1 << 10,
			qualifier_noperspective = 1 << 11,
			qualifier_centroid = 1 << 12,
			qualifier_nointerpolation = 1 << 13,
		};

		static unsigned int rank(const type_node &src, const type_node &dst);

		inline bool is_array() const { return array_length != 0; }
		inline bool is_matrix() const { return rows >= 1 && cols > 1; }
		inline bool is_vector() const { return rows > 1 && !is_matrix(); }
		inline bool is_scalar() const { return !is_array() && !is_matrix() && !is_vector() && is_numeric(); }
		inline bool is_numeric() const { return is_boolean() || is_integral() || is_floating_point(); }
		inline bool is_void() const { return basetype == datatype_void; }
		inline bool is_boolean() const { return basetype == datatype_bool; }
		inline bool is_integral() const { return basetype == datatype_int || basetype == datatype_uint; }
		inline bool is_floating_point() const { return basetype == datatype_float; }
		inline bool is_texture() const { return basetype == datatype_texture; }
		inline bool is_sampler() const { return basetype == datatype_sampler; }
		inline bool is_struct() const { return basetype == datatype_struct; }
		inline bool has_qualifier(qualifier qualifier) const { return (qualifiers & qualifier) == qualifier; }

		datatype basetype;
		unsigned int qualifiers;
		unsigned int rows : 4, cols : 4;
		int array_length;
		spv::Id definition;
	};

	struct spv_node
	{
		spv::Op op = spv::OpNop;
		spv::Id result = 0;
		spv::Id result_type = 0;
		std::vector<spv::Id> operands;
		size_t index = -1;
		type_node type = {};
		location location = {};

		explicit spv_node(spv::Op op = spv::OpNop) : op(op) { }
		explicit spv_node(spv::Op op, spv::Id result) : op(op), result_type(result) { }
		explicit spv_node(spv::Op op, spv::Id type, spv::Id result) : op(op), result_type(type), result(result) { }

		spv_node &add(spv::Id operand)
		{
			operands.push_back(operand);
			return *this;
		}
		spv_node &add_string(const char *str)
		{
			uint32_t word = 0;

			while (*str || word & 0x10000000)
			{
				word = 0;

				for (uint32_t i = 0; i < 4 && *str; ++i)
				{
					reinterpret_cast<uint8_t *>(&word)[i] = *str++;
				}

				add(word);
			}

			return *this;
		}
	};

	struct function_properties
	{
		spv::Id return_type;
		std::string name;
		std::vector<type_node> parameter_list;
		std::string return_semantic;
		spv::Id definition;
	};
	struct variable_properties
	{
		std::unordered_map<std::string, reshade::variant> annotation_list;
		spv::Id texture;
		unsigned int width = 1, height = 1, depth = 1, levels = 1;
		bool srgb_texture;
		reshade::texture_format format = reshade::texture_format::rgba8;
		reshade::texture_filter filter = reshade::texture_filter::min_mag_mip_linear;
		reshade::texture_address_mode address_u = reshade::texture_address_mode::clamp;
		reshade::texture_address_mode address_v = reshade::texture_address_mode::clamp;
		reshade::texture_address_mode address_w = reshade::texture_address_mode::clamp;
		float min_lod, max_lod = FLT_MAX, lod_bias;
	};

	struct pass_properties
	{
		enum : unsigned int
		{
			NONE = 0,

			ZERO = 0,
			ONE = 1,
			SRCCOLOR,
			INVSRCCOLOR,
			SRCALPHA,
			INVSRCALPHA,
			DESTALPHA,
			INVDESTALPHA,
			DESTCOLOR,
			INVDESTCOLOR,

			ADD = 1,
			SUBTRACT,
			REVSUBTRACT,
			MIN,
			MAX,

			KEEP = 1,
			REPLACE = 3,
			INCRSAT,
			DECRSAT,
			INVERT,
			INCR,
			DECR,

			NEVER = 1,
			LESS,
			EQUAL,
			LESSEQUAL,
			GREATER,
			NOTEQUAL,
			GREATEREQUAL,
			ALWAYS
		};

		location location;
		std::string name;
		std::unordered_map<std::string, reshade::variant> annotation_list;
		spv::Id render_targets[8] = {};
		spv::Id vertex_shader = 0, pixel_shader = 0;
		bool clear_render_targets = true, srgb_write_enable, blend_enable, stencil_enable;
		unsigned char color_write_mask = 0xF, stencil_read_mask = 0xFF, stencil_write_mask = 0xFF;
		unsigned int blend_op = ADD, blend_op_alpha = ADD, src_blend = ONE, dest_blend = ZERO, src_blend_alpha = ONE, dest_blend_alpha = ZERO;
		unsigned int stencil_comparison_func = ALWAYS, stencil_reference_value, stencil_op_pass = KEEP, stencil_op_fail = KEEP, stencil_op_depth_fail = KEEP;
	};
	struct technique_properties
	{
		location location;
		std::string name, unique_name;
		std::unordered_map<std::string, reshade::variant> annotation_list;
		std::vector<pass_properties> pass_list;
	};

	struct spv_section
	{
		std::vector<spv_node> instructions;
	};

	/// <summary>
	/// A parser for the ReShade FX language.
	/// </summary>
	class parser
	{
	public:
		/// <summary>
		/// Construct a new parser instance.
		/// </summary>
		parser();
		parser(const parser &) = delete;
		~parser();

		parser &operator=(const parser &) = delete;

		/// <summary>
		/// Gets the list of error messages.
		/// </summary>
		const std::string &errors() const { return _errors; }

		/// <summary>
		/// Parse the provided input string.
		/// </summary>
		/// <param name="source">The string to analyze.</param>
		/// <returns>A boolean value indicating whether parsing was successful or not.</returns>
		bool run(const std::string &source);

	private:
		void error(const location &location, unsigned int code, const std::string &message);
		void warning(const location &location, unsigned int code, const std::string &message);

		void backup();
		void restore();

		bool peek(tokenid tokid) const;
		bool peek(char tok) const { return peek(static_cast<tokenid>(tok)); }
		bool peek_multary_op(spv::Op &op, unsigned int &precedence) const;
		void consume();
		void consume_until(tokenid tokid);
		void consume_until(char tok) { return consume_until(static_cast<tokenid>(tok)); }
		bool accept(tokenid tokid);
		bool accept(char tok) { return accept(static_cast<tokenid>(tok)); }
		bool accept_type_class(type_node &type);
		bool accept_type_qualifiers(type_node &type);
		bool accept_unary_op(spv::Op &op);
		bool accept_postfix_op(spv::Op &op);
		bool accept_assignment_op(spv::Op &op);
		bool expect(tokenid tokid);
		bool expect(char tok) { return expect(static_cast<tokenid>(tok)); }

		bool parse_top_level();
		bool parse_namespace();
		bool parse_type(type_node &type);
		bool parse_expression(spv_section &section, spv::Id &node);
		bool parse_expression_unary(spv_section &section, spv::Id &node);
		bool parse_expression_multary(spv_section &section, spv::Id &left, unsigned int precedence = 0);
		bool parse_expression_assignment(spv_section &section, spv::Id &left);
		bool parse_statement(spv_section &section, bool scoped = true);
		bool parse_statement_block(spv_section &section, spv::Id &label, bool scoped = true);
		bool parse_array(int &size);
		bool parse_annotations(std::unordered_map<std::string, reshade::variant> &annotations);
		bool parse_struct(spv::Id &structure);
		bool parse_function_declaration(type_node &type, std::string name, spv::Id &function);
		bool parse_variable_declaration(spv_section &section, type_node &type, std::string name, spv::Id &variable, bool global = false);
		bool parse_variable_assignment(spv_section &section, spv::Id &expression);
		bool parse_variable_properties(variable_properties &props);
		bool parse_variable_properties_expression(spv::Id &expression);
		bool parse_technique(technique_properties &technique);
		bool parse_technique_pass(pass_properties &pass);
		bool parse_technique_pass_expression(spv::Id &expression);

		spv_section _entries;
		spv_section _strings;
		spv_section _annotations;
		spv_section _variables;
		spv_section _functions;
		spv_section _temporary;
		std::vector<technique_properties> techniques;
		std::vector<std::pair<spv_section *, size_t>> _id_lookup;

		spv::Id _next_id = 100;

		spv_node &add_node(spv_section &section, location loc, spv::Op op, spv::Id type = 0)
		{
			spv_node &instruction = add_node_without_result(section, loc, op);
			instruction.result = _next_id++;
			instruction.result_type = type;

			_id_lookup.push_back({ &section, instruction.index });

			return instruction;
		}
		spv_node &add_node_without_result(spv_section &section, location loc, spv::Op op)
		{
			spv_node &instruction = section.instructions.emplace_back();
			instruction.op = op;
			instruction.index = section.instructions.size() - 1;
			instruction.location = loc;

			return instruction;
		}
		spv_node &lookup_id(spv::Id id)
		{
			return _id_lookup[id - 100].first->instructions[_id_lookup[id - 100].second];
		}

		spv::Id lookup_type(const type_node &type)
		{
			if (type.is_floating_point())
			{
				if (type.is_matrix())
					return type_float4x4;
				else
					return type_float + type.rows - 1;
			}

			return -1;
		}

		std::string _errors;
		std::unique_ptr<lexer> _lexer, _lexer_backup;
		token _token, _token_next, _token_backup;
		std::unique_ptr<class symbol_table> _symbol_table;
	};
}
