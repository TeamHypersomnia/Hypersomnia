---
title: Reinference
tags: [topics, ECS] 
hide_sidebar: true
permalink: reinference
summary: |
  **Reinference** is a sequence of two operations: destroying an [inferred cache](inferred_cache) and inferring it once again, possibly from new [significant state](significant_state).
---

## Examples

- When a [cosmos](cosmos) receives new solvable [significant state](cosmos_solvable#significant), either from disk or through the network, it must perform reinference so that the inferred caches remain consistent.
