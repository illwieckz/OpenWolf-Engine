/*[Vertex]*/
attribute vec3  attr_Position;
attribute vec3  attr_Normal;
attribute vec4  attr_TexCoord0;

//#if defined(USE_VERTEX_ANIMATION)
attribute vec3  attr_Position2;
attribute vec3  attr_Normal2;
//#endif

//#if defined(USE_DEFORM_VERTEXES)
uniform int     u_DeformGen;
uniform float    u_DeformParams[5];
//#endif

uniform float   u_Time;
uniform mat4    u_ModelViewProjectionMatrix;

uniform mat4   u_ModelMatrix;

//#if defined(USE_VERTEX_ANIMATION)
uniform float   u_VertexLerp;
//#endif

varying vec3    var_Position;

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

void main()
{
	vec3 position  = mix(attr_Position, attr_Position2, u_VertexLerp);
	vec3 normal    = mix(attr_Normal,   attr_Normal2,   u_VertexLerp);

	position = DeformPosition(position, normal, attr_TexCoord0.st);

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);
	
	var_Position  = (u_ModelMatrix * vec4(position, 1.0)).xyz;
}

/*[Fragment]*/
uniform vec4  u_LightOrigin;
uniform float u_LightRadius;

varying vec3  var_Position;

void main()
{
#if defined(USE_DEPTH)
	float depth = length(u_LightOrigin.xyz - var_Position) / u_LightRadius;
 #if 0
	// 32 bit precision
	const vec4 bitSh = vec4( 256 * 256 * 256,   256 * 256,         256,           1);
	const vec4 bitMsk = vec4(              0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
	
	vec4 comp;
	comp = depth * bitSh;
	comp.xyz = fract(comp.xyz);
	comp -= comp.xxyz * bitMsk;
	gl_FragColor = comp;
 #endif

 #if 1
	// 24 bit precision
	const vec3 bitSh = vec3( 256 * 256,         256,           1);
	const vec3 bitMsk = vec3(        0, 1.0 / 256.0, 1.0 / 256.0);
	
	vec3 comp;
	comp = depth * bitSh;
	comp.xy = fract(comp.xy);
	comp -= comp.xxy * bitMsk;
	gl_FragColor = vec4(comp, 1.0);
 #endif

 #if 0
	// 8 bit precision
	gl_FragColor = vec4(depth, depth, depth, 1);
 #endif
#else
	gl_FragColor = vec4(0, 0, 0, 1);
#endif
}
