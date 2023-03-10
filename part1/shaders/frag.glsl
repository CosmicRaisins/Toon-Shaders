#version 330 core

#define MAX_ITER	100
#define MAX_DIST	300.0
#define EPSILON		0.0001
#define EDGE_THICKNESS 0.025

// Take in our previous texture coordinates.
in vec3 FragPos;
in vec2 v_texCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;


// If we have texture coordinates, they are stored in this sampler.
uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;

uniform vec3 lightPos; // Our light source position from where light is hitting this object
uniform vec3 viewPos;  // Where our camera is

uniform int type; // type of shading selected
uniform float blend; // blend ratio of cel and smooth shading
uniform float time;
uniform int shaderIndex;

out vec4 FragColor;


float Sphere( in vec3 p, in float r )
{
    return length(p)-r;
}

float Torus(vec3 pos, vec2 t)
{
    return length( vec2(length(pos.xz)-t.x,pos.y) )-t.y;
}

float SmoothUnion( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

float distFunc(vec3 pos)
{
    float t = 0.7 * sin(time);
    float r = sin(time + 15);
    vec2 torusSize = vec2(2.0, t < 0.4 ? 0.4 : t);
    float sphereRadius = 2.0 * (r < 0.5 ? 0.5 : r);
    float d1 = Torus(vec3(pos.x + sin(time), pos.y + sin(time - 10), pos.z + sin(time - 15)), torusSize);
    float d2 = Sphere(pos,sphereRadius);
    float dt = SmoothUnion(d1, d2, 0.45);
    return dt;
}

float balance(float s, float weight) {
    if (weight < 1.0)
    weight = s + weight;
    return clamp(pow(weight, 5.0), 0.0, 1.0);
}

float circles(float s, float thickness) {
    vec2 pixel = floor(vec2(gl_FragCoord));
    float b = thickness / 2.0;
    if (mod((pixel.y), thickness * 2.0) > thickness)
        pixel.x += b;
    pixel = mod(pixel, vec2(thickness));
    float a = distance(pixel, vec2(b)) / (thickness * 0.38);
    return balance(s, a);
}

float diagonal(float s, float thickness) {
    vec2 pixel = floor(vec2(gl_FragCoord));
    float a = 1.0;
    float b = mod(pixel.x - pixel.y, thickness);
    float c = thickness / 2.0;
    if (b < thickness)
        a = abs(b - c) / c;
    return balance(s, a);
}

void main()
{
    if (shaderIndex == 1) {
        vec3 camOrigin	= vec3(2.0);
        vec3 camTarget	= vec3(0.0);
        vec3 upDir		= vec3(0.0, 1.0, 0.0);

        vec3 camDir		= normalize(camTarget - camOrigin);
        vec3 camRight	= normalize(cross(upDir, camOrigin));
        vec3 camUp		= cross(camDir, camRight);

        vec2 screenPos = -1.0 + 2.0 * gl_FragCoord.xy / vec2(1280.0,720.0);
        screenPos.x *= 1280.0 / 720.0;

        vec3 rayDir = normalize(camRight * screenPos.x + camUp * screenPos.y + camDir) + vec3(sin(time), sin(time * 2.0) / 2, cos(time) + 0.25) / 5 + 0.1;

        float totalDist = 0.0;
        vec3 pos = camOrigin;
        float dist = EPSILON;

        float edgeLegnth = MAX_DIST;

        for (int i = 0; i < MAX_ITER; i++) {
            if ((dist > edgeLegnth && edgeLegnth <= EDGE_THICKNESS) || totalDist > MAX_DIST) {
                break;
            }

            dist = distFunc(pos);
            edgeLegnth = min(dist, edgeLegnth);
            totalDist += dist;
            pos += dist * rayDir;

        }
        vec3 normal = normalize(vec3(
        distFunc(vec3(pos.x + EPSILON, pos.y, pos.z)) - distFunc(vec3(pos.x - EPSILON, pos.y, pos.z)),
        distFunc(vec3(pos.x, pos.y + EPSILON, pos.z)) - distFunc(vec3(pos.x, pos.y - EPSILON, pos.z)),
        distFunc(vec3(pos.x, pos.y, pos.z + EPSILON)) - distFunc(vec3(pos.x, pos.y, pos.z - EPSILON))));
        if (dist < EPSILON) {				//the ray hit the object

            float diffuse = max(0.0, dot(-rayDir, normal));
            float specular = pow(diffuse, 128.0);
            float lighting = diffuse + specular;
            vec3 color;

            //map lighting information to discrete values
            if (lighting < 0.278) {
                color = vec3(0.1,0.17,0.28);
            } else if (lighting < 0.435) {
                color = vec3(0.15,0.33,0.45);
            } else if (lighting < 0.852) {
                color = vec3(0.45,0.74,0.87);
            } else {
                color = vec3(0.95,0.97,1.0);
            }

            color = color * (1 - blend) + color * lighting * blend;
            color = color * (1 + blend / 1.5);

            float rimAmount = 0.84;
            vec3 rimColor = FragColor.rgb + 0.75;
            float rimDot = 1 - dot(-rayDir, normal);

            float rimIntensity = smoothstep(rimAmount - 0.01, rimAmount + 0.01, rimDot);
            vec3 rim = rimIntensity * rimColor;

            // halftone
            vec3 cl = (color + rim * 1.2);
            vec3 e = -rayDir;
            vec3 n = normal;

            float scale = cl.r;
            float hatch = circles(scale, 0.5225);
            if (hatch > 0.65)
            cl = vec3(1.0);
            vec3 halftone = cl / 17.5 * hatch;

            FragColor = vec4(color + rim + halftone, 1.0);
        } else if (dist > edgeLegnth && edgeLegnth <= EDGE_THICKNESS){   // the ray hit the edges
            FragColor = vec4(0.0,0.0,0.0,1.0);
        } else {					// the ray didn't hit anything
            FragColor = vec4(0.9, 0.9, 0.9, 1.0);
        }
    } else {
        //    vec3 lightPos = vec3(2.0f,2.4f,-5.5f);
        //    vec3 viewPos= vec3(0,0,0);  // Where our camera is
        // Store the texture coordinates
        vec3 normal = texture(u_NormalMap, v_texCoord).rgb;
        // transform normal vector to range [-1,1]
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
        vec3 color =  texture(u_DiffuseMap, v_texCoord).rgb;
//        color = (color + vec3(-0.3, 0.2, 0.5)) / 1.75;


        // diffuse
        vec3 lightDir = normalize(TangentLightPos - TangentFragPos);

        float diff = max(dot(lightDir, normal), 0.0);
        //    float NdotL = dot(normal, lightDir);
        float lightIntensity = diff > 0 ? 1 : 0;
        vec3 cel = lightIntensity * color;
        vec3 smoth = diff * color;
        vec3 diffuse = cel * (1.0 - blend) + smoth * blend;

        // ambient
        vec3 ambient = lightIntensity > 0 ? 0.65 * diffuse : 0.85 * color;

        // specular
        vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        //    float specIntensity = smoothstep(0.005, 0.01, spec);
        float specIntensity = spec > 0.75 ? 0.75 : 0;
        vec3 specColor = diffuse;
        vec3 specular = specColor * specIntensity;



        // shadow light
        float rimDot = 1 - dot(viewDir, normal);
        float thick = 0.85;
        vec3 lineColor = (color + vec3(-0.15, -0.1, 0.35)) / 2;

        //    float lineIntensity = smoothstep(thick - 0.01, thick + 0.01, rimDot);
        float lineIntensity = smoothstep(thick - 0.075, thick + 0.01, rimDot);
        vec3 line = lineIntensity * lineColor;

        // rim light
        float rimAmount = 0.72;
        vec3 rimColor = diffuse;
        float rimThreshold = 0.125;

        float rimIntensity = rimDot * pow(dot(lightDir, normal), rimThreshold);
        rimIntensity = smoothstep(rimAmount - 0.05, rimAmount + 0.05, rimIntensity);
        vec3 rim = rimIntensity * rimColor;
        if (rim[0] > 0 && rim[1] > 0 && rim[2] > 0 && type >= 4 && type <= 6) {
            rim = rim - line;
        }

        // halftone
        vec3 halftoneColorSample = (diffuse / 1.5 + ambient / 2.5 + specular * 1.2 + rim * 1.2 + line / 1.5) * 1.9;
        float scale = (halftoneColorSample.r + halftoneColorSample.g + halftoneColorSample.b) / 3;
        float hatch = diagonal(scale, 0.52);
        halftoneColorSample = vec3(1.0);
        vec3 halftone = halftoneColorSample / 17.5 * hatch;

        if (type == 1){
            FragColor = vec4(ambient + diffuse, 1.0);
        }

        if (type == 2) {
            FragColor = vec4(ambient + diffuse + specular, 1.0);
        }

        if (type == 3) {
            FragColor = vec4(ambient + diffuse + specular + rim, 1.0);
        }

        if (type == 4) {
            FragColor = vec4(ambient + diffuse + specular + rim + line, 1.0);
        }

        if (type == 5) {
            FragColor = vec4(ambient + diffuse + specular + rim + line + halftone, 1.0);
        }

        if (type == 6) {
            FragColor = vec4(ambient + diffuse + halftone, 1.0);
        }

        if (type == 7) {
            FragColor = vec4(ambient + diffuse + specular + halftone, 1.0);
        }
    }
}