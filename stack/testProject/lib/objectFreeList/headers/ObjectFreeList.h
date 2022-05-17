#pragma once

#include "common.h"

#include "dump/headers/dump.h"
#pragma comment(lib, "lib/dump/dump")

#if defined(OBJECT_FREE_LIST_SAFE)
	#define allocObject() _allocObject(__FILEW__, __LINE__)
	#define freeObject(x) _freeObject(x, __FILEW__, __LINE__)
#else
	#define allocObject() _allocObject()
	#define freeObject(x) _freeObject(x)
#endif

#define toNode(ptr) ((stAllocNode<T>*)((unsigned __int64)ptr & 0x000007FFFFFFFFFF))
#define toPtr(cnt, pNode) ((void*)((unsigned __int64)pNode | (cnt << 43)))

template<typename T>
struct stAllocNode {
	stAllocNode() {
		init();
	}
	~stAllocNode(){
	}

	void init(){
		
		_nextPtr = nullptr;

		#if defined(OBJECT_FREE_LIST_SAFE)
			_used = false;
			_underFlowCheck = (void*)0xF9F9F9F9F9F9F9F9;
			_overFlowCheck = (void*)0xF9F9F9F9F9F9F9F9;

			_allocSourceFileName = nullptr;
			_freeSourceFileName = nullptr;
			
			_allocLine = 0;
			_freeLine = 0;

			_used = false;
		#endif

		_callDestructor = false;
	}

	#if defined(OBJECT_FREE_LIST_SAFE)
		// f9�� �ʱ�ȭ�ؼ� ����÷ο� üũ�մϴ�.
		void* _underFlowCheck;
	#endif

	// alloc �Լ����� ������ ���� ������
	T _data;
	
	#if defined(OBJECT_FREE_LIST_SAFE)
		// f9�� �ʱ�ȭ�ؼ� �����÷ο� üũ�մϴ�.
		void* _overFlowCheck;
	#endif

	// �Ҵ��� ���� ���
	void* _nextPtr;

	#if defined(OBJECT_FREE_LIST_SAFE)
		// �ҽ� ���� �̸�
		const wchar_t* _allocSourceFileName;
		const wchar_t* _freeSourceFileName;

		// �ҽ� ����
		int _allocLine;
		int _freeLine;

		// ��尡 ��������� Ȯ��
		bool _used;
	#endif

	// �Ҵ� ���� �Ҹ��ڰ� ȣ��Ǿ����� ����
	// �Ҵ� ���� ����ڰ� ��ȯ�Ͽ� �Ҹ��ڰ� ȣ��Ǿ��ٸ�
	// Ŭ������ �Ҹ��� ��, �Ҹ��ڰ� ȣ��Ǹ� �ȵȴ�.
	bool _callDestructor;
};

template<typename T>
class CObjectFreeList
{
public:

	CObjectFreeList(bool runConstructor, bool runDestructor, int _capacity = 0);
	~CObjectFreeList();


	#if defined(OBJECT_FREE_LIST_SAFE)
		T* _allocObject(const wchar_t*, int);
	#else
		T* _allocObject();
	#endif
	
	#if defined(OBJECT_FREE_LIST_SAFE)
		int _freeObject(T* data, const wchar_t*, int);
	#else
		int _freeObject(T* data);
	#endif

	inline unsigned int getCapacity() { return _capacity; }
	inline unsigned int getUsedCount() { return _usedCnt; }

private:

	// �޸� �Ҵ�, ������ ���� ��
	HANDLE _heap;

	// ��� ������ ��带 ����Ʈ�� ���·� �����մϴ�.
	// �Ҵ��ϸ� �����մϴ�.
	void* _freePtr;

	// ��ü ��� ����
	unsigned int _capacity;

	// ���� �Ҵ�� ��� ����
	unsigned int _usedCnt;

	// ������ ������(stNode->data)�� �� ���� ���ϸ� ��� ������(stNode)�� �ȴ� !
	unsigned __int64 _dataPtrToNodePtr;

	// �޸� ������
	// �ܼ� ����Ʈ
	struct stSimpleListNode {
		stAllocNode<T>* _ptr;
		stSimpleListNode* _next;
	};

	// freeList �Ҹ��ڿ��� �޸� ���������� ����մϴ�.
	// new�� �����͵�
	stSimpleListNode* _totalAllocList;
		
	// ����Ʈ ���� Ƚ��
	// ABA ������ ȸ���ϱ� ���� ����մϴ�.
	unsigned __int64 _nodeChangeCnt;

	// �Ҵ� ��, ������ ���� ���θ� ��Ÿ���ϴ�.
	bool _runConstructor;

	// ���� ��, �Ҹ��� ���� ���θ� ��Ÿ���ϴ�.
	bool _runDestructor;

};

