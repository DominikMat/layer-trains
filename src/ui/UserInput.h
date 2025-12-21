#ifndef UserInput_H
#define UserInput_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

#include "TextButton.h"

using namespace glm;

class UserInput : public UIObject
{
protected:
    TextButton* input_box;

public:
    UserInput() {}
    ~UserInput() {}
};

#endif
