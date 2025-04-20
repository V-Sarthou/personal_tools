#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void gl_message_callback(GLenum source,
                         GLenum type,
                         GLuint id,
                         GLenum severity,
                         GLsizei length,
                         const GLchar *message,
                         const void *userParam)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type, severity, message);
}

int get_gl_type_size(GLenum type)
{
  switch (type)
  {
  case GL_UNSIGNED_BYTE:
    return (int) sizeof(uint8_t);
  case GL_BYTE:
    return (int) sizeof(int8_t);
  case GL_UNSIGNED_SHORT:
    return (int) sizeof(uint16_t);
  case GL_SHORT:
    return (int) sizeof(int16_t);
  case GL_UNSIGNED_INT:
    return (int) sizeof(uint32_t);
  case GL_INT:
    return (int) sizeof(int32_t);
  case GL_FLOAT:
    return (int) sizeof(float);
  default:
    fprintf(stderr, "error: unknown type %d\n", type);
    return -1;
  }
}

void print_gl_format(GLenum format)
{
  switch (format)
  {
  case GL_RED:
    printf("GL_RED");
    break;
  case GL_RED_INTEGER:
    printf("GL_RED_INTEGER");
    break;
  default:
    printf("unknown format %d", format);
    break;
  }
}

void print_gl_internal_format(GLenum internal_format)
{
  switch (internal_format)
  {
  case GL_R8:
    printf("GL_R8");
    break;
  case GL_R8_SNORM:
    printf("GL_R8_SNORM");
    break;
  case GL_R16:
    printf("GL_R16");
    break;
  case GL_R16_SNORM:
    printf("GL_R16_SNORM");
    break;
  case GL_R32F:
    printf("GL_R32F");
    break;
  case GL_R8I:
    printf("GL_R8I");
    break;
  case GL_R8UI:
    printf("GL_R8UI");
    break;
  case GL_R16I:
    printf("GL_R16I");
    break;
  case GL_R16UI:
    printf("GL_R16UI");
    break;
  case GL_R32I:
    printf("GL_R32I");
    break;
  case GL_R32UI:
    printf("GL_R32UI");
    break;
  default:
    printf("unknown internal format %d", internal_format);
    break;
  }
}

void print_gl_type(GLenum type)
{
  switch (type)
  {
  case GL_UNSIGNED_BYTE:
    printf("GL_UNSIGNED_BYTE");
    break;
  case GL_BYTE:
    printf("GL_BYTE");
    break;
  case GL_UNSIGNED_SHORT:
    printf("GL_UNSIGNED_SHORT");
    break;
  case GL_SHORT:
    printf("GL_SHORT");
    break;
  case GL_UNSIGNED_INT:
    printf("GL_UNSIGNED_INT");
    break;
  case GL_INT:
    printf("GL_INT");
    break;
  case GL_FLOAT:
    printf("GL_FLOAT");
    break;
  default:
    printf("unknown type %d", type);
    break;
  }
}

void print_gl_data(const void *data, GLsizei numElements, GLenum type)
{
  printf("[");
  switch (type)
  {
  case GL_UNSIGNED_BYTE:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%hhu, ", ((const uint8_t *) data)[i]);
    break;
  case GL_BYTE:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%hhd, ", ((const int8_t *) data)[i]);
    break;
  case GL_UNSIGNED_SHORT:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%hu, ", ((const uint16_t *) data)[i]);
    break;
  case GL_SHORT:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%hd, ", ((const int16_t *) data)[i]);
    break;
  case GL_UNSIGNED_INT:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%u, ", ((const uint32_t *) data)[i]);
    break;
  case GL_INT:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%d, ", ((const int32_t *) data)[i]);
    break;
  case GL_FLOAT:
    for (GLsizei i = 0; i < numElements; i++)
      printf("%f, ", ((const float *) data)[i]);
    break;
  default:
    fprintf(stderr, "error: unknown type %d\n", type);
    break;
  }
  printf("\b\b]\n");
}

