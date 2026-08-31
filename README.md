# Polyray Game Engine [C++ Version]

The C++ Version of Polyray is a major rewrite of the engine architecture and design, but tries to maintain as much of the original version as possible.

Currently, it's in the process of translating over much of the Java version with many features still missing.

**[Polyray Engine Showcase Video](https://www.youtube.com/watch?v=JZVfSlKjolc)**

## Major differences (as of now)

* An actual shader pipeline with shader reflection for automatic VAO creation, no need for VertexBufferTemplate anymore!
* The ECS now owns all* component data with fully cutomizable storage layouts. (custom storages are wip)
* There are now discrete update orders: pre-physics, physics (fixed dt), post-physics, frame update, post-frame. This hopefully makes it much easier to register callbacks correctly.

## Currently working on

* Rendering pipeline.
* Audio engine.
* General design and arcitectural improvements.

##

## Future idea: Project/Module Manager

When using the engine, it's quite hard to get going, tons of groundwork has to be done in order to get a good foundation to build from, which is actually the complete opposite of what the engine is designed for. Due to this, an idea has popped up, which is to focus on the modularity aspect and create a project manager app which will take care of that. It'd write most of the boilerplate, auto-generate pretty much everything for the project, including adding all selected modules, setting up callbacks etc. and all that would be left by the user is to make the game itself.
