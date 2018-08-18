/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#include "effect_symbol_table.hpp"
//#include "effect_syntax_tree_nodes.hpp"

#include <assert.h>
#include <algorithm>
#include <functional>

#include <spirv.hpp>
namespace spv {
#include <GLSL.std.450.h>
}

using namespace reshadefx;

static type_node conv_type(type_id id)
{
	switch (id)
	{
	case type_void:
		return { type_node::datatype_void, 0, 0, 0 };
	case type_bool:
		return { type_node::datatype_bool, 0, 1, 1 };
	case type_bool2:
		return { type_node::datatype_bool, 0, 2, 1 };
	case type_bool3:
		return { type_node::datatype_bool, 0, 3, 1 };
	case type_bool4:
		return { type_node::datatype_bool, 0, 4, 1 };
	case type_int:
		return { type_node::datatype_int, 0, 1, 1 };
	case type_int2:
		return { type_node::datatype_int, 0, 2, 1 };
	case type_int3:
		return { type_node::datatype_int, 0, 3, 1 };
	case type_int4:
		return { type_node::datatype_int, 0, 4, 1 };
	case type_int2x2:
		return { type_node::datatype_int, 0, 2, 2 };
	case type_int3x3:
		return { type_node::datatype_int, 0, 3, 3 };
	case type_int4x4:
		return { type_node::datatype_int, 0, 4, 4 };
	case type_uint:
		return { type_node::datatype_uint, 0, 1, 1 };
	case type_uint2:
		return { type_node::datatype_uint, 0, 2, 1 };
	case type_uint3:
		return { type_node::datatype_uint, 0, 3, 1 };
	case type_uint4:
		return { type_node::datatype_uint, 0, 4, 1 };
	case type_uint2x2:
		return { type_node::datatype_uint, 0, 2, 2 };
	case type_uint3x3:
		return { type_node::datatype_uint, 0, 3, 3 };
	case type_uint4x4:
		return { type_node::datatype_uint, 0, 4, 4 };
	case type_float:
		return { type_node::datatype_float, 0, 1, 1 };
	case type_float2:
		return { type_node::datatype_float, 0, 2, 1 };
	case type_float3:
		return { type_node::datatype_float, 0, 3, 1 };
	case type_float4:
		return { type_node::datatype_float, 0, 4, 1 };
	case type_float2x2:
		return { type_node::datatype_float, 0, 2, 2 };
	case type_float3x3:
		return { type_node::datatype_float, 0, 3, 3 };
	case type_float4x4:
		return { type_node::datatype_float, 0, 4, 4 };
	case type_sampled_texture:
		return { type_node::datatype_sampler, 0, 0, 0 };
	default:
		return {};
	}
}

struct intrinsic
{
	intrinsic(const char *name, spv::Op op, spv::GLSLstd450 x, type_id ret_type) : op(op), glsl(x)
	{
		function.name = name;
		function.return_type = ret_type;
	}
	intrinsic(const char *name, spv::Op op, spv::GLSLstd450 x, type_id ret_type, type_id arg0_type) : op(op), glsl(x)
	{
		function.name = name;
		function.return_type = ret_type;
		function.parameter_list.push_back(conv_type(arg0_type));
	}
	intrinsic(const char *name, spv::Op op, spv::GLSLstd450 x, type_id ret_type, type_id arg0_type, type_id arg1_type) : op(op), glsl(x)
	{
		function.name = name;
		function.return_type = ret_type;
		function.parameter_list.push_back(conv_type(arg0_type));
		function.parameter_list.push_back(conv_type(arg1_type));
	}
	intrinsic(const char *name, spv::Op op, spv::GLSLstd450 x, type_id ret_type, type_id arg0_type, type_id arg1_type, type_id arg2_type) : op(op), glsl(x)
	{
		function.name = name;
		function.return_type = ret_type;
		function.parameter_list.push_back(conv_type(arg0_type));
		function.parameter_list.push_back(conv_type(arg1_type));
		function.parameter_list.push_back(conv_type(arg2_type));
	}
	intrinsic(const char *name, spv::Op op, spv::GLSLstd450 x, type_id ret_type, type_id arg0_type, type_id arg1_type, type_id arg2_type, type_id arg3_type) : op(op), glsl(x)
	{
		function.name = name;
		function.return_type = ret_type;
		function.parameter_list.push_back(conv_type(arg0_type));
		function.parameter_list.push_back(conv_type(arg1_type));
		function.parameter_list.push_back(conv_type(arg2_type));
		function.parameter_list.push_back(conv_type(arg3_type));
	}

	spv::Op op;
	spv::GLSLstd450 glsl;
	function_properties function;
};

