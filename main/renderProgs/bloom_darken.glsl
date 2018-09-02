/*[Vertex]*/
attribute vec3 attr_Position;

uniform vec4 u_Local0;
varying vec4 var_Local0;

attribute vec4 attr_TexCoord0;
varying vec2   var_TexCoords;

uniform mat4   u_ModelViewProjectionMatrix;

void main(void)
{
  gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
  var_Local0 = u_Local0;
  var_TexCoords = attr_TexCoord0.st;
}

/*[Fragment]*/
uniform sampler2D u_DiffuseMap;
varying vec4	var_Local0;
varying vec2	var_TexCoords;
void main(void)
{
  vec3 col = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
  gl_FragColor.rgb = pow( col, var_Local0.xxx );
  gl_FragColor.a	= 1.0;
}
