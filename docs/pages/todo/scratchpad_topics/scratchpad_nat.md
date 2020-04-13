## THINKING

- Increasing the success rate
	- Brute-force send packets
		- The only case against sending bruteforce packets is when both hosts are SYM-PS
			- Because the chance decreases extremely with each attempt, so we can expect many attempts

- Set 

- Remember setting the low ttl
- Consider using client_holes_opened to open holes only once
- Yeah, certainly disallow client from requesting the bruteforce open multiple times

- strange crashes on quit
	- path type destructor in nat detection settings 
	- seriously wtf...
		- also some double free, so maybe it's that
	- so be sure to make that shift+f1 hotkey or something else

- port number in start server gui: maybe somehow save that port number when the textbox deactivates...

- disallow nat on official server
 - use vim_run in a folder with a config file to deploy it to all servers

- Retry logic
	- Cumulative traversal log
		- Copy current to clipboard
		- Copy cumulative to clipboard
	- Count the total timeout from the moment we begin to traverse
		- Actually safer to just count from the beginning
	- We will retry traversals indefinitely
		- until the user presses Abort
	- We need to automatically wait when we exhaust stun servers
		- on the server too

- stun_result_info
	- remember to keep stun session in optional so that it is never initialized twice per session
		- it makes sense because it forces the client to start a new session to re-stun


## DISREGARDED

- In a single app run, keep a dictionary of opened servers and ping them regularly
	- could even perhaps ping them from the browser window
	- so, the browser window could fulfill that role entirely
	- for that, the browser window will have to use the auxiliary socket too, but why not

- If one is at most conic and the other symmetric, let the symmetric initiate connection 
	- Even if it's the server. The client will just "request_nat_hole"
	- this even handles client being a public internet host and the server being symmetric nat
	- if both are symmetric, it doesn't matter which one resolves new port first

## DONE

- Even though for reconnects, we can bind a new source port for the client,
	- with server it's not so possible
	- so if server has symmetric nat we'll still have to somehow remember the port it has opened for us
	- masterserver could somehow keep it but I have no idea how
	- app of course could keep it
	- if we restart the client though, we can get a new port
		- and then the server will map a new port anyway
		- so ironically it's worse if the server is address sensitive and not port sensitive

- Actually we won't need to ping masterserver periodically
- Use port 0 by default
	- only add a tick for a custom port
	- We may copy the address to clipboard automatically
	- but we'll be visible in the server browser anyway

- Gameserver commands
	- stunned_ping
	- ah, let's just have a single struct with flags anyways
	- nat_traversal_step

- Coordination of server operations
	- What the client needs from the server for its traversal logic?
		- Two things.
			- Pingback at a specific port, optionally open multiple ports in a sequence.
				- With a flag to multi-send. The count will be specified at the server
				- Optionally with more parameters specifying bounds for bruteforce.
			- A re-stun info.
				- This would be best if we could just handle this in C++ code in server_setup, look for packet receive overrides
					- Separate enum: NETCODE_AUXILIARY_COMMAND
						- Should actually handle ping too
						- It's a draft really, we'll later need a secure connection

	- The client, in its traversal logic, can request a specific pingback at a specific port, from the game server
		- Security: The client will later need a separate, secure connection with the masterserver with proof of identity (e.g. a challenge packet)
			- can use yojimbo for that 
		- Security: The server will later likewise need a secure connection with the masterserver

- Yeah let's also write a linear pseudo-code with ifs and elses
	- Actually think which way will be easier to comprehend...
	- Pre-stun stage
		1. Do we need to stun?
			- Only if client is symmetric
		2. Can we ping right after stun?
			- Only if server is conic
		3. If stunned, wait
		4. Determine traversal session timeout.
			- Both conic? 5s because the only 
		4. (repeated in intervals) Notify masterserver. If stunned, include the information.
			- timeout?
		5. Pingback from the game server.
			- Launch the full connection.
		5. Response from masterserver.
			- Server is at least symmetric? Get predicted port from info.
			- 
		
			

- It's most advantageous for the CLIENT to send low ttl packets
	- And it makes most sense.
	- As they won't reach the server, multiple clients connecting there won't trigger some security measures
	- More difficult to take the server down. The server can even only respond to what masterserver relays from the client, not to client's pings
		- perhaps its important somehow for our logistics?

- Remember that a changed source port MUST result in a different translated port on all NATs
	- otherwise we'd have no way to know which app the packet was delivered to

- Retrying connections
	- Sym-AS can be bounded with two stuns and bruted
		- Do we really need the upper bound? We'll just always send as many as we can... probably?
		- Having the upper bound might speed things up in the case that the port is allocated close to it
		- there is the case where the port is allocated in the middle between the two, but it's unlikely 
			- in this case we do worse
		- well it's a good heuristic anyway and we really have to bruteforce in case of a Sym-AS
			- since changing the source port on the opposite side won't work
	- Sym-PS can just pick a new port and re-stun