static const intrinsic s_intrinsics[] = {
	{ "abs", spv::OpExtInst, spv::GLSLstd450FAbs, type_float, type_float },
	{ "abs", spv::OpExtInst, spv::GLSLstd450FAbs, type_float2, type_float2 },
	{ "abs", spv::OpExtInst, spv::GLSLstd450FAbs, type_float3, type_float3 },
	{ "abs", spv::OpExtInst, spv::GLSLstd450FAbs, type_float4, type_float4 },
	{ "abs", spv::OpExtInst, spv::GLSLstd450SAbs, type_int, type_int },
	{ "abs", spv::OpExtInst, spv::GLSLstd450SAbs, type_int2, type_int2 },
	{ "abs", spv::OpExtInst, spv::GLSLstd450SAbs, type_int3, type_int3 },
	{ "abs", spv::OpExtInst, spv::GLSLstd450SAbs, type_int4, type_int4 },
	{ "acos", spv::OpExtInst, spv::GLSLstd450Acos, type_float, type_float },
	{ "acos", spv::OpExtInst, spv::GLSLstd450Acos, type_float2, type_float2 },
	{ "acos", spv::OpExtInst, spv::GLSLstd450Acos, type_float3, type_float3 },
	{ "acos", spv::OpExtInst, spv::GLSLstd450Acos, type_float4, type_float4 },
	{ "all", spv::OpAll, spv::GLSLstd450Bad, type_bool, type_bool },
	{ "all", spv::OpAll, spv::GLSLstd450Bad, type_bool, type_bool2 },
	{ "all", spv::OpAll, spv::GLSLstd450Bad, type_bool, type_bool3 },
	{ "all", spv::OpAll, spv::GLSLstd450Bad, type_bool, type_bool4 },
	{ "any", spv::OpAny, spv::GLSLstd450Bad, type_bool, type_bool },
	{ "any", spv::OpAny, spv::GLSLstd450Bad, type_bool, type_bool2 },
	{ "any", spv::OpAny, spv::GLSLstd450Bad, type_bool, type_bool3 },
	{ "any", spv::OpAny, spv::GLSLstd450Bad, type_bool, type_bool4 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float, type_int },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float2, type_int2 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float3, type_int3 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float4, type_int4 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float, type_uint },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float2, type_uint2 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float3, type_uint3 },
	{ "asfloat", spv::OpBitcast, spv::GLSLstd450Bad, type_float4, type_uint4 },
	{ "asin", spv::OpExtInst, spv::GLSLstd450Asin, type_float, type_float },
	{ "asin", spv::OpExtInst, spv::GLSLstd450Asin, type_float2, type_float2 },
	{ "asin", spv::OpExtInst, spv::GLSLstd450Asin, type_float3, type_float3 },
	{ "asin", spv::OpExtInst, spv::GLSLstd450Asin, type_float4, type_float4 },
	{ "asint", spv::OpBitcast, spv::GLSLstd450Bad, type_int, type_float },
	{ "asint", spv::OpBitcast, spv::GLSLstd450Bad, type_int2, type_float2 },
	{ "asint", spv::OpBitcast, spv::GLSLstd450Bad, type_int3, type_float3 },
	{ "asint", spv::OpBitcast, spv::GLSLstd450Bad, type_int4, type_float4 },
	{ "asuint", spv::OpBitcast, spv::GLSLstd450Bad, type_uint, type_float },
	{ "asuint", spv::OpBitcast, spv::GLSLstd450Bad, type_uint2, type_float2 },
	{ "asuint", spv::OpBitcast, spv::GLSLstd450Bad, type_uint3, type_float3 },
	{ "asuint", spv::OpBitcast, spv::GLSLstd450Bad, type_uint4, type_float4 },
	{ "atan", spv::OpExtInst, spv::GLSLstd450Atan, type_float, type_float },
	{ "atan", spv::OpExtInst, spv::GLSLstd450Atan, type_float2, type_float2 },
	{ "atan", spv::OpExtInst, spv::GLSLstd450Atan, type_float3, type_float3 },
	{ "atan", spv::OpExtInst, spv::GLSLstd450Atan, type_float4, type_float4 },
	{ "atan2", spv::OpExtInst, spv::GLSLstd450Atan2, type_float, type_float, type_float },
	{ "atan2", spv::OpExtInst, spv::GLSLstd450Atan2, type_float2, type_float2, type_float2 },
	{ "atan2", spv::OpExtInst, spv::GLSLstd450Atan2, type_float3, type_float3, type_float3 },
	{ "atan2", spv::OpExtInst, spv::GLSLstd450Atan2, type_float4, type_float4, type_float4 },
	{ "ceil", spv::OpExtInst, spv::GLSLstd450Ceil, type_float, type_float },
	{ "ceil", spv::OpExtInst, spv::GLSLstd450Ceil, type_float2, type_float2 },
	{ "ceil", spv::OpExtInst, spv::GLSLstd450Ceil, type_float3, type_float3 },
	{ "ceil", spv::OpExtInst, spv::GLSLstd450Ceil, type_float4, type_float4 },
	{ "clamp", spv::OpExtInst, spv::GLSLstd450NClamp, type_float, type_float, type_float, type_float },
	{ "clamp", spv::OpExtInst, spv::GLSLstd450NClamp, type_float2, type_float2, type_float2, type_float2 },
	{ "clamp", spv::OpExtInst, spv::GLSLstd450NClamp, type_float3, type_float3, type_float3, type_float3 },
	{ "clamp", spv::OpExtInst, spv::GLSLstd450NClamp, type_float4, type_float4, type_float4, type_float4 },
	{ "cos", spv::OpExtInst, spv::GLSLstd450Cos, type_float, type_float },
	{ "cos", spv::OpExtInst, spv::GLSLstd450Cos, type_float2, type_float2 },
	{ "cos", spv::OpExtInst, spv::GLSLstd450Cos, type_float3, type_float3 },
	{ "cos", spv::OpExtInst, spv::GLSLstd450Cos, type_float4, type_float4 },
	{ "cosh", spv::OpExtInst, spv::GLSLstd450Cosh, type_float, type_float },
	{ "cosh", spv::OpExtInst, spv::GLSLstd450Cosh, type_float2, type_float2 },
	{ "cosh", spv::OpExtInst, spv::GLSLstd450Cosh, type_float3, type_float3 },
	{ "cosh", spv::OpExtInst, spv::GLSLstd450Cosh, type_float4, type_float4 },
	{ "cross", spv::OpExtInst, spv::GLSLstd450Cross, type_float3, type_float3, type_float3 },
	{ "ddx", spv::OpDPdx, spv::GLSLstd450Bad, type_float, type_float },
	{ "ddx", spv::OpDPdx, spv::GLSLstd450Bad, type_float2, type_float2 },
	{ "ddx", spv::OpDPdx, spv::GLSLstd450Bad, type_float3, type_float3 },
	{ "ddx", spv::OpDPdx, spv::GLSLstd450Bad, type_float4, type_float4 },
	{ "ddy", spv::OpDPdy, spv::GLSLstd450Bad, type_float, type_float },
	{ "ddy", spv::OpDPdy, spv::GLSLstd450Bad, type_float2, type_float2 },
	{ "ddy", spv::OpDPdy, spv::GLSLstd450Bad, type_float3, type_float3 },
	{ "ddy", spv::OpDPdy, spv::GLSLstd450Bad, type_float4, type_float4 },
	{ "degrees", spv::OpExtInst, spv::GLSLstd450Degrees, type_float, type_float },
	{ "degrees", spv::OpExtInst, spv::GLSLstd450Degrees, type_float2, type_float2 },
	{ "degrees", spv::OpExtInst, spv::GLSLstd450Degrees, type_float3, type_float3 },
	{ "degrees", spv::OpExtInst, spv::GLSLstd450Degrees, type_float4, type_float4 },
	{ "determinant", spv::OpExtInst, spv::GLSLstd450Determinant, type_float, type_float2x2 },
	{ "determinant", spv::OpExtInst, spv::GLSLstd450Determinant, type_float, type_float3x3 },
	{ "determinant", spv::OpExtInst, spv::GLSLstd450Determinant, type_float, type_float4x4 },
	{ "distance", spv::OpExtInst, spv::GLSLstd450Distance, type_float, type_float, type_float },
	{ "distance", spv::OpExtInst, spv::GLSLstd450Distance, type_float, type_float2, type_float2 },
	{ "distance", spv::OpExtInst, spv::GLSLstd450Distance, type_float, type_float3, type_float3 },
	{ "distance", spv::OpExtInst, spv::GLSLstd450Distance, type_float, type_float4, type_float4 },
	{ "dot", spv::OpDot, spv::GLSLstd450Bad, type_float, type_float, type_float },
	{ "dot", spv::OpDot, spv::GLSLstd450Bad, type_float, type_float2, type_float2 },
	{ "dot", spv::OpDot, spv::GLSLstd450Bad, type_float, type_float3, type_float3 },
	{ "dot", spv::OpDot, spv::GLSLstd450Bad, type_float, type_float4, type_float4 },
	{ "exp", spv::OpExtInst, spv::GLSLstd450Exp, type_float, type_float },
	{ "exp", spv::OpExtInst, spv::GLSLstd450Exp, type_float2, type_float2 },
	{ "exp", spv::OpExtInst, spv::GLSLstd450Exp, type_float3, type_float3 },
	{ "exp", spv::OpExtInst, spv::GLSLstd450Exp, type_float4, type_float4 },
	{ "exp2", spv::OpExtInst, spv::GLSLstd450Exp2, type_float, type_float },
	{ "exp2", spv::OpExtInst, spv::GLSLstd450Exp2, type_float2, type_float2 },
	{ "exp2", spv::OpExtInst, spv::GLSLstd450Exp2, type_float3, type_float3 },
	{ "exp2", spv::OpExtInst, spv::GLSLstd450Exp2, type_float4, type_float4 },
	{ "faceforward", spv::OpExtInst, spv::GLSLstd450FaceForward, type_float, type_float, type_float, type_float },
	{ "faceforward", spv::OpExtInst, spv::GLSLstd450FaceForward, type_float2, type_float2, type_float2, type_float2 },
	{ "faceforward", spv::OpExtInst, spv::GLSLstd450FaceForward, type_float3, type_float3, type_float3, type_float3 },
	{ "faceforward", spv::OpExtInst, spv::GLSLstd450FaceForward, type_float4, type_float4, type_float4, type_float4 },
	{ "floor", spv::OpExtInst, spv::GLSLstd450Floor, type_float, type_float },
	{ "floor", spv::OpExtInst, spv::GLSLstd450Floor, type_float2, type_float2 },
	{ "floor", spv::OpExtInst, spv::GLSLstd450Floor, type_float3, type_float3 },
	{ "floor", spv::OpExtInst, spv::GLSLstd450Floor, type_float4, type_float4 },
	{ "frac", spv::OpExtInst, spv::GLSLstd450Fract, type_float, type_float },
	{ "frac", spv::OpExtInst, spv::GLSLstd450Fract, type_float2, type_float2 },
	{ "frac", spv::OpExtInst, spv::GLSLstd450Fract, type_float3, type_float3 },
	{ "frac", spv::OpExtInst, spv::GLSLstd450Fract, type_float4, type_float4 },
	{ "frexp", spv::OpExtInst, spv::GLSLstd450Frexp, type_float, type_float, type_float },
	{ "frexp", spv::OpExtInst, spv::GLSLstd450Frexp, type_float2, type_float2, type_float2 },
	{ "frexp", spv::OpExtInst, spv::GLSLstd450Frexp, type_float3, type_float3, type_float3 },
	{ "frexp", spv::OpExtInst, spv::GLSLstd450Frexp, type_float4, type_float4, type_float4 },
	{ "fwidth", spv::OpFwidth, spv::GLSLstd450Bad, type_float, type_float },
	{ "fwidth", spv::OpFwidth, spv::GLSLstd450Bad, type_float2, type_float2 },
	{ "fwidth", spv::OpFwidth, spv::GLSLstd450Bad, type_float3, type_float3 },
	{ "fwidth", spv::OpFwidth, spv::GLSLstd450Bad, type_float4, type_float4 },
	{ "isinf", spv::OpIsInf, spv::GLSLstd450Bad, type_float, type_float },
	{ "isinf", spv::OpIsInf, spv::GLSLstd450Bad, type_float2, type_float2 },
	{ "isinf", spv::OpIsInf, spv::GLSLstd450Bad, type_float3, type_float3 },
	{ "isinf", spv::OpIsInf, spv::GLSLstd450Bad, type_float4, type_float4 },
	{ "isnan", spv::OpIsNan, spv::GLSLstd450Bad, type_float, type_float },
	{ "isnan", spv::OpIsNan, spv::GLSLstd450Bad, type_float2, type_float2 },
	{ "isnan", spv::OpIsNan, spv::GLSLstd450Bad, type_float3, type_float3 },
	{ "isnan", spv::OpIsNan, spv::GLSLstd450Bad, type_float4, type_float4 },
	{ "ldexp", spv::OpExtInst, spv::GLSLstd450Ldexp, type_float, type_float, type_float },
	{ "ldexp", spv::OpExtInst, spv::GLSLstd450Ldexp, type_float2, type_float2, type_float2 },
	{ "ldexp", spv::OpExtInst, spv::GLSLstd450Ldexp, type_float3, type_float3, type_float3 },
	{ "ldexp", spv::OpExtInst, spv::GLSLstd450Ldexp, type_float4, type_float4, type_float4 },
	{ "length", spv::OpExtInst, spv::GLSLstd450Length, type_float, type_float },
	{ "length", spv::OpExtInst, spv::GLSLstd450Length, type_float, type_float2 },
	{ "length", spv::OpExtInst, spv::GLSLstd450Length, type_float, type_float3 },
	{ "length", spv::OpExtInst, spv::GLSLstd450Length, type_float, type_float4 },
	{ "lerp", spv::OpExtInst, spv::GLSLstd450FMix, type_float, type_float, type_float, type_float },
	{ "lerp", spv::OpExtInst, spv::GLSLstd450FMix, type_float2, type_float2, type_float2, type_float2 },
	{ "lerp", spv::OpExtInst, spv::GLSLstd450FMix, type_float3, type_float3, type_float3, type_float3 },
	{ "lerp", spv::OpExtInst, spv::GLSLstd450FMix, type_float4, type_float4, type_float4, type_float4 },
	{ "log", spv::OpExtInst, spv::GLSLstd450Log, type_float, type_float },
	{ "log", spv::OpExtInst, spv::GLSLstd450Log, type_float2, type_float2 },
	{ "log", spv::OpExtInst, spv::GLSLstd450Log, type_float3, type_float3 },
	{ "log", spv::OpExtInst, spv::GLSLstd450Log, type_float4, type_float4 },
	//{ "log10", spv::GLSLstd450Log10, type_float, type_float },
	//{ "log10", spv::GLSLstd450Log10, type_float2, type_float2 },
	//{ "log10", spv::GLSLstd450Log10, type_float3, type_float3 },
	//{ "log10", spv::GLSLstd450Log10, type_float4, type_float4 },
	{ "log2", spv::OpExtInst, spv::GLSLstd450Log2, type_float, type_float },
	{ "log2", spv::OpExtInst, spv::GLSLstd450Log2, type_float2, type_float2 },
	{ "log2", spv::OpExtInst, spv::GLSLstd450Log2, type_float3, type_float3 },
	{ "log2", spv::OpExtInst, spv::GLSLstd450Log2, type_float4, type_float4 },
	{ "mad", spv::OpExtInst, spv::GLSLstd450Fma, type_float, type_float, type_float, type_float },
	{ "mad", spv::OpExtInst, spv::GLSLstd450Fma, type_float2, type_float2, type_float2, type_float2 },
	{ "mad", spv::OpExtInst, spv::GLSLstd450Fma, type_float3, type_float3, type_float3, type_float3 },
	{ "mad", spv::OpExtInst, spv::GLSLstd450Fma, type_float4, type_float4, type_float4, type_float4 },
	{ "max", spv::OpExtInst, spv::GLSLstd450NMax, type_float, type_float, type_float },
	{ "max", spv::OpExtInst, spv::GLSLstd450NMax, type_float2, type_float2, type_float2 },
	{ "max", spv::OpExtInst, spv::GLSLstd450NMax, type_float3, type_float3, type_float3 },
	{ "max", spv::OpExtInst, spv::GLSLstd450NMax, type_float4, type_float4, type_float4 },
	{ "min", spv::OpExtInst, spv::GLSLstd450NMin, type_float, type_float, type_float },
	{ "min", spv::OpExtInst, spv::GLSLstd450NMin, type_float2, type_float2, type_float2 },
	{ "min", spv::OpExtInst, spv::GLSLstd450NMin, type_float3, type_float3, type_float3 },
	{ "min", spv::OpExtInst, spv::GLSLstd450NMin, type_float4, type_float4, type_float4 },
	{ "modf", spv::OpExtInst, spv::GLSLstd450Modf, type_float, type_float, type_float },
	{ "modf", spv::OpExtInst, spv::GLSLstd450Modf, type_float2, type_float2, type_float2 },
	{ "modf", spv::OpExtInst, spv::GLSLstd450Modf, type_float3, type_float3, type_float3 },
	{ "modf", spv::OpExtInst, spv::GLSLstd450Modf, type_float4, type_float4, type_float4 },
	{ "mul", spv::OpFMul, spv::GLSLstd450Bad, type_float, type_float, type_float },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float2, type_float, type_float2 },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float3, type_float, type_float3 },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float4, type_float, type_float4 },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float2, type_float2, type_float },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float3, type_float3, type_float },
	{ "mul", spv::OpVectorTimesScalar, spv::GLSLstd450Bad, type_float4, type_float4, type_float },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float2x2, type_float, type_float2x2 },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float3x3, type_float, type_float3x3 },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float4x4, type_float, type_float4x4 },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float2x2, type_float2x2, type_float },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float3x3, type_float3x3, type_float },
	{ "mul", spv::OpMatrixTimesScalar, spv::GLSLstd450Bad, type_float4x4, type_float4x4, type_float },
	{ "mul", spv::OpVectorTimesMatrix, spv::GLSLstd450Bad, type_float2, type_float2, type_float2x2 },
	{ "mul", spv::OpVectorTimesMatrix, spv::GLSLstd450Bad, type_float3, type_float3, type_float3x3 },
	{ "mul", spv::OpVectorTimesMatrix, spv::GLSLstd450Bad, type_float4, type_float4, type_float4x4 },
	{ "mul", spv::OpMatrixTimesVector, spv::GLSLstd450Bad, type_float2, type_float2x2, type_float2 },
	{ "mul", spv::OpMatrixTimesVector, spv::GLSLstd450Bad, type_float3, type_float3x3, type_float3 },
	{ "mul", spv::OpMatrixTimesVector, spv::GLSLstd450Bad, type_float4, type_float4x4, type_float4 },
	{ "mul", spv::OpMatrixTimesMatrix, spv::GLSLstd450Bad, type_float2x2, type_float2x2, type_float2x2 },
	{ "mul", spv::OpMatrixTimesMatrix, spv::GLSLstd450Bad, type_float3x3, type_float3x3, type_float3x3 },
	{ "mul", spv::OpMatrixTimesMatrix, spv::GLSLstd450Bad, type_float4x4, type_float4x4, type_float4x4 },
	{ "normalize", spv::OpExtInst, spv::GLSLstd450Normalize, type_float, type_float },
	{ "normalize", spv::OpExtInst, spv::GLSLstd450Normalize, type_float2, type_float2 },
	{ "normalize", spv::OpExtInst, spv::GLSLstd450Normalize, type_float3, type_float3 },
	{ "normalize", spv::OpExtInst, spv::GLSLstd450Normalize, type_float4, type_float4 },
	{ "pow", spv::OpExtInst, spv::GLSLstd450Pow, type_float, type_float, type_float },
	{ "pow", spv::OpExtInst, spv::GLSLstd450Pow, type_float2, type_float2, type_float2 },
	{ "pow", spv::OpExtInst, spv::GLSLstd450Pow, type_float3, type_float3, type_float3 },
	{ "pow", spv::OpExtInst, spv::GLSLstd450Pow, type_float4, type_float4, type_float4 },
	{ "radians", spv::OpExtInst, spv::GLSLstd450Radians, type_float, type_float },
	{ "radians", spv::OpExtInst, spv::GLSLstd450Radians, type_float2, type_float2 },
	{ "radians", spv::OpExtInst, spv::GLSLstd450Radians, type_float3, type_float3 },
	{ "radians", spv::OpExtInst, spv::GLSLstd450Radians, type_float4, type_float4 },
	//{ "rcp", intrinsic_expression_node::rcp, type_float, type_float },
	//{ "rcp", intrinsic_expression_node::rcp, type_float2, type_float2 },
	//{ "rcp", intrinsic_expression_node::rcp, type_float3, type_float3 },
	//{ "rcp", intrinsic_expression_node::rcp, type_float4, type_float4 },
	{ "reflect", spv::OpExtInst, spv::GLSLstd450Reflect, type_float, type_float, type_float },
	{ "reflect", spv::OpExtInst, spv::GLSLstd450Reflect, type_float2, type_float2, type_float2 },
	{ "reflect", spv::OpExtInst, spv::GLSLstd450Reflect, type_float3, type_float3, type_float3 },
	{ "reflect", spv::OpExtInst, spv::GLSLstd450Reflect, type_float4, type_float4, type_float4 },
	{ "refract", spv::OpExtInst, spv::GLSLstd450Refract, type_float, type_float, type_float, type_float },
	{ "refract", spv::OpExtInst, spv::GLSLstd450Refract, type_float2, type_float2, type_float2, type_float2 },
	{ "refract", spv::OpExtInst, spv::GLSLstd450Refract, type_float3, type_float3, type_float3, type_float3 },
	{ "refract", spv::OpExtInst, spv::GLSLstd450Refract, type_float4, type_float4, type_float4, type_float4 },
	{ "round", spv::OpExtInst, spv::GLSLstd450Round, type_float, type_float },
	{ "round", spv::OpExtInst, spv::GLSLstd450Round, type_float2, type_float2 },
	{ "round", spv::OpExtInst, spv::GLSLstd450Round, type_float3, type_float3 },
	{ "round", spv::OpExtInst, spv::GLSLstd450Round, type_float4, type_float4 },
	{ "rsqrt", spv::OpExtInst, spv::GLSLstd450InverseSqrt, type_float, type_float },
	{ "rsqrt", spv::OpExtInst, spv::GLSLstd450InverseSqrt, type_float2, type_float2 },
	{ "rsqrt", spv::OpExtInst, spv::GLSLstd450InverseSqrt, type_float3, type_float3 },
	{ "rsqrt", spv::OpExtInst, spv::GLSLstd450InverseSqrt, type_float4, type_float4 },
	//{ "saturate", intrinsic_expression_node::saturate, type_float, type_float },
	//{ "saturate", intrinsic_expression_node::saturate, type_float2, type_float2 },
	//{ "saturate", intrinsic_expression_node::saturate, type_float3, type_float3 },
	//{ "saturate", intrinsic_expression_node::saturate, type_float4, type_float4 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450FSign, type_int, type_float },
	{ "sign", spv::OpExtInst, spv::GLSLstd450FSign, type_int2, type_float2 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450FSign, type_int3, type_float3 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450FSign, type_int4, type_float4 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450SSign, type_int, type_int },
	{ "sign", spv::OpExtInst, spv::GLSLstd450SSign, type_int2, type_int2 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450SSign, type_int3, type_int3 },
	{ "sign", spv::OpExtInst, spv::GLSLstd450SSign, type_int4, type_int4 },
	{ "sin", spv::OpExtInst, spv::GLSLstd450Sin, type_float, type_float },
	{ "sin", spv::OpExtInst, spv::GLSLstd450Sin, type_float2, type_float2 },
	{ "sin", spv::OpExtInst, spv::GLSLstd450Sin, type_float3, type_float3 },
	{ "sin", spv::OpExtInst, spv::GLSLstd450Sin, type_float4, type_float4 },
	//{ "sincos", intrinsic_expression_node::sincos, type_void, type_float, type_float, type_float },
	//{ "sincos", intrinsic_expression_node::sincos, type_void, type_float2, type_float2, type_float2 },
	//{ "sincos", intrinsic_expression_node::sincos, type_void, type_float3, type_float3, type_float3 },
	//{ "sincos", intrinsic_expression_node::sincos, type_void, type_float4, type_float4, type_float4 },
	{ "sinh", spv::OpExtInst, spv::GLSLstd450Sinh, type_float, type_float },
	{ "sinh", spv::OpExtInst, spv::GLSLstd450Sinh, type_float2, type_float2 },
	{ "sinh", spv::OpExtInst, spv::GLSLstd450Sinh, type_float3, type_float3 },
	{ "sinh", spv::OpExtInst, spv::GLSLstd450Sinh, type_float4, type_float4 },
	{ "smoothstep", spv::OpExtInst, spv::GLSLstd450SmoothStep, type_float, type_float, type_float, type_float },
	{ "smoothstep", spv::OpExtInst, spv::GLSLstd450SmoothStep, type_float2, type_float2, type_float2, type_float2 },
	{ "smoothstep", spv::OpExtInst, spv::GLSLstd450SmoothStep, type_float3, type_float3, type_float3, type_float3 },
	{ "smoothstep", spv::OpExtInst, spv::GLSLstd450SmoothStep, type_float4, type_float4, type_float4, type_float4 },
	{ "sqrt", spv::OpExtInst, spv::GLSLstd450Sqrt, type_float, type_float },
	{ "sqrt", spv::OpExtInst, spv::GLSLstd450Sqrt, type_float2, type_float2 },
	{ "sqrt", spv::OpExtInst, spv::GLSLstd450Sqrt, type_float3, type_float3 },
	{ "sqrt", spv::OpExtInst, spv::GLSLstd450Sqrt, type_float4, type_float4 },
	{ "step", spv::OpExtInst, spv::GLSLstd450Step, type_float, type_float, type_float },
	{ "step", spv::OpExtInst, spv::GLSLstd450Step, type_float2, type_float2, type_float2 },
	{ "step", spv::OpExtInst, spv::GLSLstd450Step, type_float3, type_float3, type_float3 },
	{ "step", spv::OpExtInst, spv::GLSLstd450Step, type_float4, type_float4, type_float4 },
	{ "tan", spv::OpExtInst, spv::GLSLstd450Tan, type_float, type_float },
	{ "tan", spv::OpExtInst, spv::GLSLstd450Tan, type_float2, type_float2 },
	{ "tan", spv::OpExtInst, spv::GLSLstd450Tan, type_float3, type_float3 },
	{ "tan", spv::OpExtInst, spv::GLSLstd450Tan, type_float4, type_float4 },
	{ "tanh", spv::OpExtInst, spv::GLSLstd450Tanh, type_float, type_float },
	{ "tanh", spv::OpExtInst, spv::GLSLstd450Tanh, type_float2, type_float2 },
	{ "tanh", spv::OpExtInst, spv::GLSLstd450Tanh, type_float3, type_float3 },
	{ "tanh", spv::OpExtInst, spv::GLSLstd450Tanh, type_float4, type_float4 },
	{ "tex2D", spv::OpImageSampleImplicitLod, spv::GLSLstd450Bad, type_float4, type_sampled_texture, type_float2 },
	{ "tex2Dfetch", spv::OpImageFetch, spv::GLSLstd450Bad,  type_float4, type_sampled_texture, type_int4 },
	{ "tex2Dgather", spv::OpImageGather, spv::GLSLstd450Bad, type_float4, type_sampled_texture, type_float2, type_int },
	//{ "tex2Dgatheroffset", intrinsic_expression_node::texture_gather_offset, type_float4, type_sampler, type_float2, type_int2, type_int },
	//{ "tex2Dgrad", spv::OpImageGradi intrinsic_expression_node::texture_gradient, type_float4, type_sampler, type_float2, type_float2, type_float2 },
	{ "tex2Dlod", spv::OpImageSampleExplicitLod, spv::GLSLstd450Bad, type_float4, type_sampled_texture, type_float4 },
	//{ "tex2Dlodoffset", intrinsic_expression_node::texture_level_offset, type_float4, type_sampler, type_float4, type_int2 },
	//{ "tex2Doffset", intrinsic_expression_node::texture_offset, type_float4, type_sampler, type_float2, type_int2 },
	{ "tex2Dproj", spv::OpImageSampleProjImplicitLod, spv::GLSLstd450Bad, type_float4, type_sampled_texture, type_float4 },
	{ "tex2Dsize", spv::OpImageQuerySize, spv::GLSLstd450Bad, type_int2, type_sampled_texture, type_int },
	{ "transpose", spv::OpTranspose, spv::GLSLstd450Bad, type_float2x2, type_float2x2 },
	{ "transpose", spv::OpTranspose, spv::GLSLstd450Bad, type_float3x3, type_float3x3 },
	{ "transpose", spv::OpTranspose, spv::GLSLstd450Bad, type_float4x4, type_float4x4 },
	{ "trunc", spv::OpExtInst, spv::GLSLstd450Trunc, type_float, type_float },
	{ "trunc", spv::OpExtInst, spv::GLSLstd450Trunc, type_float2, type_float2 },
	{ "trunc", spv::OpExtInst, spv::GLSLstd450Trunc, type_float3, type_float3 },
	{ "trunc", spv::OpExtInst, spv::GLSLstd450Trunc, type_float4, type_float4 },
};

