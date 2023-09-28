#pragma once
#pragma warning(push)
#pragma warning(disable : 4005)
#include <type_traits>
#pragma warning(pop)

#include "../addresses.h"
#include "tier0/dbg.h"
#include "const.h"
#include "../../utils/virtual.h"
#undef schema

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;
constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline constexpr uint32_t hash_32_fnv1a_const(const char *const str, const uint32_t value = val_32_const) noexcept
{
	return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

inline constexpr uint64_t hash_64_fnv1a_const(const char *const str, const uint64_t value = val_64_const) noexcept
{
	return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}

#define SCHEMA_FIELD_OFFSET(type, varName, extra_offset)														\
	std::add_lvalue_reference_t<type> varName()																	\
	{																											\
		static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClass);									\
		static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);										\
																												\
		static const auto m_offset =																			\
			schema::GetOffset(ThisClass, datatable_hash, #varName, prop_hash);									\
																												\
		return *reinterpret_cast<std::add_pointer_t<type>>(														\
			(uintptr_t)(this) + m_offset + extra_offset);														\
	}																											\
	void varName(type val)																						\
	{																											\
		static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClass);									\
		static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);										\
																												\
		static const auto m_offset =																			\
			schema::GetOffset(ThisClass, datatable_hash, #varName, prop_hash);									\
																												\
		static const auto m_chain =																				\
			schema::FindChainOffset(ThisClass);																	\
																												\
		if (m_chain != 0)																						\
		{																										\
			ConMsg("Found chain offset %d for %s::%s\n", m_chain, ThisClass, #varName);							\
			addresses::NetworkStateChanged((uintptr_t)(this) + m_chain, m_offset + extra_offset, 0xFFFFFFFF);	\
		}																										\
		else																									\
		{																										\
			/* WIP: Works fine for most props, but inlined classes in the middle of a class will
				need to have their this pointer corrected by the offset .*/										\
			ConMsg("Attempting to call update on on %s::%s\n", ThisClass, #varName);							\
			CALL_VIRTUAL(void, IsStruct ? 1 : 21, this, m_offset + extra_offset, 0xFFFFFFFF, 0xFFFF);			\
		}																										\
		*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + m_offset + extra_offset) = val;			\
	}

#define SCHEMA_FIELD(type, varName) \
	SCHEMA_FIELD_OFFSET(type, varName, 0)


#define PSCHEMA_FIELD_OFFSET(type, varName, extra_offset) \
	auto varName()																\
	{																			\
		static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClass);	\
		static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);		\
																				\
		static const auto m_offset =											\
			schema::GetOffset(ThisClass, datatable_hash, #varName, prop_hash);	\
																				\
		return reinterpret_cast<std::add_pointer_t<type>>(						\
			(uintptr_t)(this) + m_offset + extra_offset);						\
	}

#define PSCHEMA_FIELD(type, varName) \
	PSCHEMA_FIELD_OFFSET(type, varName, 0)

namespace schema
{
	int16_t FindChainOffset(const char *className);
	int16_t GetOffset(const char *className, uint32_t classKey, const char *memberName, uint32_t memberKey);
}

#define DECLARE_SCHEMA_CLASS_BASE(className, isStruct) \
	static constexpr inline const char *ThisClass = #className;      \
	static constexpr inline bool IsStruct = isStruct;

#define DECLARE_SCHEMA_CLASS(className) DECLARE_SCHEMA_CLASS_BASE(className, false)

// Use this for classes that can be wholly included within other classes (like CCollisionProperty within CBaseModelEntity)
#define DECLARE_SCHEMA_CLASS_INLINE(className) DECLARE_SCHEMA_CLASS_BASE(className, true)