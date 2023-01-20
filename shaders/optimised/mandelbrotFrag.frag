#version 150
uniform vec2 u_resolution;
uniform vec2 u_mouse;
out vec4 o_color;
void main ()
{
  vec2 z_3;
  vec3 col_4;
  vec2 uv_5;
  vec2 st_6;
  vec2 tmpvar_7;
  tmpvar_7 = ((gl_FragCoord.xy / u_resolution) - u_mouse);
  st_6.y = tmpvar_7.y;
  st_6.x = (tmpvar_7.x - 0.2);
  vec2 tmpvar_8;
  tmpvar_8 = ((st_6 * 2.0) - 1.0);
  uv_5.y = tmpvar_8.y;
  uv_5.x = (tmpvar_8.x * (u_resolution.x / u_resolution.y));
  col_4 = vec3(0.0, 0.0, 0.0);
  z_3 = vec2(0.0, 0.0);
  for (int n_1 = 0; n_1 < 4; n_1++) {
    for (int m_9 = 0; m_9 < 4; m_9++) {
      vec2 p_10;
      vec2 tmpvar_11;
      tmpvar_11.x = float(n_1);
      tmpvar_11.y = float(m_9);
      p_10 = (uv_5 + (tmpvar_11 / u_resolution));
      z_3 = vec2(0.0, 0.0);
      for (float i_2 = 0.0; i_2 < 100.0; i_2 += 1.0) {
        vec2 tmpvar_12;
        tmpvar_12.x = ((z_3.x * z_3.x) - (z_3.y * z_3.y));
        tmpvar_12.y = ((2.0 * z_3.x) * z_3.y);
        z_3 = (tmpvar_12 + p_10);
        float tmpvar_13;
        tmpvar_13 = dot (z_3, z_3);
        if ((tmpvar_13 > 4.0)) {
          break;
        };
      };
      float tmpvar_14;
      tmpvar_14 = (-(log(
        log(dot (z_3, z_3))
      )) / 0.6931472);
      float tmpvar_15;
      float tmpvar_16;
      tmpvar_16 = (min (abs(
        (z_3.y / z_3.x)
      ), 1.0) / max (abs(
        (z_3.y / z_3.x)
      ), 1.0));
      float tmpvar_17;
      tmpvar_17 = (tmpvar_16 * tmpvar_16);
      tmpvar_17 = (((
        ((((
          ((((-0.01213232 * tmpvar_17) + 0.05368138) * tmpvar_17) - 0.1173503)
         * tmpvar_17) + 0.1938925) * tmpvar_17) - 0.3326756)
       * tmpvar_17) + 0.9999793) * tmpvar_16);
      tmpvar_17 = (tmpvar_17 + (float(
        (abs((z_3.y / z_3.x)) > 1.0)
      ) * (
        (tmpvar_17 * -2.0)
       + 1.570796)));
      tmpvar_15 = (tmpvar_17 * sign((z_3.y / z_3.x)));
      if ((abs(z_3.x) > (1e-8 * abs(z_3.y)))) {
        if ((z_3.x < 0.0)) {
          if ((z_3.y >= 0.0)) {
            tmpvar_15 += 3.141593;
          } else {
            tmpvar_15 = (tmpvar_15 - 3.141593);
          };
        };
      } else {
        tmpvar_15 = (sign(z_3.y) * 1.570796);
      };
      col_4 = (col_4 + (0.5 + (0.5 * 
        cos((((tmpvar_14 * 0.1) + tmpvar_15) + vec3(0.0, 1.57075, 3.1415)))
      )));
    };
  };
  col_4 = (col_4 / 16.0);
  vec4 tmpvar_18;
  tmpvar_18.w = 1.0;
  tmpvar_18.xyz = col_4;
  o_color = tmpvar_18;
}

