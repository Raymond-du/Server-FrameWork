#include "byte.h"
#include <cstring>

namespace raymond {

	Byte::Byte() {
		m_data = new char[64];
		m_capacity = 64;
	}

	Byte::Byte(unsigned int length) {
		m_data = new char[length];
		m_capacity = length;
	}

	Byte::Byte(const char* data, int length) {
		m_data = new char[length];
		m_capacity = length;
		memcpy(m_data, data, length);
		m_length = length;
	}

	Byte::~Byte() {
		delete[] m_data;
	}

	bool Byte::resize(unsigned int size) {
		if (size < m_length) {
			return false;
		}
		char* tmp = new char[size];
		m_capacity = size;
		memcpy(tmp, m_data, m_length);
		delete[] m_data;
		m_data = tmp;
		return true;
	}

	Byte& Byte::append(const char* data, int length) {
		int surplus = m_capacity - m_length;
		if (surplus < length) {
			int newSize = m_capacity / 2 + m_capacity;
			newSize = newSize < m_length + length ? m_length + length : newSize;
			resize(newSize);
		}
		memcpy(m_data + m_length, data, length);
		m_length += length;
		return *this;
	}
}
