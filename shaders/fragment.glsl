#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D ourTexture;

void main() {
  // I have absolutely no clue why this is needed, but for some reason disabling gl normalization and dividing by 256 NOT 255
  // fixes texture bleeding floating point errors. Writing code this bad makes me nostalgic for the days when people didn't have AI.

  // Actually now that I think about it, it does make sense because how could you store information for 256x256 grid if you divide it 
  // by 255. I guess in this case it does actually make sense to divide by the image width/height and not normalize it, doesn't make
  // it any less cursed
  FragColor = texture(ourTexture, TexCoord / 256.0f ); 
}
