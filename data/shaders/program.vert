#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform int computeReflect;
uniform int computeRefract;
uniform float refractCoef;
uniform vec3 eyePos;
uniform int normalMapping; 

in vec4 vpos;
in vec4 vnorm;
in vec4 vtan; 
in vec2 vtextcoord;

out mat3 TBN; 
out vec4 fpos;
out vec4 fnormal;
out vec3 ftextCubeRfl;
out vec3 ftextCubeRfr;
out vec2 ftextcoord;

void main()
{
	mat4 mvp = proj * view * model;
	mat4 invT = inverse(transpose(model));

	fnormal = invT * vnorm;
	fpos = model * vpos;
	ftextcoord = vtextcoord;

	if (normalMapping == 1) // Normal texture
	{
		vec3 tan = normalize((invT * vtan).xyz);
		vec3 btan = cross(tan, normalize(fnormal.xyz));
		TBN = transpose(mat3(tan, btan, normalize(fnormal)));
	}

	if (computeReflect == 1)
	{
		vec3 I = normalize(fpos.xyz - eyePos);
		ftextCubeRfl = reflect(I, normalize(fnormal.xyz));
	}

	if (computeRefract == 1)
	{
		vec3 I = normalize(fpos.xyz - eyePos);
		ftextCubeRfr = refract(I, normalize(fnormal.xyz), refractCoef);
	}

	gl_Position = mvp * vpos;
}