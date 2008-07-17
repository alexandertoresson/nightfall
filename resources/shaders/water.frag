uniform sampler2D waterTexture;
varying vec3 N, L;
varying float C;
varying float flight;

void main(void)
{
	vec3 N_norm = normalize(N);
	vec3 H = normalize(L + vec3(0.0, 0.0, 1.0));
	
	float NdotL = max(0.0, dot(N_norm, L));
	vec4 diffuse = vec4(vec3(NdotL), 1.0) * gl_FrontLightProduct[0].diffuse;
	vec4 ambient = vec4(vec3(gl_FrontLightProduct[0].ambient) * vec3(C), 1.0);
	float NdotH = max(0.0, dot(N_norm, H));
	vec4 specular = vec4(0.0);
	if (NdotH > 0.0)
		specular = vec4(vec3(pow(NdotH, gl_FrontMaterial.shininess)), 1.0) * gl_FrontLightProduct[0].specular;

	gl_FragColor = (ambient + diffuse + specular) * vec4(flight, flight, flight, 1.0);
}
