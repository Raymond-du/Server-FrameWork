
#ifndef __HOOK_H
#define __HOOK_H

namespace raymond{
	/**
	 * @brief is_hook_enable 判断当前线程是否允许被hook
	 *
	 * @return bool
	 */
	bool is_hook_enable();

	/**
	 * @brief set_hook_enable 设置当前线程允许被hook
	 *
	 * @param flag bool
	 */
	void set_hook_enable(bool flag);

}
//使用c风格
extern "C"{
	typedef unsigned int (*sleep_func)(unsigned int seconds);
	//extern 表示定义在其他的文件中  sleep_f 保存这原本的函数
	extern sleep_func sleep_f;
};

#endif

