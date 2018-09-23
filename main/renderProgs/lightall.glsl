/*[Vertex]*/
attribute vec4 attr_TexCoord0;
uniform vec4	u_Local1; // parallaxScale, 0, 0, 0
uniform vec2	u_Dimensions;

varying vec4	var_Local1; // parallaxScale, 0, 0, 0
varying vec2	var_Dimensions;

#if defined(USE_LIGHTMAP) || defined(USE_TCGEN)
attribute vec4 attr_TexCoord1;
#endif
attribute vec4 attr_Color;

attribute vec3 attr_Position;
attribute vec3 attr_Normal;
attribute vec4 attr_Tangent;

#if defined(USE_VERTEX_ANIMATION)
attribute vec3 attr_Position2;
attribute vec3 attr_Normal2;
attribute vec4 attr_Tangent2;
#endif

#if defined(USE_LIGHT) && !defined(USE_LIGHT_VECTOR)
attribute vec3 attr_LightDirection;
#endif

#if defined(USE_DELUXEMAP)
uniform vec4   u_EnableTextures; // x = normal, y = deluxe, z = specular, w = cube
#endif

#if !defined(USE_LIGHT)
uniform vec3   u_ViewOrigin;
varying vec3   var_Normal;
varying vec3   var_ViewDir;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec3   u_ViewOrigin;
#endif

#if defined(USE_TCGEN)
uniform int    u_TCGen0;
uniform vec3   u_TCGen0Vector0;
uniform vec3   u_TCGen0Vector1;
uniform vec3   u_LocalViewOrigin;
#endif

#if defined(USE_TCMOD)
uniform vec4   u_DiffuseTexMatrix;
uniform vec4   u_DiffuseTexOffTurb;
#endif

uniform mat4   u_ModelViewProjectionMatrix;
uniform vec4   u_BaseColor;
uniform vec4   u_VertColor;

#if defined(USE_MODELMATRIX)
uniform mat4   u_ModelMatrix;
#endif

#if defined(USE_VERTEX_ANIMATION)
uniform float  u_VertexLerp;
#endif

#if defined(USE_LIGHT_VECTOR)
uniform vec4   u_LightOrigin;
uniform float  u_LightRadius;
uniform vec3   u_DirectedLight;
uniform vec3   u_AmbientLight;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
uniform vec4  u_PrimaryLightOrigin;
uniform float u_PrimaryLightRadius;
#endif

varying vec4   var_TexCoords;

varying vec4   var_Color;
#if defined(USE_LIGHT_VECTOR) && !defined(USE_FAST_LIGHT)
varying vec4   var_ColorAmbient;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
varying vec4   var_LightDir;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
varying vec4   var_PrimaryLightDir;
#endif

#if defined(USE_TCGEN)
vec2 GenTexCoords(int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1)
{
	vec2 tex = attr_TexCoord0.st;

	if (TCGen == TCGEN_LIGHTMAP)
	{
		tex = attr_TexCoord1.st;
	}
	else if (TCGen == TCGEN_ENVIRONMENT_MAPPED)
	{
		vec3 viewer = normalize(u_LocalViewOrigin - position);
		vec2 ref = reflect(viewer, normal).yz;
		tex.s = ref.x * -0.5 + 0.5;
		tex.t = ref.y *  0.5 + 0.5;
	}
	else if (TCGen == TCGEN_VECTOR)
	{
		tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));
	}

	return tex;
}
#endif

#if defined(USE_TCMOD)
vec2 ModTexCoords(vec2 st, vec3 position, vec4 texMatrix, vec4 offTurb)
{
	float amplitude = offTurb.z;
	float phase = offTurb.w * 2.0 * M_PI;
	vec2 st2;
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);

	vec2 offsetPos = vec2(position.x + position.z, position.y);

	vec2 texOffset = sin(offsetPos * (2.0 * M_PI / 1024.0) + vec2(phase));

	return st2 + texOffset * amplitude;	
}
#endif

