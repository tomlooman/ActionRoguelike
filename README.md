# Project Orion: Co-op Action Rogue-like Sample Game in Unreal Engine

![Course Header](https://tomlooman.com/assets/images/coursecpp_banner_widenarrow-3.png)

Welcome to **"Project Orion", a co-op Action Roguelike Sample Game** made in Unreal Engine 5 and C++. The project is an expansion on the game that you can build during the [Professional Game Development in C++ and Unreal Engine 5](https://tomlooman.com/courses/unrealengine-cpp) Course and is my continued effort at building the most advanced sample game for Unreal Engine, while being easy to understand, learn from and adapt to your own games.

**Main Branch Engine Version: 5.6** <br>

The main branch is a bit of a playground for experimentation of new systems that may eventually turn into a blog post or in one of my courses (C++ or Game Optimization). For example, the projectiles have both Object Pooling mechanism AND an experimental Data Oriented Design approach to projectiles using no Actors at all. This may affect stability and is not always supporting multiplayer yet until the systems stabilize over time. Many of the experimental features are disabled by default using `#define` directives to easily toggle on/off during compilation.

> ### Learn Unreal Engine C++ The Epic Way
> **Want to learn how to build this UE5 C++ Game from scratch? Learn more at [Professional Game Development in C++ and Unreal Engine Course](https://tomlooman.com/courses/unrealengine-cpp)**

# Project Documentation

You can read detailed information about this project and game features on the [Documentation Page](https://tomlooman.com/unreal-engine-sample-game-action-roguelike). This includes references to tutorials and other articles I have written about this project such as how to implement the [Save Game System](https://tomlooman.com/unreal-engine-cpp-save-system/) for your own game.

# Course Project Branches

The game on the main branch has been updated over the years to keep up with the latest Unreal Engine release. Additionally, many new features are added to the project, often related to new Articles or Tutorials posted on [tomlooman.com](https://tomlooman.com). For students following the Unreal Engine C++ Courses, use one of these two branches:

**For C++ Course Students:** 
- **For the original UE4 version:** [Lecture29-FinishedProject](https://github.com/tomlooman/ActionRoguelike/tree/Lecture29-FinishedProject) for finished course code without additions all the way back to UE4.25.
- **For the new UE5.6 version of the course:** [UE5.6-CourseProject](https://github.com/tomlooman/ActionRoguelike/tree/UE5.6-CourseProject) which has a commit URL with each lesson of the C++ course for easy reference.

![GitHub Branch Selection Info](https://tomlooman.com/assets/images/github_branchesinfo.jpg)

# Project Features

- Third-person Action Character Movement
- **Enhanced Input**
- **Action System** (similar to Gameplay Ability System in design)
  - Dash Ability (Teleporting via projectile)
  - Blackhole Ability
  - Magic Projectile Attack
  - "Thorns" buff (reflecting damage)
  - Burning Damage-over-time effect
- AttributeComponent (Holding health etc.)
- **SaveGame System** for persisting progress of character and world state.
- Event-based logic to drive UI and gameplay reactions.
- Mix of C++ & Blueprint and how to combine them effectively.
- **GameplayTags** to mark-up Actors, Buffs, Actions.
- **Multiplayer support** for all features
- GameMode Logic
  - EQS for binding bot/powerup spawn locations.
  - Bot spawning system (bots cost points to spawn, gamemode gains points over time to spend)
  - DataTable holds bot information
  - DataAssets to hold enemy configurations
- **Asset Manager:** Async loading of data assets
- Async loading of UI icons
- AI
  - Minion AI with Behavior Trees (Roam, See, Chase, Attack, Flee/Heal)
  - C++ Custom Behavior Trees Nodes
  - EQS for attack/cover locations by AI
- Powerups
  - Powerup pickups to heal, gain credits/actions.
- UI (UMG)
  - Main menu to host/join game
  - UI elements for player attributes and projected widgets for powerups and enemy health.
  - C++ Localized Text
- Experimental / WIP
  - Aggregate Ticking (Projectiles)
  - Actor Pooling (Projectiles)
- Async Line tracing Example
- PSO Precaching & Bundled PSOs Setup for Windows DX12

# Credits

Huge thanks to [tharlevfx](https://www.tharlevfx.com/) for converting all the Paragon Cascade effects to Niagara! Check out his [VFX courses here](https://tharlevfx.gumroad.com/)!

Another big thanks to [Sander van Zanten](https://www.sandervanzanten.nl/) for the audio overhaul in the project!


**Game Assets:** Licensed for use with the Unreal Engine only. Without a custom license you cannot use to create sequels, remasters, or otherwise emulate the original game or use the original game’s trademarks, character names, or other IP to advertise or name your game. (Unreal Engine EULA applies) (Please note this applies to the Game Assets that refer to Epic's Paragon, you can still use the project code and content to build your own Unreal Engine game)
