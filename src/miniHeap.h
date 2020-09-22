
#ifndef __MINIHEAP_H
#define __MINIHEAP_H

namespace raymond{
	/**
	 * @brief 最小堆 允许存储10000个对象
	 *
	 * @tparam T 存储的对象类型
	 */
	template <class T>
	class MiniHeap{
	private:
		const int MAX_LAYERSIZE = 100;
		int m_layers = 0; //数组的使用层数
		T* m_members[100]; //
		
	public:
		MiniHeap(){
			//先创建一层数组
			m_members[0] = new T[MAX_LAYERSIZE];
			m_layers++;
		}

		void add:q
	};
}


#endif
