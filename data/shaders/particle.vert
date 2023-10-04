#version 430

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float radius;

in vec4 ppos;
in float pscalar;
in vec4 vpos;
in vec2 vtextcoord;

out vec2 ftextcoord;
out float fscalar;
flat out mat4 fmodel;
out vec4 fpos;

void main()
{
    // Particle world position
    vec4 particlePos = model * ppos;
    
    // Build model matrix for billboards
    float diam = radius * 2.1; // para ocultar los bordes de la textura se añade 0.1 al factor 2
    mat4 scale = mat4(diam);
    scale[3][3] = 1.0f;

    fmodel = view;
    fmodel[0][3] = particlePos.x;
	fmodel[1][3] = particlePos.y;
	fmodel[2][3] = particlePos.z;
	fmodel[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    fmodel = scale * fmodel;
	fmodel = transpose(fmodel);

    fpos = fmodel * vpos;
    fscalar = pscalar;
    ftextcoord = vtextcoord;
    gl_Position = proj * view * fpos;
}