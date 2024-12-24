#include "keyvalues3.h"
#include <cstdint>

// Nasty hack to redefine gcc's offsetof which doesn't like GET_OUTER macro
#ifdef COMPILER_GCC
#undef offsetof
#define offsetof(s, m) (size_t) & (((s*)0)->m)
#endif

KeyValues3::KeyValues3(KV3TypeEx_t type, KV3SubType_t subtype)
	: m_bExternalStorage(true), m_TypeEx(type), m_SubType(subtype), m_nFlags(0), m_nReserved(0), m_nNumArrayElements(0), m_nData(0) {
	ResolveUnspecified();
	Alloc();
}

KeyValues3::KeyValues3(int cluster_elem, KV3TypeEx_t type, KV3SubType_t subtype)
	: m_bExternalStorage(false), m_TypeEx(type), m_SubType(subtype), m_nFlags(0), m_nClusterElement(cluster_elem), m_nNumArrayElements(0),
	m_nReserved(0), m_nData(0) {
	ResolveUnspecified();
	Alloc();
}

KeyValues3::~KeyValues3() {
	Free();
};

#include "tier0/memdbgoff.h"

void KeyValues3::Alloc(int nAllocSize, uint64 a3, int nValidBytes, uint8 a5) {
	switch (GetTypeEx()) {
	case KV3_TYPEEX_ARRAY: {
		if (nValidBytes <= 0) {
			CKeyValues3Context* context = GetContext();
			if (context) {
				m_pArray = context->AllocArray(nAllocSize);
				if (!m_pArray) {
					int nSize = 64;
					if (nAllocSize > 0) {
						nSize = 8 * nAllocSize + 16;
					}

					m_pArray = (CKeyValues3Array*)g_pMemAlloc->RegionAlloc(58, nSize);
					new (m_pArray) CKeyValues3Array(nAllocSize);
				}
			}
			else {
				m_pArray = new CKeyValues3Array(nAllocSize);
			}
		}
		else {
		}
		break;
	}
	case KV3_TYPEEX_TABLE: {
		if (nValidBytes <= 0) {
			CKeyValues3Context* context = GetContext();
			if (context) {
				m_pTable = context->AllocTable(nAllocSize);
				if (!m_pTable) {
					int nSize = 32;
					if (nAllocSize > 0) {
						nSize = 17 * nAllocSize + KV3Helpers::CalcAlighedChunk(nAllocSize) + 24;
					}

					m_pTable = (CKeyValues3Table*)g_pMemAlloc->RegionAlloc(58, nSize);
					new (m_pTable) CKeyValues3Table(nAllocSize);
				}
			}
			else {
				m_pTable = new CKeyValues3Table(nAllocSize);
			}
		}
		else {
		}
		break;
	}
	case KV3_TYPEEX_ARRAY_FLOAT32:
	case KV3_TYPEEX_ARRAY_FLOAT64:
	case KV3_TYPEEX_ARRAY_INT16:
	case KV3_TYPEEX_ARRAY_INT32:
	case KV3_TYPEEX_ARRAY_UINT8_SHORT:
	case KV3_TYPEEX_ARRAY_INT16_SHORT: {
		m_bFreeArrayMemory = false;
		m_nNumArrayElements = 0;
		m_nData = 0;
		break;
	}
	default:
		break;
	}
}

#include "tier0/memdbgon.h"

void KeyValues3::Free(bool bClearingContext) {
	switch (GetTypeEx()) {
	case KV3_TYPEEX_STRING: {
		free((void*)m_pString);
		m_pString = NULL;
		break;
	}
	case KV3_TYPEEX_BINARY_BLOB: {
		if (m_pBinaryBlob) {
			free(m_pBinaryBlob);
		}
		m_pBinaryBlob = NULL;
		break;
	}
	case KV3_TYPEEX_BINARY_BLOB_EXTERN: {
		if (m_pBinaryBlob) {
			if (m_pBinaryBlob->m_bFreeMemory) {
				free((void*)m_pBinaryBlob->m_pubData);
			}
			free(m_pBinaryBlob);
		}
		m_pBinaryBlob = NULL;
		break;
	}
	case KV3_TYPEEX_ARRAY: {
		m_pArray->Purge(bClearingContext);

		CKeyValues3Context* context = GetContext();

		if (context) {
			if (!bClearingContext) {
				context->FreeArray(m_pArray);
			}
		}
		else {
			delete m_pArray;
		}

		m_pArray = NULL;
		break;
	}
	case KV3_TYPEEX_TABLE: {
		m_pTable->Purge(bClearingContext);

		CKeyValues3Context* context = GetContext();

		if (context) {
			if (!bClearingContext) {
				context->FreeTable(m_pTable);
			}
		}
		else {
			delete m_pTable;
		}

		m_pTable = NULL;
		break;
	}
	case KV3_TYPEEX_ARRAY_FLOAT32:
	case KV3_TYPEEX_ARRAY_FLOAT64:
	case KV3_TYPEEX_ARRAY_INT16:
	case KV3_TYPEEX_ARRAY_INT32:
	case KV3_TYPEEX_ARRAY_UINT8_SHORT:
	case KV3_TYPEEX_ARRAY_INT16_SHORT: {
		if (m_bFreeArrayMemory) {
			free(m_pData);
		}
		m_bFreeArrayMemory = false;
		m_nNumArrayElements = 0;
		m_nData = 0;
		break;
	}
	default:
		break;
	}
}

void KeyValues3::ResolveUnspecified() {
	if (GetSubType() == KV3_SUBTYPE_UNSPECIFIED) {
		switch (GetType()) {
		case KV3_TYPE_NULL:
			m_SubType = KV3_SUBTYPE_NULL;
			break;
		case KV3_TYPE_BOOL:
			m_SubType = KV3_SUBTYPE_BOOL8;
			break;
		case KV3_TYPE_INT:
			m_SubType = KV3_SUBTYPE_INT64;
			break;
		case KV3_TYPE_UINT:
			m_SubType = KV3_SUBTYPE_UINT64;
			break;
		case KV3_TYPE_DOUBLE:
			m_SubType = KV3_SUBTYPE_FLOAT64;
			break;
		case KV3_TYPE_STRING:
			m_SubType = KV3_SUBTYPE_STRING;
			break;
		case KV3_TYPE_BINARY_BLOB:
			m_SubType = KV3_SUBTYPE_BINARY_BLOB;
			break;
		case KV3_TYPE_ARRAY:
			m_SubType = KV3_SUBTYPE_ARRAY;
			break;
		case KV3_TYPE_TABLE:
			m_SubType = KV3_SUBTYPE_TABLE;
			break;
		default:
			m_SubType = KV3_SUBTYPE_INVALID;
			break;
		}
	}
}

void KeyValues3::PrepareForType(KV3TypeEx_t type, KV3SubType_t subtype, int size, uint64 a5, int validBytes, uint8 a7) {
	if (GetTypeEx() == type) {
		switch (type) {
		case KV3_TYPEEX_STRING:
		case KV3_TYPEEX_BINARY_BLOB:
		case KV3_TYPEEX_BINARY_BLOB_EXTERN:
		case KV3_TYPEEX_ARRAY_FLOAT32:
		case KV3_TYPEEX_ARRAY_FLOAT64:
		case KV3_TYPEEX_ARRAY_INT16:
		case KV3_TYPEEX_ARRAY_INT32:
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
		case KV3_TYPEEX_ARRAY_INT16_SHORT: {
			Free();
			break;
		}
		default:
			break;
		}
	}
	else {
		Free();
		m_TypeEx = type;
		m_nData = 0;
		Alloc();
	}

	m_SubType = subtype;
}

void KeyValues3::OnClearContext() {
	Free(true);
	m_TypeEx = KV3_TYPEEX_NULL;
	m_nData = 0;
}

CKeyValues3Cluster* KeyValues3::GetCluster() const {
	if (m_bExternalStorage) {
		return NULL;
	}

	return GET_OUTER(CKeyValues3Cluster, m_KeyValues[m_nClusterElement]);
}

CKeyValues3Context* KeyValues3::GetContext() const {
	CKeyValues3Cluster* cluster = GetCluster();

	if (cluster) {
		return cluster->GetContext();
	}
	else {
		return NULL;
	}
}

KV3MetaData_t* KeyValues3::GetMetaData(CKeyValues3Context** ppCtx) const {
	CKeyValues3Cluster* cluster = GetCluster();

	if (cluster) {
		if (ppCtx) {
			*ppCtx = cluster->GetContext();
		}

		return cluster->GetMetaData(m_nClusterElement);
	}
	else {
		if (ppCtx) {
			*ppCtx = NULL;
		}

		return NULL;
	}
}

const char* KeyValues3::GetString(const char* defaultValue) const {
	switch (GetTypeEx()) {
	case KV3_TYPEEX_STRING:
	case KV3_TYPEEX_STRING_EXTERN:
		return m_pString;
	case KV3_TYPEEX_STRING_SHORT:
		return m_szStringShort;
	default:
		return defaultValue;
	}
}

