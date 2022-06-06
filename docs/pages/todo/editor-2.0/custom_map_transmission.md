# Custom map transmission

- We'll probably just send file-by-file to have nice granularity and progress response
	- And it doesn't make sense to compress the heaviest of files which will be pngs and oggs

- We'll only compress the project files on the go with lz4

- max_block_size_v is 2 MB.
	- We can easily impose a per-file limit like that

- Actually we can only verify the map upon completely receiving it

- We oughtta move the connected players to spectators right away even while they are still downloading
	- and perhaps show their downloaded %

- For upgrading maps, it would be cool to retransmit only the stuff that changed
	- For quick testing cycles
	- Server easily first sends the temporary_id:path:checksum list
	- Client then requests a list of temporary_ids whose checksums don't match locally
	- Upon loading the map, the server will keep all project files in memory and compress the relevant ones so that it does not have to do it every time someone connects
		- It can also calculate checksums on that occasion, let's not store them in any cache please

- Upgrading and old map retention
	- I guess screw this - the history will be preserved anyway
		- ...though not the assets
		- perhaps it's pointless to send history file then?
			- It might be the biggest of all.
	- For now let's just upgrade the map in-place, if someone wants they can keep a git repository

