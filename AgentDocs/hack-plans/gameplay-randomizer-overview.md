# Gameplay Randomizer Hack Plan

## Summary
Build a per-save deterministic gameplay randomizer on top of `pokeemerald-expansion` stable `1.16.2`, keeping Emerald's map/story for now. Each save gets one randomizer seed; the same species always has the same randomized ability and learnset within that save, including after evolution.

## Architecture Decisions
- Randomized data is resolved at runtime from the save seed. A build-time generated
  table cannot vary between save files.
- Randomization uses a stateless hash, not the game's mutable RNG stream. A result is
  keyed by the save seed, a versioned category, and explicit context values. Looking
  up one result must not affect any other result.
- Category identifiers and hash behavior are save-format API. Once released, changing
  either requires incrementing the randomizer algorithm version.
- Version 2 saves store a nonzero `u32 randomizerSeed` and a randomizer algorithm
  version in `SaveBlock3`. New-game initialization creates both values. Version 2
  intentionally replaces the unreleased version-1 learnset mapping with bucketed
  move selection; pre-version-2 development saves are not supported. Save
  compatibility with unmodified Expansion saves is not required for the initial
  romhack release.
- A single eligibility helper will define the general species pool. It must require an
  enabled, real Pokemon species and exclude Mega Evolutions, Primal Reversions, Ultra
  Burst forms, Gigantamax forms, Tera forms, Totem forms, and other forms found to be
  unusable outside their special context. Feature-specific filters may narrow this
  pool further.
- Evolution-family identity and randomized ability resolution are implemented
  through the shared deterministic family policy documented in the handoff.
- Randomized learnsets resolve at runtime on `romhack/main`. The version-1 move
  pool, weighting, duplicate policy, schedule, and access seam are documented in
  the implementation handoff. Add a broader cache only if profiling shows it is
  needed.

## Roadmap and Task List

Status in this list is authoritative for phase-level planning. Detailed completed
behavior and validation belong in `AgentDocs/randomizer-implementation.md`.
The current release branch is `romhack/nuzlocke-1.1.0`; implemented there does
not mean merged into `romhack/main`.

Current execution order:

Manual gameplay checks remain recorded in `AgentDocs/manual-validation-checklist.md`,
but they are not the next agent work item unless specifically requested. Continue
adding new review and validation tasks to that checklist as implementation work
uncovers them.

1. Phase 10's implemented QoL set includes level-to-cap, Portable Healer, Repel
   Toggle, global Set battle style, numeric IV summary, badge HM actions, Oldale
   supplies, hidden-item removal, and trainer AI tiers. Remaining work is manual
   gameplay and broader battle-scenario validation, not another implementation
   layer for those features.
2. Phase 11 implementation and correctness review are complete on
   `romhack/nuzlocke-1.1.0`: visible item balls, eligible direct gifts, the
   64-slot Items pocket, 50 unique randomized TM moves, protected transactions,
   atomic vendor/reward handling, and the case-by-case special-reward audit are
   committed. The consolidated manual checklist remains the release gate.
3. Battle and item EV gains are disabled. The Route 117 Day Care entrance is
   blocked, Shoal Cave is permanently low tide, and the documented finite-reward
   redesigns are implemented. Full engine-level breeding/egg cleanup and broader
   Phase 12 streamlining remain future work.
4. Do not merge the release branch into `romhack/main` until the user explicitly
   approves it. If implementation resumes before that decision, prioritize
   clearly bounded Phase 12 work or defects found by manual validation.

1. Foundation — complete on `romhack/main`:
   - Add a master compile-time config gate, enabled for this hack.
   - Store and initialize the per-save seed and algorithm version.
   - Add a pure, deterministic, category-separated hash API.
   - Add the shared species eligibility helper.
   - Add focused unit tests for initialization, determinism, category/key separation,
     and representative eligible/ineligible species.
2. Encounters — complete on `romhack/main`; manual gameplay/performance checks remain:
   - Specify which normal, fishing, rock-smash, outbreak, Feebas, scripted, and
     DexNav paths are randomized.
   - Implement a deterministic eligible-species pool and hook the agreed paths after
     vanilla slot and level selection.
   - Scale the eligible encounter pool by route difficulty so low-level areas draw
     from lower-BST species and high-level areas draw from higher-BST species.
   - Derive the difficulty band from stable encounter-table data rather than the
     mutable level roll, preserving deterministic slot mappings.
