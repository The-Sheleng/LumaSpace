# actions

Action IDs use lowercase dot-separated namespaces, for example
`app.mode.overlay`. Hyphens and underscores are allowed within a segment. IDs
are stable machine-facing identifiers; labels and descriptions are user-facing
metadata and may change independently.

Dispatch is synchronous. Reentrant dispatch and registry mutation from a handler
are rejected so a handler observes a stable registry. The owner must keep the
registry, its captured dependencies, and registered handlers alive in that order.
