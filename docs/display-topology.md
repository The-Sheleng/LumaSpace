# Display topology contract

LumaSpace enumerates active Win32 monitors into stable `MonitorDescriptor`
values: a device ID, signed physical-pixel bounds in virtual-desktop space, and
the primary flag. Negative origins and stacked layouts are preserved verbatim;
the topology layer never relocates document content.

Overlay mode owns exactly one native surface per normalized descriptor. A
display-change notification reuses unchanged surfaces, creates replacements
before releasing old surfaces, and removes disconnected displays. If enumeration
returns no valid display or any replacement cannot be created, every overlay
surface is released and the coordinator requests Disabled mode so desktop input
cannot be stranded behind a partially refreshed topology.