float CalcLightAttenuation(float distance, float radius)
{
	float d = pow(distance / radius, 4.0);
	float attenuation = clamp(1.0 - d, 0.0, 1.0);
	attenuation *= attenuation;
	attenuation /= distance * distance + 1.0;
	// don't attenuate directional light
	attenuation = attenuation + float(radius < 1.0);

	return clamp(attenuation, 0.0, 1.0);
}

void main()
{
#if defined(USE_VERTEX_ANIMATION)
	vec3 position  = mix(attr_Position,    attr_Position2,    u_VertexLerp);
	vec3 normal    = mix(attr_Normal,      attr_Normal2,      u_VertexLerp);
  #if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 tangent   = mix(attr_Tangent.xyz, attr_Tangent2.xyz, u_VertexLerp);
  #endif
#else
	vec3 position  = attr_Position;
	vec3 normal    = attr_Normal;
  #if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 tangent   = attr_Tangent.xyz;
  #endif
#endif

#if defined(USE_TCGEN)
	vec2 texCoords = GenTexCoords(u_TCGen0, position, normal, u_TCGen0Vector0, u_TCGen0Vector1);
#else
	vec2 texCoords = attr_TexCoord0.st;
#endif

#if defined(USE_TCMOD)
	var_TexCoords.xy = ModTexCoords(texCoords, position, u_DiffuseTexMatrix, u_DiffuseTexOffTurb);
#else
	var_TexCoords.xy = texCoords;
#endif

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);

#if defined(USE_MODELMATRIX)
	position  = (u_ModelMatrix * vec4(position, 1.0)).xyz;
	normal    = (u_ModelMatrix * vec4(normal,   0.0)).xyz;
  #if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	tangent   = (u_ModelMatrix * vec4(tangent,  0.0)).xyz;
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 bitangent = cross(normal, tangent) * attr_Tangent.w;
#endif

#if defined(USE_LIGHT_VECTOR)
	vec3 L = u_LightOrigin.xyz - (position * u_LightOrigin.w);
#elif defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 L = attr_LightDirection;
  #if defined(USE_MODELMATRIX)
	L = (u_ModelMatrix * vec4(L, 0.0)).xyz;
	L += normal * 0.00001;
  #endif
#endif

#if defined(USE_LIGHTMAP)
	var_TexCoords.zw = attr_TexCoord1.st;
#endif

	var_Color = u_VertColor * attr_Color + u_BaseColor;

#if defined(USE_LIGHT_VECTOR)
  #if defined(USE_FAST_LIGHT)
	float sqrLightDist = dot(L, L);
	float NL = clamp(dot(normalize(normal), L) / sqrt(sqrLightDist), 0.0, 1.0);
	float attenuation = CalcLightAttenuation(u_LightOrigin.w, u_LightRadius * u_LightRadius / sqrLightDist);

	var_Color.rgb *= u_DirectedLight * (attenuation * NL) + u_AmbientLight;
  #else
	var_ColorAmbient.rgb = u_AmbientLight * var_Color.rgb;
	var_Color.rgb *= u_DirectedLight;
    #if defined(USE_PBR)
	var_ColorAmbient.rgb *= var_ColorAmbient.rgb;
    #endif
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT) && defined(USE_PBR)
	var_Color.rgb *= var_Color.rgb;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
	var_PrimaryLightDir.xyz = u_PrimaryLightOrigin.xyz - (position * u_PrimaryLightOrigin.w);
	var_PrimaryLightDir.w = u_PrimaryLightRadius * u_PrimaryLightRadius;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
  #if defined(USE_LIGHT_VECTOR)
	var_LightDir = vec4(L, u_LightRadius * u_LightRadius);
  #else
	var_LightDir = vec4(L, 0.0);
  #endif
  #if defined(USE_DELUXEMAP)
	var_LightDir -= u_EnableTextures.y * var_LightDir;
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 viewDir = u_ViewOrigin - position;
	// store view direction in tangent space to save on varyings
	var_Normal    = vec4(normal,    viewDir.x);
	var_Tangent   = vec4(tangent,   viewDir.y);
	var_Bitangent = vec4(bitangent, viewDir.z);
#endif

