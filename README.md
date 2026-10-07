# Polyray Game Engine

**[Polyray Engine Showcase Video](https://www.youtube.com/watch?v=T_zjOHQ6_jU)**

## Features

* No OOP, inheritance or virtuals, everything is data oriented
* Custom made ECS
  - Supports custom component storage implementations
* 2D and 3D scene systems
* 2D and 3D Verlet integrated physics engine
  - Supports custom collider types
* Rendering features
  - Default materials are PBR
  - Shadow mapping
  - SSAO
  - Bloom
  - Improved Alpha 2 Cover
* Animation system
  - Can animate any member of a component
* Skinning system
* Compile-time scripting system
* GLSL-like math library (prvl)
  - Aims to be as close to real glsl as possible
* GLTF loader
  - Constructs a scene complete with meshes, animations skins, lights and cameras
* Multiplayer
  - Custom packet protocol
  - Has both a client manager and server hosting
* Profiler
  - High resolution profiler using `rdtsc` (~20 cycle overhead)
* Both Windows and Linux compatible


## Work in progress

* Animation system
* GLTF loader
* Skinning system
* Rendering pipeline
* Better multiplayer system
* More rendering features

##

## Images

#### Current status
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(60).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(61).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(62).png" width="300"><br></td>
  </tr>
</table>

*Note: Features shown below are not yet avaliable. They will be made into modules in the near future however*
#### 2D Probe-based RTGI
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(28).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(29).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(30).png" width="300"><br></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(31).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(40).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(41).png" width="300"><br></td>
  </tr>
</table>

#### Heightmap terrain
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(8).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(17).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(19).png" width="300"><br></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(16).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(21).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(32).png" width="300"><br></td>
  </tr>
</table>

#### Voxel raytracing
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(35).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(37).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(38).png" width="300"><br></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(43).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(44).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(45).png" width="300"><br></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(46).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(49).png" width="300"><br></td>
    <td><img src="https://raw.githubusercontent.com/givejavaachance/PolyrayGameEngine/polyray-cpp/examples/images/image%20(50).png" width="300"><br></td>
  </tr>
</table>

*More images can be found in* [examples/images](https://github.com/GiveJavaAChance/PolyrayGameEngine/tree/polyray-cpp/examples/images)