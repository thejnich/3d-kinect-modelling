#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>

class Mutex {
public:
	Mutex() {
		pthread_mutex_init( &m_mutex, NULL );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
	
	class ScopedLock
	{
		Mutex & _mutex;
	public:
		ScopedLock(Mutex & mutex)
		: _mutex(mutex)
		{
			_mutex.lock();
		}
		~ScopedLock()
		{
			_mutex.unlock();
		}
	};
private:
	pthread_mutex_t m_mutex;
};

#endif /* _MUTEX_H */