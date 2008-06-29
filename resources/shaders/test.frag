uniform vec4 playerColour0, playerColour1, playerColour2;
uniform float materialModifier;
uniform sampler2D mainTexture, colourTexture;
varying vec3 N, L;

void main(void)
{
	vec3 N_norm = normalize(N);
	vec3 H = normalize(L + vec3(0.0, 0.0, 1.0));
	
	float NdotL = max(0.0, dot(N_norm, L));
	vec4 diffuse = vec4(vec3(NdotL), 1.0) * gl_FrontLightProduct[0].diffuse;
	vec4 ambient = vec4(vec3(gl_FrontLightProduct[0].ambient), 1.0);
	float NdotH = max(0.0, dot(N_norm, H));
	vec4 specular = vec4(0.0);
	if (NdotH > 0.0)
		specular = vec4(vec3(pow(NdotH, gl_FrontMaterial.shininess)), 1.0) * gl_FrontLightProduct[0].specular;

	vec4 main = texture2D(mainTexture, gl_TexCoord[0].st);
	vec4 pmask = texture2D(colourTexture, gl_TexCoord[1].st);
	float total = pmask.r + pmask.g + pmask.b + pmask.a;
	vec4 color = ((main * vec4(pmask.a) + playerColour0 * vec4(pmask.r) + playerColour1 * vec4(pmask.g) + playerColour2 * vec4(pmask.b)) / vec4(total) * (ambient + diffuse) + specular);
	gl_FragColor = vec4(color.rgb * vec3(materialModifier), color.a);
}
