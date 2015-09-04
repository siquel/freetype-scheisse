#include <GL/glew.h>
#include <glm/matrix.hpp>
#include "singleton.hpp"
#include <iostream>
#include <cassert>

#define SINGLETON(c) friend class Singleton<c>


class Unisade : public Singleton<Unisade>{
	SINGLETON(Unisade);
private:
	Unisade() {

	}
public:
	void gahvee() {
		std::cout << "gimme gahvee" << std::endl;
	}
};

int main(int argc, char** argv) {
	glm::mat4 alegs;
	Unisade::getInstance().gahvee();
	assert(&Unisade::getInstance() == &Unisade::getInstance());
	

	return 0;
}