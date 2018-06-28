/*[Vertex]*/
attribute vec3 attr_Position;
attribute vec4 attr_TexCoord0;
attribute vec3 attr_Normal;

uniform vec4   u_DlightInfo;

#if defined(USE_DEFORM_VERTEXES)
uniform int    u_DeformGen;
uniform float  u_DeformParams[5];
uniform float  u_Time;
#endif

uniform mat4   u_ModelViewProjectionMatrix;

uniform vec4   u_LightColor;
uniform vec4   u_LightOrigin;
uniform float   u_LightRadius;

varying vec2   var_Tex1;
varying vec3   var_Normal;
varying vec3   var_Position;
varying vec4	var_LightColor;
varying vec4	var_LightOrigin;
varying float   var_LightRadius;

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

void main()
{
	vec3 position = attr_Position;
	vec3 normal = attr_Normal;

#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition(position, normal, attr_TexCoord0.st);
#endif

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);

	// Calculate the normal (N)
	var_Normal = normalize(gl_NormalMatrix * gl_Normal);

	// Transform the vertex position to eye space (V)
	var_Position = vec3(u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0));
		
	vec3 dist = u_LightOrigin.xyz - position;

	var_Tex1 = dist.xy * u_LightOrigin.a + vec2(0.5);
	float dlightmod = step(0.0, dot(dist, normal));
	dlightmod *= clamp(2.0 * (1.0 - abs(dist.x + dist.y + dist.z) * u_LightOrigin.a), 0.0, 1.0);
	
	var_LightColor = (u_LightColor * dlightmod);
	var_LightOrigin = u_LightOrigin;
	var_LightRadius = u_LightRadius;
}

/*[Fragment]*/
uniform sampler2D u_DiffuseMap;

uniform int       u_AlphaTest;

varying vec2		var_Tex1;
varying vec3		var_Normal;
varying vec3		var_Position;
varying vec4		var_Color;
varying vec4		var_LightColor;
varying vec4		var_LightOrigin;
varying float		var_LightRadius;

vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

float constantAttenuation = 0.0;
float linearAttenuation = 1.0;
float quadraticAttenuation = 2.0;

float calculateAttenuation(in int i, in float dist)
{
    return(1.0 / (constantAttenuation +
                  (linearAttenuation * dist) +
                  (quadraticAttenuation * dist * dist)));
}
vec3 projectOnPlane(in vec3 p, in vec3 pc, in vec3 pn)
{
    float distance = dot(pn, p-pc);
    return p - distance*pn;
}
int sideOfPlane(in vec3 p, in vec3 pc, in vec3 pn){
   if (dot(p-pc,pn)>=0.0) return 1; else return 0;
}
vec3 linePlaneIntersect(in vec3 lp, in vec3 lv, in vec3 pc, in vec3 pn){
   return lp+lv*(dot(pn,pc-lp)/dot(pn,lv));
}
void areaLight(in int i, in vec3 N, in vec3 V, in float shininess)
{
    //vec3 right = normalize(vec3(gl_ModelViewMatrix*gl_LightSource[i].ambient));
	vec3 right = normalize(vec3(gl_ModelViewMatrix*var_LightColor));
    //vec3 pnormal = normalize(gl_LightSource[i].spotDirection);
	//vec3 pnormal = normalize(vec3(1.0, 1.0, 1.0)); // UQ1: omni...
	vec3 pnormal = normalize(N); // UQ1: omni...
    vec3 up = normalize(cross(right,pnormal));

    //width and height of the area light:
    float width = 1.0; 
    float height = 1.0;

    //project onto plane and calculate direction from center to the projection.
    vec3 projection = projectOnPlane(V,vec3(var_LightOrigin.xyz),pnormal);// projection in plane
    vec3 dir = projection-vec3(var_LightOrigin.xyz);

    //calculate distance from area:
    vec2 diagonal = vec2(dot(dir,right),dot(dir,up));
    vec2 nearest2D = vec2(clamp( diagonal.x,-width,width  ),clamp(  diagonal.y,-height,height));
    vec3 nearestPointInside = vec3(var_LightOrigin.xyz)+(right*nearest2D.x+up*nearest2D.y);

    float dist = distance(V,nearestPointInside);//real distance to area rectangle

    vec3 L = normalize(nearestPointInside - V);
    float attenuation = calculateAttenuation(i, dist);

    float nDotL = dot(pnormal,-L);

    if (nDotL > 0.0 && sideOfPlane(V,vec3(var_LightOrigin.xyz),pnormal) == 1) //looking at the plane
    {   
        //shoot a ray to calculate specular:
        vec3 R = reflect(normalize(-V), N);
        vec3 E = linePlaneIntersect(V,R,vec3(var_LightOrigin.xyz),pnormal);

        float specAngle = dot(R,pnormal);
        if (specAngle > 0.0)
		{
			vec3 dirSpec = E-vec3(var_LightOrigin.xyz);
    	    vec2 dirSpec2D = vec2(dot(dirSpec,right),dot(dirSpec,up));
			vec2 nearestSpec2D = vec2(clamp( dirSpec2D.x,-width,width  ),clamp(  dirSpec2D.y,-height,height));
    	    float specFactor = 1.0-clamp(length(nearestSpec2D-dirSpec2D)*shininess,0.0,1.0);
			specular += var_LightColor * attenuation * specFactor * specAngle;   
			//specular.b = 1.0f;
        }
        diffuse  += var_LightColor  * attenuation * nDotL;
		//diffuse.r = 1.0f;

		//ambient  += var_LightColor * attenuation;
    }
}

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_Tex1);

	float alpha = color.a * var_Color.a;
	if (u_AlphaTest == 1)
	{
		if (alpha == 0.0)
			discard;
	}
	else if (u_AlphaTest == 2)
	{
		if (alpha >= 0.5)
			discard;
	}
	else if (u_AlphaTest == 3)
	{
		if (alpha < 0.5)
			discard;
	}
	
	float shininess = 0.5;
	ambient = texture2D(u_DiffuseMap, var_Tex1) * 0.33333;
	diffuse = ambient;
	specular = ambient;

	int lightNum = 0;
	areaLight(lightNum, var_Normal.xyz, var_Position.xyz, shininess);
	gl_FragColor = ambient + (((diffuse * var_LightColor) + (specular * var_LightColor)) / 3.0);
	gl_FragColor.a = 1.0;
}
