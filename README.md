# kglobalacceld

kglobalacceld is the daemon component responsible for global shortcut
registration and activation in KDE software.

### Shortcut allowlist

In some scenarios (for example lock screen, login screen, or kiosk-like setups)
you may want to allow only a specific set of global shortcuts to be activated,
while preventing all other global shortcuts from triggering.

kglobalacceld supports an **allowlist** mode for this.

#### How it works

- When allowlist mode is **disabled**, all registered global shortcuts behave
  normally.
- When allowlist mode is **enabled**, **only** shortcuts explicitly listed in
  the allowlist are permitted to trigger.

The check is done using the pair:

- the shortcut’s **component name** (the component owning the shortcut; for
  example `kwin`, `kaccess`, etc.)
- the shortcut’s **shortcut name** (the action/shortcut identifier within that
  component)

If a shortcut is not present in the allowlist, it will be ignored and will not
emit/trigger its action.

#### Configuration file and keys

The allowlist is configured in `~/.config/kglobalaccelrc`.

- Group `[General]`
  - `useAllowList=true|false`

- Group `[AllowedShortcuts]`
  - Keys are **component names**.
  - Values are a comma-separated **list of shortcut names** for that component.

#### Example

Allow only two shortcuts owned by the `kwin` component, and the default shortcut
that launches System Settings:

```ini
[General]
useAllowList=true

[AllowedShortcuts]
kwin=ShowDesktop,Walk Through Windows
systemsettings.desktop=_launch
```

Disable allowlist mode entirely:

```ini
[General]
useAllowList=false
```

It may be helpful to reference the component and shortcut names from existing
shortcuts you have configured in your system by examining
`~/.config/kglobalshortcutsrc`.

## Development

_TODO: document logging, debugging, code layout, and contribution notes._

## Testing

_TODO: document how to run tests._

## License

See the files in [LICENSES/](LICENSES/) for details.
