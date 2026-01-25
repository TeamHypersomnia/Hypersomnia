# Stateless calculators

These functions are able to statelessly calculate the requested bot's input depending on the current behavior and its internal state. They are always calculated only once, after the behavior tree finishes evaluation.

This should be the preferred way to define new complex conditions for differing behaviors.