- Client traversal logic
	- The subject never does multi-prediction When the other party is a cone
	- Server is cone
		- Client is cone
			- (repeat, timeout=5s) notify
			- ping
			- response? LAUNCH
		- Client is symmetric (AS or PS)
			- stun
			- ping
			- wait
			- (repeat, timeout=1s) notify
				- no response might be due to server's misprediction (client port might actually be absolute shit) so less timeout
				- timeout?
					- change source port
					- next stun
					- ping
					- wait
					- (...) ad infinitum
			- response? LAUNCH
	- Server is symmetric (AS or PS)
		- Client is cone
			- notify (repeat, timeout=5s)
			- info? predict next port, open multiple holes (low ttl)
			- (repeat, timeout=1s) notify
				- no response might be due to client's misprediction (incoming server's port might actually be absolute shit) so less timeout
				- timeout?
					- Server is PS?
						- change source port
						- notify
						- (...) ad infinitum
					- Server is AS?
						- We're fucked.
							- Client opens holes by brute-force.
			- response? LAUNCH
		- Client is symmetric (AS or PS)
			- stun
			- wait
			- (repeat, timeout=5s) notify
			- info? predict next port, open multiple holes (low ttl)
				- AS->AS?
					- Open multiple holes
					- (repeat, timeout=1s) notify	
						- timeout? 
							- We're fucked.
								- Client opens holes by brute-force.
				- PS->AS? (draw this out please)
					- Open multiple holes
					- (repeat, timeout=1s) notify	
						- timeout? 
							- We're fucked.
								- Client opens holes by brute-force.
				- AS->PS?
					- Open more multiple holes (to account for ports going up at the server side) (low ttl)
					- (repeat, timeout=1s) notify
						- timeout? 
							- change source port 
							- next stun 
							- (...) ad infinitum
				- PS->PS?
					- Open one or two holes
					- (repeat, timeout=1s) notify	
						- timeout? 
							- change source port
							- next stun 
							- (...) ad infinitum

				- no response might be due to either's misprediction (client/server port might actually be absolute shit) so less timeout
				- timeout? next stun 
				- change source port and ping
				- wait
				- (...) ad infinitum
			- response? LAUNCH

- Traversal logic
	- masterserver could hold open mappings too
		- possibly scales very poorly
		- at least in the beginning
		- just a vector for each server heartbeat
	- The client shall let masterserver know what kind of NAT it has.
	- Client should always be the one to ultimately decide to launch the connection now.
		- So client always sends low ttl's to open hole for the server
	- Client is cone/cone preserving, and server is cone/cone preserving
		- Same procedure for all combinations.
			- client requests the masterserver to request a pingback
				- masterserver relays the client's source port straight from the request packet
			- client pings the server
			- server receives request from masterserver
			- server pings the client
			- when client receives a pingback, launch the full connection
		- Client is cone/cone preserving, and server is address-sensitive
			- client requests the masterserver to request a pingback
				- masterserver relays the client's source port straight from the request packet
			- server receives request from masterserver
			- at the same time:
				- server sends STUN request
				- server pings client right away
			- server receives STUN response and relays it to the masterserver
			- masterserver receives the new server's port and relays it to the client
			- client predicts the game server port from masterserver's info + known delta
			- client pings the server at predicted and many successive ports
			- server responds to any arrived pings of client
			- when client receives a pingback, launch the full connection
		- Client is cone/cone preserving, and server is port-sensitive
			- Same story as above
			- Except that the server 

- Don't use client/server's underlying socket for other stuff!!!
	- It might break netcode logic because netcode packets will be hijacked by the other logic

		
- I guess client connecting screen will just show a separate window with autoresizing, same as in server start gui
	- Can only show the last log line in the small status window
	- and full log in details?
	- nah, maybe let's just show everything in the small window somehow and forget the separate window (?)

- Symmetric nat punch (finally)
	- Punching servers one by one
	- While sending a punch request to masterserver, include our own last predicted port
		- If we're punching for the first time, we can get it from the my network details window state
	- Let's first acquire all data
	- Server can too send its nat type to the masterserver
	- links:
		- https://slideplayer.com/slide/3159695/

- Test results:
	- Two totally different addresses gave us proximate ports, so there's a chance for us
		- 35.205.19.61:43214 -> 31.182.205.239:28794
		- 35.205.19.61:43215 -> 31.182.205.239:28795
		- 104.199.81.130:8430 -> 31.182.205.239:28778
		- 104.199.81.130:8415 -> 31.182.205.239:28776
		- later 31.182.205.239:28816 and 31.182.205.239:28817 for the official server

- after testing with another nat masterserver, looks like ports are assigned similarly here
	- just incremented
	- we'll see it on another day
	- test with a completely different port at the server
	- test on Windows server

- Symmetric nat punch
	- We can easily determine server's port allocation rule
		- if both are address sensitive 
		- if both are port sensitive we're fugged
		- if at least one is non-port sensitive it should be relatively easy to establish connection with bruteforce
	- remember to punch nats one after another
	- Also client's port allocation rule
	- And with just the masterserver. It will just open another port that will respond
	- https://sketchboard.me/zBYca50ZCLaL#/

- Remember to account for duplicate requests to restun on the server
	- we'll keep a map of session guid to address with a timeout


- Symmetric nat punch
	- https://github.com/ph4r05/NATPaper
	- https://github.com/P2PSP/core/blob/master/doc/NTS/NAT_traversal.md
	- https://doc-kurento.readthedocs.io/en/6.11.0/knowledge/nat.html
