# Rounds-External
FOR EDUCATIONAL AND RESEARCH PURPOSES ONLY

A simple external mod menu made for the popular game "ROUNDS" 

The goal of this project was to demonstrate knowledge of Windows API functions in order to sucessfully enumerate running programs, enumerate loaded modules, and modify values in memory related to the game.

The GUI was creating using the ImGui API, and game attributes were found using a combination of ReClass, Cheat Engine, and Unity Explorer.

## **Offsets**

|  Object  |  Offset  |
|-----------------|----------|
| Player Entity | 0x1CB3A80->0x8->0x50->0x48->0x40->0xF8->0xA0 |
| Health | Player Entity->0x108 |
| Move Speed | Player Entity->0x80->0x88 |
| Respawn Count | Player Entity->0x80->0xB8 |
| Infinite Block | Player Entity->0x78->0xB4 |
||
| Gun Entity | Player Entity->0x88->0x28 |
| Reload Speed | Gun Entity->0x78, 0x74 |
| Damage | Gun Entity-> ?? |
| Knockback | Gun Entity-> ?? |
| Fire Rate | Gun Entity->0xC0 |
| Bullet Speed | Gun Entity->0xC4 |
| Bullet Bounce | Gun Entity->0xFC |
| Bullet Size | Gun Entity->0xE8 |
| Wallbang | Gun Entity->0x15A |
| Ammo Count | Gun Entity->0x78->0x68 |

## **What To Do Better**

This was my first ever go at something like this, so the logic is by no means perfect nor is some of the coding strategies you may find. Here is a list of things I will either fix or do better for next time:
- Declare the offsets for the gun object and the player object, then declare offsets relative to those objects.
- Be able to handle the game not being open, or match not started. (Don't check every frame, or if you do, handle the game not being open).

## **What To Do Next**
- Implement an advanced technique such as thread hijacking
- Implement some sort of ESP (probably will be a different game)
