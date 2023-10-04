#version 330
#extension GL_NV_shadow_samplers_cube : enable

struct lightInfo 
{
	vec3 position;
	vec3 color;
	int  type;
	float intensity;
	float linearAtt;

};

uniform int textType; 
uniform int computeReflect;
uniform int computeRefract;
uniform int texturing;
uniform int lighting;
uniform int normalMapping;
uniform int shininess;

uniform sampler2D colorText;
uniform samplerCube cubeText; 
uniform samplerCube reflectText;
uniform samplerCube refractText;
uniform sampler2D normalText; 

uniform lightInfo lights[8];
uniform int numLights;
uniform vec3 ambient;

uniform vec3 eyePos;
uniform vec4 baseColor;

in vec2 ftextcoord;
in vec4 fpos;
in vec4 fnormal;
in vec3 ftextCubeRfl;
in vec3 ftextCubeRfr;
in mat3 TBN; 

out vec4 fragColor;

void main()
{
	// Diferentes combinaciones
	// Iluminado + no text		 -> colorIluminacion * colorBase
	// Iluminado + text			 -> colorIluminacion * colorBase * colorText
	// No iluminado + text		 -> colorBase * colorText
	// No iluminado + no text	 -> colorBase

	// De momento un objeto puede tener:
	//	Su propia textura (Color2D o Color3D) + textura de reflexion + textura de refraccion
	
	vec4 objectColor = baseColor;

	if (texturing == 1)
	{
		if (textType == 0) // Color2D
		{
			objectColor = objectColor * texture2D(colorText, ftextcoord);
		}
		else if (textType == 1) // Color3D (skybox o objetos reflectantes o refractantes)
		{
			objectColor = objectColor * textureCube(cubeText, fpos.xyz);
		}
	}

	if (computeReflect == 1 && computeRefract == 1)
	{
		vec4 refractColor = textureCube(refractText, ftextCubeRfr);
		vec4 reflectColor = textureCube(reflectText, ftextCubeRfl);

		objectColor = objectColor * mix(refractColor, reflectColor, 0.5); // Pseudo-fresnel
	}
	else if (computeReflect == 1)
	{
		objectColor = objectColor * textureCube(reflectText, ftextCubeRfl);
	}
	else if (computeRefract == 1)
	{
		objectColor = objectColor * textureCube(refractText, ftextCubeRfr);
	}

	if (lighting == 1)
	{
		vec3 N;

		if(normalMapping == 0)		// Normal from vertex
		{
			N = normalize(fnormal.xyz);
		}
		else if (normalMapping == 1)	// Normal texture
		{
			vec3 normalTextValue = texture2D(normalText, ftextcoord).xyz;
			normalTextValue = normalTextValue * 2.0f - 1.0f;
			N = normalize(TBN * normalTextValue);
		}

		float diff = 0.0f;
		float spec = 0.0f;	
		vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

		for (int i  = 0; i < numLights; i++)
		{
			vec3 L = vec3(0.0f,0.0f,0.0f);
			float distance = 0;

			if(lights[i].type == 0)
			{
				L = normalize(lights[i].position); // Cuando la luz es direccional position == direction
			}
			else if(lights[i].type == 1)
			{
				L = lights[i].position - fpos.xyz;
				distance = length(L);
				L = normalize(L);
			}

			vec3 R = normalize(reflect(L, N));
			vec3 EYE = normalize(fpos.xyz - eyePos);
			float attenuation = 1.0f / (1.0f + lights[i].linearAtt * distance); 

			diff += attenuation * lights[i].intensity * max( dot(L, N), 0.0f);
			spec += attenuation * lights[i].intensity * pow(max(dot(R, EYE), 0.0f), shininess);

			lightColor *= lights[i].color;
		}

		lightColor = normalize(lightColor);

		vec3 diffuse = diff * lightColor;
		vec3 specular = spec * vec3(1.0f);

		objectColor = vec4(ambient + diffuse + specular, 1.0) * objectColor;
	}

	fragColor = objectColor;
}

