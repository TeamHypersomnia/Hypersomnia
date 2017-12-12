---
title: Message queue
tags: [topics, ECS] 
hide_sidebar: true
permalink: message_queue
summary: |
    A **message queue** is a vector of [messages](message) of the same type.  
    The lifetime of all message queues is identical to the lifetime of a [logic step](logic_step) to which they are tied.  
    This means that the [cosmos's advance method](cosmos#the-advance-method) always starts with empty queues and always finishes with empty queues.
---