template <typename T>
CObjectFreeList<T>::CObjectFreeList(bool runConstructor, bool runDestructor, int size) {
	
	_heap = HeapCreate(0, 0, 0);

	_freePtr = nullptr;
	_totalAllocList = nullptr;
	
	_capacity = size;
	_usedCnt = 0;

	_runConstructor = runConstructor;
	_runDestructor = runDestructor;
	
	_nodeChangeCnt = 0;

	// ���� ���� ����� data���� �Ÿ� ���
	stAllocNode<T>* tempNode = (stAllocNode<T>*)HeapAlloc(_heap, 0, sizeof(stAllocNode<T>));
	_dataPtrToNodePtr = (unsigned __int64)tempNode - (unsigned __int64)&tempNode->_data;
	HeapFree(_heap, 0, tempNode);

	if (size == 0) {
		return;
	}

	for(int nodeCnt = 0; nodeCnt < size; ++nodeCnt){

		// �̸� �������� ������ŭ ��带 ����� ����
		stAllocNode<T>* newNode = (stAllocNode<T>*)HeapAlloc(_heap, 0, sizeof(stAllocNode<T>));

		// T type�� ���� ������ ȣ�� ���θ� ����
		if(runConstructor == false) {
			new (newNode) stAllocNode<T>;
		} else {
			newNode->init();
		}

		newNode->_nextPtr = _freePtr;
		_freePtr = newNode;

		{
			// ��ü alloc list�� �߰�
			// �Ҹ��ڿ��� �ϰ������� �޸� �����ϱ� ����
			stSimpleListNode* totalAllocNode = (stSimpleListNode*)HeapAlloc(_heap, 0, sizeof(stSimpleListNode));

			totalAllocNode->_ptr = newNode;
			totalAllocNode->_next = _totalAllocList;

			_totalAllocList = totalAllocNode;

		}

	}


}

template <typename T>
CObjectFreeList<T>::~CObjectFreeList() {

	while(_totalAllocList != nullptr){
		stAllocNode<T>* freeNode = _totalAllocList->_ptr;
		if(freeNode->_callDestructor == false){
			freeNode->~stAllocNode();
		}
		HeapFree(_heap, 0, freeNode);
		_totalAllocList = _totalAllocList->_next;
	}

}

template<typename T>
T* CObjectFreeList<T>::_allocObject(
	#if defined(OBJECT_FREE_LIST_SAFE)
		const wchar_t* fileName, int line
	#endif
) {
	
	InterlockedIncrement(&_usedCnt);
	
	stAllocNode<T>* allocNode;

	void* freePtr;
	void* nextPtr;

	unsigned __int64 nodeChangeCnt;
	
	_nodeChangeCnt += 1;

	do {
		// ���� ������ ����
		freePtr = _freePtr;
		nodeChangeCnt = _nodeChangeCnt;

		allocNode = toNode(freePtr);

		if (allocNode == nullptr) {

			// �߰� �Ҵ�
			allocNode = (stAllocNode<T>*)HeapAlloc(_heap, 0, sizeof(stAllocNode<T>));
			if(_runConstructor == false) {
				new (allocNode) stAllocNode<T>;
			} else {
				allocNode->init();
			}
		
			// ��ü alloc list�� �߰�
			// �Ҹ��ڿ��� �ϰ������� �޸� �����ϱ� ����
			stSimpleListNode* totalAllocNode = (stSimpleListNode*)HeapAlloc(_heap, 0, sizeof(stSimpleListNode));
			stSimpleListNode* totalAllocList;

			do {

				totalAllocList = _totalAllocList;

				totalAllocNode->_ptr = allocNode;
				totalAllocNode->_next = totalAllocList;

			} while( InterlockedCompareExchange64((LONG64*)&_totalAllocList, (LONG64)totalAllocNode, (LONG64)totalAllocList) != (LONG64)totalAllocList );

			_capacity += 1;

			break;

		}

		nextPtr = allocNode->_nextPtr;

	} while(InterlockedCompareExchange64((LONG64*)&_freePtr, (LONG64)nextPtr, (LONG64)freePtr) != (LONG64)freePtr);
	
	// �Ҹ��� ȣ�� ���� �ʱ�ȭ
	allocNode->_callDestructor = false;

	#if defined(OBJECT_FREE_LIST_SAFE)
		// ��带 ��������� üũ��
		allocNode->_used = true;

		// �Ҵ� ��û�� �ҽ����ϰ� �ҽ������� �����
		allocNode->_allocSourceFileName = fileName;
		allocNode->_allocLine = line;
	#endif
	
	T* data = &allocNode->_data;

	// ������ ����
	if(_runConstructor == true){
		new (data) T();
	}

	return data;
}

template <typename T>
int CObjectFreeList<T>::_freeObject(T* data	
	#if defined(OBJECT_FREE_LIST_SAFE)
		, const wchar_t* fileName, int line
	#endif
) {

	stAllocNode<T>* usedNode = (stAllocNode<T>*)(((char*)data) + _dataPtrToNodePtr);
	
	#if defined(OBJECT_FREE_LIST_SAFE)
		// �ߺ� free üũ
		if(usedNode->_used == false){
			CDump::crash();
		}

		// �����÷ο� üũ
		if((unsigned __int64)usedNode->_overFlowCheck != 0xF9F9F9F9F9F9F9F9){
			CDump::crash();
		}

		// ����÷ο� üũ
		if((unsigned __int64)usedNode->_underFlowCheck != 0xF9F9F9F9F9F9F9F9){
			CDump::crash();
		}

		// ����� ����� �÷��׸� ����
		usedNode->_used = false;
	#endif

	// �Ҹ��� ����
	if(_runDestructor == true){
		data->~T();
		usedNode->_callDestructor = true;
	}

	void* freePtr;
	void* nextPtr;

	unsigned __int64 nodeChangeCnt = 0;

	_nodeChangeCnt += 1;

	do {

		// ���� ������ ����
		freePtr = _freePtr;
		nodeChangeCnt = _nodeChangeCnt;
			
		// ����ߴ� ����� next�� ���� top���� ����
		usedNode->_nextPtr = freePtr;

		nextPtr = toPtr(nodeChangeCnt, usedNode);

	} while(InterlockedCompareExchange64((LONG64*)&_freePtr, (LONG64)nextPtr, (LONG64)freePtr) != (LONG64)freePtr);
	
	InterlockedDecrement(&_usedCnt);

	return 0;

}