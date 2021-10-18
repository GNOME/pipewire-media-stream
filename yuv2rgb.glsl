uniform sampler2D u_texture1;

vec3 YUV_to_RGB(vec3 yuv)
{
  float r = yuv.x + 1.140 * yuv.z;
  float g = yuv.x - 0.395 * yuv.y - 0.581 * yuv.z;
  float b = yuv.x + 2.032 * yuv.y;


/*
  float y = 1.1643 * (yuv.x - 0.0625);
  float u = yuv.y - 0.5;
  float v = yuv.z - 0.5;

  float r = v * 1.5958 + y;
  float g = -0.39173 * u + (-0.81290 * v + y);
  float b = 2.017 * u + y;
*/
/*
  float r = yuv.x * 1.0 + yuv.y * 0.0 + yuv.z * 1.4;
  float g = yuv.x * 1.0 + yuv.y * -0.343 + yuv.z * -0.711;
  float b = yuv.x * 1.0 + yuv.y * 1.765 + yuv.z * 0.0;
*/
/*
  float r = (yuv.x + 1.402 * (yuv.z - 128.0));
  float g = (yuv.x - 0.344 * (yuv.y - 128.0) - 0.714 * (yuv.z - 128.0));
  float b = (yuv.x + 1.722 * (yuv.y - 128.0));
*/
	return vec3(r, g, b);
}

void mainImage(out vec4 fragColor,
               in vec2 fragCoord,
               in vec2 resolution,
               in vec2 uv)
{
/*
  vec4 sourceYUV = GskTexture(u_texture1, uv);

  vec4 yuva = vec4(sourceYUV.x, (sourceYUV.y - 0.5), (sourceYUV.z - 0.5), 1.0);
  vec4 rgba = vec4(0.0);
  rgba.r = yuva.x * 1.0 + yuva.y * 0.0 + yuva.z * 1.4;
  rgba.g = yuva.x * 1.0 + yuva.y * -0.343 + yuva.z * -0.711;
  rgba.b = yuva.x * 1.0 + yuva.y * 1.765 + yuva.z * 0.0;
  rgba.a = 1.0;

  //fragColor = vec4(1.0,1.0,0.0,1.0);
  fragColor = sourceYUV;
*/

	vec4 y2uv = GskTexture(u_texture1, uv);
	vec2 y01 = y2uv.zx;
	vec2 cbcr = y2uv.yw;
	float leftover = fract(uv.x);
	float y = (leftover < 0.5) ? y01.x : y01.y;
	vec3 yuv = vec3(y, cbcr);
	vec3 rgb = YUV_to_RGB(yuv);

	fragColor = vec4(rgb, 1.0);
}
