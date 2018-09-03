#version 400

uniform vec3 resolution;
uniform vec3 center;
uniform vec3 eye;
uniform vec3 up;

uniform int selection;
uniform int max_steps;
uniform vec3 background_color;

uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color_base;
uniform float base_color_strength;
uniform float dist0to1;
uniform float dist1to2;
uniform float dist2to3;
uniform float dist3to0;
uniform float cycle_intensity;
uniform float palette_offset;
uniform vec4 orbit_strength;

uniform float shine;
uniform float ambient_intensity;
uniform float diffuse_intensity;
uniform float specular_intensity;
uniform int gamma_correction;

uniform int mandelbulb_iterations;
uniform float mandelbulb_bailout;
uniform float mandelbulb_power;

uniform float mandelbox_scale;
uniform float mandelbox_fixed_radius;
uniform float mandelbox_min_radius;

uniform int light_method;
uniform vec3 light_position;

out vec4 colorOut;

vec4 orbitTrap = vec4(10000.0);

float torusDF( vec3 p, vec3 c, vec2 t )
{
  p -= c;
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float mandelboxDF(vec3 p) {
    vec3 p0 = p;
    float dr = 1.0f;
    for(int n = 0; n < 13; n++) {
        p = (clamp(p,-1.0f,1.0f) * 2.0f) - p;
        float r2 = dot(p,p);
        if(r2<mandelbox_min_radius) {
            float t = (mandelbox_fixed_radius/mandelbox_min_radius);
            p *= t;
            dr *= t;
        } else if(r2<mandelbox_fixed_radius) {
            float t = (mandelbox_fixed_radius/r2);
            p *= t;
            dr *= t;
        }
        p = p * mandelbox_scale + p0;
        dr = dr * abs(mandelbox_scale) + 1.0f;
    }
    return length(p)/abs(dr);
}

float mandelbulbDF(vec3 p)
{
  vec3 z = p;
  float dr = 1.0;
  float r = 0.0;
  for(int i = 0; i < mandelbulb_iterations; i++)
  {
    r = length(z);
    if(r > mandelbulb_bailout) 
      break;
    float theta = acos(z.z/r);
    float phi = atan(z.y, z.x);
    dr = pow(r, mandelbulb_power - 1.0) * mandelbulb_power * dr + 1.0;

    float zr = pow(r, mandelbulb_power);
    theta = theta * mandelbulb_power;
    phi *= mandelbulb_power;

    z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
    z += p;
  }
  return 0.5*log(r) * r / dr;
}

float map(vec3 p)
{
  if(selection == 0)
    return mandelboxDF(p);
  else if(selection == 1)
    return mandelbulbDF(p);
  else
    return torusDF(p, vec3(0.0), vec2(5.0, 3.0));
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

mat3 look_at(in vec3 eye, in vec3 centre, in vec3 up)
{
    vec3 cw = normalize(centre-eye);
    vec3 cu = normalize(cross(cw,up));
    vec3 cv = normalize(cross(cu,cw));
    return mat3(cu, cv, cw);
}

// https://github.com/jon-grangien/OpenGL-mandelbulb-explorer/blob/master/shaders/mandel_raymarch.frag
vec3 colorFromOrbitTrap()
{
  float paletteCycleDist = dist0to1 + dist1to2 + dist2to3 + dist3to0;
  float dist01 = dist0to1 / paletteCycleDist;
  float dist12 = dist1to2 / paletteCycleDist;
  float dist23 = dist2to3 / paletteCycleDist;
  float dist30 = dist3to0 / paletteCycleDist;
  float cycle_intensity_a = cycle_intensity * 0.1;
  float poffset = palette_offset / 100.0;
  vec3 color;

  float orbitTot = dot(orbit_strength, orbitTrap);
  orbitTot = mod(abs(orbitTot) * cycle_intensity_a, 1.0);
  orbitTot = mod(orbitTot + poffset, 1.0);

  if(orbitTot <= dist01)
    color = mix(color0, color1, smoothstep(0.1, 1.0, abs(orbitTot) / (dist01)));
  else if(orbitTot <= dist01 + dist12)
    color = mix(color1, color2, smoothstep(0.1, 1.0, abs(orbitTot-dist01) / abs(dist12)));
  else if(orbitTot <= dist01 + dist12 + dist23)
    color = mix(color2, color3, smoothstep(0.1, 1.0, abs(orbitTot-dist01-dist12) / abs(dist23)));
  else
    color = mix(color3, color0, smoothstep(0.1, 1.0, abs(orbitTot-dist01-dist12-dist23) / abs(dist30)));

  color = mix(color, color_base, smoothstep(0.0, 1.0, base_color_strength));
  color = max(color, 0.0);
  color = min(color, 1.0);
  return color;
}

float soft_shadow(in vec3 ray_origin, in vec3 ray_direction, float mint, float maxt)
{
  float t = mint;
  float res = 1.0;
  float ph = 1e20;
  while(t < maxt)
  {
    float h = map(ray_origin + ray_direction*t);
    if(h < 0.001)
      return 0.0;
    float h2 = h*h;
    float y = h2/(2.0*ph);
    float d = sqrt(h2-y*y);
    res = min(res, 1.0*d/max(0.0, t-y));
    ph = h;
    t += h;
  }
  return res;
}

vec3 march(vec3 ray_origin, vec3 ray_direction, vec2 uv, out vec3 pos)
{
  float total_distance = 0.0;
  const float MIN_HIT_DISTANCE = 0.001;
  const float MAX_TRACE_DISTANCE = 1000.0;
  vec3 current_position;

  for(int i = 0; i < max_steps; i++)
  {
    current_position = ray_origin + total_distance * ray_direction;
    float distance_function_result = map(current_position);

    if(distance_function_result < MIN_HIT_DISTANCE)
    {
      vec3 normal = normal(current_position);
      vec3 direction_to_light = normalize(light_position);
      vec3 col = colorFromOrbitTrap();
      if(light_method)
      {
        col *= clamp(dot(normal, direction_to_light), 0.0, 1.0) * soft_shadow(current_position, direction_to_light, 0.01, 3.0);
        return col;
      }
      else 
      {
        float diffuse = max(0.0, dot(normal, direction_to_light));
        return col * diffuse;
      }
    }

    if(total_distance > MAX_TRACE_DISTANCE)
    {
      break;
    }

    total_distance += distance_function_result;
  }
  pos = current_position;
  return background_color;
}

vec3 blinn_phong(vec3 diff_color, vec3 p, vec3 ray_direction)
{
  vec3 ambient_color = diff_color * 0.8;
  const vec3 light_color = vec3(1.0);
  const vec3 spec_color = vec3(1.0);
  const float gamma = 2.2;

  vec3 normal = normal(p);
  vec3 eye_vector = normalize(eye - p);
  vec3 light_vector = normalize(light_position - p);
  vec3 H = normalize(light_vector + eye_vector);

  float light_power = 0.45;
  float lambertian = max(dot(light_vector, normal), 0.0);
  float specular = pow(max(dot(H, normal), 0.0), shine);

  vec3 blinn_phong_color = ambient_intensity * ambient_color +
    diffuse_intensity * diff_color * lambertian * light_color * light_power +
    specular_intensity * spec_color * specular * light_color * light_power;
  
  return mix(blinn_phong_color, pow(blinn_phong_color, vec3(1.0/gamma)), float(gamma_correction));
}

void main()
{
  vec2 uv = gl_FragCoord.xy / resolution.xy * 2.0 - 1.0;
  vec3 p;
  uv.x *= resolution.x / resolution.y;

  //vec3 camera_position = vec3(0.0, 0.0, -5.0);

  vec3 ray_origin = eye;
  vec3 ta = center;

  mat3 ca = look_at(ray_origin, ta, up);

  vec3 ray_direction = ca * normalize(vec3(uv.xy, 2.0));

  vec3 color = march(ray_origin, ray_direction, uv, p);

  color = mix(color, blinn_phong(color, p, ray_direction), 1.0);

  //color *= 1.5;
  //color = mix(0.5 * color, color, color);

  color = max(color, 0.0);
  color = min(color, 1.0);

  colorOut = vec4(color, 1.0);
}