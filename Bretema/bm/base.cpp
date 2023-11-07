#include "base.hpp"

BM_NO_WARNING_PUSH
BM_NO_WARNING_TYPE_LIMITS

#ifndef STB_IMAGE_IMPLEMENTATION
#    define STB_IMAGE_IMPLEMENTATION
#    define STB_IMAGE_WRITE_IMPLEMENTATION
#    include <stb_image.h>
#    include <stb_image_write.h>
#endif

BM_NO_WARNING_POP