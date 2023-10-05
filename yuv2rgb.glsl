uniform sampler2D u_texture1;

vec4 yuv_to_rgb(vec4 yuva)
{
  vec4 res;
  float Y = 1.16438356 * (yuva.x - 0.0625);
  float su = yuva.y - 0.5;
  float sv = yuva.z - 0.5;
  res.r = Y                   + 1.59602678 * sv;
  res.g = Y - 0.39176229 * su - 0.81296764 * sv;
  res.b = Y + 2.01723214 * su;
  res.rgb *= yuva.w;
  res.a = yuva.w;
  return res;
}

void mainImage(out vec4 fragColor,
               in vec2 fragCoord,
               in vec2 resolution,
               in vec2 uv)
{
  vec4 y2uv = GskTexture(u_texture1, uv);
  vec2 y01 = y2uv.zx;
  vec2 cbcr = y2uv.yw;
  float leftover = fract(uv.x);
  float y = (leftover < 0.5) ? y01.x : y01.y;
  vec3 yuv = vec3(y, cbcr);
  vec3 rgb = yuv_to_rgb(yuv);

  vec4 yuva = vec4(yuv, 1.0);
  fragColor = yuv_to_rgb(yuva);
}