void KeyValues3::SetString(const char* pString, KV3SubType_t subtype) {
	if (!pString) {
		pString = "";
	}

	if (strlen(pString) < sizeof(m_szStringShort)) {
		PrepareForType(KV3_TYPEEX_STRING_SHORT, subtype);
		V_strncpy(m_szStringShort, pString, sizeof(m_szStringShort));
	}
	else {
		PrepareForType(KV3_TYPEEX_STRING, subtype);
		m_pString = strdup(pString);
	}
}

void KeyValues3::SetStringExternal(const char* pString, KV3SubType_t subtype) {
	if (strlen(pString) < sizeof(m_szStringShort)) {
		PrepareForType(KV3_TYPEEX_STRING_SHORT, subtype);
		V_strncpy(m_szStringShort, pString, sizeof(m_szStringShort));
	}
	else {
		PrepareForType(KV3_TYPEEX_STRING_EXTERN, subtype);
		m_pString = pString;
	}
}

const byte* KeyValues3::GetBinaryBlob() const {
	switch (GetTypeEx()) {
	case KV3_TYPEEX_BINARY_BLOB:
		return m_pBinaryBlob ? m_pBinaryBlob->m_ubData : NULL;
	case KV3_TYPEEX_BINARY_BLOB_EXTERN:
		return m_pBinaryBlob ? m_pBinaryBlob->m_pubData : NULL;
	default:
		return NULL;
	}
}

int KeyValues3::GetBinaryBlobSize() const {
	if (GetType() != KV3_TYPE_BINARY_BLOB || !m_pBinaryBlob) {
		return 0;
	}

	return (int)m_pBinaryBlob->m_nSize;
}

void KeyValues3::SetToBinaryBlob(const byte* blob, int size) {
	PrepareForType(KV3_TYPEEX_BINARY_BLOB, KV3_SUBTYPE_BINARY_BLOB);

	if (size > 0) {
		m_pBinaryBlob = (KV3BinaryBlob_t*)malloc(sizeof(size_t) + size);
		m_pBinaryBlob->m_nSize = size;
		memcpy(m_pBinaryBlob->m_ubData, blob, size);
	}
	else {
		m_pBinaryBlob = NULL;
	}
}

void KeyValues3::SetToBinaryBlobExternal(const byte* blob, int size, bool free_mem) {
	PrepareForType(KV3_TYPEEX_BINARY_BLOB_EXTERN, KV3_SUBTYPE_BINARY_BLOB);

	if (size > 0) {
		m_pBinaryBlob = (KV3BinaryBlob_t*)malloc(sizeof(KV3BinaryBlob_t));
		m_pBinaryBlob->m_nSize = size;
		m_pBinaryBlob->m_pubData = blob;
		m_pBinaryBlob->m_bFreeMemory = free_mem;
	}
	else {
		m_pBinaryBlob = NULL;
	}
}

Color KeyValues3::GetColor(const Color& defaultValue) const {
	int32 color[4];
	if (ReadArrayInt32(4, color)) {
		return Color(color[0], color[1], color[2], color[3]);
	}
	else if (ReadArrayInt32(3, color)) {
		return Color(color[0], color[1], color[2], 255);
	}
	else {
		return defaultValue;
	}
}

void KeyValues3::SetColor(const Color& color) {
	if (color.a() == 255) {
		AllocArray<uint8>(3, &color[0], KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT, KV3_TYPEEX_INVALID, KV3_SUBTYPE_COLOR32,
			KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8);
	}
	else {
		AllocArray<uint8>(4, &color[0], KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT, KV3_TYPEEX_INVALID, KV3_SUBTYPE_COLOR32,
			KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8);
	}
}

int KeyValues3::GetArrayElementCount() const {
	if (GetType() != KV3_TYPE_ARRAY) {
		return 0;
	}

	if (GetTypeEx() == KV3_TYPEEX_ARRAY) {
		return m_pArray->Count();
	}
	else {
		return m_nNumArrayElements;
	}
}

KeyValues3** KeyValues3::GetArrayBase() {
	if (GetType() != KV3_TYPE_ARRAY) {
		return NULL;
	}

	NormalizeArray();

	return m_pArray->Base();
}

KeyValues3* KeyValues3::GetArrayElement(int elem) {
	if (GetType() != KV3_TYPE_ARRAY) {
		return NULL;
	}

	NormalizeArray();

	if (elem < 0 || elem >= m_pArray->Count()) {
		return NULL;
	}

	return m_pArray->Element(elem);
}

KeyValues3* KeyValues3::InsertArrayElementBefore(int elem) {
	if (GetType() != KV3_TYPE_ARRAY) {
		return NULL;
	}

	NormalizeArray();

	return *m_pArray->InsertBeforeGetPtr(elem, 1);
}

KeyValues3* KeyValues3::AddArrayElementToTail() {
	if (GetType() != KV3_TYPE_ARRAY) {
		PrepareForType(KV3_TYPEEX_ARRAY, KV3_SUBTYPE_ARRAY);
	}
	else {
		NormalizeArray();
	}

	return *m_pArray->InsertBeforeGetPtr(m_pArray->Count(), 1);
}

void KeyValues3::SetArrayElementCount(int count, KV3TypeEx_t type, KV3SubType_t subtype) {
	if (GetType() != KV3_TYPE_ARRAY) {
		PrepareForType(KV3_TYPEEX_ARRAY, KV3_SUBTYPE_ARRAY);
	}
	else {
		NormalizeArray();
	}

	m_pArray->SetCount(count, type, subtype);
}

void KeyValues3::RemoveArrayElements(int elem, int num) {
	if (GetType() != KV3_TYPE_ARRAY) {
		return;
	}

	NormalizeArray();

	m_pArray->RemoveMultiple(elem, num);
}

void KeyValues3::NormalizeArray() {
	switch (GetTypeEx()) {
	case KV3_TYPEEX_ARRAY_FLOAT32: {
		NormalizeArray<float>(KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32, m_nNumArrayElements, m_f32Array, m_bFreeArrayMemory);
		break;
	}
	case KV3_TYPEEX_ARRAY_FLOAT64: {
		NormalizeArray<double>(KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT64, m_nNumArrayElements, m_f64Array, m_bFreeArrayMemory);
		break;
	}
	case KV3_TYPEEX_ARRAY_INT16: {
		NormalizeArray<int16>(KV3_TYPEEX_INT, KV3_SUBTYPE_INT16, m_nNumArrayElements, m_i16Array, m_bFreeArrayMemory);
		break;
	}
	case KV3_TYPEEX_ARRAY_INT32: {
		NormalizeArray<int32>(KV3_TYPEEX_INT, KV3_SUBTYPE_INT32, m_nNumArrayElements, m_i32Array, m_bFreeArrayMemory);
		break;
	}
	case KV3_TYPEEX_ARRAY_UINT8_SHORT: {
		uint8 u8ArrayShort[8];
		memcpy(u8ArrayShort, m_u8ArrayShort, sizeof(u8ArrayShort));
		NormalizeArray<uint8>(KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8, m_nNumArrayElements, u8ArrayShort, false);
		break;
	}
	case KV3_TYPEEX_ARRAY_INT16_SHORT: {
		int16 i16ArrayShort[4];
		memcpy(i16ArrayShort, m_i16ArrayShort, sizeof(i16ArrayShort));
		NormalizeArray<int16>(KV3_TYPEEX_INT, KV3_SUBTYPE_INT16, m_nNumArrayElements, i16ArrayShort, false);
		break;
	}
	default:
		break;
	}
}

bool KeyValues3::ReadArrayInt32(int dest_size, int32* data) const {
	int src_size = 0;

	if (GetType() == KV3_TYPE_STRING) {
		CSplitString values(GetString(), " ");
		src_size = values.Count();
		int count = MIN(src_size, dest_size);
		for (int i = 0; i < count; ++i) {
			data[i] = V_StringToInt32(values[i], 0, NULL, NULL, PARSING_FLAG_SKIP_ASSERT);
		}
	}
	else {
		switch (GetTypeEx()) {
		case KV3_TYPEEX_ARRAY: {
			src_size = m_pArray->Count();
			int count = MIN(src_size, dest_size);
			KeyValues3** arr = m_pArray->Base();
			for (int i = 0; i < count; ++i) {
				data[i] = arr[i]->GetInt();
			}
			break;
		}
		case KV3_TYPEEX_ARRAY_INT16: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			for (int i = 0; i < count; ++i) {
				data[i] = (int32)m_i16Array[i];
			}
			break;
		}
		case KV3_TYPEEX_ARRAY_INT32: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			memcpy(data, m_i32Array, count * sizeof(int32));
			break;
		}
		case KV3_TYPEEX_ARRAY_UINT8_SHORT: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			for (int i = 0; i < count; ++i) {
				data[i] = (int32)m_u8ArrayShort[i];
			}
			break;
		}
		case KV3_TYPEEX_ARRAY_INT16_SHORT: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			for (int i = 0; i < count; ++i) {
				data[i] = (int32)m_i16ArrayShort[i];
			}
			break;
		}
		default:
			break;
		}
	}

	if (src_size < dest_size) {
		memset(&data[src_size], 0, (dest_size - src_size) * sizeof(int32));
	}

	return (src_size == dest_size);
}

