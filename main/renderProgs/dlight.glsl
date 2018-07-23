/*[Vertex]*/
attribute vec3 attr_Position;
attribute vec2 attr_TexCoord0;
attribute vec3 attr_Normal;

uniform vec4   u_DlightInfo;

#if 0
#if defined(USE_DEFORM_VERTEXES)
uniform int    u_DeformGen;
uniform float  u_DeformParams[5];
uniform float  u_Time;
#endif
#endif

uniform mat4   u_ModelViewProjectionMatrix;

uniform vec4   u_LightColor;
uniform vec4   u_LightColor1;

uniform vec4   u_LightOrigin;
uniform vec4   u_LightOrigin1;

#define			MAX_VARYING_LIGHTS 2

vec4			LightOrigins[MAX_VARYING_LIGHTS];

varying vec3	var_Normal;
varying vec3	var_Position;
varying vec2	var_Tex1[MAX_VARYING_LIGHTS];
varying vec4	var_LightColor[MAX_VARYING_LIGHTS];
varying vec3	var_LightDir[MAX_VARYING_LIGHTS];

#if 0
#if defined(USE_DEFORM_VERTEXES)
vec3 DeformPosition(const vec3 pos, const vec3 normal, const vec2 st)
{
	if (u_DeformGen == 0)
	{
		return pos;
	}

	float base =      u_DeformParams[0];
	float amplitude = u_DeformParams[1];
	float phase =     u_DeformParams[2];
	float frequency = u_DeformParams[3];
	float spread =    u_DeformParams[4];

	if (u_DeformGen == DGEN_BULGE)
	{
		phase *= st.x;
	}
	else // if (u_DeformGen <= DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		phase += dot(pos.xyz, vec3(spread));
	}

	float value = phase + (u_Time * frequency);
	float func;

	if (u_DeformGen == DGEN_WAVE_SIN)
	{
		func = sin(value * 2.0 * M_PI);
	}
	else if (u_DeformGen == DGEN_WAVE_SQUARE)
	{
		func = sign(0.5 - fract(value));
	}
	else if (u_DeformGen == DGEN_WAVE_TRIANGLE)
	{
		func = abs(fract(value + 0.75) - 0.5) * 4.0 - 1.0;
	}
	else if (u_DeformGen == DGEN_WAVE_SAWTOOTH)
	{
		func = fract(value);
	}
	else if (u_DeformGen == DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		func = (1.0 - fract(value));
	}
	else // if (u_DeformGen == DGEN_BULGE)
	{
		func = sin(value);
	}

	return pos + normal * (base + func * amplitude);
}
#endif
#endif

void main()
{
	vec3 position = attr_Position;
	vec3 normal = attr_Normal * 2.0 - vec3(1.0);
	const int maxVaryingLights = MAX_VARYING_LIGHTS;

#if 0
#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition(position, normal, attr_TexCoord0.st);
#endif
#endif

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);

	// Calculate the normal (N)
	var_Normal = normalize(gl_NormalMatrix * gl_Normal);

	// Transform the vertex position to eye space (V)
	var_Position = vec3(u_ModelViewProjectionMatrix * vec4(position, 1.0));

	LightOrigins[0] = u_LightOrigin;
	LightOrigins[1] = u_LightOrigin1;

	var_LightColor[0] = u_LightColor;
	var_LightColor[1] = u_LightColor1;
	
	for (int light; light < maxVaryingLights; light++)
	{
		if (var_LightColor[light].x == 0.0 && var_LightColor[light].y == 0.0 && var_LightColor[light].z == 0.0)
		{
			break; // We found the last one...
		}

		vec3 dist = LightOrigins[light].xyz - position;

		var_Tex1[light] = dist.xy * LightOrigins[light].a + vec2(0.5);

		float dlightmod = step(0.0, dot(dist, normal));
		dlightmod *= clamp(2.0 * (1.0 - abs(dist.x + dist.y + dist.z) * LightOrigins[light].a), 0.0, 1.0);
	
		var_LightColor[light] = (var_LightColor[light] * dlightmod);
		var_LightDir[light] = dist;
	}
}

/*[Fragment]*/
uniform sampler2D u_DiffuseMap;

#define				MAX_VARYING_LIGHTS 2

varying vec3		var_Normal;
varying vec3		var_Position;
varying vec2		var_Tex1[MAX_VARYING_LIGHTS];
varying vec4		var_LightColor[MAX_VARYING_LIGHTS];
varying vec3		var_LightDir[MAX_VARYING_LIGHTS];

void main (void)
{
	vec3 normal = var_Normal;
	vec3 eyeVec = var_Position;
	const int maxVaryingLights = MAX_VARYING_LIGHTS;
	vec4 out_color = vec4(0.0);

	for (int light = 0; light < maxVaryingLights; light++)
	{
		float shininess = 2.0;
		vec4 tex_color = texture2D(u_DiffuseMap, var_Tex1[light]);
		vec4 tex_ambient = tex_color * 0.33333;
		vec4 tex_diffuse = tex_ambient;
		vec4 tex_specular = tex_ambient;

		vec4 final_color = tex_ambient;// + (lightColor * tex_ambient);

		vec3 lightDir = var_LightDir[light];
		vec4 lightColor = var_LightColor[light]; //*0.33333;
							
		vec3 N = normalize(normal);
		vec3 L = normalize(lightDir);
	
		float lambertTerm = dot(N,L);
	
		if(lambertTerm > 0.0)
		{
			final_color += lightColor * tex_diffuse * lambertTerm;	
		
			vec3 E = normalize(eyeVec);
			vec3 R = reflect(-L, N);
			float specular = pow( max(dot(R, E), 0.0), shininess );
			final_color += lightColor * tex_specular * specular;	
		}

		out_color += final_color;
	}

	gl_FragColor = out_color;			
}
