#version 400

uniform vec3 resolution;
uniform vec3 camera_position;

out vec4 colorOut;

float sphereDF(vec3 p, vec3 c, float r)
{
  return length(p - c) - r;
}

float torusDF( vec3 p, vec3 c, vec2 t )
{
  p -= c;
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float boxDF(vec3 p, vec3 b)
{
  return length(max(abs(p)-b,0.0));
}

float mandelboxDF(vec3 p) {
    float Scale = -1.77f, fixedRadius2 = 1.0f, minRadius2 = (0.5f*0.5f);
    vec3 p0 = p;
    float dr = 1.0f;
    for(int n = 0; n < 13; n++) {
        // Reflect
        p = (clamp(p,-1.0f,1.0f) * 2.0f) - p;
        // Sphere Inversion
        float r2 = dot(p,p);
        if(r2<minRadius2) {
            float t = (fixedRadius2/minRadius2);
            p *= t;
            dr *= t;
        } else if(r2<fixedRadius2) {
            float t = (fixedRadius2/r2);
            p *= t;
            dr *= t;
        }
        // Scale & Translate
                p = p * Scale + p0;
                dr = dr * abs(Scale) + 1.0f;
    }
    return length(p)/abs(dr);
}

float map(vec3 p)
{
  float sphere_b = sphereDF(p, vec3(3.0, 0.5, 0.5), 0.8);
  float v = sphere_b;
  float torus = torusDF(p, vec3(-3.0, 0.0, 0.0), vec2(1.0, 0.7));
  v = min(v, torus);
  float mandelbox = mandelboxDF(p);
  v = min(v, mandelbox);
  return v;
}

vec3 normal(vec3 p)
{
  float e = 2e-6f;
  float n = map(p);
  const vec3 small_step = vec3(0.001, 0.0, 0.0);
  float gradient_x = map(p + vec3(e, 0.0, 0.0)) - n;
  float gradient_y = map(p + vec3(0.0, e, 0.0)) - n;
  float gradient_z = map(p + vec3(0.0, 0.0, e)) - n;

  vec3 normal = vec3(gradient_x, gradient_y, gradient_z);
  return normalize(normal);
}

vec3 march(vec3 ray_origin, vec3 ray_direction, vec2 uv)
{
  float total_distance = 0.0;
  const int MAX_STEPS = 100;
  const float MIN_HIT_DISTANCE = 0.001;
  const float MAX_TRACE_DISTANCE = 1000.0;

  for(int i = 0; i < MAX_STEPS; i++)
  {
    vec3 current_position = ray_origin + total_distance * ray_direction;
    float distance_function_result = map(current_position);

    if(distance_function_result < MIN_HIT_DISTANCE)
    {
      vec3 normal = normal(current_position);
      vec3 light_position = vec3(2.0, -5.0, 3.0);
      vec3 direction_to_light = normalize(current_position - light_position);
      float diffuse = max(0.0, dot(normal, direction_to_light));
      return vec3(1.0, 1.0, 1.0) * diffuse;
    }

    if(total_distance > MAX_TRACE_DISTANCE)
    {
      break;
    }

    total_distance += distance_function_result;
  }
  return vec3(sin(uv.x), uv.y * sin(uv.x), uv.y);
}

void main()
{
  vec2 uv = gl_FragCoord.xy / resolution.xy * 2.0 - 1.0;
  uv.x *= resolution.x / resolution.y;

  //vec3 camera_position = vec3(0.0, 0.0, -5.0);
  vec3 ray_origin = camera_position;
  vec3 ray_direction = vec3(uv, 1.0);

  vec3 color = march(ray_origin, ray_direction, uv);

  color = max(color, 0.0);
  color = min(color, 1.0);

  colorOut = vec4(color, 1.0);
}