static int compare_functions(const std::vector<type_node> &arguments, const function_properties *function1, const function_properties *function2)
{
	if (function2 == nullptr)
	{
		return -1;
	}

	const size_t count = arguments.size();

	bool function1_viable = true;
	bool function2_viable = true;
	const auto function1_ranks = static_cast<unsigned int *>(alloca(count * sizeof(unsigned int)));
	const auto function2_ranks = static_cast<unsigned int *>(alloca(count * sizeof(unsigned int)));

	for (size_t i = 0; i < count; ++i)
	{
		function1_ranks[i] = type_node::rank(arguments[i], function1->parameter_list[i]);

		if (function1_ranks[i] == 0)
		{
			function1_viable = false;
			break;
		}
	}
	for (size_t i = 0; i < count; ++i)
	{
		function2_ranks[i] = type_node::rank(arguments[i], function2->parameter_list[i]);

		if (function2_ranks[i] == 0)
		{
			function2_viable = false;
			break;
		}
	}

	if (!(function1_viable && function2_viable))
	{
		return function2_viable - function1_viable;
	}

	std::sort(function1_ranks, function1_ranks + count, std::greater<unsigned int>());
	std::sort(function2_ranks, function2_ranks + count, std::greater<unsigned int>());

	for (size_t i = 0; i < count; ++i)
	{
		if (function1_ranks[i] < function2_ranks[i])
		{
			return -1;
		}
		else if (function2_ranks[i] < function1_ranks[i])
		{
			return +1;
		}
	}

	return 0;
}

