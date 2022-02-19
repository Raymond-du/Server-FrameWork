#ifndef __BYTE_H
#define __BYTE_H

#include <memory>
namespace raymond {

	class Byte {
	
	private:
		char* m_data;
		int m_length;
		unsigned int m_capacity;
	public:
		typedef std::shared_ptr<Byte> ptr;

		Byte();
		Byte(unsigned int length);
		Byte(const char* data, int lenght);
		~Byte();
		char* getData() {return m_data;}
		unsigned int getCapacity() { return m_capacity;}
		unsigned int getLength() {return m_length;}
		Byte& append(const char* data, int lenght);
		bool resize(unsigned int size);
	};
}

#endif
