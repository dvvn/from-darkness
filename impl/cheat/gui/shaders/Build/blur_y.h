#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
// Parameters:
//
//   sampler2D texSampler;
//   float texelHeight;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   texelHeight  c0       1
//   texSampler   s0       1
//

    ps_2_0
    def c1, 0.227027029, 1.38461542, 0.31621623, 3.23076916
    def c2, 0.0702702701, 0, 0, 0
    dcl t0.xy  // uv<0,1>
    dcl_2d s0

#line 14 "F:\1necheats\GOESP-master\GOESP\Resources\Shaders\blur_y.hlsl"
    mov r0.yw, c1  // ::offsets<1,2>
    mul r0.x, r0.y, c0.x
    mov r0.x, -r0.x
    add r1.y, r0.x, t0.y
    mov r1.x, t0.x
    mul r0.x, r0.y, c0.x
    add r0.y, r0.x, t0.y
    mov r0.x, t0.x
    mul r0.z, r0.w, c0.x
    mov r0.z, -r0.z
    add r2.y, r0.z, t0.y
    mov r2.x, t0.x
    mul r0.z, r0.w, c0.x
    add r3.y, r0.z, t0.y
    mov r3.x, t0.x

#line 10
    texld r4, t0, s0  // ::color<0,1,2,3>

#line 14
    texld r1, r1, s0
    texld r0, r0, s0
    texld r2, r2, s0
    texld r3, r3, s0

#line 11
    mul r5.xyz, r4, c1.x  // ::color<0,1,2>

#line 14
    mul r1.xyz, r1, c1.z
    add r1.xyz, r1, r5  // ::color<0,1,2>
    mul r0.xyz, r0, c1.z
    add r0.xyz, r0, r1  // ::color<0,1,2>
    mul r1.xyz, r2, c2.x
    add r0.xyz, r0, r1  // ::color<0,1,2>
    mul r1.xyz, r3, c2.x
    add r4.xyz, r0, r1  // ::color<0,1,2>

#line 18
    mov r4.xyz, r4  // ::main<0,1,2>
    mov r4.w, r4.w  // ::main<3>

#line 8
    mov oC0, r4  // ::main<0,1,2,3>

// approximately 32 instruction slots used (5 texture, 27 arithmetic)
#endif

