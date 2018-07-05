#version 330 core

in vec4 al_pos;
//in vec3 al_user_attr_0;

uniform mat4 al_projview_matrix;

/* pixel_position and normal are used to compute the reflections in the pixel shader */
//out vec3 pixel_position;
//out vec3 normal;

void main()
{
   // pixel_position = al_pos.xyz;
   // normal = al_user_attr_0;
    gl_Position = al_projview_matrix * al_pos;
}
