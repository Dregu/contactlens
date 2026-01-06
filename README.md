# contactlens

Soft monocle layout for Hyprland, fixes weird resizing behavior when cycling maximized windows in dwindle and master.

## Usage

Install with hyprpm.

```
# at least first one is required
misc:on_focus_under_fullscreen = 1
misc:exit_window_retains_fullscreen = 1
binds:movefocus_cycles_fullscreen = 1

# works only in maximize mode
bind = SUPER SHIFT, F, fullscreen, 1
bind = ALT, Tab, cyclenext,
```
