#ifndef VBO_H_
#define VBO_H_

class Vbo {
public:
	virtual ~Vbo() = default;

	virtual void Bind() noexcept = 0;
};

#endif //VBO_H_
