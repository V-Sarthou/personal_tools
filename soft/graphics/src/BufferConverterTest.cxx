#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <glad/gl.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef void (*ConvertFunc)(void* dstData, const void *srcData, size_t numElements);

template<typename T>
T clamp(T value, T minValue, T maxValue)
{
  return std::max(minValue, std::min(maxValue, value));
}

template<typename T> struct FloatType { typedef float type; };
template<> struct FloatType<uint8_t> { typedef float type; };
template<> struct FloatType<int8_t> { typedef float type; };
template<> struct FloatType<uint16_t> { typedef float type; };
template<> struct FloatType<int16_t> { typedef float type; };
template<> struct FloatType<uint32_t> { typedef double type; };
template<> struct FloatType<int32_t> { typedef double type; };

template<typename srcType, typename dstType>
void convertCast(void *dstData, const void *srcData, size_t numElements)
{
  dstType *dst = static_cast<dstType *>(dstData);
  dstType *const dstEnd = dst + numElements;
  const srcType *src = static_cast<const srcType *>(srcData);
  while (dst < dstEnd)
    *(dst++) = static_cast<dstType>(*(src++));
}

template<typename srcType, typename dstType>
void convertClamp(void *dstData, const void *srcData, size_t numElements)
{
  dstType *dst = static_cast<dstType *>(dstData);
  dstType *const dstEnd = dst + numElements;
  const srcType *src = static_cast<const srcType *>(srcData);
  static const int64_t minValue = std::max(static_cast<int64_t>(std::numeric_limits<dstType>::min()), static_cast<int64_t>(std::numeric_limits<srcType>::min()));
  static const int64_t maxValue = std::min(static_cast<int64_t>(std::numeric_limits<dstType>::max()), static_cast<int64_t>(std::numeric_limits<srcType>::max()));
  while (dst < dstEnd)
    *(dst++) = static_cast<dstType>(clamp(static_cast<int64_t>(*(src++)), minValue, maxValue));
}

template<typename srcType, typename dstType>
void convertRenormU(void *dstData, const void *srcData, size_t numElements)
{
  dstType *dst = static_cast<dstType *>(dstData);
  dstType *const dstEnd = dst + numElements;
  const srcType *src = static_cast<const srcType *>(srcData);
  static const int64_t srcMin = static_cast<int64_t>(std::numeric_limits<srcType>::min());
  static const int64_t dstMin = static_cast<int64_t>(std::numeric_limits<dstType>::min());
  static const int64_t valueMin = std::max(static_cast<int64_t>(std::numeric_limits<dstType>::min()), static_cast<int64_t>(std::numeric_limits<srcType>::min()));
  static const int64_t valueMax = std::min(static_cast<int64_t>(std::numeric_limits<dstType>::max()), static_cast<int64_t>(std::numeric_limits<srcType>::max()));
  static const int64_t factorM = std::max(1LL, (static_cast<int64_t>(std::numeric_limits<dstType>::max()) - dstMin) / (static_cast<int64_t>(std::numeric_limits<srcType>::max()) - srcMin));
  static const int64_t factorD = std::max(1LL, (static_cast<int64_t>(std::numeric_limits<srcType>::max()) - srcMin) / (static_cast<int64_t>(std::numeric_limits<dstType>::max()) - dstMin));
  while (dst < dstEnd)
    *(dst++) = static_cast<dstType>(clamp(dstMin + factorM * (static_cast<int64_t>(*(src++)) - srcMin) / factorD, valueMin, valueMax));
}

template<typename uType>
void convertUToFloat(void *dstData, const void *srcData, size_t numElements)
{
  float *dst = static_cast<float *>(dstData);
  float *const dstEnd = dst + numElements;
  const uType *src = static_cast<const uType *>(srcData);
  static const typename FloatType<uType>::type maxValue = static_cast<typename FloatType<uType>::type>(std::numeric_limits<uType>::max());
  while (dst < dstEnd)
    *(dst++) = static_cast<float>(static_cast<typename FloatType<uType>::type>(*(src++)) / maxValue);
}

