#version 150

uniform mat4 model;
uniform mat4 camera;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 modelView;

uniform vec4 lightPos;
uniform float global_time;

in vec4 vPosition;
in vec4 colors;

out vec3 N;
out vec3 L;
out vec3 E;
out vec4 color;

out float transparency;

void main() 
{
  vec3 vNormal=vec3(0,0,1);
  N = normalMatrix*vNormal;
  L = (camera*lightPos).xyz-(modelView*vPosition).xyz;
  E = -(modelView*vPosition).xyz; //from pt to viewer

  color = colors;

  transparency = 0.6;
  gl_Position = projection *modelView * vPosition;

} 
