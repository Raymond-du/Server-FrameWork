#ifndef __OARCHIVE_H
#define __OARCHIVE_H

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>
#include <list>	
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace raymond {
	template <class T>
	class OArchive {
	public:
		OArchive(T& stream) :
				m_stream(stream) {
		}

		/**
		 * @brief EncodeZigzag 加码有符号的整形数值
		 *			将value左移,并将最高位的符号为移到最低位
		 * @tparam Type 数值类型
		 * @param value 值
		 *
		 * @return 加码后的数值
		 */
		template <class Type>
		static Type EncodeZigzag(Type value){
			if(value < 0) {
				return (value << 1) | 1;
			} else {
				return value << 1;
			}
		}
		
		/**
		 * @brief writeUint32 压缩32位无符号整形
		 *		使用7位保存数值,第一位为标志位,
		 *		如果保存的数值超过0x80,的最高位变为1;
		 * @param value 需要序列化的值
		 */
		void writeUint32(uint32_t value) {
			uint8_t tmp[5];
			uint8_t i = 0;
			while(value >= 0x80) {
				tmp[i++] = (value & 0x7F) | 0x80;
				value >>= 7;
			}
			tmp[i++] = value;
			m_stream.write(tmp,i);
		}

		void writeInt32(int32_t value) {
			writeUint32((uint32_t)EncodeZigzag(value));	
		}

		void writeUint64(uint64_t value) {
			uint8_t tmp[10];
			uint8_t i = 0;
			while(value >= 0x80) {
				tmp[i++] = (value & 0x7F) | 0x80;
				value >>= 7;
			}
			tmp[i++] = value;
			m_stream.write(tmp,i);
		}

		void writeInt64(int64_t value) {
			writeUint64((uint64_t)EncodeZigzag(value));
		}

		template<class Type>
		OArchive& operator << (Type& value) {
			return (*this & value);
		}

		OArchive& operator & (bool value) {
			m_stream.write(&value,1);
			return *this;
		}

		OArchive& operator & (char value) {
			m_stream.write(&value,1);
			return *this;
		}

		OArchive& operator & (uint8_t value) {
			m_stream.write(&value,1);
			return *this;
		}

		OArchive& operator & (uint16_t value) {
			m_stream.write(&value,2);
			return *this;
		}


		OArchive& operator & (uint32_t value) {
			writeUint32(value);
			return *this;
		}

		OArchive& operator & (int32_t value) {
			writeInt32(value);
			return *this;
		}

		OArchive& operator & (uint64_t value) {
			writeUint32(value);
			return *this;
		}

		OArchive& operator & (int64_t value) {
			writeInt64(value);
			return *this;
		}
	
		OArchive& operator & (float value) {
			writeInt32((int32_t) value);
			return *this;
		}

		OArchive& operator & (double value) {
			writeUint64((uint64_t) value);
			return *this;
		}	

		OArchive& operator & (std::string& str) {
			writeUint32(str.size());
			m_stream.write(str.c_str(),str.size());
			return *this;
		}

		template <class Type>
		OArchive& operator & (std::vector<Type>& vec) {
			writeUint32(vec.size());
			for(auto& i : vec) {
				*this & i;
			}
			return *this;
		}

		template <class Type>
		OArchive& operator & (std::list<Type>& list) {
			writeUint32(list.size());
			for(auto& i : list) {
				*this & i;
			}
			return *this;
		}

		template <class Type>
		OArchive& operator & (std::set<Type>& set) {
			writeUint32(set.size());
			for(auto& i : set) {
				*this & i;
			}
			return *this;
		}

		template <class Key,class Value>
		OArchive& operator & (std::map<Key,Value>& map) {
			writeUint32(map.size());
			for(auto& it : map) {
				*this & it.first;
				*this & it.second;
			}
			return *this;
		}

		template <class Key,class Value>
		OArchive& operator & (std::unordered_map<Key,Value>& map) {
			writeUint32(map.size());
			for(auto& i : map) {
				*this & i.first;
				*this & i.second;
			}
			return *this;
		}

		template <class Type>
		OArchive& operator & (std::shared_ptr<Type>& ptr) {
			return *this & *(ptr.get());
		}

		template<class Type>
		OArchive& operator & (Type& value) {
			if(std::is_class<Type>::value) {
				value.serialize(*this);
			} else {
				m_stream.write(value,sizeof(Type));
			}
			return *this;
		}

	private:
		T & m_stream;
	};

}
#endif
