varying vec3 N, L;
attribute float waterBack, waterFront;
uniform float mixLevels, waterLevel;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.x, mix(waterBack, waterFront, mixLevels) + waterLevel, gl_Vertex.z, 1.0);

	N = normalize(gl_NormalMatrix * gl_Normal);
	L = vec3(normalize(gl_LightSource[0].position));
}