#if !defined(USE_LIGHT)
	vec3 viewDir = u_ViewOrigin - position;
	var_Normal = normal;
	var_ViewDir = viewDir;
#endif

	var_Local1 = u_Local1;
	var_Dimensions = u_Dimensions;
}

/*[Fragment]*/
uniform sampler2D u_DiffuseMap;
uniform vec2	u_Dimensions;
varying vec4	var_Local1; // surfaceType, 0, 0, 0
varying vec2	var_Dimensions;
uniform vec4	u_Local1;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;
#endif

//#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
//#endif

#if defined(USE_DELUXEMAP)
uniform sampler2D u_DeluxeMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

#if defined(USE_CUBEMAP)
#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube u_CubeMap;
#endif

#if defined(USE_NORMALMAP) || defined(USE_DELUXEMAP) || defined(USE_SPECULARMAP) || defined(USE_CUBEMAP)
// y = deluxe, w = cube
uniform vec4      u_EnableTextures; 
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;
#endif

#if !defined(USE_LIGHT)
uniform vec4   u_NormalScale;
varying vec3   var_Normal;
#endif

varying vec3   var_ViewDir;

varying vec3 var_N;

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4      u_NormalScale;
uniform vec4      u_SpecularScale;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
#if defined(USE_CUBEMAP)
uniform vec4      u_CubeMapInfo;
#endif
#endif

uniform int       u_AlphaTest;

varying vec4      var_TexCoords;

varying vec4      var_Color;
#if (defined(USE_LIGHT) && !defined(USE_FAST_LIGHT))
varying vec4      var_ColorAmbient;
#endif

#if (defined(USE_LIGHT) && !defined(USE_FAST_LIGHT))
varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
varying vec4      var_LightDir;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
varying vec4      var_PrimaryLightDir;
#endif


#define EPSILON 0.00000001

#if defined(USE_PARALLAXMAP) || defined(USE_PARALLAXMAP_NONORMALS)
  #if defined(USE_PARALLAXMAP)
	float SampleDepth(sampler2D normalMap, vec2 t)
	{
		#if defined(SWIZZLE_NORMALMAP)
			return 1.0 - texture2D(normalMap, t).r;
		#else
			return 1.0 - texture2D(normalMap, t).a;
		#endif
	}
  #endif //USE_PARALLAXMAP

  #if defined(USE_PARALLAXMAP_NONORMALS)
	float SampleDepth(sampler2D normalMap, vec2 t)
	{
		vec3 color = texture2D(u_DiffuseMap, t).rgb;

#define const_1 ( 16.0 / 255.0)
#define const_2 (255.0 / 219.0)
		vec3 color2 = ((color - const_1) * const_2);
#define const_3 ( 125.0 / 255.0)
#define const_4 (255.0 / 115.0)
		color = ((color - const_3) * const_4);

		color = clamp(color * color * (color * 5.0), 0.0, 1.0); // 1st half "color * color" darkens, 2nd half "* color * 5.0" increases the mids...

		vec3 orig_color = color + color2;

		orig_color = clamp(orig_color * 2.5, 0.0, 1.0); // Lightens the new mixed version...

		float combined_color2 = orig_color.r + orig_color.g + orig_color.b;
		combined_color2 /= 4.0; // Darkens the whole thing a litttle...

		// Returns inverse of the height. Result is mostly around 1.0 (so we don't stand on a surface far below us), with deep dark areas (cracks, edges, etc)...
		float height = clamp(1.0 - combined_color2, 0.0, 1.0);
		return height;
	}
  #endif


