#pragma once

#define OBJECT_FREE_LIST_TLS_SAFE

namespace objectFreeListTLS{
	// T type data �ּҿ� �� ���� ���ϸ� node �ּҰ� �˴ϴ�.
	#if defined(OBJECT_FREE_LIST_TLS_SAFE)
		constexpr __int64 _dataToNodePtr = -8;
	#else
		constexpr __int64 _dataToNodePtr = 0;
	#endif

	// �ϳ��� ûũ���� ����� T type ����� �� �Դϴ�.
	constexpr int _chunkSize = 2;

	#if defined(OBJECT_FREE_LIST_TLS_SAFE)
		constexpr unsigned __int64 UNDERFLOW_CHECK_VALUE = 0xAABBCCDDDDCCBBAA;
		constexpr unsigned __int64 OVERFLOW_CHECK_VALUE  = 0xFFEEDDCCCCDDEEFF;
	#endif
};