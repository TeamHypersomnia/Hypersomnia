---
title: Replicated state
tags: [topics, ECS] 
note: This name does not appear anywhere in the source code. It is only introduced for the sake of this wiki.
hide_sidebar: true
permalink: replicated_state
summary: |
    A state is called **replicated** if, at any point in the code, efforts are made to replicate it *exactly* on a remote machine,  
    whether by transferring its bytes directly, or by [inference](inferred_cache) from what has already been transferred.  

    It is not to be confused with [significant state](significant_state).  
    Additionally, it should be clear that both significant state and [inferred state](inferred_cache) can be **replicated state**.
---

## Examples

- The [cosmos](cosmos).
- [Viewables](viewable).
