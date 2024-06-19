---
title: Rankeds
tags: [planning]
hide_sidebar: true
permalink: rankeds
summary: That which we are brainstorming at the moment.
---

- Negative MMR should have a half-life of 2 days
    - halver service decreasing it every 24 hours
    - we can just say "is halved every two days"
    - until it's less than -1 so you still have to put in some work

- matches: show how long ago and a map miniature
- last matches on the homepage

- Post ranked deplyoment
    - Remove test id replacement to match nick for testing
        - this is actually just in prod
        - (was #if 0 in server setup)
    - && is_ranked_server in find best ranked
    - default_config.json: uncomment check_ban_url = "https://hypersomnia.xyz/check_ban"
    - multiple api keys, one per official server
    - disable "Play ranked" button in non-steam clients
    - Look at "TODO_RANKED"

- article about matchmaking rules
    - event multiplier
    - abandoning players don't show in death log as perpetrators

- Ranks
    - 1st: Sol Invictus (gold)
    - 2nd: Aurora (violet bird)
    - 3nd: Boreas (silver bird)

- make defuse not require holding so it's less abusable by suspends
    - and if defusing, always restore the mouse no matter what

- POST-CRASH REJOINING: Create a 'user/ranked.json' file whenever a match starts
    - We could just have the server heartbeat include a match id and save last_match_id.json
        - then just download the server list if this file exists
        - if match doesn't exist, delete this file
    - Should just spawn a modal that overrides the crash notification modal
    - (Disregarded) But when to delete it?
        - AUTOMATICALLY AFTER AN HOUR. (or exactly 1.5min * 30 rounds = 45 minutes which is the maximum duration of a match)
        - match summary
        - abandon button in the modal

- While quit to menu/alt f4 should technically abandon match right away to force people to at least disconnect the network cable/kill process to abuse it, it's still a useful mechanic when we want to pause for any reason
    - we should just implement a /pee command that'll count suspension without the disconnect

- fix tick playing once in the beginning
- maybe lock mouse cursor if someone has u
    - Or just restore the cursor - easier and more comfortable
    - Maybe we should do it for all
    - If you back up on suspend, only back up if unsuspended already
        - cause it might overwrite backup and then there'd be a surprise
    - Maybe don't overwrite base crosshair after all? tho backup is not much of a problem

- We could have nickname-based authorization for integrated servers without a steam api who want to run a "ranked" mode
    - Will be hella useful for offline tournaments

- Check bans asynchronously on connect
    - Restart the match if you kick because of a ban
- on ranked start and just restart the match inconsequentially

- note a match freeze could occur during a sensitive moment like bomb defusing
    - We might make defusing a toggle later to avoid accidents
        - Easier to press H/G too
    - Though we could skip freezing if the character who disconnected is dead
    - Right now let's only implement freezing on the next round


