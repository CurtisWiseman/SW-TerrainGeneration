#include <vector>

class Perlin {
	// Permutation vector
	std::vector<int> p;

public:
	Perlin();

	// Generate a permutation vector
	Perlin(unsigned int seed);

	// Get a noise value
	double octaveNoise
	(double x, double y, double z, int octaves, 
		double persistence);

private:
	double noise(double x, double y, double z);
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);
};