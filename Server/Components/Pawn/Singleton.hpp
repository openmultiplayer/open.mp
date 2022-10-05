/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

template <class T>
class Singleton
{
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
		if (m_Instance != nullptr)
		{
			delete m_Instance;
			m_Instance = nullptr;
		}
	}
};

template <class T>
T* Singleton<T>::m_Instance = nullptr;
