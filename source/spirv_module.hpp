/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#pragma once

#include "effect_lexer.hpp"
#include "spirv_instruction.hpp"
#include <unordered_map>
#include <unordered_set>

namespace reshadefx
{
	struct spirv_struct_member_info
	{
		type type;
		std::string name;
		std::string semantic;
		std::vector<spv::Decoration> decorations;
	};

	struct spirv_struct_info
	{
		spv::Id definition;
		std::string name;
		std::string unique_name;
		std::vector<spirv_struct_member_info> member_list;
	};

	struct spirv_function_info
	{
		spv::Id definition;
		std::string name;
		std::string unique_name;
		type return_type;
		std::string return_semantic;
		std::vector<spirv_struct_member_info> parameter_list;
		spv::Id entry_point = 0;
	};

	struct spirv_uniform_info
	{
		std::string name;
		spv::Id type_id = 0;
		spv::Id struct_type_id = 0;
		uint32_t member_index = 0;
		std::unordered_map<std::string, std::string> annotations;
		bool has_initializer_value = false;
		constant initializer_value;
	};

	struct spirv_texture_info
	{
		spv::Id id = 0;
		std::string semantic;
		std::string unique_name;
		std::unordered_map<std::string, std::string> annotations;
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t levels = 1;
		uint32_t format = 8; // RGBA8
	};

	struct spirv_sampler_info
	{
		spv::Id id = 0;
		uint32_t set = 0;
		uint32_t binding = 0;
		std::string unique_name;
		std::string texture_name;
		std::unordered_map<std::string, std::string> annotations;
		uint32_t filter = 0x1; // LINEAR
		uint32_t address_u = 3; // CLAMP
		uint32_t address_v = 3;
		uint32_t address_w = 3;
		float min_lod = -FLT_MAX;
		float max_lod = +FLT_MAX;
		float lod_bias = 0.0f;
		uint8_t srgb = false;
	};

	struct spirv_pass_info
	{
		std::string render_target_names[8] = {};
		std::string vs_entry_point;
		std::string ps_entry_point;
		//char vs_entry_point[64];
		//char ps_entry_point[64];
		uint8_t clear_render_targets = true;
		uint8_t srgb_write_enable = false;
		uint8_t blend_enable = false;
		uint8_t stencil_enable = false;
		uint8_t color_write_mask = 0xF;
		uint8_t stencil_read_mask = 0xFF;
		uint8_t stencil_write_mask = 0xFF;
		uint32_t blend_op = 1; // ADD
		uint32_t blend_op_alpha = 1; // ADD
		uint32_t src_blend = 1; // ONE
		uint32_t dest_blend = 0; // ZERO
		uint32_t src_blend_alpha = 1; // ONE
		uint32_t dest_blend_alpha = 0; // ZERO
		uint32_t stencil_comparison_func = 8; // ALWAYS
		uint32_t stencil_reference_value = 0;
		uint32_t stencil_op_pass = 1; // KEEP
		uint32_t stencil_op_fail = 1; // KEEP
		uint32_t stencil_op_depth_fail = 1; // KEEP
	};

	struct spirv_technique_info
	{
		std::string name;
		std::vector<spirv_pass_info> passes;
		std::unordered_map<std::string, std::string> annotations;
	};

	class spirv_module
	{
	public:
		const uint32_t glsl_ext = 1;

		struct function_info2
		{
			spirv_instruction declaration;
			spirv_basic_block variables;
			spirv_basic_block definition;
			type return_type;
			std::vector<type> param_types;
		};

		void write_module(std::vector<uint32_t> &stream) const;

		spirv_instruction &add_instruction(spirv_basic_block &section, const location &loc, spv::Op op, spv::Id type = 0);
		spirv_instruction &add_instruction_without_result(spirv_basic_block &section, const location &loc, spv::Op op);

		spv::Id convert_type(const type &info);
		spv::Id convert_type(const function_info2 &info);

		spv::Id convert_constant(const type &type, const constant &data);

		void add_capability(spv::Capability capability);

