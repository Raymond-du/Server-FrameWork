#ifndef __BYTEARRAY_H
#define __BYTEARRAY_H

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstring>
#include <list>
#include <ostream>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
namespace raymond{
	class ByteArray;

	class ByteArray{
	private:
		struct Node{
			Node(size_t new_len);
			~Node();

			Node* m_next = nullptr;
			char* m_ptr = nullptr;
			size_t m_size = 0;

		};
	
	private:
		/**
		 * @brief 每一个node的容量
		 */
		size_t m_baseSize = 0;
		/**
		 * @brief 读写游标的位置
		 */
		size_t m_inputPos = 0;
		size_t m_outputPos = 0;
		/**
		 * @brief 所有结点总的容量
		 */
		size_t m_capacity = 0;
		/**
		 * @brief 内容的大小
		 */
		size_t m_size = 0;
		
		//当前的读取或者写入的位置
		Node *m_curOutput = nullptr;
		Node *m_curInput = nullptr;
		
		//根结点Node的指针
		Node *m_root = nullptr;
		//最后结点Node的指针
		Node *m_last = nullptr;
	public:
		ByteArray(int baseSize = 4096);
		~ByteArray();

		void toHexStream(std::ostream & os);

		/**
		 * @brief adaptSize 如果剩余的容量大小大于size
		 *					则不增加capacity 否则增加capacity
		 *
		 * @param size
		 */
		void adaptSize(size_t size);
		/**
		 * @brief addCapacity 容量的扩充
		 *
		 * @param size 扩充的大小
		 */
		void addCapacity(size_t size);
		/**
		 * @brief getCapacity 获取当前的容量
		 *
		 * @return 容量大小
		 */
		size_t getCapacity();
		/**
		 * @brief getFreeCapacity 获取剩余容量
		 *
		 * @return 
		 */
		size_t getFreeCapacity();

		/**
		 * @brief getSize 获取保存数据的长度
		 *
		 * @return 
		 */
		size_t getSize();

		/**
		 * @brief seekg 设置输入流提取下一个字符的位置
		 *
		 * @param position 位置
		 */
		void seekg(size_t position);
		/**
		 * @brief seekp 设置下一个字符输入的位置
		 *
		 * @param position 位置
		 */
		void seekp(size_t position);

		void write(const void* value,size_t len);
		void read(void *value,size_t len);
		
		/**
		 * @brief writeToFile 将容器中的序列化数据写入到文件中
		 *
		 * @param name 文件名
		 *
		 * @return 是否写入成功
		 */
		bool writeToFile(const std::string & name) const;
		/**
		 * @brief writeToFile 将文件中的序列化数据读到容器中
		 *
		 * @param name 文件名
		 *
		 * @return 是否读取成功
		 */
		bool readFromFile(const std::string & name);
	};
}
#endif
