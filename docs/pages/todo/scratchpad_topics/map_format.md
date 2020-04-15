- Honestly, we should get ready to make these three maps from scratch again.
	- Writing conversion code will be a LOT more work.
	- It will be the ultimate test for the new system.
	- The conversion code will NOT create a correctly prefabbed map anyway - we have to do it by hand
	- We can just export the important layout somehow



- Prefabs?



- The new map format.
	- Textual. That one is certain.
		- We might want to make binary caches for quick load.

- Map version considerations
	- Communiy map migration
		- copy user folder to OLD_HYPERSOMNIA, in case there are some important maps too
		- Solution for map conversions: just keep binary versions in caches?

	- Check official map version upon connection
		- if mismatch
			- custom? re-download
			- official? disconnect