		std::vector<spirv_texture_info> _textures;
		std::vector<spirv_sampler_info> _samplers;
		std::vector<spirv_uniform_info> _uniforms;
		std::vector<spirv_technique_info> _techniques;

	protected:
		spv::Id make_id() { return _next_id++; }

		void define_struct(spv::Id id, const char *name, const location &loc, const std::vector<spv::Id> &members);
		void define_function(spv::Id id, const char *name, const location &loc, const type &ret_type);
		void define_function_param(spv::Id id, const char *name, const location &loc, const type &type);
		void define_variable(spv::Id id, const char *name, const location &loc, const type &type, spv::StorageClass storage, spv::Id initializer = 0);

		void add_name(spv::Id object, const char *name);
		void add_builtin(spv::Id object, spv::BuiltIn builtin);
		void add_decoration(spv::Id object, spv::Decoration decoration, const char *string);
		void add_decoration(spv::Id object, spv::Decoration decoration, std::initializer_list<uint32_t> values = {});
		void add_member_name(spv::Id object, uint32_t member_index, const char *name);
		void add_member_builtin(spv::Id object, uint32_t member_index, spv::BuiltIn builtin);
		void add_member_decoration(spv::Id object, uint32_t member_index, spv::Decoration decoration, const char *string);
		void add_member_decoration(spv::Id object, uint32_t member_index, spv::Decoration decoration, std::initializer_list<uint32_t> values = {});
		void add_entry_point(const char *name, spv::Id function, spv::ExecutionModel model, const std::vector<spv::Id> &io);

		void add_cast_operation(expression &chain, const type &type);
		void add_member_access(expression &chain, size_t index, const type &type);
		void add_static_index_access(expression &chain, size_t index);
		void add_dynamic_index_access(expression &chain, spv::Id index_expression);
		void add_swizzle_access(expression &chain, signed char swizzle[4], size_t length);

		spv::Id add_unary_operation(spirv_basic_block &block, const location &loc, tokenid op, const type &type, spv::Id val);
		spv::Id add_binary_operation(spirv_basic_block &block, const location &loc, tokenid op, const type &type, spv::Id lhs, spv::Id rhs) { return add_binary_operation(block, loc, op, type, type, lhs, rhs); }
		spv::Id add_binary_operation(spirv_basic_block &block, const location &loc, tokenid op, const type &res_type, const type &type, spv::Id lhs, spv::Id rhs);
		spv::Id add_ternary_operation(spirv_basic_block &block, const location &loc, tokenid op, const type &type, spv::Id condition, spv::Id true_value, spv::Id false_value);
		spv::Id add_phi_operation(spirv_basic_block &block, const type &type, spv::Id value0, spv::Id parent0, spv::Id value1, spv::Id parent1);

		void enter_block(spirv_basic_block &section, spv::Id id);
		void leave_block_and_kill(spirv_basic_block &section);
		void leave_block_and_return(spirv_basic_block &section, spv::Id value);
		void leave_block_and_branch(spirv_basic_block &section, spv::Id target);
		void leave_block_and_branch_conditional(spirv_basic_block &section, spv::Id condition, spv::Id true_target, spv::Id false_target);
		void leave_function();

		spv::Id construct_type(spirv_basic_block &section, const type &type, std::vector<expression> &arguments);

		spv::Id access_chain_load(spirv_basic_block &section, const expression &chain);
		void    access_chain_store(spirv_basic_block &section, const expression &chain, spv::Id value, const type &value_type);

		std::vector<function_info2> _functions2;

		spv::Id _current_block = 0;
		size_t  _current_function = std::numeric_limits<size_t>::max();

	private:
		spv::Id _next_id = 10;

		spirv_basic_block _entries;
		spirv_basic_block _debug_a;
		spirv_basic_block _debug_b;
		spirv_basic_block _annotations;
		spirv_basic_block _types_and_constants;
		spirv_basic_block _variables;

		std::unordered_set<spv::Capability> _capabilities;
		std::vector<std::pair<type, spv::Id>> _type_lookup;
		std::vector<std::pair<function_info2, spv::Id>> _function_type_lookup;
		std::vector<std::tuple<type, constant, spv::Id>> _constant_lookup;
	};
}
