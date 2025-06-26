#include "ObjectsStore.hpp"
#include <string>
#include <fstream>
#include "Main.h"
#include "EngineClasses.hpp"
#include "Tools.h"


ObjectsIterator ObjectsStore::begin()
{
	return ObjectsIterator(*this, 0);
}

ObjectsIterator ObjectsStore::begin() const
{
	return ObjectsIterator(*this, 0);
}

ObjectsIterator ObjectsStore::end()
{
	return ObjectsIterator(*this);
}

ObjectsIterator ObjectsStore::end() const
{
	return ObjectsIterator(*this);
}

/*ObjectsStore ObjectsStore::GetInstance() const{
	return *this;
}
*/
UEClass ObjectsStore::FindClass(const std::string& name) const
{
	for (auto obj : *this)
	{
		if (obj.GetFullName() == name)
		{
			return obj.Cast<UEClass>();
		}
	}
	return UEClass(nullptr);
}

ObjectsIterator::ObjectsIterator(const ObjectsStore& _store)
		: store(_store),
		  index(_store.GetObjectsNum())
{
}

ObjectsIterator::ObjectsIterator(const ObjectsStore& _store, size_t _index)
		: store(_store),
		  index(_index),
		  current(_store.GetById(_index))
{
}

ObjectsIterator::ObjectsIterator(const ObjectsIterator& other)
		: store(other.store),
		  index(other.index),
		  current(other.current)
{
}

ObjectsIterator::ObjectsIterator(ObjectsIterator&& other) noexcept
		: store(other.store),
		  index(other.index),
		  current(other.current)
{
}

ObjectsIterator& ObjectsIterator::operator=(const ObjectsIterator& rhs)
{
	index = rhs.index;
	current = rhs.current;
	return *this;
}

void ObjectsIterator::swap(ObjectsIterator& other) noexcept
{
	std::swap(index, other.index);
	std::swap(current, other.current);
}

ObjectsIterator& ObjectsIterator::operator++()
{
	for (++index; index < store.GetObjectsNum(); ++index)
	{
		current = store.GetById(index);
		if (current.IsValid())
		{
			break;
		}
	}
	return *this;
}

ObjectsIterator ObjectsIterator::operator++(int)
{
	auto tmp(*this);
	++(*this);
	return tmp;
}

bool ObjectsIterator::operator==(const ObjectsIterator& rhs) const
{
	return index == rhs.index;
}

bool ObjectsIterator::operator!=(const ObjectsIterator& rhs) const
{
	return index != rhs.index;
}

UEObject ObjectsIterator::operator*() const
{
	assert(current.IsValid() && "ObjectsIterator::current is not valid!");

	return current;
}

UEObject ObjectsIterator::operator->() const
{
	return operator*();
}


//------------------------------------------------------------------------

class FUObjectItem
{
public:
	UObject* Object; //0x0000
	int32_t Flags; //0x0008
	int32_t ClusterIndex; //0x000C
	int32_t SerialNumber; //0x0010
	//char Unknown[0x4];
	//char pad2[0x0010];
	//char pad3[0x0000];
};



class TUObjectArray
{
public:
	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;
};





class FUObjectArray
{
public:
	int32_t ObjFirstGCIndex;
	int32_t ObjLastNonGCIndex;
	int32_t MaxObjectsNotConsideredByGC;
	int32_t OpenForDisregardForGC;
	TUObjectArray ObjObjects;
};
FUObjectArray* GUObjectArray = nullptr;
static TUObjectArray& GetGlobalObjects();



bool ObjectsStore::Initialize()
{
	static bool blGUObject = false;
	if (!blGUObject) {
       auto UE4 = Tools::GetBaseAddress("libUE4.so");
       while (!UE4) {
           UE4 = Tools::GetBaseAddress("libUE4.so");
           sleep(1);
          }      
       GUObjectArray = (FUObjectArray*) (UE4 + 0x8f43790);
		blGUObject = true;
       }
	return true;
}




static TUObjectArray& GetGlobalObjects() {
   return GUObjectArray->ObjObjects;
}


void* ObjectsStore::GetAddress()
{
	//LOGE("7!");
	return GUObjectArray;
}

size_t ObjectsStore::GetObjectsNum() const
{
	//LOGE("8!");
	return GUObjectArray->ObjObjects.NumElements;
}

UEObject ObjectsStore::GetById(size_t id) const
{
	//LOGE("9!");
	return GUObjectArray->ObjObjects.Objects[id].Object;
}




