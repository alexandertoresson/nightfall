varying vec3 N, L;
varying float flight;
attribute float light;

void main(void)
{
	gl_Position = ftransform();

	N = normalize(gl_NormalMatrix * gl_Normal);
	L = vec3(normalize(gl_LightSource[0].position));
	flight = light;
}
