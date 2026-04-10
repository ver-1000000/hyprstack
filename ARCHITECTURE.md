# hyprstack Architecture

`hyprstack` is structured as a small layered plugin.

The goal of the current layout is to keep the stack model independent from Hyprland-specific APIs, while still making the plugin entrypoint easy to follow.

## Layers

### Domain

The domain layer contains the pure stack model.

Its main responsibility is `WorkspaceStack`, which defines:

- stable window order
- current and previous window tracking
- neighbor lookup
- swap behavior

This layer does not depend on Hyprland.

### Application

The application layer contains the use cases that operate on the domain model.

Its main parts are:

- `StackStore`
- `StateSyncService`
- query handling
- dispatch target resolution

`StateSyncService` takes an observed snapshot and updates the in-memory store.
`StackStore` keeps workspace-local stack state and exposes snapshot and swap operations to the rest of the plugin.

### Infra

The infrastructure layer is the Hyprland adapter boundary.

It is responsible for:

- observing Hyprland windows and workspaces
- executing focus and swap side effects
- translating Hyprland events into dirty-state invalidation

Hyprland-specific types and APIs are intended to stay in this layer.

### Plugin

The plugin layer wires everything together.

`PluginApp` owns the runtime state, refreshes it on demand, and serves both:

- `hyprctl hyprstack ...` queries
- dispatcher entrypoints such as `stackfocus` and `stackswap`

The exported plugin entrypoint is kept thin and delegates runtime behavior to `PluginApp`.

## Runtime Flow

The runtime flow is:

1. Hyprland events mark the plugin state as dirty
2. `PluginApp` requests a fresh observed snapshot only when a query or dispatcher needs it
3. `StateSyncService` applies the snapshot to `StackStore`
4. query and dispatcher logic read from the synchronized store
5. Hyprland side effects are performed through the executor layer

## Design Intent

This layout is meant to keep the codebase easy to refactor in small steps.

In practice, that means:

- core stack semantics can be tested without Hyprland
- Hyprland integration stays isolated
- entrypoint code remains mostly wiring
- query and dispatch behavior can evolve without reshaping the whole plugin
