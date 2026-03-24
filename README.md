# Polyray Game Engine [C++ Version]

The C++ Version of Polyray is a major rewrite of the engine architecture and design, but tries to maintain as much of the original version as possible.

Currently, it's in the process of translating over much of the Java version with many features still missing.

### **[Polyray Engine Showcase Video](https://www.youtube.com/watch?v=JZVfSlKjolc)**
#### **[Read the Full Technical Report (PDF)](https://givejavaachance.github.io/PolyrayGameEngine/Polyray%20Game%20Engine%20Report.pdf)**

## Major differences (as of now):
* An actual shader pipeline with shader reflection for automatic VAO creation, no need for VertexBufferTemplate anymore!
* The ECS now owns all* component data with fully cutomizable storage layouts. (custom storages are wip)
* There are now discrete update orders: pre-physics, physics (fixed dt), post-physics, frame update, pre-render, render. This hopefully makes it much easier to register callbacks correctly.

### *There are a couple of files for testing purposes which may be moved or removed later*
Files such as [InstancedRenderSystem.h](https://github.com/GiveJavaAChance/PolyrayGameEngine/blob/polyray-cpp/src/InstancedRenderSystem.h) or [ScriptSystem.h](https://github.com/GiveJavaAChance/PolyrayGameEngine/blob/polyray-cpp/src/ScriptSystem.h) are mostly for figuring out if the ECS works, especially the InstancedRenderSystem. The ScriptingSystem might actually be good and therefore kept, but it might still need to be moved into a separate module in the future.

## Future idea: Project/Module Manager
When using the engine, it's quite hard to get going, tons of groundwork has to be done in order to get a good foundation to build from, which is actually the complete opposite of what the engine is designed for. Due to this, an idea has popped up, which is to focus on the modlarity aspect and create a project manager app which will take care of that. It'd write most of the boilerplate, auto-generate pretty much everything for the project, including adding all selected modules, setting up callbacks etc. and all that would be left by the user is to make the game itself.
