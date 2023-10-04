
#version 430

struct lightInfo 
{
	vec3 position;
	vec3 color;
	int  type;
	float intensity;
	float linearAtt;
};

uniform sampler2D colorText;
uniform int texturing;
uniform float radius;
uniform mat4 proj;
uniform int lighting;
uniform lightInfo lights[8];
uniform int numLights;
uniform vec3 eyePos;
uniform vec3 ambient;
uniform int shininess;
uniform mat4 view;

in vec2 ftextcoord;
flat in mat4 fmodel;
in float fscalar;
in vec4 fpos;

out vec4 fragColor;

vec4 computeLighting(vec3 normal)
{
    float diff = 0.0f;
    float spec = 0.0f;
    vec3 lightColor = vec3(1.0f);

    for (int i = 0; i < numLights; ++i)
    {
        vec3 lightDir = vec3(0.0f);
        float dist = 0.0f;

        if (lights[i].type == 0)
        {
            lightDir = normalize(lights[i].position);
        }
        else if (lights[i].type == 1)
        {
            lightDir = lights[i].position - fpos.xyz;
            dist = length(lightDir);
            lightDir = normalize(lightDir);
        }

        vec3 reflectDir = normalize(reflect(lightDir, normal));
        vec3 eyeDir = normalize(fpos.xyz - eyePos);
        float intensity = lights[i].intensity / (1.0f + lights[i].linearAtt * dist);

        diff += intensity * max(dot(lightDir, normal), 0.0f);
        if (texturing == 100)
            spec += intensity * pow(max(dot(reflectDir, eyeDir), 0.0f), shininess);

        lightColor *= lights[i].color;
    }

    lightColor = normalize(lightColor);

    return vec4(ambient + diff * lightColor + spec * vec3(1.0f), 1.0f);
}

vec3 computeDepthAndNormal()
{
    vec2 xy = ftextcoord * 2.0f - 1.0f;
    float dist = dot(xy, xy);

    // Descartar fragmentos fuera del circulo
    if (dist > 0.9) discard; 
    vec3 normal = vec3(xy, sqrt(1.0f - dist));

    // Transformar normal a coordenadas del mundo
    mat4 transInvModel = transpose(inverse(mat4(fmodel)));
    normal = normalize(vec3(transInvModel * vec4(normal, 1.0f)));
    // //mat3 normalMatrix = transpose(inverse(mat3(fmodel)));
    // //vec3 normal = normalize(normalMatrix * vec3(mapping, z));

    // Corrección de perspectiva
    vec4 clipPos = proj * view * (fpos + radius * vec4(normal, 1.0)); 
    float depth = clipPos.z / clipPos.w;

    // Proyección NDC
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;

    return normal;
}

void main()
{
    // Velocity color
    const vec4 blue = vec4(0.0, 0.58, 1.0, 1.0);
	const vec4 white = vec4(0.95, 0.95, 0.95, 1.0);
	fragColor = mix(blue, white, clamp(fscalar / 3.1, 0, 1));

    // Pressure color
    // const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	// const vec4 white = vec4(0.95, 0.95, 0.95, 1.0);
	// fragColor = mix(white, red, clamp(fscalar / 10000.0, 0, 1));

    if (lighting == 1)
	{
		vec3 normal = computeDepthAndNormal();
        fragColor = computeLighting(normal) * fragColor;
	}

    if (texturing == 1)
    {
        fragColor = texture2D(colorText, ftextcoord) * fragColor;
    }
}        


