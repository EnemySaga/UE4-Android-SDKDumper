#pragma once

#include <set>
#include <string>
#include <codecvt>

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

// Signed base types.
typedef signed char __int8;
typedef signed short int __int16;
typedef signed int __int32;
typedef signed long long __int64;

struct FPointer
{
	uintptr_t Dummy;
};

struct FQWord
{
	int A;
	int B;
};

struct FName
{
	int32_t ComparisonIndex;
	int32_t Number;
};

template<class T>
struct TArray
{
	friend struct FString;

public:
	TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	size_t Num() const
	{
		return Count;
	};

	T& operator[](size_t i)
	{
		return Data[i];
	};

	const T& operator[](size_t i) const
	{
		return Data[i];
	};

	bool IsValidIndex(size_t i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

template<typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType   Key;
	ValueType Value;
};

struct FString : public TArray<wchar_t>
{
	std::string ToString() const
	{
		/*int size = WideCharToMultiByte(CP_UTF8, 0, Data, Count, nullptr, 0, nullptr, nullptr);
		std::string str(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, Data, Count, &str[0], size, nullptr, nullptr);
		*/std::string s(Data, Data + Count);      
		return s;
	
	}
};

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	UObject* GetObject() const
	{
		return ObjectPointer;
	}

	UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

struct FText
{
	#if defined(__LP64__)
	char pad[0x18];
	#else
	char pad[0xC];
	#endif
};

struct FWeakObjectPtr
{
	int32_t ObjectIndex;
	int32_t ObjectSerialNumber;
};

struct FStringAssetReference
{
	FString AssetLongPathname;
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
	FWeakObjectPtr WeakPtr;
	int32_t TagAtLastTest;
	TObjectID ObjectID;
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{

};

struct FGuid
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
};

struct FUniqueObjectGuid
{
	FGuid Guid;
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid>
{

};

struct FScriptDelegate
{
	 char pad[0x10];
};

struct FScriptMulticastDelegate
{
	char pad1[0x8];
	char pad2[sizeof(uintptr_t)];
};

class UClass;

class UObject
{
public:
	FPointer VTableObject;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;
};

class UField : public UObject
{
public:
	UField* Next;
};

class UEnum : public UField
{
public:
	FString CppType; //0x0030 
	TArray<TPair<FName, uint64_t>> Names; //0x0040 
	__int64 CppForm; //0x0050 
};

class UStruct : public UField
{
public:
	UStruct* SuperStruct;
	UField* Children;
	int32_t PropertySize;
	int32_t MinAlignment;
	
	TArray<uint8_t> Script;
	uintptr_t PropertyLink;
	uintptr_t RefLink;
	uintptr_t DestructorLink;
	uintptr_t PostConstructLink;
	TArray<UObject*> ScriptObjectReferences;
	
};

class UScriptStruct : public UStruct
{
public:
	//char pad_0x0088[0x10]; //0x0088
};

class UFunction : public UStruct
{
public:
    int32_t FunctionFlags;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
    int8_t  NumParms;                                                 // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
    int16_t ParmsSize;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
    int16_t ReturnValueOffset;                                        // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
    int16_t RPCId;                                                    // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
    int16_t RPCResponseId; 
	//uintptr_t FirstPropertyToInit;
	//uintptr_t EventGraphFunction;
	//int32_t EventGraphCallOffset;
    void*   Func;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

};

class UClass : public UStruct
{
public:
	//char pad_0x0088[0x198]; //0x0088
};

class UProperty : public UField
{
public:
	__int32 ArrayDim; //0x0030 
	__int32 ElementSize; //0x0034 
	FQWord PropertyFlags; //0x0038
	uint16_t RepIndex;
	uint8_t BlueprintReplicationCondition;
	int32_t Offset_Internal;
	FName RepNotifyFunc;
	uintptr_t PropertyLinkNext;
	uintptr_t NextRef;
	uintptr_t DestructorLinkNext;
	uintptr_t PostConstructLinkNext;

};

class UNumericProperty : public UProperty
{
public:
	
};

class UByteProperty : public UNumericProperty
{
public:
	UEnum*		Enum;										// 0x0088 (0x04)
};

class UUInt16Property : public UNumericProperty
{
public:

};

class UUInt32Property : public UNumericProperty
{
public:

};

class UUInt64Property : public UNumericProperty
{
public:

};

class UInt8Property : public UNumericProperty
{
public:

};

class UInt16Property : public UNumericProperty
{
public:

};

class UIntProperty : public UNumericProperty
{
public:

};

class UInt64Property : public UNumericProperty
{
public:

};

class UFloatProperty : public UNumericProperty
{
public:

};

class UDoubleProperty : public UNumericProperty
{
public:

};

class UBoolProperty : public UProperty
{
public:
	uint8_t FieldSize;
	uint8_t ByteOffset;
	uint8_t ByteMask;
	uint8_t FieldMask;
};

class UObjectPropertyBase : public UProperty
{
public:
	UClass* PropertyClass;
};

class UObjectProperty : public UObjectPropertyBase
{
public:

};

class UClassProperty : public UObjectProperty
{
public:
	UClass* MetaClass;
};

class UInterfaceProperty : public UProperty
{
public:
	UClass* InterfaceClass;
};

class UWeakObjectProperty : public UObjectPropertyBase
{
public:

};

class ULazyObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetClassProperty : public UAssetObjectProperty
{
public:
	UClass* MetaClass;
};

class UNameProperty : public UProperty
{
public:

};

class UStructProperty : public UProperty
{
public:
	UScriptStruct* Struct;
};

class UStrProperty : public UProperty
{
public:

};

class UTextProperty : public UProperty
{
public:

};

class UArrayProperty : public UProperty
{
public:
	UProperty* Inner;
};

class UMapProperty : public UProperty
{
public:
	UProperty* KeyProp;
	UProperty* ValueProp;
};

class UDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UMulticastDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UEnumProperty : public UProperty
{
public:
	class UNumericProperty* UnderlyingProp; //0x0070
	class UEnum* Enum; //0x0078
}; //Size: 0x0080
