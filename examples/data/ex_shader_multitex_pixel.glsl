#version 330 core
/* Simple fragment shader which uses the fractional texture coordinate to
 * look up the color of the second texture (scaled down by factor 100).
 */
/*
#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D al_tex;
uniform sampler2D tex2;
in vec2 varying_texcoord;
*/
out vec3 fragmentColor;
void main()
{
    //vec4 color = texture(tex2, fract(varying_texcoord * 100.0));
    fragmentColor = vec3(1.0, 0.0, 0.0);//color * texture(al_tex, varying_texcoord);
}
