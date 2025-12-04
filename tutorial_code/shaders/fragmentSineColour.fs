#version 330 core
in vec3 ourColor;
out vec4 FragColor;
uniform float timeMult;
void main()
{
   FragColor = vec4(ourColor.x*timeMult,ourColor.y*timeMult,ourColor.z*timeMult, 1.0f);
}