template<typename iType>
void convertIToFloat(void *dstData, const void *srcData, size_t numElements)
{
  float *dst = static_cast<float *>(dstData);
  float *const dstEnd = dst + numElements;
  const iType *src = static_cast<const iType *>(srcData);
  static const typename FloatType<iType>::type minValue = static_cast<typename FloatType<iType>::type>(std::numeric_limits<iType>::min());
  static const typename FloatType<uType>::type valueRange = static_cast<typename FloatType<uType>::type>(std::numeric_limits<iType>::max()) - minValue;
  while (dst < dstEnd)
    *(dst++) = -1.0f + 2.0f * static_cast<float>((minValue + static_cast<typename FloatType<iType>::type>(*(src++))) / valueRange);
}

template<typename uType>
void convertFloatToU(void *dstData, const void *srcData, size_t numElements)
{
  uType *dst = static_cast<uType *>(dstData);
  uType *const dstEnd = dst + numElements;
  const float *src = static_cast<const float *>(srcData);
  static const typename FloatType<uType>::type maxValue = static_cast<typename FloatType<uType>::type>(std::numeric_limits<uType>::max());
  while (dst < dstEnd)
    *(dst++) = static_cast<uType>(clamp<typename FloatType<uType>::type>(static_cast<typename FloatType<uType>::type>(*(src++)), 0.0, 1.0) * maxValue);
}

template<typename iType>
void convertFloatToI(void *dstData, const void *srcData, size_t numElements)
{
  iType *dst = static_cast<iType *>(dstData);
  iType *const dstEnd = dst + numElements;
  const float *src = static_cast<const float *>(srcData);
  static const typename FloatType<iType>::type minValue = static_cast<typename FloatType<iType>::type>(std::numeric_limits<iType>::min());
  static const typename FloatType<iType>::type valueRange = static_cast<typename FloatType<iType>::type>(std::numeric_limits<iType>::max()) - minValue;
  while (dst < dstEnd)
    *(dst++) = static_cast<iType>(clamp<typename FloatType<uType>::type>(static_cast<typename FloatType<uType>::type>(*(src++)), -1.0f, 1.0f) * valueRange + minValue);
}

