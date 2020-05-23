#ifndef ASSERTPOINTER
#define ASSERTPOINTER

#include <assert.h>

template<class T> class AssertPointer
{
	private:
		T *_data = nullptr;
	#ifdef _DEBUG
		size_t _size = 0;
	#endif

	public:
		void operator=(T *data);
		bool operator==(T *data);
		T *&data();
		void set_size(size_t size);
		T &operator[](size_t i);
};

template<class T> void AssertPointer<T>::operator=(T *data)
{
	_data = data;
};

template<class T> bool AssertPointer<T>::operator==(T *data)
{
	return _data == data;
};

template<class T> T *&AssertPointer<T>::data()
{
	return _data;
};

template<class T> void AssertPointer<T>::set_size(size_t size)
{
	#ifdef _DEBUG
		_size = size;
	#endif
};

template<class T> T &AssertPointer<T>::operator[](size_t i)
{
	#ifdef _DEBUG
		assert(i < _size);
	#endif
	return _data[i];
};

#endif