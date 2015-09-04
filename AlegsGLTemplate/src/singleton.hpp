#pragma once

template <class T>
class Singleton {
private:
	Singleton(Singleton const&) = delete;
	void operator=(Singleton const&) = delete;
	Singleton& operator =(Singleton&& singleton) = delete;
	Singleton(Singleton&& ) = delete;
protected:
	inline explicit Singleton();
	inline ~Singleton();
public:
	static T& getInstance();

};

template<typename T>
T& Singleton<T>::getInstance() {

	static T instance;
	return instance;
}


template<typename T>
Singleton<T>::Singleton() {

}

template<typename T>
Singleton<T>::~Singleton() {

}
