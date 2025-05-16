typedef unsigned int u32;

#define global_variable static;
#define internal static;

// Prevent the pixels from being rendered outside of the window range.
inline int clamp(int min, int val, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}