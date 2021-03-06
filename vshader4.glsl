#version 130

uniform mat4 model;
uniform mat4 camera;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 modelView;

uniform vec4 lightPos;

in vec4 vPosition;
in vec4 vColor;

out vec4 color;
out vec3 N;
out vec3 L;
out vec3 E;

out float transparency;

void main()
{

  vec3 vNormal=vec3(0,0,1);
  N = normalMatrix*vNormal;
  L = (camera*lightPos).xyz-(modelView*vPosition).xyz;
  E = -(modelView*vPosition).xyz; //from pt to viewer

  transparency = 0.9;
  color = vColor;
  gl_Position = projection *modelView * vPosition;

}
