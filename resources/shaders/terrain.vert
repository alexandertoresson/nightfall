varying vec3 N, L;
varying float flight;
attribute float light;
uniform float baseX, baseY;

void main(void)
{
	gl_Position = ftransform();

	N = normalize(gl_NormalMatrix * gl_Normal);
	L = vec3(normalize(gl_LightSource[0].position));
	flight = light;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[0].s += baseX;
	gl_TexCoord[0].t += baseY;
}
