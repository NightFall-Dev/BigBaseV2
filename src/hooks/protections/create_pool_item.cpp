#include "hooking/hooking.hpp"
#include "gta/pools.hpp"

namespace big
{
	void* hooks::create_pool_item(GenericPool* pool)
	{
		auto item = g_hooking->get_original<hooks::create_pool_item>()(pool);

		if (!item)
		{
			if (pool && pool->m_size > 0 && pool->m_item_size > 0 && pool->m_item_size < 0x100000 && pool->m_item_count < 0x100000)
			{
				auto caller_offset = (__int64)_ReturnAddress() - (__int64)GetModuleHandleA(0);
				LOGF(WARNING, "Pool allocation failed from offset GTA5.exe+0x{:X}, in Pool Base Addr: 0x{:X}, with item_size={} item_count={}", caller_offset, reinterpret_cast<uintptr_t>(pool), pool->m_item_size, pool->m_item_count);
			}
		}

		return item;
	}
}