bool KeyValues3::ReadArrayFloat32(int dest_size, float* data) const {
	int src_size = 0;

	if (GetType() == KV3_TYPE_STRING) {
		CSplitString values(GetString(), " ");
		src_size = values.Count();
		int count = MIN(src_size, dest_size);
		for (int i = 0; i < count; ++i) {
			data[i] = (float)V_StringToFloat64(values[i], 0, NULL, NULL, PARSING_FLAG_SKIP_ASSERT);
		}
	}
	else {
		switch (GetTypeEx()) {
		case KV3_TYPEEX_ARRAY: {
			src_size = m_pArray->Count();
			int count = MIN(src_size, dest_size);
			KeyValues3** arr = m_pArray->Base();
			for (int i = 0; i < count; ++i) {
				data[i] = arr[i]->GetFloat();
			}
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT32: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			memcpy(data, m_f32Array, count * sizeof(float));
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT64: {
			src_size = m_nNumArrayElements;
			int count = MIN(src_size, dest_size);
			for (int i = 0; i < count; ++i) {
				data[i] = (float)m_f64Array[i];
			}
			break;
		}
		default:
			break;
		}
	}

	if (src_size < dest_size) {
		memset(&data[src_size], 0, (dest_size - src_size) * sizeof(float));
	}

	return (src_size == dest_size);
}

int KeyValues3::GetMemberCount() const {
	if (GetType() != KV3_TYPE_TABLE) {
		return 0;
	}

	return m_pTable->GetMemberCount();
}

KeyValues3* KeyValues3::GetMember(KV3MemberId_t id) {
	if (GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_pTable->GetMemberCount()) {
		return NULL;
	}

	return m_pTable->GetMember(id);
}

const char* KeyValues3::GetMemberName(KV3MemberId_t id) const {
	if (GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_pTable->GetMemberCount()) {
		return NULL;
	}

	return m_pTable->GetMemberName(id);
}

CKV3MemberName KeyValues3::GetMemberNameEx(KV3MemberId_t id) const {
	if (GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_pTable->GetMemberCount()) {
		return CKV3MemberName();
	}

	return CKV3MemberName(m_pTable->GetMemberHash(id), m_pTable->GetMemberName(id));
}

unsigned int KeyValues3::GetMemberHash(KV3MemberId_t id) const {
	if (GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_pTable->GetMemberCount()) {
		return 0;
	}

	return m_pTable->GetMemberHash(id);
}

KeyValues3* KeyValues3::FindMember(const CKV3MemberName& name, KeyValues3* defaultValue) {
	if (GetType() != KV3_TYPE_TABLE) {
		return defaultValue;
	}

	KV3MemberId_t id = m_pTable->FindMember(name);

	if (id == KV3_INVALID_MEMBER) {
		return defaultValue;
	}

	return m_pTable->GetMember(id);
}

KeyValues3* KeyValues3::FindOrCreateMember(const CKV3MemberName& name, bool* pCreated) {
	if (GetType() != KV3_TYPE_TABLE) {
		PrepareForType(KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE);
	}

	KV3MemberId_t id = m_pTable->FindMember(name);

	if (id == KV3_INVALID_MEMBER) {
		if (pCreated) {
			*pCreated = true;
		}

		id = m_pTable->CreateMember(name);
	}
	else {
		if (pCreated) {
			*pCreated = false;
		}
	}

	return m_pTable->GetMember(id);
}

void KeyValues3::SetToEmptyTable() {
	PrepareForType(KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE);
	m_pTable->RemoveAll();
}

bool KeyValues3::RemoveMember(KV3MemberId_t id) {
	if (GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_pTable->GetMemberCount()) {
		return false;
	}

	m_pTable->RemoveMember(id);

	return true;
}

bool KeyValues3::RemoveMember(const KeyValues3* kv) {
	if (GetType() != KV3_TYPE_TABLE) {
		return false;
	}

	KV3MemberId_t id = m_pTable->FindMember(kv);

	if (id == KV3_INVALID_MEMBER) {
		return false;
	}

	m_pTable->RemoveMember(id);

	return true;
}

bool KeyValues3::RemoveMember(const CKV3MemberName& name) {
	if (GetType() != KV3_TYPE_TABLE) {
		return false;
	}

	KV3MemberId_t id = m_pTable->FindMember(name);

	if (id == KV3_INVALID_MEMBER) {
		return false;
	}

	m_pTable->RemoveMember(id);

	return true;
}

const char* KeyValues3::GetTypeAsString() const {
	static const char* s_Types[] = { "invalid", "null", "bool", "int", "uint", "double", "string", "binary_blob", "array", "table", NULL };

	KV3Type_t type = GetType();

	if (type < KV3_TYPE_COUNT) {
		return s_Types[type];
	}

	return "<unknown>";
}

const char* KeyValues3::GetSubTypeAsString() const {
	static const char* s_SubTypes[] = {
		"invalid",  "resource",        "resource_name", "panorama", "soundevent", "subclass",  "entity_name",  "unspecified", "null",   "binary_blob",
		"array",    "table",           "bool8",         "char8",    "uchar32",    "int8",      "uint8",        "int16",       "uint16", "int32",
		"uint32",   "int64",           "uint64",        "float",    "double",     "string",    "pointer",      "color32",     "vector", "vector2d",
		"vector4d", "rotation_vector", "quaternion",    "qangle",   "matrix3x4",  "transform", "string_token", "ehandle",     NULL };

	KV3SubType_t subtype = GetSubType();

	if (subtype < KV3_SUBTYPE_COUNT) {
		return s_SubTypes[subtype];
	}

	return "<unknown>";
}

const char* KeyValues3::ToString(CBufferString& buff, uint flags) const {
	if ((flags & KV3_TO_STRING_DONT_CLEAR_BUFF) != 0) {
		flags &= ~KV3_TO_STRING_DONT_APPEND_STRINGS;
	}
	else {
		buff.ToGrowable()->Clear();
	}

	KV3Type_t type = GetType();

	switch (type) {
	case KV3_TYPE_NULL: {
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_BOOL: {
		const char* str = m_Bool ? "true" : "false";

		if ((flags & KV3_TO_STRING_DONT_APPEND_STRINGS) != 0) {
			return str;
		}

		buff.Insert(buff.ToGrowable()->GetTotalNumber(), str);
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_INT: {
		buff.AppendFormat("%lld", m_Int);
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_UINT: {
		if (GetSubType() == KV3_SUBTYPE_POINTER) {
			if ((flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS) == 0) {
				buff.Insert(buff.ToGrowable()->GetTotalNumber(), "<pointer>");
			}

			if ((flags & KV3_TO_STRING_RETURN_NON_NUMERICS) == 0) {
				return NULL;
			}

			return buff.ToGrowable()->Get();
		}

		buff.AppendFormat("%llu", m_UInt);
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_DOUBLE: {
		buff.AppendFormat("%g", m_Double);
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_STRING: {
		const char* str = GetString();

		if ((flags & KV3_TO_STRING_DONT_APPEND_STRINGS) != 0) {
			return str;
		}

		buff.Insert(buff.ToGrowable()->GetTotalNumber(), str);
		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_BINARY_BLOB: {
		if ((flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS) == 0) {
			buff.AppendFormat("<binary blob: %u bytes>", GetBinaryBlobSize());
		}

		if ((flags & KV3_TO_STRING_RETURN_NON_NUMERICS) == 0) {
			return NULL;
		}

		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_ARRAY: {
		int elements = GetArrayElementCount();

		if (elements > 0 && elements <= 4) {
			switch (GetTypeEx()) {
			case KV3_TYPEEX_ARRAY: {
				bool unprintable = false;
				CBufferStringGrowable<128> temp;

				KeyValues3** arr = m_pArray->Base();
				for (int i = 0; i < elements; ++i) {
					switch (arr[i]->GetType()) {
					case KV3_TYPE_INT:
						temp.AppendFormat("%lld", arr[i]->m_Int);
						break;
					case KV3_TYPE_UINT:
						if (arr[i]->GetSubType() == KV3_SUBTYPE_POINTER) {
							unprintable = true;
						}
						else {
							temp.AppendFormat("%llu", arr[i]->m_UInt);
						}
						break;
					case KV3_TYPE_DOUBLE:
						temp.AppendFormat("%g", arr[i]->m_Double);
						break;
					default:
						unprintable = true;
						break;
					}

					if (unprintable) {
						break;
					}

					if (i != elements - 1) {
						temp.Insert(temp.ToGrowable()->GetTotalNumber(), " ");
					}
				}

				if (unprintable) {
					break;
				}

				buff.Insert(buff.ToGrowable()->GetTotalNumber(), temp.Get());
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_FLOAT32: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%g", m_f32Array[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_FLOAT64: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%g", m_f64Array[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_INT16: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%d", m_i16Array[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_INT32: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%d", m_i32Array[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_UINT8_SHORT: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%u", m_u8ArrayShort[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			case KV3_TYPEEX_ARRAY_INT16_SHORT: {
				for (int i = 0; i < elements; ++i) {
					buff.AppendFormat("%d", m_i16ArrayShort[i]);
					if (i != elements - 1) {
						buff.Insert(buff.ToGrowable()->GetTotalNumber(), " ");
					}
				}
				return buff.ToGrowable()->Get();
			}
			default:
				break;
			}
		}

		if ((flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS) == 0) {
			buff.AppendFormat("<array: %u elements>", elements);
		}

		if ((flags & KV3_TO_STRING_RETURN_NON_NUMERICS) == 0) {
			return NULL;
		}

		return buff.ToGrowable()->Get();
	}
	case KV3_TYPE_TABLE: {
		if ((flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS) == 0) {
			buff.AppendFormat("<table: %u members>", GetMemberCount());
		}

		if ((flags & KV3_TO_STRING_RETURN_NON_NUMERICS) == 0) {
			return NULL;
		}

		return buff.ToGrowable()->Get();
	}
	default: {
		if ((flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS) == 0) {
			buff.AppendFormat("<unknown KV3 basic type '%s' (%d)>", GetTypeAsString(), type);
		}

		if ((flags & KV3_TO_STRING_RETURN_NON_NUMERICS) == 0) {
			return NULL;
		}

		return buff.ToGrowable()->Get();
	}
	}
}

void KeyValues3::CopyFrom(const KeyValues3* pSrc) {
	SetToNull();

	CKeyValues3Context* context;
	KV3MetaData_t* pDestMetaData = GetMetaData(&context);

	if (pDestMetaData) {
		KV3MetaData_t* pSrcMetaData = pSrc->GetMetaData();

		if (pSrcMetaData) {
			context->CopyMetaData(pDestMetaData, pSrcMetaData);
		}
		else {
			pDestMetaData->Clear();
		}
	}

	KV3SubType_t eSrcSubType = pSrc->GetSubType();

	switch (pSrc->GetType()) {
	case KV3_TYPE_BOOL:
		SetBool(pSrc->m_Bool);
		break;
	case KV3_TYPE_INT:
		SetValue<int64>(pSrc->m_Int, KV3_TYPEEX_INT, eSrcSubType);
		break;
	case KV3_TYPE_UINT:
		SetValue<uint64>(pSrc->m_UInt, KV3_TYPEEX_UINT, eSrcSubType);
		break;
	case KV3_TYPE_DOUBLE:
		SetValue<double>(pSrc->m_Double, KV3_TYPEEX_DOUBLE, eSrcSubType);
		break;
	case KV3_TYPE_STRING:
		SetString(pSrc->GetString(), eSrcSubType);
		break;
	case KV3_TYPE_BINARY_BLOB:
		SetToBinaryBlob(pSrc->GetBinaryBlob(), pSrc->GetBinaryBlobSize());
		break;
	case KV3_TYPE_ARRAY: {
		switch (pSrc->GetTypeEx()) {
		case KV3_TYPEEX_ARRAY: {
			PrepareForType(KV3_TYPEEX_ARRAY, KV3_SUBTYPE_ARRAY);
			m_pArray->CopyFrom(pSrc->m_pArray);
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT32:
			AllocArray<float>(pSrc->m_nNumArrayElements, pSrc->m_f32Array, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID,
				KV3_TYPEEX_ARRAY_FLOAT32, eSrcSubType, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32);
			break;
		case KV3_TYPEEX_ARRAY_FLOAT64:
			AllocArray<double>(pSrc->m_nNumArrayElements, pSrc->m_f64Array, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID,
				KV3_TYPEEX_ARRAY_FLOAT64, eSrcSubType, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT64);
			break;
		case KV3_TYPEEX_ARRAY_INT16:
			AllocArray<int16>(pSrc->m_nNumArrayElements, pSrc->m_i16Array, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_INT16_SHORT,
				KV3_TYPEEX_ARRAY_INT16, eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT16);
			break;
		case KV3_TYPEEX_ARRAY_INT32:
			AllocArray<int32>(pSrc->m_nNumArrayElements, pSrc->m_i32Array, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID, KV3_TYPEEX_ARRAY_INT32,
				eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT32);
			break;
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
			AllocArray<uint8>(pSrc->m_nNumArrayElements, pSrc->m_u8ArrayShort, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT,
				KV3_TYPEEX_INVALID, eSrcSubType, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8);
			break;
		case KV3_TYPEEX_ARRAY_INT16_SHORT:
			AllocArray<int16>(pSrc->m_nNumArrayElements, pSrc->m_i16ArrayShort, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_INT16_SHORT,
				KV3_TYPEEX_ARRAY_INT16, eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT16);
			break;
		default:
			break;
		}
		break;
	}
	case KV3_TYPE_TABLE: {
		PrepareForType(KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE);
		m_pTable->CopyFrom(pSrc->m_pTable);
		break;
	}
	default:
		break;
	}

	m_SubType = eSrcSubType;
	m_nFlags = pSrc->m_nFlags;
}

KeyValues3& KeyValues3::operator=(const KeyValues3& src) {
	if (this == &src) {
		return *this;
	}

	CopyFrom(&src);

	return *this;
}

CKeyValues3Array::CKeyValues3Array(int nAllocSize, int cluster_elem) : m_nCount(0), m_IsDynamicallySized(false), m_nUnk001(0), m_Data{} {
	m_Chunk.m_nClusterElement = cluster_elem;

	if (nAllocSize > 255) {
		m_nInitialSize = -1;
		m_Chunk.m_nAllocatedChunks = nAllocSize;
	}
	else {
		m_nInitialSize = KV3_ARRAY_MAX_FIXED_MEMBERS;
		m_Chunk.m_nAllocatedChunks = KV3_ARRAY_MAX_FIXED_MEMBERS;
	}
}

void CKeyValues3Array::Init(int nAllocSize, int cluster_elem) {
	m_nCount = 0;
	m_IsDynamicallySized = false;
	m_nUnk001 = 0;
	m_Data = {};
	m_Chunk.m_nClusterElement = cluster_elem;

	if (nAllocSize > 255) {
		m_nInitialSize = -1;
		m_Chunk.m_nAllocatedChunks = nAllocSize;
	}
	else {
		m_nInitialSize = KV3_ARRAY_MAX_FIXED_MEMBERS;
		m_Chunk.m_nAllocatedChunks = KV3_ARRAY_MAX_FIXED_MEMBERS;
	}
}

CKeyValues3ArrayCluster* CKeyValues3Array::GetCluster() const {
	if (m_Chunk.m_nClusterElement == -1) {
		return NULL;
	}

	return GET_OUTER(CKeyValues3ArrayCluster, m_Elements[m_Chunk.m_nClusterElement]);
}

CKeyValues3Context* CKeyValues3Array::GetContext() const {
	CKeyValues3ArrayCluster* cluster = GetCluster();

	if (cluster) {
		return cluster->GetContext();
	}
	else {
		return NULL;
	}
}

void CKeyValues3Array::SetCount(int count, KV3TypeEx_t type, KV3SubType_t subtype) {
	int nOldSize = m_nCount;

	CKeyValues3Context* context = GetContext();

	for (int i = count; i < nOldSize; ++i) {
		if (context) {
			context->FreeKV(m_Data.m_Static.m_Elements[i]);
		}
		else {
			delete m_Data.m_Static.m_Elements[i];
		}
	}

	m_nCount = count;

	for (int i = nOldSize; i < count; ++i) {
		if (context) {
			m_Data.m_Static.m_Elements[i] = context->AllocKV(type, subtype);
		}
		else {
			m_Data.m_Static.m_Elements[i] = new KeyValues3(type, subtype);
		}
	}
}

KeyValues3** CKeyValues3Array::InsertBeforeGetPtr(int elem, int num) {
	KeyValues3** kv = &m_Data.m_Static.m_Elements[elem];

	CKeyValues3Context* context = GetContext();

	for (int i = 0; i < num; ++i) {
		if (context) {
			m_Data.m_Static.m_Elements[elem + i] = context->AllocKV();
		}
		else {
			m_Data.m_Static.m_Elements[elem + i] = new KeyValues3;
		}
	}

	return kv;
}

void CKeyValues3Array::CopyFrom(const CKeyValues3Array* pSrc) {
	int nNewSize = m_nCount;

	SetCount(nNewSize);

	for (int i = 0; i < nNewSize; ++i) {
		*m_Data.m_Static.m_Elements[i] = *pSrc->m_Data.m_Static.m_Elements[i];
	}
}

void CKeyValues3Array::RemoveMultiple(int elem, int num) {
	CKeyValues3Context* context = GetContext();

	for (int i = 0; i < num; ++i) {
		if (context) {
			context->FreeKV(m_Data.m_Static.m_Elements[elem + i]);
		}
		else {
			delete m_Data.m_Static.m_Elements[elem + i];
		}
	}

	delete m_Data.m_Static.m_Elements[elem + num];
}

void CKeyValues3Array::Purge(bool bClearingContext) {
	CKeyValues3Context* context = GetContext();

	for (auto& elem : m_Data.m_Static.m_Elements) {
		if (context) {
			if (!bClearingContext) {
				context->FreeKV(elem);
			}
		}

		delete elem;
	}

	/*FOR_EACH_LEANVEC(m_StaticBuffer.m_Elements, iter) {
		if (context) {
			if (!bClearingContext)
				context->FreeKV(m_StaticBuffer.m_Elements[iter]);
		} else {
			delete m_StaticBuffer.m_Elements[iter];
		}
	}*/

	// m_StaticBuffer.m_Elements.Purge();
}

CKeyValues3Table::CKeyValues3Table(int nAllocSize, int cluster_elem)
	: m_pFastSearch(NULL), m_nCount(0), m_bIsDynamicallySized(false), m_nUnk1(0), m_Data{} {
	m_Chunk.m_nClusterElement = cluster_elem;

	if (nAllocSize > 255) {
		m_nInitialSize = -1;
		m_Chunk.m_nAllocatedChunks = nAllocSize;
	}
	else {
		m_nInitialSize = KV3_TABLE_MAX_FIXED_MEMBERS;
		m_Chunk.m_nAllocatedChunks = KV3_TABLE_MAX_FIXED_MEMBERS;
	}
}

void CKeyValues3Table::Init(int nAllocSize, int cluster_elem) {
	m_pFastSearch = NULL;
	m_nCount = 0;
	m_bIsDynamicallySized = false;
	m_nUnk1 = 0;
	m_Data = {};
	m_Chunk.m_nClusterElement = cluster_elem;

	if (nAllocSize > 255) {
		m_nInitialSize = -1;
		m_Chunk.m_nAllocatedChunks = nAllocSize;
	}
	else {
		m_nInitialSize = KV3_TABLE_MAX_FIXED_MEMBERS;
		m_Chunk.m_nAllocatedChunks = KV3_TABLE_MAX_FIXED_MEMBERS;
	}
}

inline void* CKeyValues3Table::Base() {
	if (m_nCount) {
		if (m_bIsDynamicallySized) {
			return m_Data.m_pChunks;
		}
		return &m_Data.m_FixedAlloc;
	}
	return NULL;
}

CKeyValues3Table::Hash_t* CKeyValues3Table::HashesBase() {
	if (m_bIsDynamicallySized) {
		return &m_Data.m_pChunks->m_Hash;
	}
	return &m_Data.m_FixedAlloc.m_Hashes[0];
}

CKeyValues3Table::Member_t* CKeyValues3Table::MembersBase() {
	if (m_bIsDynamicallySized) {
		return &m_Data.m_pChunks[KV3Helpers::CalcAlighedChunk(GetAllocatedChunks()) / 8].m_Member;
	}
	return &m_Data.m_FixedAlloc.m_Members[0];
}

CKeyValues3Table::Name_t* CKeyValues3Table::NamesBase() {
	if (m_bIsDynamicallySized) {
		return &m_Data.m_pChunks[(KV3Helpers::CalcAlighedChunk(GetAllocatedChunks()) / 8) + GetAllocatedChunks()].m_Name;
	}
	return &m_Data.m_FixedAlloc.m_Names[0];
}

CKeyValues3Table::IsExternalName_t* CKeyValues3Table::IsExternalNameBase() {
	if (m_bIsDynamicallySized) {
		return &m_Data.m_pChunks[(KV3Helpers::CalcAlighedChunk(GetAllocatedChunks()) / 8) + GetAllocatedChunks() * 2].m_IsExternalName;
	}
	return &m_Data.m_FixedAlloc.m_IsExternalName[0];
}

CKeyValues3TableCluster* CKeyValues3Table::GetCluster() const {
	if (m_Chunk.m_nClusterElement == -1) {
		return NULL;
	}

	return GET_OUTER(CKeyValues3TableCluster, m_Elements[m_Chunk.m_nClusterElement]);
}

KeyValues3* CKeyValues3Table::GetMember(KV3MemberId_t id) {
	Assert(0 <= id && id < m_nCount);
	return MembersBase()[id];
}

const CKeyValues3Table::Name_t CKeyValues3Table::GetMemberName(KV3MemberId_t id) const {
	Assert(0 <= id && id < m_nCount);
	return NamesBase()[id];
}

const CKeyValues3Table::Hash_t CKeyValues3Table::GetMemberHash(KV3MemberId_t id) const {
	Assert(0 <= id && id < m_nCount);
	return HashesBase()[id];
}

CKeyValues3Context* CKeyValues3Table::GetContext() const {
	CKeyValues3TableCluster* cluster = GetCluster();

	if (cluster) {
		return cluster->GetContext();
	}
	else {
		return NULL;
	}
}

void CKeyValues3Table::EnableFastSearch() {
	if (m_pFastSearch) {
		m_pFastSearch->m_member_ids.RemoveAll();
	}
	else {
		m_pFastSearch = new kv3tablefastsearch_t;
	}

	const Hash_t* pHashes = HashesBase();
	for (int i = 0; i < m_nCount; ++i) {
		m_pFastSearch->m_member_ids.Insert(pHashes[i], i);
	}

	m_pFastSearch->m_ignore = false;
	m_pFastSearch->m_ignores_counter = 0;
}

void CKeyValues3Table::EnsureMemberCapacity(int num, bool force, bool dont_move) {
	auto nChunks = GetAllocatedChunks();
	if (num <= nChunks) {
		return;
	}

	const int nMinAllocated = KV3_MIN_CHUNKS;
	const int nMaxAllocated = KV3_MAX_CHUNKS;
	const int nAllocatedChunksSize = nChunks * sizeof(void*);
	const int nAllocatedChunksDoubleSize = nAllocatedChunksSize * 2;
	int nNewAllocatedChunks = nChunks;
	if (num > nMaxAllocated) {
		Plat_FatalErrorFunc("%s member count overflow (%u)\n", __FUNCTION__, num);
		DebuggerBreak();
	}

	if (force) {
		nNewAllocatedChunks = num;
	}
	else {
		if (nNewAllocatedChunks < nMinAllocated) {
			nNewAllocatedChunks = nMinAllocated;
		}
		while (nNewAllocatedChunks < num) {
			if (nNewAllocatedChunks < nMaxAllocated / 2) {
				nNewAllocatedChunks = MAX(nNewAllocatedChunks * 2, nMinAllocated);
			}
			else {
				nNewAllocatedChunks = nMaxAllocated;
			}
		}
	}
	const int nAlignedChunk = KV3Helpers::CalcAlighedChunk(nNewAllocatedChunks);
	const int nNewSize =
		nAlignedChunk + nNewAllocatedChunks * (nMinAllocated * (sizeof(Hash_t) + sizeof(Member_t) + sizeof(Name_t) + sizeof(IsExternalName_t)));
	void* pNew = m_bIsDynamicallySized ? realloc(m_Data.m_pChunks, nNewSize) : malloc(nNewSize);
	uintp nNewAddress = (uintp)pNew;
	bool bDontMove = true;
	if (m_nCount) {
		bDontMove = dont_move;
	}
	if (!bDontMove) {
		if (m_bIsDynamicallySized) {
			memmove((void*)(nNewAddress + nAlignedChunk + nAllocatedChunksDoubleSize), IsExternalNameBase(), m_nCount * sizeof(IsExternalName_t));
			memmove((void*)(nNewAddress + nAlignedChunk + nAllocatedChunksSize), NamesBase(), m_nCount * sizeof(Name_t));
			memmove((void*)(nNewAddress + nAlignedChunk), MembersBase(), m_nCount * sizeof(Member_t));
		}
		else {
			memmove(pNew, HashesBase(), m_nCount * sizeof(Hash_t));
			memmove((void*)(nNewAddress + nAlignedChunk), MembersBase(), m_nCount * sizeof(Member_t));
			memmove((void*)(nNewAddress + nAlignedChunk + nAllocatedChunksSize), NamesBase(), m_nCount * sizeof(Name_t));
			memmove((void*)(nNewAddress + nAlignedChunk + nAllocatedChunksDoubleSize), IsExternalNameBase(), m_nCount * sizeof(IsExternalName_t));
		}
	}
	m_Data.m_pChunks = (Data_t::DynamicBuffer_t*)pNew;
	m_Chunk.m_nAllocatedChunks = nNewAllocatedChunks;
	m_bIsDynamicallySized = true;
}

KV3MemberId_t CKeyValues3Table::FindMember(const KeyValues3* kv) const {
	const Member_t* pMembers = MembersBase();
	for (int i = 0; i < m_nCount; ++i) {
		if (pMembers[i] == kv) {
			return i;
		}
	}

	return KV3_INVALID_MEMBER;
}

KV3MemberId_t CKeyValues3Table::FindMember(const CKV3MemberName& name) {
	bool bFastSearch = false;

	if (m_pFastSearch) {
		if (m_pFastSearch->m_ignore) {
			if (++m_pFastSearch->m_ignores_counter > 4) {
				EnableFastSearch();
				bFastSearch = true;
			}
		}
		else {
			bFastSearch = true;
		}
	}

	if (bFastSearch) {
		UtlHashHandle_t h = m_pFastSearch->m_member_ids.Find(name.GetHashCode());

		if (h != m_pFastSearch->m_member_ids.InvalidHandle()) {
			return m_pFastSearch->m_member_ids[h];
		}
	}
	else {
		const Hash_t* pHashes = HashesBase();
		for (int i = 0; i < m_nCount; ++i) {
			if (pHashes[i] == name.GetHashCode()) {
				return i;
			}
		}
	}

	return KV3_INVALID_MEMBER;
}

KV3MemberId_t CKeyValues3Table::CreateMember(const CKV3MemberName& name) {
	KV3MemberId_t memberId = m_nCount;

	if (GetMemberCount() >= 128 && !m_pFastSearch) {
		EnableFastSearch();
	}

	int nNewSize = m_nCount + 1;
	if (nNewSize > KV3_TABLE_MAX_FIXED_MEMBERS) {
		EnsureMemberCapacity(nNewSize);
	}

	Hash_t* pHashes = HashesBase();
	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	IsExternalName_t* pExternalName = IsExternalNameBase();

	pHashes[memberId] = name.GetHashCode();
	pExternalName[memberId] = false;

	CKeyValues3Context* context = GetContext();
	if (context) {
		pMembers[memberId] = context->AllocKV();
		pNames[memberId] = context->AllocString(name.GetString());
	}
	else {
		pMembers[memberId] = new KeyValues3;
		pNames[memberId] = strdup(name.GetString());
	}

	if (m_pFastSearch && !m_pFastSearch->m_ignore) {
		m_pFastSearch->m_member_ids.Insert(name.GetHashCode(), memberId);
	}

	m_nCount = nNewSize;
	return memberId;
}

void CKeyValues3Table::CopyFrom(const CKeyValues3Table* pSrc) {
	int nNewSize = m_nCount;

	RemoveAll(nNewSize);

	CKeyValues3Context* context = GetContext();

	Hash_t* pHashes = HashesBase();
	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	const Hash_t* pCopyHashes = pSrc->HashesBase();
	const Member_t* pCopyMembers = pSrc->MembersBase();
	const Name_t* pCopyNames = pSrc->NamesBase();
	for (int i = 0; i < nNewSize; ++i) {
		pHashes[i] = pCopyHashes[i];

		if (context) {
			pMembers[i] = context->AllocKV();
			pNames[i] = context->AllocString(pCopyNames[i]);
		}
		else {
			pMembers[i] = new KeyValues3;
			pNames[i] = strdup(pCopyNames[i]);
		}

		*pCopyMembers[i] = *pCopyMembers[i];
	}

	if (nNewSize >= 128) {
		EnableFastSearch();
	}
}

void CKeyValues3Table::RemoveMember(KV3MemberId_t id) {
	CKeyValues3Context* context = GetContext();

	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	if (context) {
		context->FreeKV(pMembers[id]);
	}
	else {
		pMembers[id]->Free(true);
		free(pMembers[id]);
		free((void*)pNames[id]);
	}

	if (m_pFastSearch) {
		m_pFastSearch->m_ignore = true;
		m_pFastSearch->m_ignores_counter = 1;
	}

	m_nCount--;
}

void CKeyValues3Table::RemoveAll(int nAllocSize) {
	CKeyValues3Context* context = GetContext();

	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	for (int i = 0; i < m_nCount; ++i) {
		if (context) {
			context->FreeKV(pMembers[i]);
		}
		else {
			pMembers[i]->Free(true);
			free(pMembers[i]);
			free((void*)pNames[i]);
		}
	}

	if (nAllocSize > KV3_TABLE_MAX_FIXED_MEMBERS) {
		EnsureMemberCapacity(nAllocSize, true, true);
	}
	else if (m_bIsDynamicallySized) {
		free(m_Data.m_pChunks);
	}
	m_nCount = nAllocSize;

	if (m_pFastSearch) {
		if (nAllocSize >= 128) {
			m_pFastSearch->Clear();
		}
		else {
			m_pFastSearch->Clear();
			free(m_pFastSearch);
			m_pFastSearch = NULL;
		}
	}
}

void CKeyValues3Table::Purge(bool bClearingContext) {
	CKeyValues3Context* context = GetContext();

	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	for (int i = 0; i < m_nCount; ++i) {
		if (context) {
			if (!bClearingContext) {
				context->FreeKV(pMembers[i]);
			}
		}
		else {
			pMembers[i]->Free(true);
			free(pMembers[i]);
			free((void*)pNames[i]);
		}
	}

	if (m_bIsDynamicallySized) {
		free(m_Data.m_pChunks);
	}

	m_Chunk.m_nAllocatedChunks = 0;
	m_nCount = 0;

	if (m_pFastSearch) {
		m_pFastSearch->Clear();
		free(m_pFastSearch);
	}

	m_pFastSearch = NULL;
}

CKeyValues3BaseCluster::CKeyValues3BaseCluster(CKeyValues3Context* context)
	: m_pContext(context), m_pNextFreeNode(NULL), m_nAllocatedElements(0), m_nElementCount(0), m_pMetaData(NULL), m_pPrev(NULL), m_pNext(NULL) {}

void CKeyValues3BaseCluster::Purge() {
	m_nAllocatedElements = 0;
}

CKeyValues3Cluster::CKeyValues3Cluster(CKeyValues3Context* context) : CKeyValues3BaseCluster(context) {
	m_nAllocatedElements = KV3_CLUSTER_MAX_ELEMENTS | 0x80000000;

	KeyValues3ClusterNode* node = nullptr;
	if (2 * m_nAllocatedElements > 0) {
		for (int i = KV3_CLUSTER_MAX_ELEMENTS - 1, j = 0; j < (2 * m_nAllocatedElements >> 1); i--, j++) {
			m_KeyValues[i].m_pNextFree = node;
			node = &m_KeyValues[i];
		}
	}
	m_pNextFreeNode = &m_KeyValues[0];
}

CKeyValues3Cluster::~CKeyValues3Cluster() {
	FreeMetaData();
}

#include "tier0/memdbgoff.h"

KeyValues3* CKeyValues3Cluster::Alloc(KV3TypeEx_t type, KV3SubType_t subtype) {
	Assert(IsFree());
	/*int element = KV3Helpers::BitScanFwd(~m_nAllocatedElements);
	m_nAllocatedElements |= (1ull << element);*/
	KeyValues3* kv = &m_KeyValues[m_nElementCount - 1].m_KeyValue;
	new (kv) KeyValues3(m_nElementCount - 1, type, subtype);
	return kv;
}

#include "tier0/memdbgon.h"

void CKeyValues3Cluster::Free(int element) {
	Assert(element >= 0 && element < KV3_CLUSTER_MAX_ELEMENTS);
	KeyValues3* kv = &m_KeyValues[element].m_KeyValue;
	Destruct(kv);
	memset((void*)kv, 0, sizeof(KeyValues3));
	m_nAllocatedElements &= ~(1ull << element);
}

void CKeyValues3Cluster::PurgeElements() {
	uint64 mask = 1;
	for (int i = 0; i < KV3_CLUSTER_MAX_ELEMENTS; ++i) {
		if ((m_nAllocatedElements & mask) != 0) {
			m_KeyValues[i].m_KeyValue.OnClearContext();
		}
		mask <<= 1;
	}

	m_nAllocatedElements = 0;
}

void CKeyValues3Cluster::Purge() {
	PurgeElements();

	if (m_pMetaData) {
		for (int i = 0; i < KV3_CLUSTER_MAX_ELEMENTS; ++i) {
			m_pMetaData->m_elements[i].Purge();
		}
	}
}

void CKeyValues3Cluster::Clear() {
	PurgeElements();

	if (m_pMetaData) {
		for (int i = 0; i < KV3_CLUSTER_MAX_ELEMENTS; ++i) {
			m_pMetaData->m_elements[i].Clear();
		}
	}
}

void CKeyValues3BaseCluster::EnableMetaData(bool bEnable) {
	if (bEnable) {
		if (!m_pMetaData) {
			m_pMetaData = new kv3metadata_t;
		}
	}
	else {
		FreeMetaData();
	}
}

void CKeyValues3BaseCluster::FreeMetaData() {
	if (m_pMetaData) {
		delete m_pMetaData;
	}
	m_pMetaData = NULL;
}

KV3MetaData_t* CKeyValues3BaseCluster::GetMetaData(int element) const {
	Assert(element >= 0 && element < KV3_CLUSTER_MAX_ELEMENTS);

	if (!m_pMetaData) {
		return NULL;
	}

	return &m_pMetaData->m_elements[element];
}

CKeyValues3ContextBase::CKeyValues3ContextBase(CKeyValues3Context* context)
	: m_pContext(context), m_KV3BaseCluster(context), m_pKV3FreeClusterAllocator(NULL), m_pKV3FreeClusterAllocatorCopy(NULL), m_pKV3UnkCluster(NULL),
	m_pKV3UnkCluster2(NULL), m_pArrayCluster(NULL), m_pArrayClusterCopy(NULL), m_pEmptyArrayCluster(NULL), m_pEmptyArrayClusterCopy(NULL),
	m_nArrayClusterSize(0), m_nArrayClusterAllocationCount(0), m_pDynamicArray(NULL), m_pTableCluster(NULL), m_pTableClusterCopy(NULL),
	m_pEmptyTableCluster(NULL), m_pEmptyTableClusterCopy(NULL), m_nTableClusterSize(0), m_nTableClusterAllocationCount(0), m_pDynamicTable(NULL),
	m_bMetaDataEnabled(false), m_bFormatConverted(false), m_bRootAvailabe(true), m_pParsingErrorListener(NULL) {}

CKeyValues3ContextBase::~CKeyValues3ContextBase() {
	Purge();
}

void CKeyValues3ContextBase::Clear() {
	m_BinaryData.Clear();

	m_KV3BaseCluster.Clear();
	m_KV3BaseCluster.SetNextFree(NULL);
	/*m_pKV3FreeCluster = &m_KV3BaseCluster;

	FOR_EACH_LEANVEC(m_KV3Clusters, iter) {
		m_KV3Clusters[iter]->Clear();
		m_KV3Clusters[iter]->SetNextFree(m_pKV3FreeCluster);
		m_pKV3FreeCluster = m_KV3Clusters[iter];
	}

	m_pArrayFreeCluster = NULL;

	FOR_EACH_LEANVEC(m_ArrayClusters, iter) {
		m_ArrayClusters[iter]->Clear();
		m_ArrayClusters[iter]->SetNextFree(m_pArrayFreeCluster);
		m_pArrayFreeCluster = m_ArrayClusters[iter];
	}

	m_pTableFreeCluster = NULL;

	FOR_EACH_LEANVEC(m_TableClusters, iter) {
		m_TableClusters[iter]->Clear();
		m_TableClusters[iter]->SetNextFree(m_pTableFreeCluster);
		m_pTableFreeCluster = m_TableClusters[iter];
	}*/

	m_Symbols.RemoveAll();

	m_bFormatConverted = false;
}

void CKeyValues3ContextBase::Purge() {
	m_BinaryData.Purge();

	m_KV3BaseCluster.Purge();
	m_KV3BaseCluster.SetNextFree(NULL);
	/*m_pKV3FreeCluster = &m_KV3BaseCluster;

	FOR_EACH_LEANVEC(m_KV3Clusters, iter) {
		m_KV3Clusters[iter]->Purge();
		delete m_KV3Clusters[iter];
	}

	m_KV3Clusters.Purge();

	FOR_EACH_LEANVEC(m_ArrayClusters, iter) {
		m_ArrayClusters[iter]->Purge();
		delete m_ArrayClusters[iter];
	}

	m_pArrayFreeCluster = NULL;
	m_ArrayClusters.Purge();

	FOR_EACH_LEANVEC(m_TableClusters, iter) {
		m_TableClusters[iter]->Purge();
		delete m_TableClusters[iter];
	}

	m_pTableFreeCluster = NULL;
	m_TableClusters.Purge();*/

	m_Symbols.Purge();

	m_bFormatConverted = false;
}

CKeyValues3Context::CKeyValues3Context(bool bNoRoot) : BaseClass(this) {
	if (bNoRoot) {
		m_bRootAvailabe = false;
	}
	else {
		m_bRootAvailabe = true;
		m_KV3BaseCluster.Alloc();
	}

	m_bMetaDataEnabled = false;
	m_bFormatConverted = false;

	memset(pad, 0, ARRAYSIZE(pad));
}

CKeyValues3Context::~CKeyValues3Context() {}

void CKeyValues3Context::Clear() {
	BaseClass::Clear();

	if (m_bRootAvailabe) {
		m_KV3BaseCluster.Alloc();
	}
}

void CKeyValues3Context::Purge() {
	BaseClass::Purge();

	if (m_bRootAvailabe) {
		m_KV3BaseCluster.Alloc();
	}
}

CKeyValues3Array* CKeyValues3Context::AllocArray(int nAllocSize) {
	CKeyValues3Array* pArray = nullptr;

	int nSize = (nAllocSize <= 0) ? 32 : 8 * nAllocSize + 24;
	if ((m_nArrayClusterSize >= m_nArrayClusterAllocationCount) || (m_nArrayClusterAllocationCount - m_nArrayClusterSize < nSize)) {
		if (nAllocSize > KV3_ARRAY_MAX_FIXED_MEMBERS) {
			return NULL;
		}

		if (m_pArrayCluster) {
			pArray = (CKeyValues3Array*)m_pArrayCluster->m_pNextFreeNode;
			if (pArray) {
				++m_pArrayCluster->m_nElementCount;
				m_pArrayCluster->m_pNextFreeNode = (KeyValues3ClusterNode*)pArray->m_pNextFree;

				int nClusterElement = ((uintptr_t)pArray - (uintptr_t)m_pArrayCluster - sizeof(CKeyValues3BaseCluster)) / sizeof(CKeyValues3Array);
				pArray->Init(nAllocSize, nClusterElement);
			}

			if (m_pArrayCluster->m_nElementCount == (2 * m_pArrayCluster->m_nAllocatedElements) >> 1) {
				auto pAllocator = m_pArrayCluster;
				auto v19 = pAllocator->m_pPrev;
				auto v20 = pAllocator->m_pNext;
				if (v19)
					v19->m_pNext = v20;
				else
					m_pArrayClusterCopy = (CKeyValues3ArrayCluster*)v20;

				auto v21 = pAllocator->m_pNext;
				auto v22 = pAllocator->m_pPrev;
				if (v21)
					v21->m_pPrev = v22;
				else
					m_pArrayCluster = (CKeyValues3ArrayCluster*)v22;

				pAllocator->m_pNext = NULL;
				pAllocator->m_pPrev = NULL;

				auto pEmptyAllocator = m_pEmptyArrayCluster;
				if (pEmptyAllocator)
					pEmptyAllocator->m_pNext = (CKeyValues3Cluster*)pAllocator;
				else
					m_pEmptyArrayClusterCopy = pAllocator;

				pAllocator->m_pNext = NULL;
				pAllocator->m_pPrev = (CKeyValues3Cluster*)m_pEmptyArrayCluster;

				m_pEmptyArrayCluster = pAllocator;
			}
		}
		else {
			m_pArrayCluster = new CKeyValues3ArrayCluster(m_pContext);
			m_pArrayClusterCopy = m_pArrayCluster;

			pArray = (CKeyValues3Array*)m_pArrayCluster->m_pNextFreeNode;
			if (pArray) {
				++m_pArrayCluster->m_nElementCount;
				m_pArrayCluster->m_pNextFreeNode = (KeyValues3ClusterNode*)pArray->m_pNextFree;

				int nClusterElement = ((uintptr_t)pArray - (uintptr_t)m_pArrayCluster - sizeof(CKeyValues3BaseCluster)) / sizeof(CKeyValues3Array);
				pArray->Init(nAllocSize, nClusterElement);
			}
		}

		return pArray;
	}
	else {
		if (!m_nArrayClusterAllocationCount) {
			DebuggerBreak();
		}
	}

	return pArray;
}

CKeyValues3Table* CKeyValues3Context::AllocTable(int nAllocSize) {
	CKeyValues3Table* pTable = nullptr;

	int nSize = (nAllocSize <= 0)
		? 40
		: (17 * nAllocSize + (KV3Helpers::CalcAlighedChunk(nAllocSize) + 31) & KV3_CHUNK_BITMASK) + KV3_TABLE_MAX_FIXED_MEMBERS;
	if ((m_nTableClusterSize >= m_nTableClusterAllocationCount) || (m_nTableClusterAllocationCount - m_nTableClusterSize < nSize)) {
		if (nAllocSize > KV3_TABLE_MAX_FIXED_MEMBERS) {
			return NULL;
		}

		if (m_pTableCluster) {
			pTable = (CKeyValues3Table*)m_pTableCluster->m_pNextFreeNode;
			if (pTable) {
				++m_pTableCluster->m_nElementCount;
				m_pTableCluster->m_pNextFreeNode = (KeyValues3ClusterNode*)pTable->m_pNextFree;

				int nClusterElement = ((uintptr_t)pTable - (uintptr_t)m_pTableCluster - sizeof(CKeyValues3BaseCluster)) / sizeof(CKeyValues3Table);
				pTable->Init(nAllocSize, nClusterElement);
			}

			if (m_pTableCluster->m_nElementCount == (2 * m_pTableCluster->m_nAllocatedElements) >> 1) {
				auto pAllocator = m_pTableCluster;
				auto v19 = pAllocator->m_pPrev;
				auto v20 = pAllocator->m_pNext;
				if (v19)
					v19->m_pNext = v20;
				else
					m_pTableClusterCopy = (CKeyValues3TableCluster*)v20;

				auto v21 = pAllocator->m_pNext;
				auto v22 = pAllocator->m_pPrev;
				if (v21)
					v21->m_pPrev = v22;
				else
					m_pTableCluster = (CKeyValues3TableCluster*)v22;

				pAllocator->m_pNext = NULL;
				pAllocator->m_pPrev = NULL;

				auto pEmptyAllocator = m_pEmptyTableCluster;
				if (pEmptyAllocator)
					pEmptyAllocator->m_pNext = (CKeyValues3Cluster*)pAllocator;
				else
					m_pEmptyTableClusterCopy = pAllocator;

				pAllocator->m_pNext = NULL;
				pAllocator->m_pPrev = (CKeyValues3Cluster*)m_pEmptyTableCluster;

				m_pEmptyTableCluster = pAllocator;
			}
		}
		else {
			m_pTableCluster = new CKeyValues3TableCluster(m_pContext);
			m_pTableClusterCopy = m_pTableCluster;

			pTable = (CKeyValues3Table*)m_pTableCluster->m_pNextFreeNode;
			if (pTable) {
				++m_pTableCluster->m_nElementCount;
				m_pTableCluster->m_pNextFreeNode = (KeyValues3ClusterNode*)pTable->m_pNextFree;

				int nClusterElement = ((uintptr_t)pTable - (uintptr_t)m_pTableCluster - sizeof(CKeyValues3BaseCluster)) / sizeof(CKeyValues3Table);
				pTable->Init(nAllocSize, nClusterElement);
			}
		}

		return pTable;
	}
	else {
		if (!m_nTableClusterAllocationCount) {
			DebuggerBreak();
		}
	}

	return pTable;
}

KeyValues3* CKeyValues3Context::Root() {
	if (!m_bRootAvailabe) {
		Plat_FatalErrorFunc("FATAL: {} called on a pool context (no root available)\n", __FUNCTION__);
		DebuggerBreak();
	}

	return m_KV3BaseCluster.Head();
}

const char* CKeyValues3Context::AllocString(const char* pString) {
	return m_Symbols.AddString(pString).String();
}

void CKeyValues3Context::EnableMetaData(bool bEnable) {
	if (bEnable != m_bMetaDataEnabled) {
		m_KV3BaseCluster.EnableMetaData(bEnable);

		/*for (int i = 0; i < m_KV3Clusters.Count(); ++i)
			m_KV3Clusters[i]->EnableMetaData(bEnable);*/

		m_bMetaDataEnabled = bEnable;
	}
}

void CKeyValues3Context::CopyMetaData(KV3MetaData_t* pDest, const KV3MetaData_t* pSrc) {
	pDest->m_nLine = pSrc->m_nLine;
	pDest->m_nColumn = pSrc->m_nColumn;
	pDest->m_nFlags = pSrc->m_nFlags;
	pDest->m_sName = m_Symbols.AddString(pSrc->m_sName.String());

	pDest->m_Comments.Purge();
	pDest->m_Comments.EnsureCapacity(pSrc->m_Comments.Count());

	FOR_EACH_MAP_FAST(pSrc->m_Comments, iter) {
		CBufferStringGrowable<8> buff;
		buff.Insert(0, pSrc->m_Comments[iter].Get());

		pDest->m_Comments.Insert(pSrc->m_Comments.Key(iter), buff);
	}
}

KeyValues3* CKeyValues3Context::AllocKV(KV3TypeEx_t type, KV3SubType_t subtype) {
	KeyValues3* kv;

	if (m_pKV3FreeClusterAllocator) {
		KeyValues3ClusterNode* node = m_pKV3FreeClusterAllocator->m_pNextFreeNode;
		if (node) {
			++m_pKV3FreeClusterAllocator->m_nElementCount;
			m_pKV3FreeClusterAllocator->m_pNextFreeNode = node->m_pNextFree;
		}

		kv = m_pKV3FreeClusterAllocator->Alloc(type, subtype);

		if (m_pKV3FreeClusterAllocator->m_nElementCount == (2 * m_pKV3FreeClusterAllocator->m_nAllocatedElements) >> 1) {
			auto pAllocator = m_pKV3FreeClusterAllocator;
			auto m_pPrev = pAllocator->m_pPrev;
			auto m_pNext = pAllocator->m_pNext;
			if (m_pPrev)
				m_pPrev->m_pNext = m_pNext;
			else
				m_pKV3FreeClusterAllocatorCopy = m_pNext;

			auto v10 = pAllocator->m_pNext;
			auto v11 = pAllocator->m_pPrev;
			if (v10)
				v10->m_pPrev = v11;
			else
				m_pKV3FreeClusterAllocator = v11;

			pAllocator->m_pNext = NULL;
			pAllocator->m_pPrev = NULL;

			auto pUnkCluster = m_pKV3UnkCluster;
			if (pUnkCluster)
				pUnkCluster->m_pNext = pAllocator;
			else
				m_pKV3UnkCluster2 = pAllocator;

			pAllocator->m_pNext = NULL;
			pAllocator->m_pPrev = m_pKV3UnkCluster;

			m_pKV3UnkCluster = pAllocator;
		}
	}
	else {
		m_pKV3FreeClusterAllocator = new CKeyValues3Cluster(m_pContext);
		m_pKV3FreeClusterAllocator->EnableMetaData(m_bMetaDataEnabled);

		m_pKV3FreeClusterAllocatorCopy = m_pKV3FreeClusterAllocator;

		KeyValues3ClusterNode* node = m_pKV3FreeClusterAllocator->m_pNextFreeNode;
		if (node) {
			++m_pKV3FreeClusterAllocator->m_nElementCount;
			m_pKV3FreeClusterAllocator->m_pNextFreeNode = node->m_pNextFree;
		}

		kv = m_pKV3FreeClusterAllocator->Alloc(type, subtype);
	}

	return kv;
}

void CKeyValues3Context::FreeKV(KeyValues3* kv) {
	CKeyValues3Context* context;
	KV3MetaData_t* metadata = kv->GetMetaData(&context);

	Assert(context == m_pContext);

	if (metadata) {
		metadata->Clear();
	}

	// Free<KeyValues3, CKeyValues3Cluster, KV3ClustersVec_t>(kv, &m_KV3BaseCluster, m_pKV3FreeCluster, m_KV3Clusters);
}

CKeyValues3ArrayCluster::CKeyValues3ArrayCluster(CKeyValues3Context* context) : CKeyValues3BaseCluster(context) {
	m_nAllocatedElements = KV3_ARRAY_INIT_SIZE | 0x80000000;

	CKeyValues3ArrayNode* node = nullptr;
	if (2 * m_nAllocatedElements > 0) {
		for (int i = KV3_ARRAY_INIT_SIZE - 1, j = 0; j < (2 * m_nAllocatedElements >> 1); i--, j++) {
			m_Elements[i].m_pNextFree = node;
			node = &m_Elements[i];
		}
	}

	m_pNextFreeNode = (KeyValues3ClusterNode*)&m_Elements[0];
}

CKeyValues3TableCluster::CKeyValues3TableCluster(CKeyValues3Context* context) : CKeyValues3BaseCluster(context) {
	m_nAllocatedElements = KV3_TABLE_INIT_SIZE | 0x80000000;

	CKeyValues3TableNode* node = nullptr;
	if (2 * m_nAllocatedElements > 0) {
		for (int i = KV3_TABLE_INIT_SIZE - 1, j = 0; j < (2 * m_nAllocatedElements >> 1); i--, j++) {
			m_Elements[i].m_pNextFree = node;
			node = &m_Elements[i];
		}
	}

	m_pNextFreeNode = (KeyValues3ClusterNode*)&m_Elements[0];
}
