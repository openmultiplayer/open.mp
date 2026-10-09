/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include <string>
#include <ompcapi.h>

#define EXPORT_OMP_API extern "C" SDK_EXPORT

using OutputStringViewPtr = CAPIStringView*;
using OutputStringBufferPtr = CAPIStringBuffer*;
using StringCharPtr = const char*;
using objectPtr = void*;
using voidPtr = void*;

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__

#define OMP_CAPI__WITHOUT_PARAMS_size_t(...) size_t
#define OMP_CAPI__WITHOUT_PARAMS_unsigned(...) unsigned
#define OMP_CAPI__WITHOUT_PARAMS_int(...) int
#define OMP_CAPI__WITHOUT_PARAMS_float(...) float
#define OMP_CAPI__WITHOUT_PARAMS_bool(...) bool
#define OMP_CAPI__WITHOUT_PARAMS_void(...) void
#define OMP_CAPI__WITHOUT_PARAMS_cell(...) cell
#define OMP_CAPI__WITHOUT_PARAMS_id(...) id
#define OMP_CAPI__WITHOUT_PARAMS_int8_t(...) int8_t
#define OMP_CAPI__WITHOUT_PARAMS_int16_t(...) int16_t
#define OMP_CAPI__WITHOUT_PARAMS_int32_t(...) int32_t
#define OMP_CAPI__WITHOUT_PARAMS_int64_t(...) int64_t
#define OMP_CAPI__WITHOUT_PARAMS_uint8_t(...) uint8_t
#define OMP_CAPI__WITHOUT_PARAMS_uint16_t(...) uint16_t
#define OMP_CAPI__WITHOUT_PARAMS_uint32_t(...) uint32_t
#define OMP_CAPI__WITHOUT_PARAMS_uint64_t(...) uint64_t
#define OMP_CAPI__WITHOUT_PARAMS_objectPtr(...) objectPtr
#define OMP_CAPI__WITHOUT_PARAMS_voidPtr(...) voidPtr

#define OMP_CAPI__WITHOUT_RETURN_size_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_unsigned(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_int(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_float(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_bool(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_void(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_cell(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_id(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_int8_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_int16_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_int32_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_int64_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_uint8_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_uint16_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_uint32_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_uint64_t(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_objectPtr(...) __VA_ARGS__
#define OMP_CAPI__WITHOUT_RETURN_voidPtr(...) __VA_ARGS__

#define OMP_CAPI__RETURN(params) OMP_CAPI__WITHOUT_PARAMS_##params
#define OMP_CAPI__PARAMETERS(params) CAT(OMP_CAPI__WITHOUT_RETURN_, params)

#define OMP_CAPI(name, params) \
	EXPORT_OMP_API OMP_CAPI__RETURN(params) name(OMP_CAPI__PARAMETERS(params))

#define RETURN_VALUE(x) returnValue(x)
#define RETURN_ERROR(x) returnError(x)
#define UNDEFINED_FAILED_RETURN(x) RETURN_ERROR("undefined error")
#define FUNCTION_FAIL_RETURN RETURN_ERROR(this->name_ + ": error while executing.")
