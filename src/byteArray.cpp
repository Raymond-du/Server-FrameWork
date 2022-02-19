#include "byteArray.h"
#include <algorithm>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <list>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
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
		if (m_next != nullptr) {
			delete m_next;
			m_next = nullptr;
		}
		delete []m_ptr;
		m_ptr = nullptr;
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
	//	m_capacity = baseSize;
	}

	ByteArray::~ByteArray() {
		delete m_root;
		m_root = nullptr;
		//Node* tmp = m_root;
		//while(tmp) {
		//	m_curOutput = tmp;
		//	tmp = tmp->m_next;
		//	delete m_curOutput;
		//}
	}

	//void ByteArray::toHexStream(std::ostream & os){
	//	
	//	Node *pre = m_root;
	//	uint32_t rlen = 0;
	//	size_t nsize = 0;
	//	while(pre){
	//		if(pre == m_last) {
	//			nsize = m_size % m_baseSize;
	//		} else {
	//			nsize = pre->m_size;
	//		}
	//		for(int i = 0 ;i < nsize ;i++) {
	//			if(rlen > 0 && rlen % 16 == 0) {
	//				os <<std::endl;
	//				rlen = 0;
	//			}
	//			os << "0x" << std::setw(2) << std::setfill('0') << std::hex <<
	//				(int)(uint8_t)pre->m_ptr[i] << " ";
	//			rlen ++;
	//		}
	//		pre = pre->m_next;
	//	}
	//}

	void ByteArray::adaptSize(size_t size){
		size_t ncap = m_baseSize - m_outputPos;
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
			//m_capacity += m_baseSize;
		}
	}

	//size_t ByteArray::getCapacity(){
	//	return m_capacity;
	//}

	//size_t ByteArray::getFreeCapacity(){
	//	return m_baseSize - m;
	//}

	size_t ByteArray::getSize() const {
		return m_size;
	}

	//void ByteArray::seekg(size_t position){
	//	if(position > m_size){
	//		std::__throw_out_of_range("position cross board");
	//		return;
	//	}
	//	m_inputPos = position;
	//	if(position == 0){
	//		m_curInput = m_root;
	//	}else{
	//		m_curInput = m_root;
	//		int count = m_inputPos / m_baseSize;
	//		while(count --){
	//			m_curInput = m_curInput->m_next;
	//		}
	//	}
	//}

	//void ByteArray::seekp(size_t position){
	//	if(position > m_size){
	//		std::__throw_out_of_range("position cross board");
	//		return;
	//	}
	//	m_outputPos = position;
	//	if(position == 0){
	//		m_curOutput = m_root;
	//	}else{
	//		m_curInput = m_root;
	//		int count = m_outputPos / m_baseSize;
	//		while(count --){
	//			m_curOutput = m_curOutput->m_next;
	//		}
	//	}
	//}
	
	ssize_t ByteArray::write(const void *value,size_t len){
		if(len <= 0){
			return 0;
		}
		std::unique_lock<std::mutex> lock(m_mutex); 
		//使容量可以满足len的长度
		adaptSize(len);
		//当前节点可以保存多少数据
		size_t ncap = m_baseSize - m_outputPos;
		//写入value的位置
		size_t bpos = 0;
		while(len > 0){
			if((m_baseSize - m_outputPos) >= len){
				memcpy(m_curOutput->m_ptr + m_outputPos,(char *)value + bpos,len);
				//if(m_curOutput->m_size == (npos + len)) {
				//	m_curOutput = m_curOutput->m_next;

				//}
				m_outputPos += len;
				//m_size += len;
				break;
			}else{
				memcpy(m_curOutput->m_ptr + m_outputPos,(char *)value + bpos,ncap);
				m_outputPos = 0;
				m_curOutput = m_curOutput->m_next;
				len -= ncap;
				bpos += ncap;
				ncap = m_baseSize;
				//m_size += ncap;
			}
		}
		m_size += len;
		return len;
	}

	ssize_t ByteArray::read(void *value,size_t len){
		//if(m_inputPos + len > m_size){
		//	throw std::out_of_range("not enough len");
		//	return ;
		//}	

		//但前节点复制的长度
		std::unique_lock<std::mutex> lock(m_mutex);
		size_t realSize = 0;
		size_t bpos = 0;
		while(len > 0){
			if (m_curInput == m_curOutput) {
				realSize = (m_outputPos - m_inputPos) > len ? len : m_outputPos - m_inputPos;
				memcpy((char*)value + bpos, m_curInput->m_ptr + m_inputPos, realSize);
				bpos += realSize;
				len -= realSize;
				m_inputPos += realSize;
			} else {
				realSize = m_baseSize - m_inputPos > len ? len : m_baseSize - m_inputPos;
				memcpy((char*)value + bpos, m_curInput->m_ptr + m_inputPos, realSize);
				bpos += realSize;
				len -= realSize;
				m_inputPos = 0;
				Node* tmp = m_curInput;
				m_curInput = m_curInput->m_next;
				delete tmp;
			}
			//if(ncap >= len){
			//	memcpy((char *)value + bpos,m_curInput->m_ptr + npos,len);
			//	if(m_curInput->m_size == (npos + len)) {
			//		m_curInput = m_curInput->m_next;
			//	}
			//	m_inputPos += len;
			//	break;
			//}else{
			//	memcpy((char *)value + bpos,m_curInput->m_ptr + npos,ncap);
			//	m_inputPos += ncap;
			//	m_curInput = m_curInput->m_next;
			//	len -= ncap;
			//	bpos += ncap;
			//	ncap = m_curInput->m_size;
			//	npos = 0;
			//}
		}
		m_size -= bpos;
		return bpos;
	}

	//bool ByteArray::writeToFile(const std::string & name) const {
	//	std::ofstream os;
	//	os.open(name,std::ios::trunc | std::ios::binary);
	//	if(!os) {
	//		RAYMOND_LOG_FMT_ERROR(g_logger,"write to file %s error : %s",
	//							name.c_str(),std::strerror(errno));
	//		return false;
	//	}

	//	Node* tmp = m_root;
	//	while(1) {
	//		if (tmp == m_last) {
	//			os.write(tmp->m_ptr,m_size % m_baseSize);
	//			break;
	//		} else {
	//			os.write(tmp->m_ptr,m_baseSize);
	//		}
	//		tmp = tmp->m_next;
	//	}
	//	os.close();
	//	return true;
	//}

	//bool ByteArray::readFromFile(const std::string & name) {
	//	std::ifstream is;
	//	is.open(name,std::ios::binary);
	//	if(!is) {
	//		RAYMOND_LOG_FMT_ERROR(g_logger,"read from file %s error : %s",
	//							name.c_str(),std::strerror(errno));
	//		return false;
	//	}
	//	
	//	std::shared_ptr<char> buff(new char[m_baseSize],
	//			[](char *ptr){delete []ptr;});
	//	while(!is.eof()){
	//		is.read(buff.get(),m_baseSize);
	//		write(buff.get(),is.gcount());
	//	}
	//	is.close();
	//	return true;

	//}
}
