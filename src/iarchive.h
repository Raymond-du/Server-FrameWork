#ifndef __IARCHIVE_H
#define __IARCHIVE_H

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <sys/types.h>
#include <type_traits>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace raymond {

	template <class T>
	class IArchive {
	public:
		IArchive(T& stream) 
			: m_stream(stream) {
		}

		/**
		 * @brief DecodeZigzag 进行解码有符号的数值
		 *			将value右移以为,并将符号位放到最高位
		 * @tparam Type 解码的类型
		 * @param value 值
		 *
		 * @return 解码后的值
		 */
		template <class Type>
		static Type DecodeZigzag(Type value){
			return (value >> 1) ^ -(value & 1);
		}
	
		/**
		 * @brief readUint32 读取32位的无符号整形
		 *		一个一个字节读取如果最高位是1 表明未读完
		 *		反之读取结束
		 * @return 解析到的数值
		 */
		uint32_t readUint32() {
			uint32_t res = 0;
			uint8_t tmp = 0;
			while(1){
				m_stream.read(&tmp,sizeof(tmp));
				if(tmp > 0x80){
					res |= (tmp & 0x7F);
					res <<= 7;
				}else{
					res |= tmp;
					break;
				}
			}
			return res;
		}
		
		int32_t readInt32() {
			return DecodeZigzag((int32_t)readUint32());
		}
		
		uint64_t readUint64() {
			uint64_t res = 0;
			uint8_t tmp = 0;
			while(1){
				m_stream.read(&tmp,sizeof(tmp));
				if(tmp > 0x80){
					res |= (tmp & 0x7F);
					res <<= 7;
				}else{
					res |= tmp;
					break;
				}
			}
			return res;
		}
		
		int64_t readInt64() {
			return DecodeZigzag((int64_t)readUint64());
		}
	
		/**
		 * @brief >> 反序列化到value中
		 *
		 * @tparam Type 类型
		 * @param value 反序列化保存的值
		 *
		 * @return 自己

		 */
		template<class Type>
		IArchive& operator >> (Type& value) {
			return *this & value;
		}

		IArchive& operator & (bool& value) {
			m_stream.read(&value,1);
			return *this;
		}

		IArchive& operator & (char& value) {
			m_stream.read(&value,1);
			return *this;
		}

		IArchive& operator & (uint8_t& value) {
			m_stream.read(&value,1);
			return *this;
		}

		IArchive& operator & (uint16_t& value) {
			m_stream.read(&value,2);
			return *this;
		}

		IArchive& operator & (uint32_t& value) {
			value = readUint32();
			return *this;
		}
		
		IArchive& operator & (int32_t& value) {
			value = readInt32();
			return *this;
		}

		IArchive& operator & (uint64_t& value) {
			value = readUint64();
			return *this;
		}
		
		IArchive& operator & (int64_t& value) {
			value = readInt64();
			return *this;
		}
		
		IArchive& operator & (float& value) {
			int32_t tmp = readInt32();
			memcpy(&value,&tmp,4);
			return *this;
		}

		IArchive& operator & (double& value) {
			int64_t tmp = readInt64();
			memcpy(&value,&tmp,8);
			return *this;
		}	
		
		IArchive& operator & (std::string& str) {
			int len = readUint32();
			str.resize(len);
			m_stream.read((void *)str.c_str(),len);
			return *this;
		}

		template <class Type>
		IArchive& operator & (std::vector<Type>& vec) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Type tmp;
				*this & tmp;
				vec.push_back(tmp);
			}
			return *this;
		}

		template <class Type>
		IArchive& operator & (std::list<Type>& list) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Type tmp;
				*this & tmp;
				list.push_back(tmp);
			}
			return *this;
		}

		template <class Type>
		IArchive& operator & (std::set<Type>& set) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Type tmp;
				*this & tmp;
				set.push_back(tmp);
			}
			return *this;
		}

		template <class Type>
		IArchive& operator & (std::unordered_set<Type>& set) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Type tmp;
				*this & tmp;
				set.push_back(tmp);
			}
			return *this;
		}
	
		template <class Key,class Value>
		IArchive& operator & (std::map<Key,Value>& map) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Key key;
				Value value;
				*this & key;
				*this & value;
				map[key] = value;
			}
			return *this;
		}
	
		template <class Key,class Value>
		IArchive& operator & (std::unordered_map<Key,Value>& map) {
			uint32_t size = readUint32();
			for(int i = 0 ;i < size ;i++) {
				Key key;
				Value value;
				*this & key;
				*this & value;
				map[key] = value;
			}
			return *this;
		}

		template <class Type>
		IArchive& operator & (std::shared_ptr<Type>& ptr) {
			Type* tmp = (Type*)new char[sizeof(Type)];
			*this & *tmp;
			ptr.reset(tmp);
			return *this;
		}

		template<class Type>
		IArchive& operator & (Type& value) {
			if(std::is_class<Type>::value) {
				value.serialize(*this);
			} else {
				m_stream.read(&value,sizeof(Type));
			}
			return *this;
		}
		
	private:
		T & m_stream;
	};
}
#endif
