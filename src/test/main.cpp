#define MODE 0

#if MODE == 0
#include <test/test-programs/Physics3DTest.h>
#elif MODE == 1
#include <test/test-programs/LidarGame.h>
#elif MODE == 2
#include <test/test-programs/VoxelRaytracer.h>
#endif