module;

#include <span>

export module cheat.csgo.interfaces.Recv;

export namespace cheat::csgo
{
	enum SendPropType
	{
		DPT_Int = 0
	  , DPT_Float
	  , DPT_Vector
	  , DPT_VectorXY
	  , DPT_String
	  , DPT_Array
	  , DPT_DataTable
	  , DPT_Int64
	};

	class DVariant
	{
	public:
		union
		{
			float m_Float;
			long m_Int;
			char* m_pString;
			void* m_pData;
			float m_Vector[3];
			__int64 m_Int64;
		};

		SendPropType m_Type;
	};

	class RecvTable;
	class RecvProp;

	class CRecvProxyData
	{
	public:
		const RecvProp* m_pRecvProp; // The property it's receiving.
		DVariant m_Value;            // The value given to you to store.
		int m_iElement;              // Which array element you're getting.
		int m_ObjectID;              // The object being referred to.
	};

	//-----------------------------------------------------------------------------
	// pStruct = the base structure of the datatable this variable is in (like C_BaseEntity)
	// pOut    = the variable that this this proxy represents (like C_BaseEntity::m_SomeValue).
	//
	// Convert the network-standard-type value in m_Value into your own format in pStruct/pOut.
	//-----------------------------------------------------------------------------
	using RecvVarProxyFn = void(*)(const CRecvProxyData* pData, void* pStruct, void* pOut);

	// ------------------------------------------------------------------------ //
	// ArrayLengthRecvProxies are optionally used to Get the length of the 
	// incoming array when it changes.
	// ------------------------------------------------------------------------ //
	using ArrayLengthRecvProxyFn = void(*)(void* pStruct, int objectID, int currentArrayLength);

	// NOTE: DataTable receive proxies work differently than the other proxies.
	// pData points at the object + the recv table's offset.
	// pOut should be Set to the location of the object to unpack the data table into.
	// If the parent object just contains the child object, the default proxy just does *pOut = pData.
	// If the parent object points at the child object, you need to dereference the pointer here.
	// NOTE: don't ever return null from a DataTable receive proxy function. Bad things will happen.
	using DataTableRecvVarProxyFn = void(*)(const RecvProp* pProp, void** pOut, void* pData, int objectID);

	class RecvProp
	{
	public:
		char* m_pVarName;
		SendPropType m_RecvType;
		int m_Flags;
		int m_StringBufferSize;
		int m_bInsideArray;
		const void* m_pExtraData;
		RecvProp* m_pArrayProp;
		ArrayLengthRecvProxyFn m_ArrayLengthProxy;
		RecvVarProxyFn m_ProxyFn;
		DataTableRecvVarProxyFn m_DataTableProxyFn;
		RecvTable* m_pDataTable;
		int m_Offset;
		int m_ElementStride;
		int m_nElements;
		const char* m_pParentArrayPropName;

#if 0
		RecvVarProxyFn GetProxyFn() const
		{
			return m_ProxyFn;
		}

		void SetProxyFn(RecvVarProxyFn fn)
		{
			m_ProxyFn = fn;
		}

		DataTableRecvVarProxyFn GetDataTableProxyFn() const
		{
			return m_DataTableProxyFn;
		}

		void SetDataTableProxyFn(DataTableRecvVarProxyFn fn)
		{
			m_DataTableProxyFn = fn;
		}
#endif
	};

	class RecvTable
	{
	public:
		//RecvProp* m_pProps;
		//int       m_nProps;
		std::span<RecvProp> props;
		void* m_pDecoder;
		char* m_pNetTableName;
		bool m_bInitialized;
		bool m_bInMainList;
	};
}
