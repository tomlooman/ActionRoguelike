# Action Roguelike Game (Course Project)

This project is part of the **[Professional Game Development in C++ and Unreal Engine](https://courses.tomlooman.com/p/unrealengine-cpp/?product_id=3423335&coupon_code=EARLYBIRD20)** Course where we build this project step-by-step.

![Course Header](https://www.tomlooman.com/wp-content/uploads/2021/09/course_heroheader-1.jpg)

Stanford University CS193U Reference Project (Fall '20) **Details: [Course Page](https://www.tomlooman.com/stanford-cs193u/)**

**Current Engine Version: 4.27**

**Use Branch "Lecture29-FinishedProject" for Course-only Code. (Made with UE 4.25)** ('Main branch' includes additional polish and features)

![GitHub Branch Selection Info](https://www.tomlooman.com/wp-content/uploads/2021/01/github_branchesinfo.jpg)

![Combat Example Still](https://www.tomlooman.com/wp-content/uploads/2021/09/ue_course_herobanner_split_small.jpg)

# Project Features
- Third-person Action Character Movement
- **Action System** (similar to Gameplay Ability System in design)
  - Dash Ability (Teleporting via projectile)
  - Blackhole Ability
  - Magic Projectile Attack
  - "Thorns" buff (reflecting damage)
  - Burning Damage-over-time effect
- AttributeComponent (Holding health etc.)
- **SaveGame System** for persisting progress of character and world state.
- Heavy use of Events to drive UI and gameplay reactions.
- Mix of C++ & Blueprint and how to combine these effectively.
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
Powerups
- Powerup pickups to heal, gain credits/actions.
UMG
- Main menu to host/join game
- UI elements for player attributes and projected widgets for powerups and enemy health.
- C++ Localized Text

This project is part of the **[Professional Game Development in C++ and Unreal Engine](https://courses.tomlooman.com/p/unrealengine-cpp/?product_id=3423335&coupon_code=EARLYBIRD20)** Course where we build this project step-by-step.

**Game Assets:** Licensed for use with the Unreal Engine only. Without a custom license you cannot use to create sequels, remasters, or otherwise emulate the original game or use the original game’s trademarks, character names, or other IP to advertise or name your game. (Unreal Engine EULA applies) (Please note this applies to the Game Assets that refer to Epic's Paragon, you can still use the project code and content to build your own Unreal Engine game)