float RayIntersectDisplaceMap(vec2 dp, vec2 ds, sampler2D normalMap)
{
	if (u_Local1.x == 0.0)
		return 0.0;
	
  #if !defined(FAST_PARALLAX)
	float MAX_SIZE = u_Local1.x / 3.0;//1.25;//1.5;//1.0;
	if (MAX_SIZE > 1.75) MAX_SIZE = 1.75;
	if (MAX_SIZE < 1.0) MAX_SIZE = 1.0;
	const int linearSearchSteps = 16;
	const int binarySearchSteps = 6;

	// current size of search window
	float size = MAX_SIZE / float(linearSearchSteps);

	// current depth position
	float depth = 0.0;

	// best match found (starts with last position 1.0)
	float bestDepth = MAX_SIZE;

#if 1

	// texture depth at best depth
	float texDepth = 0.0;

	float prevT = SampleDepth(normalMap, dp);
	float prevTexDepth = prevT;

	// search front to back for first point inside object
	for(int i = 0; i < linearSearchSteps - 1; ++i)
	{
		depth += size;
		
		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		//if(bestDepth > 0.996)		// if no depth found yet
		if(bestDepth > MAX_SIZE - (MAX_SIZE / linearSearchSteps))		// if no depth found yet
			if(depth >= t)
			{
				bestDepth = depth;	// store best depth
				texDepth = t;
				prevTexDepth = prevT;
			}
		prevT = t;
	}
#else
	bestDepth = MAX_SIZE;
#endif

	depth = bestDepth;
	
#if !defined (USE_RELIEFMAP)
	float div = 1.0 / (1.0 + (prevTexDepth - texDepth) * float(linearSearchSteps));
	bestDepth -= (depth - size - prevTexDepth) * div;
#else
	// recurse around first point (depth) for closest match
	for(int i = 0; i < binarySearchSteps; ++i)
	{
		size *= 0.5;

		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		if(depth >= t)
		{
			bestDepth = depth;
			depth -= 2.0 * size;
		}

		depth += size;
	}
#endif

	return bestDepth * u_Local1.x;
#else //FAST_PARALLAX
	float depth = SampleDepth(normalMap, dp) - 1.0;
	return depth * u_Local1.x;
#endif //FAST_PARALLAX
}
#endif //USE_PARALLAXMAP || USE_PARALLAXMAP_NONORMALS

vec3 CalcDiffuse(vec3 diffuseAlbedo, float NH, float EH, float roughness)
{
#if defined(USE_BURLEY)
	// modified from https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
	float fd90 = -0.5 + EH * EH * roughness;
	float burley = 1.0 + fd90 * 0.04 / NH;
	burley *= burley;
	return diffuseAlbedo * burley;
#else
	return diffuseAlbedo;
#endif
}

vec2 GetParallaxOffset(in vec2 texCoords, in vec3 E, in mat3 tangentToWorld )
{
#if defined(USE_PARALLAXMAP)
	vec3 offsetDir = normalize(E * tangentToWorld);
	offsetDir.xy *= -u_NormalScale.a / offsetDir.z;

	return offsetDir.xy * RayIntersectDisplaceMap(texCoords, offsetDir.xy, u_NormalMap);
#else
	return vec2(0.0);
#endif
}

vec3 EnvironmentBRDF(float roughness, float NE, vec3 specular)
{
	// from http://community.arm.com/servlet/JiveServlet/download/96891546-19496/siggraph2015-mmg-renaldas-slides.pdf
	float v = 1.0 - max(roughness, NE);
	v *= v * v;
	return vec3(v) + specular;
}

float spec_D( float NH, float roughness)
{
  // normal distribution
  // from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
  float alpha = roughness * roughness;
  float quotient = alpha / max(1e-8,(NH*NH*(alpha*alpha-1.0)+1.0));
  return (quotient * quotient) / M_PI;
}

vec3 spec_F( float EH, vec3 F0 )
{
  // Fresnel
  // from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
  float pow2 = pow(2.0, (-5.55473*EH - 6.98316) * EH);
  return F0 + (vec3(1.0) - F0) * pow2;
}

float G1( float NV, float k )
{
  return NV / (NV*(1.0-k) +  k);
}

float spec_G(float NL, float NE, float roughness )
{
  // GXX Schlick
  // from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
  float k = max(((roughness + 1.0) * (roughness + 1.0)) / 8.0, 1e-5);
  return G1(NL,k)*G1(NE,k);
}