3. Legendary encounters — complete on `romhack/main`; manual gameplay checks remain:
   - Route ordinary `setwildbattle` encounters through the regular BST-scaled
     encounter resolver using their fixed scripted level.
   - Randomize scripted/static encounters only when the original species is a
     Legendary or Paradox Pokemon through the dedicated special pool.
   - Select replacements from a dedicated pool containing only enabled, usable
     restricted Legendary, sub-Legendary, and Paradox species.
   - Exclude Mythical Pokemon and Ultra Beasts unless the policy is deliberately
     expanded later.
   - Preserve the scripted encounter's level, battle setup, flags, and progression
     behavior while replacing only its species.
   - Manually verify the Emerald Legendary encounters, including capture/defeat
     flags and repeat-entry behavior.
4. Level caps and EV removal — complete on `romhack/main`; manual gameplay checks
   remain:
   - Enable the existing hard Emerald flag-based level-cap system.
   - Prevent Rare Candies and EXP Candies from exceeding the active cap.
   - Disable battle EV gain and prevent EV-boosting items from bypassing the
     zero-EV cap.
   - Manually verify battle experience, cap progression, candy behavior, and all
     EV-changing item paths before building the shared level-to-cap action on these
     rules.
5. Starters — complete on `romhack/main`; manual gameplay checks remain:
   - Deterministically select three distinct starter choices for each save.
   - Limit choices to enabled, generally usable, non-special base-stage Pokemon
     with BST 300-350 and a complete three-stage evolution line.
   - Resolve all player-facing starter uses through `GetStarterPokemon` so the
     selection UI, granted Pokemon, party checks, and credits agree.
   - The rival is not required to choose or retain one of these species; rival
     parties follow the enemy-trainer policy below.
   - Manually verify selection labels, sprites, cries, confirmation, the granted
     Pokemon, party checks, and save-to-save variation in gameplay.
6. Enemy trainer parties — complete on `romhack/main`; manual gameplay checks
   remain:
   - Randomize ordinary enemy party species per configured trainer encounter.
   - Key each slot by the save seed, algorithm version, trainer ID, and party slot
     so the same encounter is stable within a save but differs across saves.
   - Treat separate rival trainer IDs as separate encounters. Rival teams may be
     unrelated between story battles; no persistent rival evolution line is
     required.
   - Preserve party size and levels, and select replacements near each original
     species' BST to retain approximate difficulty.
   - Use an inclusive original-BST ±50 preferred pool, expanding outward by 50
     only when that pool is empty.
   - Regenerate legal moves and validate abilities, held items, and gimmicks
     against the replacement species.
   - Initially exclude Battle Frontier, Trainer Hill, e-Reader, Secret Base, and
     player-controlled trainer parties.
   - Manually verify ordinary, rival, boss, override, and pooled trainers;
     save-to-save variation; repeated-encounter stability; and held-item/gimmick
     behavior.
7. Abilities — complete on `romhack/main`; manual gameplay checks remain:
   - Resolve one deterministic ability ID per enabled evolution-graph family.
   - Route normal party, box, battle, UI, and overworld reads through
     `GetAbilityBySpecies` while preserving the saved authored ability slot.
   - Preserve entire families containing form-driving abilities and exclude
     form/signature-only, Wonder Guard, placeholder, and unimplemented abilities
     from the version-1 candidate pool.
   - Cache family identities and resolved family abilities in EWRAM.
