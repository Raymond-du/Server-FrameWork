#ifndef __UTIL_SINGLETON_HPP
#define __UTIL_SINGLETON_HPP

#include "Any.hpp"
#include "NoCopyable.hpp"
#include <unordered_map>
#include <utility>

namespace ray
{
class Singleton : public NoCopyable
{
public:
    
    static Singleton* getInstance(); 
	
	template <class T, class... Args>
	bool addObject(Args&&... args) {
		std::size_t objid = typeid(T).hash_code();
		auto iter = m_objects.find(objid);
		if (iter != m_objects.end()) {
			return false;
		}
		Any obj;
		obj.reset<T>(std::forward<Args>(args)...);
		m_objects[objid] = obj;
		return true;
	}

	template <class T>
	T* getObject() {
		std::size_t objid = typeid(T).hash_code();
		auto iter = m_objects.find(objid);
		if (iter != m_objects.end()) {
			return &(iter->second.get<T>());
		}
		return nullptr;
	}
private:
    Singleton();
	
private:
	std::unordered_map<int, Any> m_objects;
};
}

#endif