unsigned int type_node::rank(const type_node &src, const type_node &dst)
{
	if (src.is_array() != dst.is_array() || (src.array_length != dst.array_length && src.array_length > 0 && dst.array_length > 0))
	{
		return 0;
	}
	if (src.is_struct() || dst.is_struct())
	{
		return src.definition == dst.definition;
	}
	if (src.basetype == dst.basetype && src.rows == dst.rows && src.cols == dst.cols)
	{
		return 1;
	}
	if (!src.is_numeric() || !dst.is_numeric())
	{
		return 0;
	}

	const int ranks[4][4] =
	{
		{ 0, 5, 5, 5 },
		{ 4, 0, 3, 5 },
		{ 4, 2, 0, 5 },
		{ 4, 4, 4, 0 }
	};

	assert(src.basetype <= 4 && dst.basetype <= 4);

	const int rank = ranks[static_cast<unsigned int>(src.basetype) - 1][static_cast<unsigned int>(dst.basetype) - 1] << 2;

	if (src.is_scalar() && dst.is_vector())
	{
		return rank | 2;
	}
	if (src.is_vector() && dst.is_scalar() || (src.is_vector() == dst.is_vector() && src.rows > dst.rows && src.cols >= dst.cols))
	{
		return rank | 32;
	}
	if (src.is_vector() != dst.is_vector() || src.is_matrix() != dst.is_matrix() || src.rows * src.cols != dst.rows * dst.cols)
	{
		return 0;
	}

	return rank;
}

