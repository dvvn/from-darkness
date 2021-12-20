export module cheat.csgo.tools.UtlMemory;

export namespace cheat::csgo
{
#if 0

	template <class T, class I = int>
	class CUtlMemory
	{
	public:
		// constructor, destructor
		CUtlMemory(int nGrowSize = 0, int nInitSize = 0);
		CUtlMemory(T* pMemory, int numElements);
		CUtlMemory(const T* pMemory, int numElements);
		~CUtlMemory();

		// Set the size by which the memory grows
		auto Init(int nGrowSize = 0, int nInitSize = 0) -> void;

		class Iterator_t
		{
		public:
			Iterator_t(I i) : index(i)
			{
			}

			I index;

			auto operator==(const Iterator_t it) const -> bool { return index == it.index; }
			auto operator!=(const Iterator_t it) const -> bool { return index != it.index; }
		};
		auto First() const -> Iterator_t { return Iterator_t(IsIdxValid(0) ? 0 : InvalidIndex()); }
		auto Next(const Iterator_t& it) const -> Iterator_t { return Iterator_t(IsIdxValid(it.index + 1) ? it.index + 1 : InvalidIndex()); }
		auto GetIndex(const Iterator_t& it) const -> I { return it.index; }
		auto IsIdxAfter(I i, const Iterator_t& it) const -> bool { return i > it.index; }
		auto IsValidIterator(const Iterator_t& it) const -> bool { return IsIdxValid(it.index); }
		auto InvalidIterator() const -> Iterator_t { return Iterator_t(InvalidIndex()); }

		// element access
		auto operator[](I i)->T&;
		auto operator[](I i) const -> const T&;
		auto Element(I i)->T&;
		auto Element(I i) const -> const T&;

		auto IsIdxValid(I i) const -> bool;

		static const I INVALID_INDEX = (I)-1; // For use with COMPILE_TIME_ASSERT
		static auto InvalidIndex() -> I { return INVALID_INDEX; }

		auto Base()->T*;
		auto Base() const -> const T*;

		auto SetExternalBuffer(T* pMemory, int numElements) -> void;
		auto SetExternalBuffer(const T* pMemory, int numElements) -> void;
		auto AssumeMemory(T* pMemory, int nSize) -> void;
		auto Detach()->T*;
		auto DetachMemory() -> void*;

		auto Swap(CUtlMemory<T, I>& mem) -> void;
		auto ConvertToGrowableMemory(int nGrowSize) -> void;
		auto NumAllocated() const -> int;
		auto Count() const -> int;
		auto Grow(int num = 1) -> void;
		auto EnsureCapacity(int num) -> void;
		auto Purge() -> void;
		auto Purge(int numElements) -> void;
		auto IsExternallyAllocated() const -> bool;
		auto IsReadOnly() const -> bool;
		auto SetGrowSize(int size) -> void;

	protected:
		auto ValidateGrowSize() -> void
		{
		}

		enum
		{
			EXTERNAL_BUFFER_MARKER = -1,
			EXTERNAL_CONST_BUFFER_MARKER = -2,
		};

		T* m_pMemory;
		int m_nAllocationCount;
		int m_nGrowSize;
	};

	//-----------------------------------------------------------------------------
	// constructor, destructor
	//-----------------------------------------------------------------------------

	template <class T, class I>
	CUtlMemory<T, I>::CUtlMemory(int nGrowSize, int nInitAllocationCount) : m_pMemory(0),
		m_nAllocationCount(nInitAllocationCount),
		m_nGrowSize(nGrowSize)
	{
		ValidateGrowSize();
		assert(nGrowSize >= 0);
		if (m_nAllocationCount)
		{
			m_pMemory = (T*)new unsigned char[m_nAllocationCount * sizeof(T)];
			//m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
		}
	}