vec3 CalcSpecular( in vec3 specular, in float NH, in float NL, in float NE, in float EH, in float roughness )
{
	float distrib = spec_D(NH,roughness);
	vec3 fresnel = spec_F(EH,specular);
	float vis = spec_G(NL, NE, roughness);
	return (distrib * fresnel * vis);
}

float CalcLightAttenuation(float distance, float radius)
{
	float d = pow(distance / radius, 4.0);
	float attenuation = clamp(1.0 - d, 0.0, 1.0);
	attenuation *= attenuation;
	attenuation /= distance * distance + 1.0;
	// don't attenuate directional light
	attenuation = attenuation + float(radius < 1.0);

	return clamp(attenuation, 0.0, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	vec2 tex_offset = vec2(1.0 / u_Dimensions.x, 1.0 / u_Dimensions.y);
    // number of depth layers
	float height_scale = u_Local1.x;
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale * tex_offset; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture2D(u_NormalMap, currentTexCoords).a;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture2D(u_NormalMap, currentTexCoords).a;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture2D(u_NormalMap, prevTexCoords).a - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

//---------------------------------------------------------
// get pseudo 3d bump background
//---------------------------------------------------------
vec4 BumpyBackground (sampler2D texture, vec2 pos)
{
  #define LINEAR_STEPS 20
  #define DISTANCE 0.16
  #define FEATURES 0.5

  vec4 color = vec4(0.0);
  vec2 dir = -(vec2(pos - vec2(0.5, 0.5)) * (DISTANCE / LINEAR_STEPS)) * 0.5;
    
  for (float i = 0.0; i < LINEAR_STEPS; i++) 
  {
    vec4 pixel1 = texture2D(texture, pos - i * dir);
    if (pow(length(pixel1.rgb) / 1.4, 0.20) * (1.0 - FEATURES)
       +pow(length(texture2D(texture, (pos - i * dir) * 2.0).rgb) / 1.4, 0.90) * FEATURES
       > i / LINEAR_STEPS) 
    //color = pixel1 * i / LINEAR_STEPS;
    color += 0.16 * pixel1 * i / LINEAR_STEPS;
  }
  return color;
}

void main()
{
	vec3 viewDir, lightColor, ambientColor, reflectance;
	vec3 L, N, E, H;
	float NL, NH, NE, EH, attenuation;
	vec2 tex_offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, var_Normal.xyz);
	viewDir = vec3(var_Normal.w, var_Tangent.w, var_Bitangent.w);
	E = normalize(viewDir);
#endif

	lightColor = var_Color.rgb;

#if defined(USE_LIGHTMAP)
	vec4 lightmapColor = texture2D(u_LightMap, var_TexCoords.zw);
  #if defined(RGBM_LIGHTMAP)
	lightmapColor.rgb *= lightmapColor.a;
  #endif
  #if defined(USE_PBR) && !defined(USE_FAST_LIGHT)
	lightmapColor.rgb *= lightmapColor.rgb;
  #endif
	lightColor *= lightmapColor.rgb;
#endif

	vec2 texCoords = var_TexCoords.xy;
	vec4 diffuse = BumpyBackground(u_DiffuseMap, texCoords);;

#if defined(USE_PARALLAXMAP) || defined(USE_PARALLAXMAP_NONORMALS)
	vec3 offsetDir = normalize(E * tangentToWorld);
	texCoords = ParallaxMapping(var_TexCoords.xy, offsetDir);

	diffuse = texture2D(u_DiffuseMap, texCoords);
#else
	diffuse = texture2D(u_DiffuseMap, texCoords);
#endif

	
	float alpha = diffuse.a * var_Color.a;
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


#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	L = var_LightDir.xyz;
  #if defined(USE_DELUXEMAP)
	L += (texture2D(u_DeluxeMap, var_TexCoords.zw).xyz - vec3(0.5)) * u_EnableTextures.y;
  #endif
	float sqrLightDist = dot(L, L);

  #if defined(USE_LIGHT_VECTOR)
	attenuation  = CalcLightAttenuation(float(var_LightDir.w > 0.0), var_LightDir.w / sqrLightDist);
  #else
	attenuation  = 1.0;
  #endif

  #if defined(USE_NORMALMAP)
    #if defined(SWIZZLE_NORMALMAP)
	N.xy = texture2D(u_NormalMap, texCoords).ag - vec2(0.5);
    #else
	N.xy = texture2D(u_NormalMap, texCoords).rg - vec2(0.5);
    #endif
	N.xy *= u_NormalScale.xy;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
	N = tangentToWorld * N;
  #else
	N = var_Normal.xyz;
  #endif

	N = normalize(N);
	L /= sqrt(sqrLightDist);

  #if defined(USE_SHADOWMAP) 
	vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
	float shadowValue = texture2D(u_ShadowMap, shadowTex).r;

	// surfaces not facing the light are always shadowed
	shadowValue *= float(dot(var_Normal.xyz, var_PrimaryLightDir.xyz) > 0.0);

    #if defined(SHADOWMAP_MODULATE)
	//vec3 shadowColor = min(u_PrimaryLightAmbient, lightColor);
	vec3 shadowColor = u_PrimaryLightAmbient * lightColor;

      #if 0
	// Only shadow when the world light is parallel to the primary light
	shadowValue = 1.0 + (shadowValue - 1.0) * clamp(dot(L, var_PrimaryLightDir.xyz), 0.0, 1.0);
      #endif
	lightColor = mix(shadowColor, lightColor, shadowValue);
    #endif
  #endif

  #if defined(USE_LIGHTMAP) || defined(USE_LIGHT_VERTEX)
	ambientColor = lightColor;
	float surfNL = clamp(dot(var_Normal.xyz, L), 0.0, 1.0);
	lightColor /= max(surfNL, 0.25);
	ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
  #endif
	NL = clamp(dot(N, L), 0.0, 1.0);
	NE = clamp(dot(N, E), 0.0, 1.0);
	H = normalize(L + E);
	EH = clamp(dot(E, H), 0.0, 1.0);
	NH = clamp(dot(N, H), 0.0, 1.0);

  #if defined(USE_SPECULARMAP)
	vec4 specular = texture2D(u_SpecularMap, texCoords);
  #else
	vec4 specular = vec4(1.0);
  #endif
	specular *= u_SpecularScale;


  #if defined(USE_PBR)
	// diffuse rgb is base color
	// specular red is gloss
	// specular green is metallicness
	float gloss = specular.r;
	float metal = specular.g;
	specular.rgb = metal * diffuse.rgb + vec3(0.04 - 0.04 * metal);
	diffuse.rgb *= 1.0 - metal;
  #else
	// diffuse rgb is diffuse
	// specular rgb is specular reflectance at normal incidence
	// specular alpha is gloss
	float gloss = 1.0 - min(specular.a, 0.96);

	// adjust diffuse by specular reflectance, to maintain energy conservation
	diffuse.rgb -= specular.rgb * (1.0 - u_EnableTextures.z);
#endif

  #if defined(GLOSS_IS_GLOSS)
	float roughness = exp2(-3.0 * gloss);
  #elif defined(GLOSS_IS_SMOOTHNESS)
	float roughness = 1.0 - gloss;
  #elif defined(GLOSS_IS_ROUGHNESS)
	float roughness = gloss;
  #elif defined(GLOSS_IS_SHININESS)
	float roughness = pow(2.0 / (8190.0 * gloss + 2.0), 0.25);
  #endif

  #if defined(USE_PBR)
	//diffuse.rgb *= CalcDiffuse(diffuse.rgb, NH, EH, roughness);
	//specular.rgb *= CalcSpecular(specular.rgb, NH, NL, NE, EH, roughness);
	diffuse.rgb *= diffuse.rgb;
	specular.rgb *= specular.rgb;
  #endif

    H  = normalize(L + E);
    EH = max(1e-8, dot(E, H));
    NH = max(1e-8, dot(N, H));
    NL = clamp(dot(N, L), 1e-8, 1.0);

    reflectance  = CalcDiffuse(diffuse.rgb, NH, EH, roughness);

	NE = abs(dot(N, E)) + 1e-5;
	reflectance += CalcSpecular(specular.rgb, NH, NL, NE, EH, roughness);

	//gl_FragColor.rgb  = (((lightColor   * reflectance * (attenuation * NL)) * 2.0) + (lightColor   * (reflectance * specular.a) * (attenuation * NL))) / 3.0;
	//gl_FragColor.rgb += ambientColor * (diffuse.rgb + specular.rgb);
	gl_FragColor.rgb  = lightColor   * reflectance * (attenuation * NL);
	gl_FragColor.rgb += ambientColor * diffuse.rgb;

  #if defined(USE_CUBEMAP)
	NE = clamp(dot(N, E), 0.0, 1.0);
	reflectance = EnvironmentBRDF(roughness, NE, specular.rgb);

	vec3 R = reflect(E, N);

	// parallax corrected cubemap (cheaper trick)
	// from http://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
	vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;

	vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, ROUGHNESS_MIPS * roughness).rgb * u_EnableTextures.w;

	// normalize cubemap based on last roughness mip (~diffuse)
	// multiplying cubemap values by lighting below depends on either this or the cubemap being normalized at generation
	vec3 cubeLightDiffuse = max(textureCubeLod(u_CubeMap, N, ROUGHNESS_MIPS).rgb, 0.5 / 255.0);
	cubeLightColor /= dot(cubeLightDiffuse, vec3(0.2126, 0.7152, 0.0722));

	float horiz = 1.0;
	// from http://marmosetco.tumblr.com/post/81245981087
	#if defined(HORIZON_FADE)
		const float horizonFade = HORIZON_FADE;
		horiz = clamp( 1.0 + horizonFade * dot(R,var_Normal.xyz), 0.0, 1.0 );
		horiz = 1.0 - horiz;
		horiz *= horiz;
	#endif

    #if defined(USE_PBR)
	cubeLightColor *= cubeLightColor + cubeLightColor;
    #endif

	// multiply cubemap values by lighting
	// not technically correct, but helps make reflections look less unnatural
	cubeLightColor *= lightColor * (attenuation * NL) + ambientColor;

	gl_FragColor.rgb += cubeLightColor * reflectance * horiz;
  #endif

 #if defined(USE_PRIMARY_LIGHT) || defined(SHADOWMAP_MODULATE)
	vec3 L2, H2;
	float NL2, EH2, NH2;

	L2 = var_PrimaryLightDir.xyz;

	// enable when point lights are supported as primary lights
	sqrLightDist = dot(L2, L2);
	L2 /= sqrt(sqrLightDist);

	H2  = normalize(L2 + E); 
	NL2 = clamp(dot(N, L2), 0.0, 1.0);
	NL2 = max(1e-8, abs(NL2) );
	EH2 = max(1e-8, dot(E, H2));
	NH2 = max(1e-8, dot(N, H2));

	lightColor *= u_PrimaryLightColor * var_Color.rgb;

	reflectance = CalcSpecular(specular.rgb, NH2, NL2, NE, EH2, roughness);
	reflectance += CalcDiffuse(diffuse.rgb, NH2, EH2, roughness);

	//lightColor *= u_PrimaryLightColor * var_Color.rgb;

    #if defined(USE_SHADOWMAP)
	lightColor *= shadowValue;
    #endif

	// enable when point lights are supported as primary lights
	lightColor *= CalcLightAttenuation(float(var_PrimaryLightDir.w > 0.0), var_PrimaryLightDir.w / sqrLightDist);

	gl_FragColor.rgb += lightColor * reflectance * NL2;
  #endif

  #if defined(USE_PBR)
	gl_FragColor.rgb = sqrt(gl_FragColor.rgb);
  #endif

  gl_FragColor.a = diffuse.a * var_Color.a;
  
#else
  lightColor = var_Color.rgb;

  #if defined(USE_LIGHTMAP) 
    lightColor *= lightmapColor.rgb;
  #endif

  gl_FragColor.rgb = diffuse.rgb * lightColor;

  gl_FragColor = vec4 (diffuse.rgb * lightColor, diffuse.a * var_Color.a);
#endif

  gl_FragColor.a = alpha;
}
