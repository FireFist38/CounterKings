"COUNTER KINGS" Game Design Document

> **Status**: Core loop and systems defined. Prototype-ready for Rounds 1–5.
> **Engine**: Unreal Engine 5 (GASP animation project base)
> **Aesthetic**: Low poly
> **Build Date**: 2026-05-12

---

## 1. Overview

A 1v1 PvP arena game with roguelite progression elements. Players fight across random arenas that refresh every 5 rounds. Between rounds, players access a shop and allocate attribute points to build their character.

---

## 2. Core Loop

```
Round Start → Scavenge/Position → Fight to Death → Post-Round Shop/Level Up → Repeat
```

- **Arena changes every 5 rounds** 
- **Post-round phase**: 60-second timer. Players access Shop, Level Up, and Equipment tabs. Match auto-starts when timer expires or players ready up.

---

## 3. Combat (In-Round)

- Players spawn and scavenge for weapons/items in the arena.
- Physical item drops persist if picked up.
- **Pickup cap**: Players cannot pick up unlimited items to deny the opponent. 
- **Weapon swap rule**: If slots are full, picking up a new weapon auto-drops the currently equipped one. Dropped weapons have physics and remain on the ground for either player to grab.
- Fight to the death.
- Round timer displayed top-center. 
- lose streak bonuses apply to economy. (Lost streak starts at 3 in a row, and scale / multiply up to 5 in a row)
- Both players proceed to post-round phase.

---

## 4. Attributes

Players allocate points post-round. Points held displayed in Level Up tab.

| Attribute | Effect |
|-----------|--------|
| **Health** | Increases max health |
| **Strength** | Increases melee damage |
| **Dexterity** | Increases light melee damage (quicker weapons like daggers, etc) Gun damage |
| **Magic** | Increases magic damage, mana pool |
| **Luck** | Governs item discovery, income gain per round, and shop roll quality |

- Players can invest heavily in Luck to "lose streak" for economy advantage.
- Attribute allocation via `+` and `-` buttons in Level Up tab.

---

## 5. Equipment & Inventory

### 5.1 Slots

| Slot | Count | Holds |
|------|-------|-------|
| Main Hand | 2 | Melee weapons, guns, staves |
| Off Hand | 2 | Shields, tomes, energy devices |
| Armor Set | 1 | Full armor sets with trade-offs |
| Consumable | 1 (quick-use) | Consumable items |
| Abilities | 3 | Active abilities (unlocked progressively) |
| Perks | 10 max | Passive roguelite bonuses |

### 5.2 Armor Sets
- Single slot. Full sets only, no piecemeal armor.
- Sets provide significant bonuses and penalties:
EXAMPLES:
  - **Knight Set**: Bonus armor/health, reduced movement speed
  - **Mage Set**: Boosts magic power, lower health
  - **Rogue Set**: Boosts melee attack power and movement speed, lowers magic defense
- (Additional sets TBD.)

### 5.3 Abilities
- **3 ability slots**.
- Unlock progression:
  - Ability 1: player level 3
  - Ability 2: player level 6
  - Ability 3: player level 9
- Abilities are cooldown and mana based.
- Locked abilities shown as grayed-out icons. Tooltip on attempt: "Unlocked at level X".
- Ability categories: mobility, damage, utility.
- (Specific abilities TBD.)

### 5.4 Perks
- **10 max**.
- Govern roguelite stat modifications and build expression.
- (Perk list and design TBD.)

### 5.5 Weapons
- Main hands: melee, guns, staves.
- Offhands: shields, tomes, energy-type items.
- (Weapon list and stats TBD.)

### 5.6 Consumables
- Quick-use slot. Single button press.
- Can be purchased from shop or found in arena. (Drop logic TBD.)
- (Consumable list TBD.)

---

## 6. Economy

### 6.1 Income Sources

| Source | Amount |
|--------|--------|
| Flat per round | Fixed amount |
| Lose streak | Scaling bonus |

