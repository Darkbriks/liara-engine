/**
 * @file Singleton.h
 * @brief Singleton class template
 */

#pragma once

#include <memory>
#include <mutex>

namespace Liara
{
    /**
     * @class Singleton
     * @brief Singleton class template
     *
     * This class is a template for creating singletons.
     */
    template <typename T>
    class Singleton
    {
    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;

        /**
         * @brief Returns the instance of the singleton.
         * @return The instance of the singleton.
         * @warning This function is not thread-safe. For thread-safety, see GetInstanceSync().
         */
        static T& GetInstance()
        {
            std::call_once(m_OnceFlag, []() { m_Instance.reset(new T); });
            return *m_Instance;
        }

        /**
         * @brief Returns the instance of the singleton in a thread-safe way.
         * @return The instance of the singleton.
         */
        static T& GetInstanceSync()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);
            return GetInstance();
        }

    protected:
        Singleton() = default; ///< Default constructor
        virtual ~Singleton() = default; ///< Default destructor

    private:
        static std::unique_ptr<T> m_Instance; ///< The instance of the singleton
        static std::once_flag m_OnceFlag; ///< The flag to ensure the singleton is created only once
    };

    template <typename T>
    std::unique_ptr<T> Singleton<T>::m_Instance = nullptr;

    template <typename T>
    std::once_flag Singleton<T>::m_OnceFlag;
}