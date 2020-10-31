#include "byteArray.h"
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <list>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <memory>
#include <unistd.h>
#include <iomanip>
#include "Logger.h"

namespace raymond{
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	ByteArray::Node::Node(size_t new_len){
		m_ptr = new char[new_len];
		m_size = new_len;
	}

	ByteArray::Node::~Node(){
		delete []m_ptr;
	}

	ByteArray::ByteArray(int baseSize){
		if(baseSize < 0){
			baseSize = 4096;
		}

		m_root = new Node(baseSize);
		m_curInput = m_root;
		m_curOutput = m_root;

		m_baseSize = baseSize;
		m_last = m_root;
		m_capacity = baseSize;
	}

	ByteArray::~ByteArray() {
		Node* tmp = m_root;
		while(tmp) {
			m_curOutput = tmp;
			tmp = tmp->m_next;
			delete m_curOutput;
		}
	}

	void ByteArray::toHexStream(std::ostream & os){
		
		Node *pre = m_root;
		uint32_t rlen = 0;
		size_t nsize = 0;
		while(pre){
			if(pre == m_last) {
				nsize = m_size % m_baseSize;
			} else {
				nsize = pre->m_size;
			}
			for(int i = 0 ;i < nsize ;i++) {
				if(rlen > 0 && rlen % 16 == 0) {
					os <<std::endl;
					rlen = 0;
				}
				os << "0x" << std::setw(2) << std::setfill('0') << std::hex <<
					(int)(uint8_t)pre->m_ptr[i] << " ";
				rlen ++;
			}
			pre = pre->m_next;
		}
	}

	void ByteArray::adaptSize(size_t size){
		size_t ncap = m_capacity - m_outputPos;
		if(ncap >= size){
			return ;
		}

		addCapacity(size);
	}

	void ByteArray::addCapacity(size_t size){
		int count = std::ceil((1.0 * size) / m_baseSize);
		for(int i = 0; i < count ;i++){
			m_last->m_next = new Node(m_baseSize);
			m_last = m_last->m_next;
			if (m_curOutput == nullptr){
				m_curOutput = m_last;
			}
			m_capacity += m_baseSize;
		}
	}

	size_t ByteArray::getCapacity(){
		return m_capacity;
	}

	size_t ByteArray::getFreeCapacity(){
		return m_capacity - m_size;
	}

	size_t ByteArray::getSize(){
		return m_size;
	}

	void ByteArray::seekg(size_t position){
		if(position > m_size){
			std::__throw_out_of_range("position cross board");
			return;
		}
		m_inputPos = position;
		if(position == 0){
			m_curInput = m_root;
		}else{
			m_curInput = m_root;
			int count = m_inputPos / m_baseSize;
			while(count --){
				m_curInput = m_curInput->m_next;
			}
		}
	}

	void ByteArray::seekp(size_t position){
		if(position > m_size){
			std::__throw_out_of_range("position cross board");
			return;
		}
		m_outputPos = position;
		if(position == 0){
			m_curOutput = m_root;
		}else{
			m_curInput = m_root;
			int count = m_outputPos / m_baseSize;
			while(count --){
				m_curOutput = m_curOutput->m_next;
			}
		}
	}
	
	void ByteArray::write(const void *value,size_t len){
		if(len <= 0){
			return ;
		}
		//使容量可以满足len的长度
		adaptSize(len);
		size_t npos = m_outputPos % m_baseSize;
		size_t ncap = m_baseSize - npos;
		//写入value的位置
		size_t bpos = 0;
		while(len > 0){
			if(ncap >= len){
				memcpy(m_curOutput->m_ptr + npos,(char *)value + bpos,len);
				if(m_curOutput->m_size == (npos + len)) {
					m_curOutput = m_curOutput->m_next;
				}
				m_outputPos += len;
				break;
			}else{
				memcpy(m_curOutput->m_ptr + npos,(char *)value + bpos,ncap);
				m_outputPos += ncap;
				m_curOutput = m_curOutput->m_next;
				len -= ncap;
				bpos += ncap;
				ncap = m_curOutput->m_size;
				npos = 0;

			}
		}
		if(m_outputPos > m_size){
			m_size = m_outputPos;
		}

	}

	void ByteArray::read(void *value,size_t len){
		if(m_inputPos + len > m_size){
			throw std::out_of_range("not enough len");
			return ;
		}	

		size_t npos = m_inputPos % m_baseSize;
		size_t ncap = m_baseSize - npos;
		size_t bpos = 0;
		while(len > 0){
			if(ncap >= len){
				memcpy((char *)value + bpos,m_curInput->m_ptr + npos,len);
				if(m_curInput->m_size == (npos + len)) {
					m_curInput = m_curInput->m_next;
				}
				m_inputPos += len;
				break;
			}else{
				memcpy((char *)value + bpos,m_curInput->m_ptr + npos,ncap);
				m_inputPos += ncap;
				m_curInput = m_curInput->m_next;
				len -= ncap;
				bpos += ncap;
				ncap = m_curInput->m_size;
				npos = 0;
			}
		}
	}

	bool ByteArray::writeToFile(const std::string & name) const {
		std::ofstream os;
		os.open(name,std::ios::trunc | std::ios::binary);
		if(!os) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"write to file %s error : %s",
								name.c_str(),std::strerror(errno));
			return false;
		}

		Node* tmp = m_root;
		while(1) {
			if (tmp == m_last) {
				os.write(tmp->m_ptr,m_size % m_baseSize);
				break;
			} else {
				os.write(tmp->m_ptr,m_baseSize);
			}
			tmp = tmp->m_next;
		}
		os.close();
		return true;
	}

	bool ByteArray::readFromFile(const std::string & name) {
		std::ifstream is;
		is.open(name,std::ios::binary);
		if(!is) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"read from file %s error : %s",
								name.c_str(),std::strerror(errno));
			return false;
		}
		
		std::shared_ptr<char> buff(new char[m_baseSize],
				[](char *ptr){delete []ptr;});
		while(!is.eof()){
			is.read(buff.get(),m_baseSize);
			write(buff.get(),is.gcount());
		}
		is.close();
		return true;

	}
}
