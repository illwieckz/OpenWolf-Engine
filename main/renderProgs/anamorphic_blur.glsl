/*[Vertex]*/
attribute vec3 attr_Position;

uniform vec4 u_Local0;
uniform vec2 u_Dimensions;

uniform mat4   u_ModelViewProjectionMatrix;

attribute vec4 attr_TexCoord0;

varying vec4 var_Local0;
varying vec2 var_Dimensions;

varying vec2   var_TexCoords;

void main(void)
{
  gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
  var_TexCoords = attr_TexCoord0.st;
  var_Dimensions = u_Dimensions;
  var_Local0 = u_Local0;
}

/*[Fragment]*/
uniform sampler2D u_DiffuseMap;

varying vec4	var_Local0;
varying vec2	var_Dimensions;
varying vec2	var_TexCoords;

void main(void)
{
  gl_FragColor.rgb = vec3(0.0, 0.0, 0.0);

  for (float width = 1.0; width <= var_Local0.z; width += 1.0)
  {
	vec3 col0 = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
	vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((var_Local0.xy * width) / var_Dimensions)).rgb;
	vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((var_Local0.xy * width) / var_Dimensions)).rgb;
	gl_FragColor.rgb += (col0 / 2) + (col1 + col2) / 4;
  }

  gl_FragColor.rgb /= var_Local0.z;
  gl_FragColor.a	= 1.0;
}