8. Learnsets — complete on `romhack/main`; manual gameplay checks remain:
   - Allow every enabled real Pokemon species to learn every configured TM while
     preserving authored HM and move-tutor compatibility.
   - Give each species two starting moves at level 1, then front-load new moves
     across the configured cap bands while retaining the normal 20-move table
     capacity. Keep only five total learned moves available by level 15, and move
     the two displaced early learn slots to post-eighth-gym and post-team-leader
     pre-Elite Four levels. Deterministically replace each move per save, species,
     and slot without duplicates.
   - Select those two starting slots through a dedicated aggressive opening
     bucket: strongly favor damaging moves at 40 power or below, retain only a
     small weight for 41-60 power, reject stronger damaging moves, and allow only
     basic-tier status moves. Resume the normal level-scaled buckets afterward.
   - Weight damaging moves toward a level-scaled target power and shift status
     weighting from basic to strong and elite effects as levels rise. Keep every
     eligible tier possible at nonzero weight, use a 4x STAB multiplier versus 2x
     coverage, and scale status-tier weights proportionally to retain their
     approximate relationship to STAB. Exclude unsuitable species-locked moves
     and retain the authored move as an empty-pool fallback. Saturate
     power-distance penalties at the minimum weight without unsigned subtraction
     so extreme-power moves remain rare instead of overflowing the weighted pool.
     When reviewing changes to this weighting, audit detrimental moves separately
     from raw power: `IsExplosionMove` moves, recoil-heavy attacks, fixed-damage
     cases, and other severe-drawback attacks should not become favored simply
     because their listed power is near the level target. Include Misty Explosion
     as the late-game sentinel case.
   - Resolve level-up, evolution, reminder, AI, and Pokedex paths through the
     shared full-table accessor. Initial move assignment generates the same
     deterministic sequence only through the Pokemon's current level and keeps a
     rolling window of the last four moves, avoiding work on future-level slots.
     Build a compact move-bucket index once from Expansion's authoritative
     `gMovesInfo` metadata, grouping damaging moves by exact type and normalized
     power and status moves by potency tier. Runtime selection weighs these
     buckets and subtracts prior selections instead of repeatedly scanning and
     reclassifying the complete move table. Only add a broader cache after
     measuring the remaining access cost.
   - Future move-bucket valuation task: derive an effective power used only for
     randomizer bucketing; never modify the move's real battle power. For a
     damaging move that beneficially raises the user's stats or lowers the
     target's stats, add one 10-power bucket per affected stat stage. A one-stage
     change adds 10, a "sharply" two-stage change adds 20, and multi-stat effects
     sum their beneficial stages. Give attacks that normally require two turns a
     one-bucket / 10-power penalty. Before implementation, audit the complete
     effect table and define comparable modifiers for activation chance,
     self-stat drops, recharge turns, recoil, self-KO, drain, priority,
     multi-hit/fixed-damage behavior, accuracy, weather bypasses, and damaging
     status effects. Avoid double-counting a drawback already handled by another
     weighting rule. Use the adjusted value consistently when constructing
     damaging-move buckets so TM and learnset selection share the classification,
     while retaining their existing feature-specific type, level, status-tier,
     and drawback multipliers. Add focused boundary tests for +1, +2, multi-stat,
     and two-turn adjustments plus representative mixed-benefit/drawback moves.
   - Do not expand egg-move support while breeding is planned for removal. Revisit
     egg moves only if a non-breeding acquisition path is retained or added.
9. Evolution rules — complete on `romhack/main`; manual gameplay checks remain:
   - The authoritative species-level conversion ledger is
     `AgentDocs/hack-plans/evolution-rules.md`.
   - Replace friendship evolutions from an explicit species-by-species conversion
     table, preserving applicable secondary conditions.
   - Make time-dependent and alternate-form evolution lines practical in a short
     Nuzlocke: replace day/night dependencies and deterministically randomize the
     available branch or form, including regional forms, under an explicit
     species-by-species policy.
   - Replace move requirements and audited inaccessible or grind-heavy special
     conditions with explicit levels, remove location and trade requirements,
     and consolidate item evolutions into standard stones or the Linking Cord.
     Preserve player choice for stone-based branches and the intentional
     Shedinja Poké Ball requirement.
10. Quality of life — partially complete on `romhack/main`:
   - Add a button to the party menu and Pokemon Storage that raises a selected
     Pokemon to the current level cap without requiring a boxed Pokemon to be moved
     into the party first. Process the intervening levels in order so every
     move-learning opportunity and evolution check occurs normally rather than
     jumping directly to the final level.
   - The Pokemon Skills summary page now uses the A button to toggle between the
     normal stats view and all six numeric stored IV values. Pressing A again
     returns to normal stats.
   - The Repel Toggle Key Item is implemented. It enables or disables an
     indefinite block on standard random encounters regardless of level and is
     granted with the starter.
   - Badge-authorized HM field actions are implemented on
     `romhack/qol-badge-hms`. Each action remains locked behind its corresponding
     badge but no longer requires a party Pokemon to know the HM move.
   - An Oldale Town NPC grants 200 Ultra Balls and ¥200,000 once as an early-game
     supply package on `romhack/qol-oldale-supplies`.
   - Hidden item spots are disabled on `romhack/qol-remove-hidden-items`: they
     cannot be collected or detected with the Itemfinder, so all obtainable
     overworld pickups are visibly represented to the player.
   - The Portable Healer Key Item is implemented and granted with the starter. It
     fully restores every party Pokemon outside battle without being consumed.
   - Battle style is globally locked to Set on `romhack/qol-no-free-switch`.
     Replacement Pokemon enter before the player can make a free switch, and the
     obsolete Battle Style option is removed from the Options menu.
   - Trainer AI flag assignment review is implemented. Ordinary trainers use
     Basic Trainer, Smart Mon Choices, and randomized equivalent switch-ins;
     boss-class trainers additionally use Smart Switching, restrained STAB/status
     assumptions, and PP-stall prevention. Authored Risky behavior is retained,
     facilities and player-controlled partners keep their existing policy, and
     randomized abilities remain unknown until revealed. Focused resolver and
     ability-awareness tests pass; manual battle checks remain.
