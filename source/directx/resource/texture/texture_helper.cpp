#include"texture_helper.h"

size_t AlignmentedSize(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}