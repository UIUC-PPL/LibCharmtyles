#pragma once

#include <memory>

namespace ct { namespace util {
    template <typename T, typename SingletonClass>
    class singleton
    {
    public:
        using value_type = T;
        using class_type = SingletonClass;

        // Non-copyable, non-movable
        singleton(singleton const&) = delete;
        singleton(singleton&&) = delete;
        singleton& operator=(singleton const&) = delete;
        singleton& operator=(singleton&&) = delete;

        static const std::unique_ptr<value_type>& instance()
        {
            static std::unique_ptr<value_type> inst{new value_type()};

            return inst;
        }

    protected:
        singleton() = default;
    };

}}    // namespace ct::util

#define CT_GENERATE_SINGLETON(type, name)                                      \
    class name : public ct::util::singleton<type, name>                        \
    {                                                                          \
    private:                                                                   \
        name() = default;                                                      \
    }

#define CT_ACCESS_SINGLETON(name) (*name::instance())
