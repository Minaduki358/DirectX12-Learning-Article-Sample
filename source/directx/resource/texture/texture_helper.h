#pragma once

struct TexRGBA
{
	unsigned char R, G, B, A;
};

size_t AlignmentedSize(size_t size, size_t alignment);