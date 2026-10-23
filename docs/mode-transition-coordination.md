# Mode transition coordination

`ModeTransitionCoordinator` is the transaction boundary between application mode
requests and native resources. It preflights and acquires the requested mode,
releases the previous mode, and only then commits the confirmed active mode to
`AppModeController`. Failed operations preserve or restore the prior confirmed
mode; shutdown interrupts reentrant transitions and releases acquired resources.

The M2 production adapter owns the native overlay window resources for Overlay
and deliberately reports `NotImplemented` for Editing. M4 will extend that
adapter's Editing path to coordinate frozen capture and editing input surfaces.
Both integrations must return structured failures rather than committing a mode
before its resources are active.
