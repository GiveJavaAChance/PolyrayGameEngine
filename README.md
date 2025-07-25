# Polyray Game Engine

## Key Features:
* Highly modular design with almost all steps of the pipeline being replaceable.
* Vertex buffer templates for ultimate reusability and simpler VAO setup.
* Built in renderers and audio effects for quick prototyping and getting started.
* Built in PBR lighting with gamma correction and ACES tonemapping.
* Shader Preprocessor for quality-of-life GLSL code features.
* DBR and DCDBR for Real-Time acoustics using ShaderEffector.

#### **[Read the Full Technical Report (PDF)](https://givejavaachance.github.io/PolyrayGameEngine/Polyray%20Game%20Engine%20Report.pdf)**

## Future Features:
* Discord Social SDK bindings!
* A Modular Entity system.
* A Modular Particle system.
* A Modular item system.
* P2IN parser for items.
* Mod support.
* Moddable Property class for items.
* Gizmos drawing.
* Local and Global ID manager for multiplayer.
* Extremely fast text renderer.
* Noise functions such as "Inigo Quilez noise" and Voronoi path noise.

These features will first be tested and expanded upon in the test-branch

## Dependencies:
Requires LWJGL

## Core Idea

This engine is a proof-of-concept that was built to prove that modularity, flexibility and simplicity can all coexist, even at scale. No need for a overcomplicated mess or a massive bloat.

Most engines compromise between being *flexible but bloated* or *simple but rigid*, not both. This engine takes a different path. By unifying concepts that are technically identical under the hood and exposing only what matters, it allows you to:
* Swap between 2D and 3D rendering with a couple of lines instead of treating 2D and 3D as completely separate things that can't be mixed without major changes.
* Derive new shaders using a powerful preprocessor (#append (similar to C's #include), #override (coming soon) etc.), no hardcoded logic.
* Compose your rendering pipeline by opting in only to the parts you need.
* Treat VBOs, SSBOs and UBOs as a unified ShaderBuffer with a unified interface and zero duplication.

You can strip all built in features away and make your own custom pipeline from scratch using Polyray as just a framework. That's the point, you're in full control.

### Design Through Consistency

This engine was not made to be a patchwork of tools. It was made to be one singular, elegant system, where naming, architecture, and behavior reflect the same philosophy across every module.

Most engines abstract performance away. Polyray exposes it subtly, not as punishment, but as education. Concider this:

> “Why am I pushing and popping text every frame? I can just push and pop whenever it updates!”

This design naturally encourages self-optimization thinking. A dopamine hit at every optimization.

*Many people say* "don't reinvent the wheel."
But that often leads to patchworks of tools bolted together where one system might have completely different naming conventions than another, that makes development frustrating and slow.

Developers end up spending more time figuring out the engine than actually using it.

But this engine was built from scratch, reinventing many core systems to ensure a unified naming convention and consistent design.

### Philosophy

> "An idiot admires complexity, a genius admires simplicity" - Terry Davis

This engine embraces that philosophy. It strives for simplicity without giving up power or customization. It doesn't shy away from advanced or complex techniques, it just makes them fully accessible without forcing them on you.

You can take one step at a time and dig the hole deeper when you feel like it, it doesn't go from super simple to super complex either.

Polyray isn't just an engine to be used, it's meant to teach, to inspire better design and to help developers think clearly about architecture. By unifying rendering, modularity, and abstraction under one consistent system, the engine encourages developers to explore. This exploration is key to understanding, not just using.

When you learn by building from the ground up by experimenting and exploring, you create mental building blocks. From logic gate circuits, to CPU architecture, to machine code, to high-level languages, algorithms, and rendering pipelines. Each step gives you a more accurate mental picture of what's happening under the hood. Those building blocks and tools unlock entirely new ways of solving problems.

By encouraging learning through exploration and architectural freedom, it helps you:
* See patterns between seemingly separate systems.
* Invent your own creative solutions instead of searching or copying.
* Understand what things do, but also how and why they do it.
