#ifndef STANDARD_SEMAPHORE_H
#define STANDARD_SEMAPHORE_H

#include <semaphore.h>

class Semaphore
{
public:
	Semaphore(int initial_value = 1)
	{
		sem_init(_sem, 0, initial_value);
	}

	~Semaphore()
	{
		sem_destroy(&_sem);
	}

	void wait()
	{
		sem_wait(&_sem);
	}

	void signal()
	{
		sem_post(&_sem);
	}

	int value()
	{
		int result;
		sem_getvalue(&_sem, &result);
		return result;
	}

protected:
	sem_t* _sem;
};

#endif
