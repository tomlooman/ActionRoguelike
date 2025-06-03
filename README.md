# Action Roguelike C++ Unreal Engine Game

![Course Header](https://i0.wp.com/www.tomlooman.com/wp-content/uploads/2023/05/coursecpp_banner_widenarrow-3.png)

**Main Branch Engine Version: 5.6** <br>

> ### Learn Unreal Engine C++ The Epic Way
> **Want to learn how to build this C++ Game from scratch? Learn more at [Professional Game Development in C++ and Unreal Engine Course](https://courses.tomlooman.com/p/unrealengine-cpp?coupon_code=COMMUNITY15&src=github)**

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

<br>

# Melee Combat System

The game includes Melee attacks for enemy AI behaviors. The melee system builds on the Action System (similar to GAS) and uses Behavior Trees to initiate the logic to run up and perform the melee attack.

**Walkthrough**
- The Enemy's BehaviorTree checks if target (player) is within certain distance, and initiate melee attack sequence (run closer then attack when in attack range)
- **[RogueAction_MinionMeleeAttack](https://github.com/tomlooman/ActionRoguelike/blob/master/Source/ActionRoguelike/AI/RogueAction_MinionMeleeAttack.cpp)** (Action) handles the start/stop of the attack. Runs an AnimMontage with the attack animation. 
- **[RogueAnimationInstance](https://github.com/tomlooman/ActionRoguelike/blob/master/Source/ActionRoguelike/Animation/RogueAnimInstance.cpp)** (AnimBlueprint) contains *OnMeleeOverlap* which the Melee Attack Action listens for.
- **[RogueAnimNotifyState_Melee](https://github.com/tomlooman/ActionRoguelike/blob/master/Source/ActionRoguelike/Animation/RogueAnimNotifyState_Melee.cpp)** (AnimNotify) broadcasts *OnMeleeOverlap* event when an melee overlap is found by running *OverlapMultiByChannel* colision query while the AnimNotify is active.
  - **game.drawdebugmelee 1** to visualize the overlap shape during melee attack.
  - OnMeleeOverlap is handled by the Melee Attack Action to apply Damage to the hit target.

Note: The AnimMontage holds a Melee Attack animation and requires the custom AnimNotify in order to handle the overlap checks.

# Performance & Optimization

## Animation Budget Allocator
 
Animation Budget Allocator plugin for the enemy AI. Define the allocated animation budget using scalability CVAR (**a.Budget.BudgetMs**) inside **DefaultScalability.ini**. View the budgeting debug and profiling information using **a.Budget.Debug.Enabled** and **stat AnimationBudgetAllocator**. The ARogueAICharacter class includes the optional OnReduceAnimationWork callback to allow custom logic to further throttle anim quality when necessary.

You can get a quick overview by checking out the [initial commit](https://github.com/tomlooman/ActionRoguelike/commit/bbf4ea3f1af05d2b3acdbcc3d2312137015d5789). Read more on the [Animation Budget Allocator Docs Page](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine) which contains all the steps to implement this in your own projects.

# Browsing Older Releases

The project is ocassionally updated to keep up with the latest Unreal Engine release. Additionally, new features are sometimes added to the project, often related to new Articles or Tutorials posted on [tomlooman.com](https://tomlooman.com). These will be included on the main branch, on whatever the latest UE version is at the time.

**For C++ Course Students: You may use branch "Lecture29-FinishedProject" for finished course code without additions all the way back to UE4.25**

![GitHub Branch Selection Info](https://www.tomlooman.com/wp-content/uploads/2021/01/github_branchesinfo.jpg)

# Credits

Huge thanks to [tharlevfx](https://www.tharlevfx.com/) for converting all the Paragon Cascade effects to Niagara! Check out his [VFX courses here](https://tharlevfx.gumroad.com/)!

Another big thanks to [Sander van Zanten](https://www.sandervanzanten.nl/) for the audio overhaul in the project!


**Game Assets:** Licensed for use with the Unreal Engine only. Without a custom license you cannot use to create sequels, remasters, or otherwise emulate the original game or use the original game’s trademarks, character names, or other IP to advertise or name your game. (Unreal Engine EULA applies) (Please note this applies to the Game Assets that refer to Epic's Paragon, you can still use the project code and content to build your own Unreal Engine game)