11. World items, direct gifts, and TMs — implemented; validation and audit remain:
   - Expand the general Items pocket from 30 to 64 distinct-item slots so the
     randomized useful-item pool does not create excessive inventory pressure.
     Keep the 999-per-item stack limit and all other pocket capacities unchanged.
     This is an intentional `SaveBlock1` layout change and does not replace
     proper Bag-full handling for grants or exchanges.
   - The implemented first slice randomizes ordinary visible item-ball pickups
     using a stateless hash keyed by save seed, map group, map number, object
     index, and authored item. The same pickup is stable within a save, while
     other identities or seeds can differ.
   - Use one pure-random useful-item pool. Duplicates are allowed, and no stone,
     Linking Cord, or other evolution resource is guaranteed to appear before
     it is needed. Do not add pickup counters, duplicate tracking, new save data,
     runtime map scans, progression bands, or new pickup locations.
   - Exclude Poke Ball variants, HMs, TMs as replacement items, and pure money
     rewards. Pure money rewards include Nuggets, Mushrooms, Pearls, Pearl
     String, Stardust, Star Piece, Comet Shard, Rare Bone, and equivalent
     vendor-only treasures. Do not exclude every item with a sale price when it
     has independent utility; audit Heart Scales, shards, Honey, Bottle Caps,
     and similar mixed-use items separately.
   - Authored Poke Ball and pure-money pickups become useful randomized rewards.
     Authored key items and HMs remain unchanged. Authored TMs remain TM items
     and use the separate TM mapping policy below. Invalid special templates and
     hidden items remain unchanged or disabled respectively.
   - The useful pool contains 11 evolution items and 56 held items: standard
     evolution stones, Linking Cord, independently useful former evolution-held
     items, all 18 type boosters, the Choice trio, Focus/Band items, accuracy and
     critical-hit lenses, lower-tier utility held items, and weather, terrain,
     and screen extenders, and all four terrain seeds. Keep
     medicine, X-items, berries, and other ruleset-sensitive rewards out until
     separately approved. Exclude retired species-specific evolution items unless
     they retain independent battle utility.
   - TMs retain the existing 50 item slots but will be assigned 50 unique moves
     per save from the complete implemented practical move table. Selection will
     favor high-power damaging buckets and high-tier status moves while keeping
     lower-power and detrimental moves possible but penalized for severe
     drawbacks. TMs must use the randomized mapping in item descriptions, bag
     use, compatibility, Move Reminder, Pokedex, reverse lookups, and debug
     helpers. HMs retain authored move mappings and compatibility.
   - Emerald visible item balls and eligible direct NPC/story gifts use
     deterministic save-seeded resolvers. Key items, HMs, TM slots, and berries
     remain authored. Shops, exchanges, prize tables, Battle Pyramid generated
     items, FRLG content, and internal item transfers remain outside this
     randomizer. Consider new visible optional rewards only after distribution
     review; never add hidden item spots.
   - The Mt. Chimney Lava Cookie vendor is an intentional exception to the
     general shop exclusion. It sells one deterministic randomized useful item
     for ¥200 and may be used repeatedly. Because its identity is keyed by the
     vendor's stable map and local object context, every purchase within a save
     yields the same replacement item; it is not rerolled per transaction.
   - The Seashore House Soda Pop vendor is a second intentional exception after
     the three-trainer challenge unlocks it. It sells one deterministic
     randomized useful item repeatedly for ¥300. Its stable map and owner local
     object identity keep the replacement fixed within a save rather than
     rerolling each purchase. The one-time challenge reward deliberately grants
     six copies of one deterministic randomized item. These are temporary
     retained behaviors: a later streamlining pass will disable the three house
     trainers, their challenge reward, and the unlocked vendor together.
   - Trick House is an approved challenge-reward exception to the general prize
     table exclusion. Each of its seven one-time puzzle rewards retains its
     authored reward identity as deterministic context: eligible non-TM prizes
     resolve to useful randomized items, while the authored TM slot remains that
     TM item and teaches its save-randomized move. Rewards are independently
     mapped and may duplicate one another.
   - The Route 120 Berry NPC gives exactly one authored Berry per save, selected
     from Figy, Wiki, Mago, Aguav, or Iapapa by the player's Trainer ID as in the
     original logic. A permanent received flag replaces the daily reset flag.
     The flag is set only after the Berry enters the Bag, preserving retry
     behavior when the Berry pocket is full.
   - Sootopolis Kiri gives her authored two-Berry bundle exactly once per save:
     one random Berry from Pomeg through Nomel and either Figy or Iapapa. Before
     granting either item, a preflight verifies that the Berry pocket can hold
     the complete pair. The permanent received flag is set only after both
     grants succeed, preventing partial rewards or daily farming.
   - Berry Master's wife no longer uses the Easy Chat phrase minigame. She gives
     one authored random Berry per save from the five former special-phrase
     rewards: Spelon, Pamtre, Watmel, Durin, or Belue. A permanent received flag
     is set only after the item enters the Bag, so a full Berry pocket remains
     safely retryable.
   - The Sootopolis Seedot/Lotad brothers retain their size judging and record
     displays as flavor content, but no longer grant items for qualifying new
     records. This removes the repeatable randomized reward source while leaving
     the optional comparison interaction intact.
   - The Slateport Fan Club chairman retains his five condition assessments and
     awards the authored Red, Blue, Pink, Green, or Yellow Scarf once for the
     matching condition. Received flags are set only after successful Bag
     insertion. These Contest-only rewards and the assessment interaction are
     slated for removal with the Contest system during streamlining.
   - Mirage Tower is always visible until its fossil event is completed, after
     which the existing collapse and gone states remain authoritative. The
     player chooses the authored Root Fossil or Claw Fossil, and the existing
     choice flags continue to determine the later Desert Underpass fossil. A
     future broader fossil choice is deferred until its presentation, revival,
     and remaining-fossil behavior are designed together.
   - Desert Underpass grants the authored fossil not selected in Mirage Tower,
     preserving the existing Root/Claw choice flags and full-Bag retry behavior.
     Its postgame unlock remains unchanged, so the recovered fossil is not part
     of the primary Nuzlocke route.
   - E-Reader Enigma Berry delivery grants the downloaded custom Enigma Berry
     item directly. Its validation, ownership checks, availability state, and
     full-Bag retry semantics remain authored and are not randomizer inputs.
   - During streamlining, audit every E-Reader, Mystery Gift, and external-event
     item or unlock that lacks a viable standalone acquisition path. Explicitly
     cover the Aurora Ticket and Birth Island/Deoxys, Mystic Ticket, Old Sea Map,
     Eon Ticket, and comparable distributions. Give each an intentional in-game
     unlock, enable it through progression, or remove it instead of leaving its
     content silently inaccessible.
   - Perform a dedicated whole-game vendor audit in a later item-balance pass.
     Review each ordinary shop, specialty vendor, repeatable seller, exchange
     counter, and prize shop individually before deciding whether it remains
     authored, receives a deterministic randomized inventory, or is removed by
     streamlining. Record price, repeatability, progression dependencies, Bag
     pocket behavior, and the risk of unlimited access to high-value randomized
     items. The Mt. Chimney and Seashore House decisions do not automatically
     authorize randomization of any other vendor.
