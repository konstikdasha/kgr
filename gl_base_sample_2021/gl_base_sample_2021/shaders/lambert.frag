#version 330 core

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    int light;
    float shininess;
}; 

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;
in vec3 skyBoxTex;
in mat3 TBN;

out vec4 outColor;

#define NR_DIR_LIGHTS 1 
uniform DirLight dirLight[NR_DIR_LIGHTS];


#define NR_POINT_LIGHTS 6 
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform Material material;
uniform vec3 camPos;
uniform SpotLight spotLight;
uniform samplerCube skybox;
uniform sampler2D normalMap;
uniform sampler2D hdr;
uniform bool useNormalMap;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    //vec3 lightDir = normalize(tangentLightPos - tangentFragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    // vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, vTexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, vTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vTexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 tangentFragPos)
{
    vec3 lightDir;
    if (useNormalMap){
        vec3 tangentLightPos = TBN * light.position;

        vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    }
    else {
        lightDir = normalize(light.position - fragPos);
    }
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);

    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance1 = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance1 + light.quadratic * (distance1 * distance1));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vTexCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);

}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance1 = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance1 + light.quadratic * (distance1 * distance1));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vTexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main()
{

   if (material.light == 1) {
        outColor = vec4(1.0); 
   }
   else if (material.light == 0){
       vec3 normal = texture(normalMap, vTexCoords).rgb;
       normal = normalize(normal * 2.0 - 1.0);

       vec3 tangentViewPos  = TBN * camPos;
       vec3 tangentFragPos  = TBN * vFragPosition;

       vec3 viewDir = normalize(tangentViewPos - tangentFragPos);

       vec3 color = vec3(0.0f,0.0f,0.0f);
       for (int i = 0; i < NR_DIR_LIGHTS; i++){
           color += CalcDirLight(dirLight[i], normal, viewDir);
       }

      //color += CalcSpotLight(spotLight, normal, vFragPosition, viewDir); 

       for (int i = 0; i < NR_POINT_LIGHTS; i++){
            if (useNormalMap){
                color += CalcPointLight(pointLights[i], normal, vFragPosition, viewDir, tangentFragPos);
            }
            else {
                color += CalcPointLight(pointLights[i], vNormal, vFragPosition, viewDir, tangentFragPos);
            }
       }

       vec3 hdrColor = texture(material.diffuse, vTexCoords).rgb;
       vec3 color1 = hdrColor / (hdrColor + vec3(1.0));
       //outColor = vec4(color, 1.0f)/(1+vec4(color, 1.0f));
       outColor = vec4(color, 1.0f);
   }
   else {
        vec3 color = texture(skybox, skyBoxTex).rgb;
        outColor = vec4(color, 1.0f)/(1+vec4(color, 1.0f));
   }

}