reshadefx::symbol_table::symbol_table()
{
	_current_scope.name = "::";
	_current_scope.level = 0;
	_current_scope.namespace_level = 0;
}

void reshadefx::symbol_table::enter_scope(symbol parent)
{
	if (parent || _parent_stack.empty())
	{
		_parent_stack.push(parent);
	}
	else
	{
		_parent_stack.push(_parent_stack.top());
	}

	_current_scope.level++;
}
void reshadefx::symbol_table::enter_namespace(const std::string &name)
{
	_current_scope.name += name + "::";
	_current_scope.level++;
	_current_scope.namespace_level++;
}
void reshadefx::symbol_table::leave_scope()
{
	assert(_current_scope.level > 0);

	for (auto &symbol : _symbol_stack)
	{
		auto &scope_list = symbol.second;

		for (auto scope_it = scope_list.begin(); scope_it != scope_list.end();)
		{
			if (scope_it->scope.level > scope_it->scope.namespace_level &&
				scope_it->scope.level >= _current_scope.level)
			{
				scope_it = scope_list.erase(scope_it);
			}
			else
			{
				++scope_it;
			}
		}
	}

	_parent_stack.pop();

	_current_scope.level--;
}
void reshadefx::symbol_table::leave_namespace()
{
	assert(_current_scope.level > 0);
	assert(_current_scope.namespace_level > 0);

	_current_scope.name.erase(_current_scope.name.substr(0, _current_scope.name.size() - 2).rfind("::") + 2);
	_current_scope.level--;
	_current_scope.namespace_level--;
}