	template <class T, class I>
	CUtlMemory<T, I>::CUtlMemory(T* pMemory, int numElements) : m_pMemory(pMemory),
		m_nAllocationCount(numElements)
	{
		// Special marker indicating externally supplied modifyable memory
		m_nGrowSize = EXTERNAL_BUFFER_MARKER;
	}

	template <class T, class I>
	CUtlMemory<T, I>::CUtlMemory(const T* pMemory, int numElements) : m_pMemory((T*)pMemory),
		m_nAllocationCount(numElements)
	{
		// Special marker indicating externally supplied modifyable memory
		m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
	}

	template <class T, class I>
	CUtlMemory<T, I>::~CUtlMemory()
	{
		Purge();
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Init(int nGrowSize /*= 0*/, int nInitSize /*= 0*/) -> void
	{
		Purge();

		m_nGrowSize = nGrowSize;
		m_nAllocationCount = nInitSize;
		ValidateGrowSize();
		assert(nGrowSize >= 0);
		if (m_nAllocationCount)
		{
			UTLMEMORY_TRACK_ALLOC();
			MEM_ALLOC_CREDIT_CLASS();
			m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
		}
	}

	//-----------------------------------------------------------------------------
	// Fast swap
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::Swap(CUtlMemory<T, I>& mem) -> void
	{
		V_swap(m_nGrowSize, mem.m_nGrowSize);
		V_swap(m_pMemory, mem.m_pMemory);
		V_swap(m_nAllocationCount, mem.m_nAllocationCount);
	}

	//-----------------------------------------------------------------------------
	// Switches the buffer from an external memory buffer to a reallocatable buffer
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::ConvertToGrowableMemory(int nGrowSize) -> void
	{
		if (!IsExternallyAllocated())
			return;

		m_nGrowSize = nGrowSize;
		if (m_nAllocationCount)
		{
			int nNumBytes = m_nAllocationCount * sizeof(T);
			T* pMemory = (T*)malloc(nNumBytes);
			memcpy(pMemory, m_pMemory, nNumBytes);
			m_pMemory = pMemory;
		}
		else
		{
			m_pMemory = 0;
		}
	}

	//-----------------------------------------------------------------------------
	// Attaches the buffer to external memory....
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::SetExternalBuffer(T* pMemory, int numElements) -> void
	{
		// Blow away any existing allocated memory
		Purge();

		m_pMemory = pMemory;
		m_nAllocationCount = numElements;

		// Indicate that we don't own the memory
		m_nGrowSize = EXTERNAL_BUFFER_MARKER;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::SetExternalBuffer(const T* pMemory, int numElements) -> void
	{
		// Blow away any existing allocated memory
		Purge();

		m_pMemory = const_cast<T*>(pMemory);
		m_nAllocationCount = numElements;

		// Indicate that we don't own the memory
		m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::AssumeMemory(T* pMemory, int numElements) -> void
	{
		// Blow away any existing allocated memory
		Purge();

		// Simply take the pointer but don't mark us as external
		m_pMemory = pMemory;
		m_nAllocationCount = numElements;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::DetachMemory() -> void*
	{
		if (IsExternallyAllocated())
			return 0;

		void* pMemory = m_pMemory;
		m_pMemory = 0;
		m_nAllocationCount = 0;
		return pMemory;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Detach() -> T*
	{
		return (T*)DetachMemory();
	}

	//-----------------------------------------------------------------------------
	// element access
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::operator[](I i) -> T&
	{
		assert(!IsReadOnly());
		assert(IsIdxValid(i));
		return m_pMemory[i];
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::operator[](I i) const -> const T&
	{
		assert(IsIdxValid(i));
		return m_pMemory[i];
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Element(I i) -> T&
	{
		assert(!IsReadOnly());
		assert(IsIdxValid(i));
		return m_pMemory[i];
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Element(I i) const -> const T&
	{
		assert(IsIdxValid(i));
		return m_pMemory[i];
	}

	//-----------------------------------------------------------------------------
	// is the memory externally allocated?
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::IsExternallyAllocated() const -> bool
	{
		return (m_nGrowSize < 0);
	}

	//-----------------------------------------------------------------------------
	// is the memory read only?
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::IsReadOnly() const -> bool
	{
		return (m_nGrowSize == EXTERNAL_CONST_BUFFER_MARKER);
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::SetGrowSize(int nSize) -> void
	{
		assert(!IsExternallyAllocated());
		assert(nSize >= 0);
		m_nGrowSize = nSize;
		ValidateGrowSize();
	}

	//-----------------------------------------------------------------------------
	// Gets the base utl::address (can change when adding elements!)
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::Base() -> T*
	{
		assert(!IsReadOnly());
		return m_pMemory;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Base() const -> const T*
	{
		return m_pMemory;
	}

	//-----------------------------------------------------------------------------
	// Size
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::NumAllocated() const -> int
	{
		return m_nAllocationCount;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Count() const -> int
	{
		return m_nAllocationCount;
	}

	//-----------------------------------------------------------------------------
	// Is element index valid?
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::IsIdxValid(I i) const -> bool
	{
		// GCC warns if I is an unsigned type and we do a ">= 0" against it (since the comparison is always 0).
		// We Get the warning even if we cast inside the expression. It only goes away if we assign to another variable.
		long x = i;
		return (x >= 0) && (x < m_nAllocationCount);
	}

	//-----------------------------------------------------------------------------
	// Grows the memory
	//-----------------------------------------------------------------------------
	inline auto UtlMemory_CalcNewAllocationCount(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem) -> int
	{
		if (nGrowSize)
		{
			nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
		}
		else
		{
			if (!nAllocationCount)
			{
				// Compute an allocation which is at least as big as a cache line...
				nAllocationCount = (31 + nBytesItem) / nBytesItem;
			}

			while (nAllocationCount < nNewSize)
			{
#ifndef _X360
				nAllocationCount *= 2;
#else
				int nNewAllocationCount = (nAllocationCount * 9) / 8; // 12.5 %
				if (nNewAllocationCount > nAllocationCount)
					nAllocationCount = nNewAllocationCount;
				else
					nAllocationCount *= 2;
#endif
			}
		}

		return nAllocationCount;
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Grow(int num) -> void
	{
		assert(num > 0);

		if (IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			assert(0);
			return;
		}

		auto oldAllocationCount = m_nAllocationCount;
		// Make sure we have at least numallocated + num allocations.
		// Use the grow rules specified for this memory (in m_nGrowSize)
		int nAllocationRequested = m_nAllocationCount + num;

		int nNewAllocationCount = UtlMemory_CalcNewAllocationCount(m_nAllocationCount, m_nGrowSize, nAllocationRequested, sizeof(T));

		// if m_nAllocationRequested wraps index type I, recalculate
		if ((int)(I)nNewAllocationCount < nAllocationRequested)
		{
			if ((int)(I)nNewAllocationCount == 0 && (int)(I)(nNewAllocationCount - 1) >= nAllocationRequested)
			{
				--nNewAllocationCount; // deal w/ the common case of m_nAllocationCount == MAX_USHORT + 1
			}
			else
			{
				if ((int)(I)nAllocationRequested != nAllocationRequested)
				{
					// we've been asked to grow memory to a size s.t. the index type can't utl::address the requested amount of memory
					assert(0);
					return;
				}
				while ((int)(I)nNewAllocationCount < nAllocationRequested)
				{
					nNewAllocationCount = (nNewAllocationCount + nAllocationRequested) / 2;
				}
			}
		}

		m_nAllocationCount = nNewAllocationCount;

		if (m_pMemory)
		{
			auto ptr = new unsigned char[m_nAllocationCount * sizeof(T)];

			memcpy(ptr, m_pMemory, oldAllocationCount * sizeof(T));
			m_pMemory = (T*)ptr;
		}
		else
		{
			m_pMemory = (T*)new unsigned char[m_nAllocationCount * sizeof(T)];
		}
	}

	//-----------------------------------------------------------------------------
	// Makes sure we've got at least this much memory
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::EnsureCapacity(int num) -> void
	{
		if (m_nAllocationCount >= num)
			return;

		if (IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			assert(0);
			return;
		}
		m_nAllocationCount = num;

		if (m_pMemory)
		{
			m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
		}
		else
		{
			m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
		}
	}

	//-----------------------------------------------------------------------------
	// Memory deallocation
	//-----------------------------------------------------------------------------
	template <class T, class I>
	auto CUtlMemory<T, I>::Purge() -> void
	{
		if (!IsExternallyAllocated())
		{
			if (m_pMemory)
			{
				free((void*)m_pMemory);
				m_pMemory = 0;
			}
			m_nAllocationCount = 0;
		}
	}

	template <class T, class I>
	auto CUtlMemory<T, I>::Purge(int numElements) -> void
	{
		assert(numElements >= 0);

		if (numElements > m_nAllocationCount)
		{
			// Ensure this isn't a grow request in disguise.
			assert(numElements <= m_nAllocationCount);
			return;
		}

		// If we have zero elements, simply do a purge:
		if (numElements == 0)
		{
			Purge();
			return;
		}

		if (IsExternallyAllocated())
		{
			// Can't shrink a buffer whose memory was externally allocated, fail silently like purge 
			return;
		}

		// If the number of elements is the same as the allocation count, we are done.
		if (numElements == m_nAllocationCount)
		{
			return;
		}

		if (!m_pMemory)
		{
			// Allocation count is non zero, but memory is null.
			assert(m_pMemory);
			return;
		}
		m_nAllocationCount = numElements;
		m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
	}

	//-----------------------------------------------------------------------------
	// The CUtlMemory class:
	// A growable memory class which doubles in size by default.
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	class CUtlMemoryAligned : public CUtlMemory<T>
	{
	public:
		// constructor, destructor
		CUtlMemoryAligned(int nGrowSize = 0, int nInitSize = 0);
		CUtlMemoryAligned(T* pMemory, int numElements);
		CUtlMemoryAligned(const T* pMemory, int numElements);
		~CUtlMemoryAligned();

		// Attaches the buffer to external memory....
		auto SetExternalBuffer(T* pMemory, int numElements) -> void;
		auto SetExternalBuffer(const T* pMemory, int numElements) -> void;

		// Grows the memory, so that at least allocated + num elements are allocated
		auto Grow(int num = 1) -> void;

		// Makes sure we've got at least this much memory
		auto EnsureCapacity(int num) -> void;

		// Memory deallocation
		auto Purge() -> void;

		// Purge all but the given number of elements (NOT IMPLEMENTED IN CUtlMemoryAligned)
		auto Purge(int numElements) -> void
		{
			assert(0);
		}

	private:
		auto Align(const void* pAddr) -> void*;
	};

	//-----------------------------------------------------------------------------
	// Aligns a pointer
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::Align(const void* pAddr) -> void*
	{
		size_t nAlignmentMask = nAlignment - 1;
		return (void*)(((size_t)pAddr + nAlignmentMask) & (~nAlignmentMask));
	}

	//-----------------------------------------------------------------------------
	// constructor, destructor
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	CUtlMemoryAligned<T, nAlignment>::CUtlMemoryAligned(int nGrowSize, int nInitAllocationCount)
	{
		CUtlMemory<T>::m_pMemory = 0;
		CUtlMemory<T>::m_nAllocationCount = nInitAllocationCount;
		CUtlMemory<T>::m_nGrowSize = nGrowSize;
		this->ValidateGrowSize();

		// Alignment must be a power of two
		COMPILE_TIME_ASSERT((nAlignment & (nAlignment - 1)) == 0);
		assert((nGrowSize >= 0) && (nGrowSize != CUtlMemory<T>::EXTERNAL_BUFFER_MARKER));
		if (CUtlMemory<T>::m_nAllocationCount)
		{
			UTLMEMORY_TRACK_ALLOC();
			MEM_ALLOC_CREDIT_CLASS();
			CUtlMemory<T>::m_pMemory = (T*)_aligned_malloc(nInitAllocationCount * sizeof(T), nAlignment);
		}
	}

	template <class T, int nAlignment>
	CUtlMemoryAligned<T, nAlignment>::CUtlMemoryAligned(T* pMemory, int numElements)
	{
		// Special marker indicating externally supplied memory
		CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_BUFFER_MARKER;

		CUtlMemory<T>::m_pMemory = (T*)Align(pMemory);
		CUtlMemory<T>::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory) / sizeof(T);
	}

	template <class T, int nAlignment>
	CUtlMemoryAligned<T, nAlignment>::CUtlMemoryAligned(const T* pMemory, int numElements)
	{
		// Special marker indicating externally supplied memory
		CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_CONST_BUFFER_MARKER;

		CUtlMemory<T>::m_pMemory = (T*)Align(pMemory);
		CUtlMemory<T>::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory) / sizeof(T);
	}

	template <class T, int nAlignment>
	CUtlMemoryAligned<T, nAlignment>::~CUtlMemoryAligned()
	{
		Purge();
	}

	//-----------------------------------------------------------------------------
	// Attaches the buffer to external memory....
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::SetExternalBuffer(T* pMemory, int numElements) -> void
	{
		// Blow away any existing allocated memory
		Purge();

		CUtlMemory<T>::m_pMemory = (T*)Align(pMemory);
		CUtlMemory<T>::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory) / sizeof(T);

		// Indicate that we don't own the memory
		CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_BUFFER_MARKER;
	}

	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::SetExternalBuffer(const T* pMemory, int numElements) -> void
	{
		// Blow away any existing allocated memory
		Purge();

		CUtlMemory<T>::m_pMemory = (T*)Align(pMemory);
		CUtlMemory<T>::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory) / sizeof(T);

		// Indicate that we don't own the memory
		CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_CONST_BUFFER_MARKER;
	}

	//-----------------------------------------------------------------------------
	// Grows the memory
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::Grow(int num) -> void
	{
		assert(num > 0);

		if (this->IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			assert(0);
			return;
		}

		UTLMEMORY_TRACK_FREE();

		// Make sure we have at least numallocated + num allocations.
		// Use the grow rules specified for this memory (in m_nGrowSize)
		int nAllocationRequested = CUtlMemory<T>::m_nAllocationCount + num;

		CUtlMemory<T>::m_nAllocationCount = UtlMemory_CalcNewAllocationCount(CUtlMemory<T>::m_nAllocationCount, CUtlMemory<T>::m_nGrowSize, nAllocationRequested, sizeof(T));

		UTLMEMORY_TRACK_ALLOC();

		if (CUtlMemory<T>::m_pMemory)
		{
			MEM_ALLOC_CREDIT_CLASS();
			CUtlMemory<T>::m_pMemory = (T*)MemAlloc_ReallocAligned(CUtlMemory<T>::m_pMemory, CUtlMemory<T>::m_nAllocationCount * sizeof(T), nAlignment);
			assert(CUtlMemory<T>::m_pMemory);
		}
		else
		{
			MEM_ALLOC_CREDIT_CLASS();
			CUtlMemory<T>::m_pMemory = (T*)MemAlloc_AllocAligned(CUtlMemory<T>::m_nAllocationCount * sizeof(T), nAlignment);
			assert(CUtlMemory<T>::m_pMemory);
		}
	}

	//-----------------------------------------------------------------------------
	// Makes sure we've got at least this much memory
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::EnsureCapacity(int num) -> void
	{
		if (CUtlMemory<T>::m_nAllocationCount >= num)
			return;

		if (this->IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			assert(0);
			return;
		}

		UTLMEMORY_TRACK_FREE();

		CUtlMemory<T>::m_nAllocationCount = num;

		UTLMEMORY_TRACK_ALLOC();

		if (CUtlMemory<T>::m_pMemory)
		{
			MEM_ALLOC_CREDIT_CLASS();
			CUtlMemory<T>::m_pMemory = (T*)MemAlloc_ReallocAligned(CUtlMemory<T>::m_pMemory, CUtlMemory<T>::m_nAllocationCount * sizeof(T), nAlignment);
		}
		else
		{
			MEM_ALLOC_CREDIT_CLASS();
			CUtlMemory<T>::m_pMemory = (T*)MemAlloc_AllocAligned(CUtlMemory<T>::m_nAllocationCount * sizeof(T), nAlignment);
		}
	}

	//-----------------------------------------------------------------------------
	// Memory deallocation
	//-----------------------------------------------------------------------------
	template <class T, int nAlignment>
	auto CUtlMemoryAligned<T, nAlignment>::Purge() -> void
	{
		if (!this->IsExternallyAllocated())
		{
			if (CUtlMemory<T>::m_pMemory)
			{
				UTLMEMORY_TRACK_FREE();
				MemAlloc_FreeAligned(CUtlMemory<T>::m_pMemory);
				CUtlMemory<T>::m_pMemory = 0;
			}
			CUtlMemory<T>::m_nAllocationCount = 0;
		}
	}

#endif

	//valve shit moved to read-only

	template <class T, class N = int>
	class CUtlMemory
	{
	public:
		T& operator[](N i)
		{
			return pMemory[i];
		}

		const T& operator[](N i) const
		{
			return pMemory[i];
		}

		T* data()
		{
			return pMemory;
		}

		const T* data()const
		{
			return pMemory;
		}

		int NumAllocated() const
		{
			return iAllocationCount;
		}

#if 0
		auto Grow(int iNum = 1) -> void
		{
			if (IsExternallyAllocated())
				return;

			int iOldAllocationCount = iAllocationCount;
			int iAllocationRequested = iAllocationCount + iNum;
			int iNewAllocationCount = UtlMemory_CalcNewAllocationCount(iAllocationCount, iGrowSize, iAllocationRequested, sizeof(T));

			if (static_cast<int>(static_cast<N>(iNewAllocationCount)) < iAllocationRequested)
			{
				if (static_cast<int>(static_cast<N>(iNewAllocationCount)) == 0 && static_cast<int>(static_cast<N>(iNewAllocationCount - 1)) >= iAllocationRequested)
					--iNewAllocationCount;
				else
				{
					if (static_cast<int>(static_cast<N>(iAllocationRequested)) != iAllocationRequested)
					{
						return;
					}

					while (static_cast<int>(static_cast<N>(iNewAllocationCount)) < iAllocationRequested)
						iNewAllocationCount = (iNewAllocationCount + iAllocationRequested) / 2;
				}
			}

			iAllocationCount = iNewAllocationCount;

			if (pMemory != nullptr)
			{
				//pMemory = reinterpret_cast<T*>(I::MemAlloc->Realloc(pMemory, iAllocationCount * sizeof(T)));

				std::byte* pData = new std::byte[iAllocationCount * sizeof(T)];
				std::memcpy(pData, pMemory, iOldAllocationCount * sizeof(T));
				pMemory = reinterpret_cast<T*>(pData);
			}
			else
				//pMemory = reinterpret_cast<T*>(I::MemAlloc->Alloc(iAllocationCount * sizeof(T)));
				pMemory = reinterpret_cast<T*>(new std::byte[iAllocationCount * sizeof(T)]);
		}
#endif

		bool IsExternallyAllocated() const
		{
			return iGrowSize < 0;
		}

		//--

	protected:
		T* pMemory;
		int iAllocationCount;
		int iGrowSize;
	};
}
