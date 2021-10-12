#pragma once

template <class T>
class Singleton {
protected:
    static T* m_Instance;

public:
    Singleton()
    {
    }
    virtual ~Singleton()
    {
    }

    inline static T* Get()
    {
        if (m_Instance == nullptr)
            m_Instance = new T;
        return m_Instance;
    }

    inline static void Destroy()
    {
        if (m_Instance != nullptr) {
            delete m_Instance;
            m_Instance = nullptr;
        }
    }
};

template <class T>
T* Singleton<T>::m_Instance = nullptr;
