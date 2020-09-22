#ifndef __SINGLETION_H
#define __SINGLETION_H

namespace raymond {

    template<class T>
    class SingleTon
    {
    public:
        static T & getInstance() {
            static T t;
            return t;
        }
    };
}

#endif	