12. Streamlined game progression — planned:
   - Minimize mandatory grinding so a viable party can stay near each active level
     cap through normal trainer battles and exploration.
   - Shoal Cave is intentionally locked to low tide so its complete low-tide route
     and item pickups, especially TM07, remain continuously accessible. This is a
     gameplay redesign rather than a clock workaround: the recurring Shoal
     Salt/Shoal Shell collection and Shell Bell exchange have been retired. The
     four reachable former Shoal Salt spots are one-time randomized useful-item
     pickups, while the former high-tide Shoal Shell spots are inaccessible. The
     old man only comments on the permanent low tide and no longer checks or
     resets ingredients. Shoal Salt and Shoal Shell are not members of the
     randomizer replacement pool. Consider disabling or bypassing Shoal Cave
     entirely in a later streamlining pass after relocating any rewards that
     remain important.
   - Remove breeding as a required or supported progression system; audit the Day
     Care, eggs, inherited moves, and breeding-only rewards or encounters so nothing
     important depends on breeding.
   - Remove berry planting, watering, growth, and harvesting as a supported
     progression mechanic. Audit berry plots, related NPCs and tutorials, and
     berry-dependent rewards or encounters; provide direct, finite acquisition
     paths for any berries that remain important to Nuzlocke gameplay. If
     harvesting remains supported, consider a separate berry-only harvest pool;
     never mix berries into the general useful-item pool.
   - As part of that berry rework, disable or retire berries with no held-item
     effect: Razz, Bluk, Nanab, Wepear, Pinap, Pomeg, Kelpsy, Qualot, Hondew,
     Grepa, Tamato, Cornn, Magost, Rabuta, Nomel, Spelon, Pamtre, Watmel, Durin,
     and Belue. Remove their planting and harvest paths, audit Pokeblock and NPC
     dependencies before removing authored grants, and keep the e-Reader Enigma
     Berry special slot out of ordinary distribution. Define direct finite
     acquisition for the remaining battle-useful held berries.
   - Randomize finite berry acquisition separately from the general useful-item
     pool. Convert eligible authored berry trees, pickups, and direct berry gifts
     into deterministic results drawn only from the retained battle-useful berry
     set, keyed by save seed and stable source identity. Duplicates may remain
     possible, but planting and harvesting must not provide renewable rerolls.
     Preserve scripted berries when a specific berry is required for progression
     or a retained reward until that dependency is explicitly replaced.
   - Complete the no-EV experience by removing or repurposing EV-focused items,
     rewards, dialogue, and UI that no longer provide useful choices.
   - Retire Berry Powder as a supported system in a later streamlining pass. For
     now, preserve the authored Powder Jar and vendor purchases rather than
     randomizing either transaction. The removal pass must disable or remove the
     Powder Jar grant, Berry Crush/powder-production entry points, powder display
     and exchange UI, repeatable vendor, related dialogue and tutorials, and any
     remaining rewards or checks that consume Berry Powder. Verify that no
     required progression or retained reward depends on the system before making
     its save-state fields permanently unused.
   - Retire the Coin Case and Game Corner coin loop in a later streamlining pass.
     For now, preserve the authored Harbor Mail exchange and Coin Case reward
     rather than routing it through item randomization. The removal pass must
     disable or remove coin purchasing, slot machines, roulette, coin balance
     display, Game Corner prize exchanges, Coin Case checks, and obsolete NPC
     dialogue. It must also choose and document a finite replacement reward for
     the Harbor Mail trade—or remove the trade and Harbor Mail dependency—before
     making the Coin Case unobtainable. Audit unique TMs, species, or other useful
     prizes first and relocate anything retained by the streamlined ruleset.
   - Disable the Seashore House battle-and-reward loop during streamlining. Remove
     or bypass its three trainer fights, the one-time six-item challenge reward,
     and the repeatable ¥300 randomized vendor as one coherent change. Update the
     owner and trainer dialogue and any related flags so the house does not
     advertise an unavailable challenge or leave a partially active reward path.
   - Repurpose Captain Stern's Scanner exchange into a one-time choice among all
     retained generic evolution items: the ten standard evolution stones and
     the Linking Cord. Keep the selected item authored rather than randomized,
     and consume the Scanner only after the reward is successfully added to the
     Bag. Deep Sea Tooth and Deep Sea Scale remain ordinary battle items rather
     than options in this evolution-resource exchange.
   - Review Emerald's plot progression for optional streamlining, prioritizing fewer
     forced detours, repeated conversations, and backtracking while preserving
     progression flags, essential tutorials, major encounters, and story coherence.
   - Playtest the full badge-to-Champion route against the level caps before choosing
     EXP, trainer-level, encounter-level, or plot-flow adjustments.

