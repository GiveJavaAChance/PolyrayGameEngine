# Polyray Game Engine

### Key Features:
* Highly modular design with almost all steps of the pipeline being replaceable.
* Built in renderers and audio effects for quick prototyping and getting started.
* Built in PBR lighting with gamma correction and ACES tonemapping.
* Shader Preprocessor for quality-of-life GLSL code features.
* DBR and DCDBR for real-Time acoustics using ShaderEffector.

#### **[Read the Full Technical Report (PDF)](https://givejavaachance.github.io/PolyrayGameEngine/Polyray%20Game%20Engine%20Report.pdf)**

### Upcomming features:
* An algorithm that takes a reference audio and its recorded version (as played in an environment), processes them into samples to use with the DBR or DCDBR, allowing the same environmental effects to be applied to any audio as if it were played in the same location as the reference.
* A modular multiplayer handler for sending and receiving data between clients and a server.

### Dependencies:
Requires LWJGL
