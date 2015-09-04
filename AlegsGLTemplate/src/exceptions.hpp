#pragma once
#include <stdexcept>
#include <iostream>
#include <utility>

class TooLeetToHandle : public std::exception {
public:
	virtual const char* what() const throw(){
		return "Too leet to handle!";
	}
};
class LameAssString {
private:
	char* buffer;
	LameAssString(const LameAssString&) = delete;
	void operator=(LameAssString const&) = delete;
public:
	LameAssString(char*);
	void resize(size_t vitut);

};

class Rethrowing {
public:
	Rethrowing() = default;
	~Rethrowing() = default;

	void thisIsSafe() /*noexcept */ {
		std::cout << "do i throw? " << false <<  std::endl;
	}
	void thisIsNot() {
		LameAssString gg("unijeejee");
		try {
			gg.resize(1337);
		}
		catch (TooLeetToHandle& l) {
			(void)l;
			throw;
		}
	}
};