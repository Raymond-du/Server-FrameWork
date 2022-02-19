#ifndef __SERVLET_EVENTTYPE_H
#define __SERVLET_EVENTTYPE_H

namespace raymond {
	enum class ReqEvent{
		UnkownEvent			     = 0,
		UserLoginEvent			 = 1,
		UserRegistEvent      = 2,
		GetMajorsInfoEvent	 = 3,
		UserChangeEvent			 = 4,
		GetUsersInfoEvent		 = 5,
		GetClzScheInfoEvent	 = 6,
		CreateMajorEvent		 = 7,
		DelUserEvent				 = 8,
		DelClzScheEvent			 = 9,
		ClzScheChangeEvent	 = 10,
		AddUserEvent				 = 11,
		AddCourseEvent			 = 12,


		
	};

	enum class RespCode {
		UnkownCode					 = 0,
		SuccessCode					 = 1,
		FailureCode					 = 2,
		NotFoundCode				 = 3, //找不到指定的请求码
		CreateMajorCode			 = 4, // 请求是否创建Major
	};
}
#endif
