#ifndef __NONCOPY_H
#define __NONCOPY_H

namespace raymond{
	/**
	 * @brief 子类不可以进行拷贝,赋值
	 */
	class NonCopyable{
	public:
		/**
		 * @brief NonCopyable 默认构造函数
		 */
		NonCopyable() = default;
		/**
		 * @brief ~NonCopyable 默认析构函数
		 */
		~NonCopyable() = default;
		/**
		 * @brief NonCopyable 拷贝构造函数禁用
		 */
		NonCopyable(const NonCopyable& ) = delete;
		/**
		 * @brief operator= 赋值函数的禁用
		 */
		NonCopyable& operator= (const NonCopyable &) = delete;

	};

}

#endif
