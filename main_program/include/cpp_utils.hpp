#ifndef __CPP_UTILS_HPP__
#define __CPP_UTILS_HPP__

template <typename T>
class Singleton
{
public:
    static T& getInstance();
};

template <typename T>
T& Singleton<T>::getInstance()
{
    static T instance;
    return instance;
}

#define SINGLETON_CLASS(Class_name) \
    protected: \
        Class_name() noexcept; \
        ~Class_name() noexcept; \
        Class_name(const Class_name& ) = delete; \
        Class_name& operator=(const Class_name&) = delete; \
        friend class Singleton<Class_name>;

    /*
    If the definition of the class x does not explicitly 
    declare a move constructor, one will be implicitly 
    declared as defaulted if and only if 
    - X does not have a user-declared copy constructor
    -> deleting move constructor is not needed (same as  move operator=)
    */

#define NON_INSTANTIATION_CLASS(Class_name) \
    private: \
        Class_name() = delete; \
        ~Class_name() = delete; \
        Class_name(const Class_name& ) = delete; \
        Class_name& operator=(const Class_name&) = delete; \



#endif