### Phase 1 Contract
- `RandomizerHash` is a pure function: it does not read or advance either global RNG.
- The saved seed is nonzero. New-game initialization converts a generated zero to a
  fixed nonzero fallback.
- Hash callers supply all contextual identity explicitly. The foundation API does not
  infer map, encounter, species, or slot state from globals.
- Feature-specific config gates are added with their feature, so an enabled config
  option never advertises behavior that has not been implemented.
- Save structure size tests are intentionally updated for the new romhack format.

## Key Changes
- Add randomizer config gates, default enabled for this hack:
  - Randomized encounters from all enabled Pokemon through Gen 9.
  - Randomized species abilities, stable across each evolution line.
  - Randomized learnsets, weighted toward the Pokemon's type.
  - Hard Emerald gym-based level caps.
  - No EV gain.
  - Friendship evolutions replaced by level evolutions.
  - Move-, clock-, location-, trade-, and species-specific-item evolution barriers
    replaced by explicit levels, player-selected stone branches, or the Linking
    Cord according to the authoritative evolution ledger.
- Store one `u32 randomizerSeed` in save data and initialize it during new-game setup.
- Add deterministic helper functions:
  - `GetRandomizerSeed()`
  - `RandomizerHash(category, species/map/slot/etc.)`
  - `GetRandomizedSpeciesForEncounter(originalSpecies, map, encounterType, slot)`
  - `GetRandomizedAbilityNumForSpecies(species)`
  - `GetRandomizedLevelUpMove(species, learnLevel, slot)`
- Randomization must be stable for a save, but different across new saves.

