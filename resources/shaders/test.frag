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
	vec4 ambient = gl_FrontLightProduct[0].ambient;
	float NdotH = max(0.0, dot(N_norm, H));
	vec4 specular = vec4(0.0);
	if (NdotH > 0.0)
		specular = vec4(vec3(pow(NdotH, gl_FrontMaterial.shininess)), 1.0) * gl_FrontLightProduct[0].specular;

	vec4 main = texture2D(mainTexture, gl_TexCoord[0].st);
	vec4 pmask = texture2D(colourTexture, gl_TexCoord[1].st);

	float total = pmask.r + pmask.g + pmask.b + pmask.a;
	vec4 color = ((main * pmask.a + playerColour0 * pmask.r + playerColour1 * pmask.g + playerColour2 * pmask.b) / vec4(total) * vec4(ambient + diffuse) + vec4(specular));
	gl_FragColor = vec4(color.rgb * vec3(materialModifier), color.a);
}
