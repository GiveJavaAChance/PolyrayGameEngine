#define MODE 2

#if MODE == 0
#include <test/test-programs/Physics3DTest.cpp>
#elif MODE == 1
#include <test/test-programs/LidarGame.cpp>
#else
#include <test/test-programs/VoxelRaytracer.cpp>
#endif