## Gameplay Implementation
- Encounters:
  - Intercept wild encounter species selection after the vanilla encounter slot is chosen.
  - Preserve encounter levels, encounter method, and slot rarity.
  - Replace species with a deterministic random species from enabled Pokemon in a
    BST band appropriate to the encounter table's level range.
  - Exclude invalid entries: `SPECIES_NONE`, eggs, disabled species, battle-only forms, unusable forms.
  - Use stable encounter-table difficulty data as hash/filter context. Do not use
    the mutable rolled encounter level as identity.
  - If no species is eligible in the preferred BST band, expand to the nearest
    adjacent band deterministically rather than failing the encounter.
  - Route non-special `setwildbattle` encounters through the same ordinary pool,
    using the fixed scripted level for difficulty and a distinct scripted
    encounter type for hash separation.
- Legendary encounters:
  - Detect scripted/static encounters whose original species is flagged as a
    restricted Legendary, sub-Legendary, or Paradox Pokemon.
  - Replace those species deterministically from a separate pool containing only
    enabled and generally usable Pokemon with one of those same classifications.
  - Do not include Mythical Pokemon or Ultra Beasts in the initial pool.
  - Preserve the original scripted level and progression behavior.
- Starters:
  - Replace the three displayed and granted starter species deterministically from
    the save seed while preserving each choice slot's identity.
  - Ensure the selection UI, granted Pokemon, and starter-dependent scripts agree
    on the randomized choices.
  - Decide whether choices must be distinct and whether the pool excludes special
    species, unusable forms, evolved species, or Pokemon outside an approved BST
    range before implementation.
- Enemy trainer parties:
  - Resolve replacements independently for each configured trainer ID and party
    slot. This includes each May/Brendan battle as its own encounter.
  - Keep the result deterministic within a save; do not consume mutable battle RNG
    or reroll the team when an encounter is reloaded.
  - Preserve configured party size and levels while using an original-BST-relative
    candidate pool to maintain approximate trainer difficulty.
  - Give replacement species legal moves and valid abilities, and safely handle
    incompatible species-specific items and battle gimmicks.
- Abilities:
  - Choose one legal randomized ability per evolution family/base species.
  - Apply the same ability slot result to all members of that evolution line.
  - Player, wild, gift, and trainer Pokemon all resolve through the same deterministic ability logic unless a battle gimmick/form explicitly overrides ability.
- Moves:
  - Keep move data itself intact: move power/type/effect are not randomized.
  - Randomize level-up learnsets at runtime using deterministic seed logic.
  - Weight candidate moves toward Pokemon typing: strong preference for STAB, secondary preference for coverage/status, reject unusable/special-case moves.
  - Same species gets the same learnset across encounters in the same save.
- Level caps / EVs:
  - Set `B_EXP_CAP_TYPE = EXP_CAP_HARD`.
  - Set `B_LEVEL_CAP_TYPE = LEVEL_CAP_FLAG_LIST`.
  - Set `B_RARE_CANDY_CAP = TRUE`.
  - Keep existing Emerald cap table initially: 15, 19, 24, 29, 31, 33, 42, 46, champion 58.
  - Set `B_EV_CAP_TYPE = EV_CAP_NO_GAIN`; hide/avoid EV-focused UI where practical.
- QoL:
  - Add a reusable action to both the party menu and Pokemon Storage that raises the
    selected Pokemon to the current cap. Boxed Pokemon must not need a temporary open
    party slot. Both entry points must use the same sequential level-up flow so no
    learn-move prompt or evolution opportunity is skipped; stop safely if the flow is
    cancelled or interrupted, persist all changes to the correct party or box slot,
    and never exceed the active cap.
  - The Portable Healer Key Item fully heals the party outside battle, is not
    consumed, and is granted when the starter is created.
  - On the Pokemon Skills summary page, A toggles from normal stats to all six
    numeric stored IV values and back without changing the Pokemon. Hyper Training
    does not disguise the stored IV values shown by this view.
  - The reusable Repel Toggle Key Item enables or disables an indefinite standard
    random-encounter block. It ignores wild and lead-Pokemon levels, persists
    through steps and saves, and is granted when the starter is created.
  - Badge-authorized HM field actions are implemented on
    `romhack/qol-badge-hms`. The existing badge gates remain intact, while the
    first non-Egg party Pokemon can perform an unlocked HM without knowing it.
  - The stationary girl in Oldale Town grants 200 Ultra Balls and ¥200,000 once on
    `romhack/qol-oldale-supplies`. The reward flag is set only after the balls fit
    in the Bag, allowing a failed attempt to be retried.
  - Battle style is globally locked to Set on `romhack/qol-no-free-switch`; new
    and existing saves use Set-style replacement flow, and the Battle Style row
    is removed from the Options menu.
  - Trainer AI tier assignment is implemented: ordinary trainers use competent
    scoring and randomized switch-ins, while boss-class trainers additionally use
    Smart Switching, restrained STAB/status assumptions, and PP-stall prevention.
    Authored Risky behavior is retained and omniscient knowledge is excluded.
    Broader battle-scenario coverage remains a follow-up task.