const BYTE blur_y[] =
{
      0,   2, 255, 255, 254, 255, 
    186,   0,  68,  66,  85,  71, 
     40,   0,   0,   0, 188,   2, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0,  36,   0,   0,   0, 
    108,   0,   0,   0,   4,   0, 
      0,   0, 108,   2,   0,   0, 
    236,   1,   0,   0,  70,  58, 
     92,  49, 110, 101,  99, 104, 
    101,  97, 116, 115,  92,  71, 
     79,  69,  83,  80,  45, 109, 
     97, 115, 116, 101, 114,  92, 
     71,  79,  69,  83,  80,  92, 
     82, 101, 115, 111, 117, 114, 
     99, 101, 115,  92,  83, 104, 
     97, 100, 101, 114, 115,  92, 
     98, 108, 117, 114,  95, 121, 
     46, 104, 108, 115, 108,   0, 
    171, 171,  40,   0,   0,   0, 
      0,   0, 255, 255, 164,   3, 
      0,   0,   0,   0, 255, 255, 
    188,   3,   0,   0,   0,   0, 
    255, 255, 212,   3,   0,   0, 
      0,   0, 255, 255, 224,   3, 
      0,   0,  14,   0,   0,   0, 
    236,   3,   0,   0,  14,   0, 
      0,   0, 248,   3,   0,   0, 
     14,   0,   0,   0,   8,   4, 
      0,   0,  14,   0,   0,   0, 
     20,   4,   0,   0,  14,   0, 
      0,   0,  36,   4,   0,   0, 
     15,   0,   0,   0,  48,   4, 
      0,   0,  15,   0,   0,   0, 
     64,   4,   0,   0,  15,   0, 
      0,   0,  80,   4,   0,   0, 
     14,   0,   0,   0,  92,   4, 
      0,   0,  14,   0,   0,   0, 
    108,   4,   0,   0,  14,   0, 
      0,   0, 120,   4,   0,   0, 
     14,   0,   0,   0, 136,   4, 
      0,   0,  15,   0,   0,   0, 
    148,   4,   0,   0,  15,   0, 
      0,   0, 164,   4,   0,   0, 
     15,   0,   0,   0, 180,   4, 
      0,   0,  10,   0,   0,   0, 
    192,   4,   0,   0,  14,   0, 
      0,   0, 208,   4,   0,   0, 
     15,   0,   0,   0, 224,   4, 
      0,   0,  14,   0,   0,   0, 
    240,   4,   0,   0,  15,   0, 
      0,   0,   0,   5,   0,   0, 
     11,   0,   0,   0,  16,   5, 
      0,   0,  14,   0,   0,   0, 
     32,   5,   0,   0,  14,   0, 
      0,   0,  48,   5,   0,   0, 
     15,   0,   0,   0,  64,   5, 
      0,   0,  15,   0,   0,   0, 
     80,   5,   0,   0,  14,   0, 
      0,   0,  96,   5,   0,   0, 
     14,   0,   0,   0, 112,   5, 
      0,   0,  15,   0,   0,   0, 
    128,   5,   0,   0,  15,   0, 
      0,   0, 144,   5,   0,   0, 
     18,   0,   0,   0, 160,   5, 
      0,   0,  18,   0,   0,   0, 
    172,   5,   0,   0,   8,   0, 
      0,   0, 184,   5,   0,   0, 
     99, 111, 108, 111, 114,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     19,   0,   0,   0,   0,   0, 
      1,   0,   2,   0,   3,   0, 
     24,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
     26,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
     28,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
     30,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
    109,  97, 105, 110,   0, 171, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     33,   0,   0,   0,   0,   0, 
      1,   0,   2,   0, 255, 255, 
     34,   0,   0,   0, 255, 255, 
    255, 255, 255, 255,   3,   0, 
     35,   0,   0,   0,   0,   0, 
      1,   0,   2,   0,   3,   0, 
    111, 102, 102, 115, 101, 116, 
    115,   0,   0,   0,   3,   0, 
      1,   0,   1,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
      1,   0, 255, 255,   2,   0, 
    117, 118,   0, 171,   1,   0, 
      3,   0,   1,   0,   2,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   1,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    140,   1,   0,   0, 148,   1, 
      0,   0,   6,   0,   0,   0, 
    164,   1,   0,   0,   0,   0, 
      0,   0, 236,   1,   0,   0, 
    244,   1,   0,   0,   3,   0, 
      0,   0,   4,   2,   0,   0, 
      0,   0,   0,   0,  40,   2, 
      0,   0,  48,   2,   0,   0, 
      1,   0,   0,   0,  64,   2, 
      0,   0, 236,   1,   0,   0, 
     76,   2,   0,   0,  80,   2, 
      0,   0,   1,   0,   0,   0, 
     96,   2,   0,   0,  77, 105, 
     99, 114, 111, 115, 111, 102, 
    116,  32,  40,  82,  41,  32, 
     72,  76,  83,  76,  32,  83, 
    104,  97, 100, 101, 114,  32, 
     67, 111, 109, 112, 105, 108, 
    101, 114,  32,  49,  48,  46, 
     49,   0, 254, 255,  44,   0, 
     67,  84,  65,  66,  28,   0, 
      0,   0, 131,   0,   0,   0, 
      0,   2, 255, 255,   2,   0, 
      0,   0,  28,   0,   0,   0, 
      5,   1,   0,   0, 124,   0, 
      0,   0,  68,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  80,   0,   0,   0, 
      0,   0,   0,   0,  96,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0, 108,   0, 
      0,   0,   0,   0,   0,   0, 
    116, 101, 120,  83,  97, 109, 
    112, 108, 101, 114,   0, 171, 
      4,   0,  12,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 116, 101, 
    120, 101, 108,  72, 101, 105, 
    103, 104, 116,   0,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 112, 115,  95,  50, 
     95,  48,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  49,  48,  46,  49, 
      0, 171,  81,   0,   0,   5, 
      1,   0,  15, 160, 198, 121, 
    104,  62,  20,  59, 177,  63, 
     24, 231, 161,  62, 236, 196, 
     78,  64,  81,   0,   0,   5, 
      2,   0,  15, 160, 220, 233, 
    143,  61,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  31,   0,   0,   2, 
      0,   0,   0, 128,   0,   0, 
      3, 176,  31,   0,   0,   2, 
      0,   0,   0, 144,   0,   8, 
     15, 160,   1,   0,   0,   2, 
      0,   0,  10, 128,   1,   0, 
    228, 160,   5,   0,   0,   3, 
      0,   0,   1, 128,   0,   0, 
     85, 128,   0,   0,   0, 160, 
      1,   0,   0,   2,   0,   0, 
      1, 128,   0,   0,   0, 129, 
      2,   0,   0,   3,   1,   0, 
      2, 128,   0,   0,   0, 128, 
      0,   0,  85, 176,   1,   0, 
      0,   2,   1,   0,   1, 128, 
      0,   0,   0, 176,   5,   0, 
      0,   3,   0,   0,   1, 128, 
      0,   0,  85, 128,   0,   0, 
      0, 160,   2,   0,   0,   3, 
      0,   0,   2, 128,   0,   0, 
      0, 128,   0,   0,  85, 176, 
      1,   0,   0,   2,   0,   0, 
      1, 128,   0,   0,   0, 176, 
      5,   0,   0,   3,   0,   0, 
      4, 128,   0,   0, 255, 128, 
      0,   0,   0, 160,   1,   0, 
      0,   2,   0,   0,   4, 128, 
      0,   0, 170, 129,   2,   0, 
      0,   3,   2,   0,   2, 128, 
      0,   0, 170, 128,   0,   0, 
     85, 176,   1,   0,   0,   2, 
      2,   0,   1, 128,   0,   0, 
      0, 176,   5,   0,   0,   3, 
      0,   0,   4, 128,   0,   0, 
    255, 128,   0,   0,   0, 160, 
      2,   0,   0,   3,   3,   0, 
      2, 128,   0,   0, 170, 128, 
      0,   0,  85, 176,   1,   0, 
      0,   2,   3,   0,   1, 128, 
      0,   0,   0, 176,  66,   0, 
      0,   3,   4,   0,  15, 128, 
      0,   0, 228, 176,   0,   8, 
    228, 160,  66,   0,   0,   3, 
      1,   0,  15, 128,   1,   0, 
    228, 128,   0,   8, 228, 160, 
     66,   0,   0,   3,   0,   0, 
     15, 128,   0,   0, 228, 128, 
      0,   8, 228, 160,  66,   0, 
      0,   3,   2,   0,  15, 128, 
      2,   0, 228, 128,   0,   8, 
    228, 160,  66,   0,   0,   3, 
      3,   0,  15, 128,   3,   0, 
    228, 128,   0,   8, 228, 160, 
      5,   0,   0,   3,   5,   0, 
      7, 128,   4,   0, 228, 128, 
      1,   0,   0, 160,   5,   0, 
      0,   3,   1,   0,   7, 128, 
      1,   0, 228, 128,   1,   0, 
    170, 160,   2,   0,   0,   3, 
      1,   0,   7, 128,   1,   0, 
    228, 128,   5,   0, 228, 128, 
      5,   0,   0,   3,   0,   0, 
      7, 128,   0,   0, 228, 128, 
      1,   0, 170, 160,   2,   0, 
      0,   3,   0,   0,   7, 128, 
      0,   0, 228, 128,   1,   0, 
    228, 128,   5,   0,   0,   3, 
      1,   0,   7, 128,   2,   0, 
    228, 128,   2,   0,   0, 160, 
      2,   0,   0,   3,   0,   0, 
      7, 128,   0,   0, 228, 128, 
      1,   0, 228, 128,   5,   0, 
      0,   3,   1,   0,   7, 128, 
      3,   0, 228, 128,   2,   0, 
      0, 160,   2,   0,   0,   3, 
      4,   0,   7, 128,   0,   0, 
    228, 128,   1,   0, 228, 128, 
      1,   0,   0,   2,   4,   0, 
      7, 128,   4,   0, 228, 128, 
      1,   0,   0,   2,   4,   0, 
      8, 128,   4,   0, 255, 128, 
      1,   0,   0,   2,   0,   8, 
     15, 128,   4,   0, 228, 128, 
    255, 255,   0,   0
};