bool reshadefx::symbol_table::insert(const std::string &name, symbol symbol, spv::Op type, void *props, bool global)
{
	assert(symbol != 0);

	// Make sure the symbol does not exist yet
	if (type != spv::OpFunction && find(name, _current_scope, true))
	{
		return false;
	}

	// Insertion routine which keeps the symbol stack sorted by namespace level
	const auto insert_sorted = [](auto &vec, const auto &item) {
		return vec.insert(
			std::upper_bound(vec.begin(), vec.end(), item,
				[](auto lhs, auto rhs) {
					return lhs.scope.namespace_level < rhs.scope.namespace_level;
				}), item);
	};

	// Global symbols are accessible from every scope
	if (global)
	{
		scope scope = { "", 0, 0 };

		// Walk scope chain from global scope back to current one
		for (size_t pos = 0; pos != std::string::npos; pos = _current_scope.name.find("::", pos))
		{
			// Extract scope name
			scope.name = _current_scope.name.substr(0, pos += 2);
			const auto previous_scope_name = _current_scope.name.substr(pos);

			// Insert symbol into this scope
			insert_sorted(_symbol_stack[previous_scope_name + name], symbol_data { scope, symbol, type, props });

			// Continue walking up the scope chain
			scope.level = ++scope.namespace_level;
		}
	}
	else
	{
		// This is a local symbol so it's sufficient to update the symbol stack with just the current scope
		insert_sorted(_symbol_stack[name], symbol_data { _current_scope, symbol, type, props });
	}

	return true;
}