- Friendship evolutions:
  - Replace all `IF_MIN_FRIENDSHIP` evolution conditions with level-based equivalents:
    - Baby/early evolutions: level 20.
    - Midgame evolutions: level 30.
    - Special/late evolutions: level 40.
  - Remove clock and move requirements because short-run access and randomized
    learnsets cannot reliably satisfy them.
- Time-dependent, branched, and form evolutions:
  - Remove day/night availability barriers for a quick Nuzlocke.
  - For species with alternate evolution outcomes or forms, deterministically choose
    an available result per save, including eligible regional forms.
  - Specify the eligible form pool, branch identity, evolution trigger, and handling
    of species whose regional form normally requires a different base form in the
    authoritative evolution ledger.
- World items:
  - Hidden item spots are disabled centrally on
    `romhack/qol-remove-hidden-items`. Randomize only eligible visible overworld
    item pickups deterministically from the save seed and a stable pickup identity.
  - Review the distribution and usefulness of available pickups across the game.
    Add new visible pickup spots where the balance review identifies a resource
    or progression gap; do not add new hidden spots. Prefer rewards guarded by
    optional trainers, with placement that makes the battle and reward clearly
    avoidable rather than blocking required progression.
  - Restrict the initial replacement pool to held items and evolution items that are
    usable during a Nuzlocke.
  - Do not infer usefulness from `ITEM_TYPE_EVOLUTION_ITEM`: all species-specific
    evolution routes have been consolidated into stones or the Linking Cord.
    Remove retired evolution items from the pool when they have no other useful
    battle or field effect. Items such as King's Rock, Metal Coat, Razor Claw,
    Razor Fang, Deep Sea Tooth, and Deep Sea Scale require an explicit held-effect
    decision instead of automatic exclusion.
  - Review consumables against the intended Nuzlocke rules. X-items should be
    excluded if battle stat consumables are not part of the format. Potions and
    other basic healing should be excluded if free Portable Healer access makes
    them dead or disappointing rewards; otherwise define which healing tiers are
    worthwhile rather than admitting the entire medicine pocket.
  - Specify key-item/TM handling, duplicate policy, pickup respawn behavior, and any
    progression-critical exclusions before hooking item scripts.

## Test Plan
- Build with `make -j$(nproc)`.
- Start two new saves and confirm encounter/ability/learnset randomization differs between saves.
- Confirm the three starter choices are stable within a save, differ across suitable
  seeds, and match the Pokemon granted to the player.
- Confirm an enemy trainer's randomized party is stable when repeated in one save,
  differs across suitable saves, respects the configured levels and BST policy,
  and contains legal moves and abilities.
- Confirm separate rival encounters randomize independently without requiring a
  persistent starter or evolution line.
- Within one save, confirm repeated encounters of the same species have consistent ability and learnset.
- Confirm evolution preserves the randomized ability behavior for that evolution line.
- Confirm Pokemon at cap gain no EXP and Rare Candy/EXP Candy cannot exceed cap.
- Confirm EVs do not increase from battle or EV items.
- Confirm friendship evolution species evolve by the new level thresholds.
- Confirm day/night evolution lines and alternate/regional-form branches are
  obtainable without waiting for a real-time window and remain stable within a save.
- Confirm hidden item spots cannot be collected and visible world item
  replacements are deterministic within a save, differ between saves, and never
  produce an item outside the approved Nuzlocke-useful pool of
  standard stones, Linking Cord, and independently useful held items.
- Confirm the party-menu and Pokemon Storage level-cap actions never exceed the active
  cap, process all intervening move-learning and evolution opportunities in order,
  resume safely after cancellation or interruption, and correctly persist changes to
  party and boxed Pokemon without requiring a free party slot.
- Confirm the Portable Healer works outside battle and is blocked or harmless in battle.
- Confirm both reusable Key Items appear after receiving the starter, the Repel
  Toggle persists across saving and loading, and Lures or ordinary Repels interact
  with the toggle as documented.

## Assumptions
- Development is based on `expansion/1.16.2` and integrated into `romhack/main`.
- Pokemon pool is all enabled species/forms through Gen 9, filtered for validity.
- Randomization is per-save deterministic, not build-time static and not rerolled every encounter.
- Move randomization means randomized learnsets, not randomized move effects/power/type.
- Initial level caps use Emerald's existing cap values.
