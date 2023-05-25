#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

#include "DS_List.h"
#include <assert.h>

namespace RakNet
{
	namespace DataStructures
	{
		template <class MemoryBlockType>
		class MemoryPool
		{
		public:
			MemoryPool();
			~MemoryPool();
			void Preallocate(unsigned numElements);
			MemoryBlockType *Allocate(void);
			void Release(MemoryBlockType *m);
			void Clear(void);
		protected:
			int blocksOut;
			List<MemoryBlockType*> pool;
		};

		template<class MemoryBlockType>
		MemoryPool<MemoryBlockType>::MemoryPool()
		{
	#ifdef _DEBUG
			blocksOut=0;
	#endif
		}
		template<class MemoryBlockType>
		MemoryPool<MemoryBlockType>::~MemoryPool()
		{

			RakAssert(blocksOut==0);

			unsigned i;
			for (i=0; i < pool.Size(); i++)
				delete pool[i];
		}

		template<class MemoryBlockType>
		void MemoryPool<MemoryBlockType>::Preallocate(unsigned numElements)
		{
			unsigned i;
			for (i=pool.Size(); i < numElements; i++)
			{
				pool.Insert(new MemoryBlockType);
			}
		}

		template<class MemoryBlockType>
		MemoryBlockType* MemoryPool<MemoryBlockType>::Allocate(void)
		{
	#ifdef _DEBUG
			blocksOut++;
	#endif
			if (pool.Size()==0)
				return new MemoryBlockType;
			else
			{
				MemoryBlockType* out;
				out=pool[pool.Size()-1];
				pool.Del();
				return out;
			}
		}
		template<class MemoryBlockType>
		void MemoryPool<MemoryBlockType>::Release(MemoryBlockType *m)
		{
			pool.Insert(m);
	#ifdef _DEBUG
			RakAssert(blocksOut>0);
			blocksOut--;
	#endif
		}
		template<class MemoryBlockType>
		void MemoryPool<MemoryBlockType>::Clear(void)
		{

			RakAssert(blocksOut==0);

			unsigned i;
			for (i=0; i < pool.Size(); i++)
				delete pool[i];
			pool.Clear();
		}
	}
}

#endif