reshadefx::symbol reshadefx::symbol_table::find(const std::string &name) const
{
	// Default to start search with current scope and walk back the scope chain
	return find(name, _current_scope, false);
}
reshadefx::symbol reshadefx::symbol_table::find(const std::string &name, const scope &scope, bool exclusive) const
{
	const auto stack_it = _symbol_stack.find(name);

	// Check if symbol does exist
	if (stack_it == _symbol_stack.end() || stack_it->second.empty())
	{
		return 0;
	}

	// Walk up the scope chain starting at the requested scope level and find a matching symbol
	symbol result = 0;
	const auto &scope_list = stack_it->second;

	for (auto it = scope_list.rbegin(), end = scope_list.rend(); it != end; ++it)
	{
		if (it->scope.level > scope.level ||
			it->scope.namespace_level > scope.namespace_level || (it->scope.namespace_level == scope.namespace_level && it->scope.name != scope.name))
			continue;
		if (exclusive && it->scope.level < scope.level)
			continue;

		if (it->type == spv::OpVariable || it->type == spv::OpTypeStruct)
		{
			return it->symbol;
		}
		if (result == 0)
		{
			result = it->symbol;
		}
	}

	return result;
}

bool reshadefx::symbol_table::resolve_call(const std::string &name, const std::vector<type_node> &arguments, const scope &scope, bool &is_ambiguous, spv::Op &out_op, spv::Id &out_id, spv::Id &out_type) const
{
	out_op = spv::OpFunctionCall;
	is_ambiguous = false;

	spv::Op intrinsic_op = spv::OpNop;
	const function_properties *overload_props = nullptr;
	unsigned int overload_count = 0, overload_namespace = scope.namespace_level;

	const auto stack_it = _symbol_stack.find(name);

	if (stack_it != _symbol_stack.end() && !stack_it->second.empty())
	{
		const auto &scope_list = stack_it->second;

		for (auto it = scope_list.rbegin(), end = scope_list.rend(); it != end; ++it)
		{
			if (it->scope.level > scope.level ||
				it->scope.namespace_level > scope.namespace_level ||
				it->type != spv::OpFunction)
			{
				continue;
			}

			const auto function = static_cast<function_properties *>(it->props);

			if (function->parameter_list.empty())
			{
				if (arguments.empty())
				{
					out_id = it->symbol;
					overload_props = function;
					overload_count = 1;
					break;
				}
				else
				{
					continue;
				}
			}
			else if (arguments.size() != function->parameter_list.size())
			{
				continue;
			}

			const int comparison = compare_functions(arguments, function, overload_props);

			if (comparison < 0)
			{
				out_id = it->symbol;
				overload_props = function;
				overload_count = 1;
				overload_namespace = it->scope.namespace_level;
			}
			else if (comparison == 0 && overload_namespace == it->scope.namespace_level)
			{
				++overload_count;
			}
		}
	}

	if (overload_count == 0)
	{
		for (auto &intrinsic : s_intrinsics)
		{
			if (intrinsic.function.name != name)
			{
				continue;
			}
			if (intrinsic.function.parameter_list.size() != arguments.size())
			{
				break;
			}

			const int comparison = compare_functions(arguments, &intrinsic.function, overload_props);

			if (comparison < 0)
			{
				out_op = intrinsic.op;
				out_id = intrinsic.glsl;
				out_type = intrinsic.function.return_type;
				overload_props = &intrinsic.function;
				overload_count = 1;

				intrinsic_op = intrinsic.op;
			}
			else if (comparison == 0 && overload_namespace == 0)
			{
				++overload_count;
			}
		}
	}

	if (overload_count == 1)
	{
		return true;
	}
	else
	{
		is_ambiguous = overload_count > 1;

		return false;
	}
}
