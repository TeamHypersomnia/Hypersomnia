# Redesigned folder structure

## Official

- Don't do this:
	- arenas/mine
	- arenas/official
	- arenas/community
- Rather, have:
	- content/official/
		- arenas
		- prefabs
	- Indeed we'll hold together things like calico muzzle and calico sprite
	- user/community/arenas
		- or "downloads/arenas"
	- user/projects



- Server-specific content packages
	- What if we want to introduce some funny sounds on deaths or purchases?
		- Best if it is not tied to the map
	- Ruleset packages?
	- Rulesets should ultimately never reference anything on the map for them to be universal 
	- "Content overlays"
- content/overlays
	- arena-agnostic
		- round win/lose sounds
		- za chwile wybuchnie bomba

- official/rules
	- common.rules
	- difficulties
		- casual.rules
		- competitive.rules
	- schemes
		- crazy_duel.rules ?
		- most will already be map specific, not sure if we'll have any official ones

## Project

- prefabs
	- aquarium
		- aquarium.png
		- aquarium.prefab
- rules
	- common.rules.yml
	- schemes
		- aim_pistols.rules.yml
		- aim_snipers.rules.yml
- arena.project
- miniature.png
- history.bin
	- maybe yaml for compatibility with newer versions? Let's save everything asynchronously anyway
- private.key
- view.yml

