#ifndef __CHARS_H
#define __CHARS_H

#include <bits/stdint-intn.h>
#include <cstring>
#include <ios>
#include <memory>

namespace raymond {
	
	template <class T>
	class Array {
		
	public:
		typedef std::shared_ptr<Array> ptr;
		explicit Array(T* data) {
			m_data.reset(data, std::default_delete<T[]>());
		}
		explicit Array(int32_t size) {
			m_data.reset(new T[size], std::default_delete<T[]>());
		}

		void reset(T* data) {
			m_data.reset(data, std::default_delete<T[]>());
		}

		void reset(int32_t size) {
			m_data.reset(new T[size], std::default_delete<T[]>());
		}

		Array<T> subArray(int offset, int32_t len) {
			Array res(len);
			int index = 0;
			while (len > 0) {
				res[index++] = m_data.get()[offset + index];
				len--;
			}
			return res;
		}

		T* getArray() {
			return m_data.get();
		}

		T& operator[](int index) {
			return (*m_data)[index];
		}
	private:
		std::shared_ptr<T> m_data;
	};

	Array<char> strToArray(const std::string& str) {
		Array<char> res(str.size());
		memcpy(res.getArray(), str.c_str(), str.size());
		return res;
	}
}

#endif