### 6.2 Shop
- **5 items** displayed per shop roll.
- **Slot 1**: Guaranteed weapon.
- **Slots 2–5**: Random from full pool (weapons, armor sets, perks, consumables, abilities).
- **Reroll**: Costs gold. Button in shop UI.
- **Shop quality**: Determined by current round. Round-gated rarity percentages. (Exact percentages TBD.)
- **Luck scaling**: Luck stat modifies shop roll quality percentages.
- **Individual shops**: Each player sees their own shop instance.
- Items display cost and description on hover (type, damage, etc.).
- Items can be sold from inventory.

### 6.3 Shop Rarity Tiers
- Common, uncommon, Rare, Legendary
- (Exact percentage tables per round range TBD.)

---

## 7. User Interface

### 7.1 In-Game HUD

| Element | Position |
|---------|----------|
| Health + Stamina | Top left |
| Gold ("Gold held") | Top right |
| Round # + Time Remaining | Top center |
| Consumable Quick-Use | Top of bottom cluster |
| Off Hand ×2 | Bottom left |
| Main Hand ×2 | Bottom right |
| Ability ×3 | Bottom center |
| Status Effects | Bottom right |

- Stamina governs dodge/sprint. No mana.
- Status effects: icon + timer bar. Max 3 visible.

### 7.2 Post-Round / Pre-Round UI
Three tabs:

| Tab | Content |
|-----|---------|
| **Shop** | 5 items, reroll button, gold display top right, hover descriptions |
| **Level Up** | 5 attributes with `+` / `-`, points held display |
| **Equipment** | Current inventory, drag to configure, sell items |

- Ready-up button on each panel.
- **60-second flat timer** across all rounds. Match auto-starts when timer expires.

### 7.3 Main Menu Flow

```
[Start] → [Player Name Input] → [Quick Match / Ranked / Host Game / Join Game]
[Settings]
[Quit]
```

- **Player Name**: Text input, persists.
- **Quick Match**: Casual matchmaking. Queue → lobby → load.
- **Ranked**: Locked until Level X. (Ranked ruleset TBD.)
- **Host Game**: Create lobby, invite code, configure settings.
- **Join Game**: Enter code, connect.

### 7.4 Lobby
- Both player names visible.
- Ready toggle.
- 10-second auto-start when both ready.
- Host can modify settings: round count (10/15/20), arena type, perks on/off. (Full host settings TBD.)

---

## 8. Progression & Meta

### 8.1 Leveling
- XP gained from: wins, rounds played, performance.
- Level unlocks Ranked mode at threshold Level X. (Exact threshold TBD.)

### 8.2 Ranked
- Tiered system (Bronze, Silver, Gold, etc. — thematically named, TBD).
- Ranked ruleset and differences from casual TBD.

---

## 9. Arena & PCG

- Arena changes every 5 rounds 
---

## 10. Animation & Technical

- Built from **GASP project** for premade robust animation.

---

## 11. Open Questions / TBD

- [ ] Exact pickup cap for arena items
- [ ] Round timer length (scavenge + fight phases)
- [ ] Exact win/lose streak bonus math
- [ ] Shop rarity percentages per round range
- [ ] Full weapon list with stats
- [ ] Full armor set list with stats and trade-offs
- [ ] Full perk list (10 max, design space)
- [ ] Full ability list (3 slots, mobility/damage/utility)
- [ ] Full consumable list
- [ ] Ranked level unlock threshold
- [ ] Ranked tier names and ruleset
- [ ] PCG arena generation parameters
- [ ] Host game configurable settings (full list)
- [ ] XP formula (performance metrics)
- [ ] Drop physics details (despawn timer?)
- [ ] Matchmaking region/party size limits
- [ ] Audio design direction
- [ ] Visual effects for ability unlocks
- [ ] Mercy/catch-up mechanics (explicitly none for now, but confirm)

---

## 12. Design Principles

- Simple arena combat first, roguelite elements second.
- Builds must matter for replayability.
- No mercy rules — economy management is skill expression.
- Low poly aesthetic throughout.
- Information clarity over visual noise in UI.

---

*Document confirmed by designer. Do not add unconfirmed mechanics.*