static const ConvertFunc s_convertFuncs[] = {
  convertCast<uint8_t, uint8_t>, // GL_UNSIGNED_BYTE -> GL_R8
  nullptr, // GL_UNSIGNED_BYTE -> GL_R8_SNORM
  nullptr, // GL_UNSIGNED_BYTE -> GL_R16
  nullptr, // GL_UNSIGNED_BYTE -> GL_R16_SNORM
  convertUToFloat<uint8_t>, // GL_UNSIGNED_BYTE -> GL_R32F
  nullptr, // GL_UNSIGNED_BYTE -> GL_R8I
  convertCast<uint8_t, uint8_t>, // GL_UNSIGNED_BYTE -> GL_R8UI
  convertCast<uint8_t, int16_t>, // GL_UNSIGNED_BYTE -> GL_R16I
  convertCast<uint8_t, uint16_t>, // GL_UNSIGNED_BYTE -> GL_R16UI
  convertCast<uint8_t, int32_t>, // GL_UNSIGNED_BYTE -> GL_R32I
  convertCast<uint8_t, uint32_t>, // GL_UNSIGNED_BYTE -> GL_R32UI

  nullptr, // GL_BYTE -> GL_R8
  convertCast<int8_t, int8_t>, // GL_BYTE -> GL_R8_SNORM
  nullptr, // GL_BYTE -> GL_R16
  nullptr, // GL_BYTE -> GL_R16_SNORM
  convertUToFloat<int8_t>, // GL_BYTE -> GL_R32F
  convertCast<int8_t, int8_t>, // GL_BYTE -> GL_R8I
  convertClamp<int8_t, uint8_t>, // GL_BYTE -> GL_R8UI
  convertCast<int8_t, int16_t>, // GL_BYTE -> GL_R16I
  convertClamp<int8_t, uint16_t>, // GL_BYTE -> GL_R16UI
  convertCast<int8_t, int32_t>, // GL_BYTE -> GL_R32I
  convertClamp<int8_t, uint32_t>, // GL_BYTE -> GL_R32UI

  nullptr, // GL_UNSIGNED_SHORT -> GL_R8
  nullptr, // GL_UNSIGNED_SHORT -> GL_R8_SNORM
  convertCast<uint16_t, uint16_t>, // GL_UNSIGNED_SHORT -> GL_R16
  nullptr, // GL_UNSIGNED_SHORT -> GL_R16_SNORM
  convertUToFloat<uint16_t>, // GL_UNSIGNED_SHORT -> GL_R32F
  convertClamp<uint16_t, int8_t>, // GL_UNSIGNED_SHORT -> GL_R8I
  convertClamp<uint16_t, uint8_t>, // GL_UNSIGNED_SHORT -> GL_R8UI
  convertClamp<uint16_t, int16_t>, // GL_UNSIGNED_SHORT -> GL_R16I
  convertCast<uint16_t, uint16_t>, // GL_UNSIGNED_SHORT -> GL_R16UI
  convertClamp<uint16_t, int32_t>, // GL_UNSIGNED_SHORT -> GL_R32I
  convertCast<uint16_t, uint32_t>, // GL_UNSIGNED_SHORT -> GL_R32UI

  nullptr, // GL_SHORT -> GL_R8
  nullptr, // GL_SHORT -> GL_R8_SNORM
  nullptr, // GL_SHORT -> GL_R16
  convertCast<int16_t, int16_t>, // GL_SHORT -> GL_R16_SNORM
  convertUToFloat<int16_t>, // GL_SHORT -> GL_R32F
  convertClamp<int16_t, int8_t>, // GL_SHORT -> GL_R8I
  convertClamp<int16_t, uint8_t>, // GL_SHORT -> GL_R8UI
  convertCast<int16_t, int16_t>, // GL_SHORT -> GL_R16I
  convertClamp<int16_t, uint16_t>, // GL_SHORT -> GL_R16UI
  convertCast<int16_t, int32_t>, // GL_SHORT -> GL_R32I
  convertClamp<int16_t, uint32_t>, // GL_SHORT -> GL_R32UI

  nullptr, // GL_UNSIGNED_INT -> GL_R8
  nullptr, // GL_UNSIGNED_INT -> GL_R8_SNORM
  nullptr, // GL_UNSIGNED_INT -> GL_R16
  nullptr, // GL_UNSIGNED_INT -> GL_R16_SNORM
  convertUToFloat<uint32_t>, // GL_UNSIGNED_INT -> GL_R32F
  convertClamp<uint32_t, int8_t>, // GL_UNSIGNED_INT -> GL_R8I
  convertClamp<uint32_t, uint8_t>, // GL_UNSIGNED_INT -> GL_R8UI
  convertClamp<uint32_t, int16_t>, // GL_UNSIGNED_INT -> GL_R16I
  convertClamp<uint32_t, uint16_t>, // GL_UNSIGNED_INT -> GL_R16UI
  convertClamp<uint32_t, int32_t>, // GL_UNSIGNED_INT -> GL_R32I
  convertCast<uint32_t, uint32_t>, // GL_UNSIGNED_INT -> GL_R32UI

  nullptr, // GL_INT -> GL_R8
  nullptr, // GL_INT -> GL_R8_SNORM
  nullptr, // GL_INT -> GL_R16
  nullptr, // GL_INT -> GL_R16_SNORM
  convertUToFloat<int32_t>, // GL_INT -> GL_R32F
  convertClamp<int32_t, int8_t>, // GL_INT -> GL_R8I
  convertClamp<int32_t, uint8_t>, // GL_INT -> GL_R8UI
  convertClamp<int32_t, int16_t>, // GL_INT -> GL_R16I
  convertClamp<int32_t, uint16_t>, // GL_INT -> GL_R16UI
  convertCast<int32_t, int32_t>, // GL_INT -> GL_R32I
  convertClamp<int32_t, uint32_t>, // GL_INT -> GL_R32UI

  convertFloatToU<uint8_t>, // GL_FLOAT -> GL_R8
  convertFloatToI<int8_t>, // GL_FLOAT -> GL_R8_SNORM
  convertFloatToU<uint16_t>, // GL_FLOAT -> GL_R16
  convertFloatToI<int16_t>, // GL_FLOAT -> GL_R16_SNORM
  convertCast<float, float>, // GL_FLOAT -> GL_R32F
  nullptr, // GL_FLOAT -> GL_R8I
  nullptr, // GL_FLOAT -> GL_R8UI
  nullptr, // GL_FLOAT -> GL_R16I
  nullptr, // GL_FLOAT -> GL_R16UI
  nullptr, // GL_FLOAT -> GL_R32I
  nullptr, // GL_FLOAT -> GL_R32UI
};