int main(int argc, char **argv)
{
  const GLuint WINDOW_WIDTH = 1024;
  const GLuint WINDOW_HEIGHT = 768;

  glfwInit();

  GLFWwindow *const window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Template OpenGL Project", NULL, NULL);
  glfwMakeContextCurrent(window);
  if (window == NULL)
  {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  const int gl_version = gladLoadGL(glfwGetProcAddress);
  printf("OpenGL %d.%d\n", GLAD_VERSION_MAJOR(gl_version), GLAD_VERSION_MINOR(gl_version));

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_message_callback, 0);

  static const uint8_t texture_dataU8[] = { 0, 1, 64, 128, 192, 255 };
  static const int8_t texture_dataI8[] = { -128, -64, -1, 0, 1, 64, 127 };
  static const uint16_t texture_dataU16[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65535 };
  static const int16_t texture_dataI16[] = { -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32767 };
  static const uint32_t texture_dataU32[] = { 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483648U, 3221225472U, 4294967295U };
  static const int32_t texture_dataI32[] = { -2147483648, -1073741824, -65536, -49152, -32768, -16384, -256, -192, -128, -64, -1, 0, 1, 64, 128, 192, 256, 16384, 32768, 49152, 65536, 1073741824, 2147483647 };
  static const float texture_dataF32[] = { -4294967296.0f, -3221225472.0f, -2147483648.0f, -1073741824.0f, -65536.0f, -49152.0f, -32768.0f, -16384.0f, -256.0f, -192.0f, -128.0f, -64.0f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 64.0f, 128.0f, 192.0f, 256.0f, 16384.0f, 32768.0f, 49152.0f, 65536.0f, 1073741824.0f, 2147483648.0f, 3221225472.0f, 4294967296.0f };

  typedef struct _data_infos
  {
    GLsizei num_elements;
    void *data;
    GLenum input_type;
  } data_infos;

  static const data_infos datas_infos[] = {
    { ARRAY_SIZE(texture_dataU8), texture_dataU8, GL_UNSIGNED_BYTE },
    { ARRAY_SIZE(texture_dataI8), texture_dataI8, GL_BYTE },
    { ARRAY_SIZE(texture_dataU16), texture_dataU16, GL_UNSIGNED_SHORT },
    { ARRAY_SIZE(texture_dataI16), texture_dataI16, GL_SHORT },
    { ARRAY_SIZE(texture_dataU32), texture_dataU32, GL_UNSIGNED_INT },
    { ARRAY_SIZE(texture_dataI32), texture_dataI32, GL_INT },
    { ARRAY_SIZE(texture_dataF32), texture_dataF32, GL_FLOAT },
  };
  static const int num_datas = (int) ARRAY_SIZE(datas_infos);

  typedef struct _format_infos
  {
    GLenum internal_format;
    GLenum format;
    GLenum readback_type;
  } format_infos;

  static const format_infos formats_infos[] = {
    { GL_R8, GL_RED, GL_UNSIGNED_BYTE },
    { GL_R8_SNORM, GL_RED, GL_BYTE },
    { GL_R16, GL_RED, GL_UNSIGNED_SHORT },
    { GL_R16_SNORM, GL_RED, GL_SHORT },
    { GL_R32F, GL_RED, GL_FLOAT },
    { GL_R8I, GL_RED_INTEGER, GL_BYTE },
    { GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE },
    { GL_R16I, GL_RED_INTEGER, GL_SHORT },
    { GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT },
    { GL_R32I, GL_RED_INTEGER, GL_INT },
    { GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT },
  };
  static const int num_formats = (int) ARRAY_SIZE(formats_infos);

  for (int i = 0; i < num_datas; i++)
  {
    const data_infos *const data_info = &datas_infos[i];

    // for GL_FLOAT, skip the integer formats
    const int end_format_index = (data_info->input_type == GL_FLOAT) ? 5 : num_formats;

    for (int j = 0; j < end_format_index; j++)
    {
      const format_infos *const format_info = &formats_infos[j];

      print_gl_type(data_info->input_type);
      printf(" to ");
      print_gl_internal_format(format_info->internal_format);
      printf(" (");
      print_gl_type(format_info->readback_type);
      printf("):\n");

      printf("  input: ");
      print_gl_data(data_info->data, data_info->num_elements, data_info->input_type);

      GLuint texture_id = 0;
      glGenTextures(1, &texture_id);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture_id);
      glTextureStorage2D(texture_id, 1, format_info->internal_format, 256, 1);
      glTextureSubImage2D(texture_id, 0, 0, 0, data_info->num_elements, 1, format_info->format, data_info->input_type, data_info->data);

      const GLsizei data_size = 4094 * data_info->num_elements * get_gl_type_size(data_info->input_type);
      void *const readback_data = malloc(data_size);
      glGetTextureImage(texture_id, 0, format_info->format, format_info->readback_type, data_size, readback_data);

      printf("  output: ");
      print_gl_data(readback_data, data_info->num_elements, format_info->readback_type);
      printf("\n");

      free(readback_data);

      glDeleteTextures(1, &texture_id);
    }
    printf("\n");
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