static ConvertFunc getConvertFunc(GLenum intput_format, GLenum internal_format)
{
  // TODO
  return nullptr;
}

static int getInternalFormatSize(GLenum internal_format)
{
  switch (internal_format)
  {
  case GL_R8:
  case GL_R8UI:
    return sizeof(uint8_t);
  case GL_R8_SNORM:
  case GL_R8I:
    return sizeof(int8_t);
  case GL_R16:
  case GL_R16UI:
    return sizeof(uint16_t);
  case GL_R16_SNORM:
  case GL_R16I:
    return sizeof(int16_t);
  case GL_R32F:
    return sizeof(float);
  case GL_R32I:
    return sizeof(int32_t);
  case GL_R32UI:
    return sizeof(uint32_t);
  default:
    return -1;
  }
}

int main(int argc, char *argv[])
{
  // input data
  static const uint8_t textureDataU8[] = { 0, 1, 64, 128, 192, 255 };
  static const int8_t textureDataI8[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint16_t textureDataU16[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };
  static const int16_t textureDataI16[] = { -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const uint32_t textureDataU32[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483648U, 3221225472U, 4294967295U };
  static const int32_t textureDataI32[] = { -2147483648, -1073741824, -65536, -49152, -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483647 };
  static const float textureDataF32[] = { -4294967296.0f, -3221225472.0f, -2147483648.0f, -1073741824.0f, -65536.0f, -49152.0f, -32768.0f, -16384.0f, -256.0f, -192.0f, -128.0f, -64.0f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 64.0f, 128.0f, 192.0f, 256.0f, 16384.0f, 32768.0f, 49152.0f, 65536.0f, 1073741824.0f, 2147483648.0f, 3221225472.0f, 4294967296.0f };

  // ref data for U8
  static const uint8_t refTextureDataU8_R8[] = { 0, 1, 64, 128, 192, 255 };
  static const uint8_t refTextureDataU8_R8_SNORM[] = { 0, 0, 32, 64, 96, 127 };
  static const uint16_t refTextureDataU8_R16[] = { 0, 257, 16448, 32896, 49344, 65535 };
  static const uint16_t refTextureDataU8_R16_SNORM[] = { 0, 128, 8224, 16448, 24672, 32767 };
  static const float refTextureDataU8_R32F[] = { 0.0f, 0.003922f, 0.25098f, 0.501961f, 0.752941f, 1.0f };
  static const int8_t refTextureDataU8_R8I[] = { 0, 1, 64, 127, 127, 127 };
  static const uint8_t refTextureDataU8_R8UI[] = { 0, 1, 64, 128, 192, 255 };
  static const int16_t refTextureDataU8_R16I[] = { 0, 1, 64, 128, 192, 255 };
  static const uint16_t refTextureDataU8_R16UI[] = { 0, 1, 64, 128, 192, 255 };
  static const int32_t refTextureDataU8_R32I[] = { 0, 1, 64, 128, 192, 255 };
  static const uint32_t refTextureDataU8_R32UI[] = { 0, 1, 64, 128, 192, 255 };

  // ref data for I8
  static const uint8_t refTextureDataI8_R8[] = { 0, 0, 0, 1, 3, 129, 255 };
  static const int8_t refTextureDataI8_R8_SNORM[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint16_t refTextureDataI8_R16[] = { 0, 0, 0, 257, 771, 33153, 65535 };
  static const int16_t refTextureDataI8_R16_SNORM[] = { -32768, -16320, -129, 128, 385, 16576, 32767 };
  static const float refTextureDataI8_R32F[] = { -1.0f, -0.498039f, -0.003922f, 0.003922f, 0.011765f, 0.505882f, 1.0f };
  static const int8_t refTextureDataI8_R8I[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint8_t refTextureDataI8_R8UI[] = { 0, 0, 0, 0, 1, 64, 127 };
  static const int16_t refTextureDataI8_R16I[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint16_t refTextureDataI8_R16UI[] = { 0, 0, 0, 0, 1, 64, 127 };
  static const int32_t refTextureDataI8_R32I[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint32_t refTextureDataI8_R32UI[] = { 0, 0, 0, 0, 1, 64, 127 };

  // ref data for U16
  static const uint8_t refTextureDataU16_R8[] = { 0, 0, 0, 0, 0, 1, 64, 128, 192, 255 };
  static const int8_t refTextureDataU16_R8_SNORM[] = { 0, 0, 0, 0, 0, 0, 32, 64, 96, 127 };
  static const uint16_t refTextureDataU16_R16[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };
  static const int16_t refTextureDataU16_R16_SNORM[] = { 0, 0, 32, 64, 96, 128, 8192, 16384, 24576, 32767 };
  static const float refTextureDataU16_R32F[] = { 0.0f, 0.000015f, 0.000977f, 0.001953f, 0.00293f, 0.003906f, 0.250004f, 0.500008f, 0.750011f, 1.0f };
  static const int8_t refTextureDataU16_R8I[] = { 0, 1, 64, 127, 127, 127, 127, 127, 127, 127 };
  static const uint8_t refTextureDataU16_R8UI[] = { 0, 1, 64, 128, 192, 255, 255, 255, 255, 255 };
  static const int16_t refTextureDataU16_R16I[] = { 0, 1, 64, 128, 192, 256, 16384, 32767, 32767, 32767 };
  static const uint16_t refTextureDataU16_R16UI[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };
  static const int32_t refTextureDataU16_R32I[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };
  static const uint32_t refTextureDataU16_R32UI[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };

  // ref data for I16
  static const uint8_t refTextureDataI16_R8[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 128, 255 };
  static const int8_t refTextureDataI16_R8_SNORM[] = { -128, -64, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 1, 64, 127 };
  static const uint16_t refTextureDataI16_R16[] = { 0, 0, 0, 0, 0, 0, 0, 1, 3, 129, 257, 385, 513, 32769, 65535 };
  static const int16_t refTextureDataI16_R16_SNORM[] = { -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const float refTextureDataI16_R32F[] = { -1.0f, -0.499992f, -0.007797f, -0.005844f, -0.003891f, -0.001938f, -0.000015f, 0.000015f, 0.000046f, 0.001968f, 0.003922f, 0.005875f, 0.007828f, 0.500023f, 1.0f };
  static const int8_t refTextureDataI16_R8I[] = { -128, -128, -128, -128, -128, -64, -1, 0, 1, 64, 127, 127, 127, 127, 127 };
  static const uint8_t refTextureDataI16_R8UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 255, 255, 255 };
  static const int16_t refTextureDataI16_R16I[] = { -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const uint16_t refTextureDataI16_R16UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const int32_t refTextureDataI16_R32I[] = { -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const uint32_t refTextureDataI16_R32UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 256, 16384, 32767 };

  // ref data for U32
  static const uint8_t refTextureDataU32_R8[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 128, 192, 255 };
  static const int8_t refTextureDataU32_R8_SNORM[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 64, 96, 127 };
  static const uint16_t refTextureDataU32_R16[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16384, 32768, 49152, 65535 };
  static const int16_t refTextureDataU32_R16_SNORM[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8192, 16384, 24576, 32767 };
  static const float refTextureDataU32_R32F[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.000004f, 0.000008f, 0.000011f, 0.000015f, 0.25f, 0.5f, 0.75f, 1.0f };
  static const int8_t refTextureDataU32_R8I[] = { 0, 1, 64, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
  static const uint8_t refTextureDataU32_R8UI[] = { 0, 1, 64, 128, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  static const int16_t refTextureDataU32_R16I[] = { 0, 1, 64, 128, 192, 256, 16384, 32767, 32767, 32767, 32767, 32767, 32767, 32767 };
  static const uint16_t refTextureDataU32_R16UI[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535, 65535, 65535, 65535, 65535 };
  static const int32_t refTextureDataU32_R32I[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483647, 2147483647, 2147483647 };
  static const uint32_t refTextureDataU32_R32UI[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483648U, 3221225472U, 4294967295U };

  // ref data for I32
  static const uint8_t refTextureDataI32_R8[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 255 };
  static const int8_t refTextureDataI32_R8_SNORM[] = { -128, -64, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 127 };
  static const uint16_t refTextureDataI32_R16[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 32768, 65535 };
  static const int16_t refTextureDataI32_R16_SNORM[] = { -32768, -16384, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16384, 32767 };
  static const float refTextureDataI32_R32F[] = { -1.0f, -0.5f, -0.000031f, -0.000023f, -0.000015f, -0.000008f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.000008f, 0.000015f, 0.000023f, 0.000031f, 0.5f, 1.0f };
  static const int8_t refTextureDataI32_R8I[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -64, -1, 0, 1, 64, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
  static const uint8_t refTextureDataI32_R8UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 255, 255, 255, 255, 255, 255, 255 };
  static const int16_t refTextureDataI32_R16I[] = { -32768, -32768, -32768, -32768, -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767, 32767, 32767, 32767, 32767 };
  static const uint16_t refTextureDataI32_R16UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535, 65535, 65535 };
  static const int32_t refTextureDataI32_R32I[] = { -2147483648, -1073741824, -65536, -49152, -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483647 };
  static const uint32_t refTextureDataI32_R32UI[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483647 };

  // ref data for FLOAT
  static const uint8_t refTextureDataF32_R8[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 128, 191, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  static const int8_t refTextureDataF32_R8_SNORM[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -96, -64, -32, 0, 31, 63, 95, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
  static const uint16_t refTextureDataF32_R16[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16384, 32768, 49151, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535 };
  static const int16_t refTextureDataF32_R16_SNORM[] = { -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -24576, -16384, -8192, 0, 8191, 16383, 24575, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767 };
  static const float refTextureDataF32_R32F[] = { -4294967296.0f, -3221225472.0f, -2147483648.0f, -1073741824.0f, -65536.0f, -49152.0f, -32768.0f, -16384.0f, -256.0f, -192.0f, -128.0f, -64.0f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 64.0f, 128.0f, 192.0f, 256.0f, 16384.0f, 32768.0f, 49152.0f, 65536.0f, 1073741824.0f, 2147483648.0f, 3221225472.0f, 4294967296.0f };

  struct InputData
  {
    const void *data;
    size_t numElements;
  };

  static const InputData inputDatas[] = {
    { textureDataU8, ARRAY_SIZE(textureDataU8) },
    { textureDataI8, ARRAY_SIZE(textureDataI8) },
    { textureDataU16, ARRAY_SIZE(textureDataU16) },
    { textureDataI16, ARRAY_SIZE(textureDataI16) },
    { textureDataU32, ARRAY_SIZE(textureDataU32) },
    { textureDataI32, ARRAY_SIZE(textureDataI32) },
    { textureDataF32, ARRAY_SIZE(textureDataF32) }
  };

  static const void *refDatas[][11] =
  {
    {
      refTextureDataU8_R8,
      refTextureDataU8_R8_SNORM,
      refTextureDataU8_R16,
      refTextureDataU8_R16_SNORM,
      refTextureDataU8_R32F,
      refTextureDataU8_R8I,
      refTextureDataU8_R8UI,
      refTextureDataU8_R16I,
      refTextureDataU8_R16UI,
      refTextureDataU8_R32I,
      refTextureDataU8_R32UI,
    },
    {
      refTextureDataI8_R8,
      refTextureDataI8_R8_SNORM,
      refTextureDataI8_R16,
      refTextureDataI8_R16_SNORM,
      refTextureDataI8_R32F,
      refTextureDataI8_R8I,
      refTextureDataI8_R8UI,
      refTextureDataI8_R16I,
      refTextureDataI8_R16UI,
      refTextureDataI8_R32I,
      refTextureDataI8_R32UI,
    },
    {
      refTextureDataU16_R8,
      refTextureDataU16_R8_SNORM,
      refTextureDataU16_R16,
      refTextureDataU16_R16_SNORM,
      refTextureDataU16_R32F,
      refTextureDataU16_R8I,
      refTextureDataU16_R8UI,
      refTextureDataU16_R16I,
      refTextureDataU16_R16UI,
      refTextureDataU16_R32I,
      refTextureDataU16_R32UI,
    },
    {
      refTextureDataI16_R8,
      refTextureDataI16_R8_SNORM,
      refTextureDataI16_R16,
      refTextureDataI16_R16_SNORM,
      refTextureDataI16_R32F,
      refTextureDataI16_R8I,
      refTextureDataI16_R8UI,
      refTextureDataI16_R16I,
      refTextureDataI16_R16UI,
      refTextureDataI16_R32I,
      refTextureDataI16_R32UI,
    },
    {
      refTextureDataU32_R8,
      refTextureDataU32_R8_SNORM,
      refTextureDataU32_R16,
      refTextureDataU32_R16_SNORM,
      refTextureDataU32_R32F,
      refTextureDataU32_R8I,
      refTextureDataU32_R8UI,
      refTextureDataU32_R16I,
      refTextureDataU32_R16UI,
      refTextureDataU32_R32I,
      refTextureDataU32_R32UI,
    },
    {
      refTextureDataI32_R8,
      refTextureDataI32_R8_SNORM,
      refTextureDataI32_R16,
      refTextureDataI32_R16_SNORM,
      refTextureDataI32_R32F,
      refTextureDataI32_R8I,
      refTextureDataI32_R8UI,
      refTextureDataI32_R16I,
      refTextureDataI32_R16UI,
      refTextureDataI32_R32I,
      refTextureDataI32_R32UI,
    },
    {
      refTextureDataF32_R8,
      refTextureDataF32_R8_SNORM,
      refTextureDataF32_R16,
      refTextureDataF32_R16_SNORM,
      refTextureDataF32_R32F,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
    }
  };

  static const GLenum input_formats[] = {
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    GL_UNSIGNED_INT,
    GL_INT,
    GL_FLOAT,
  };
  static const int num_input_formats = static_cast<int>(ARRAY_SIZE(input_formats));

  static const GLenum internalFormats[] = {
    GL_R8,
    GL_R8_SNORM,
    GL_R16,
    GL_R16_SNORM,
    GL_R32F,
    GL_R8I,
    GL_R8UI,
    GL_R16I,
    GL_R16UI,
    GL_R32I,
    GL_R32UI
  };
  static const int num_internal_formats = static_cast<int>(ARRAY_SIZE(internalFormats));

  for (int i = 0; i < num_input_formats; i++)
  {
    const InputData &inputData = inputDatas[i];
    for (int j = 0; j < num_internal_formats; j++)
    {
      const ConvertFunc convertFunc = getConvertFunc(input_formats[i], internalFormats[j]);
      if (convertFunc != nullptr)
      {
        const size_t outputSize = inputData.numElements * getInternalFormatSize(internalFormats[j]);
        std::vector<uint8_t> outputData(outputSize);
        convertFunc(outputData.data(), inputData.data, inputData.numElements);
        if (memcmp(outputData.data(), refDatas[i][j], outputSize) != 0)
        {
          std::cerr << "Conversion failed for input format " << input_formats[i]
            << " and internal format " << internalFormats[j] << std::endl;
        }
      }
    }
  }
}
