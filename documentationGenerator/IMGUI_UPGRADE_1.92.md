# ImGui 1.92 upgrade guide

EOP's ImGui has been updated from **1.90.2** to **1.92.9b**. EOP keeps the old behaviour wherever newer ImGui changed it, so existing scripts should keep working without changes. This page covers the little that can break and what's new.

The full list of bindings is in [LuaImGuiDocs.lua](https://github.com/youneuoy/M2TWEOP-library/blob/master/M2TWEOP%20DataFiles/eopData/eopScripts/LuaImGuiDocs.lua).

---

## What might break

### 1. Raw numbers instead of enum names

Many flag, colour and style-var values have new numbers. If your script passes raw numbers instead of names, it may now do the wrong thing. Scripts that use names are not affected.

```lua
-- Bad: the number may now mean something else
ImGui.PushStyleColor(33, 1, 0, 0, 1)

-- Good
ImGui.PushStyleColor(ImGuiCol.Tab, 1, 0, 0, 1)
```

The exceptions are the old popup mouse-button numbers (`0`, `1`, `2`) and the old "closed" value `1` for `DrawList:PathStroke`. EOP still accepts these.

### 2. `Separator()` height

`ImGui.Separator()` now takes up 1 pixel of height. Layouts that add up exact heights may shift by a pixel.

### Kept the same on purpose

Newer ImGui changed these, but EOP keeps the 1.90 behaviour so you don't have to change anything:

- Context popups (`BeginPopupContextItem` etc.) still treat `0` as the left mouse button.
- Typed values in Drag/Slider/Input widgets are still written back on every keystroke.
- `ColorEdit`/`ColorPicker`/`ColorButton` previews are still opaque unless you pass `ImGuiColorEditFlags.AlphaPreview`.
- `DragFloat`/`DragInt` with the same min and max are still unclamped.
- `ImGui.PushFont(nil)` still pushes the default font.
- EOP themes still colour checked checkboxes and the text cursor the same way.
- ImGui's new debug tooltips are hidden from players (see [Debug helpers](#debug-helpers-for-script-authors)).

### Renamed names (old names still work)

You don't have to change these, but the new names match the ImGui docs.

| Old name | New name |
|---|---|
| `ImGuiCol.TabActive` | `ImGuiCol.TabSelected` |
| `ImGuiCol.TabUnfocused` | `ImGuiCol.TabDimmed` |
| `ImGuiCol.TabUnfocusedActive` | `ImGuiCol.TabDimmedSelected` |
| `ImGuiCol.NavHighlight` | `ImGuiCol.NavCursor` |
| `ImGuiChildFlags.Border` | `ImGuiChildFlags.Borders` |
| `ImGuiSelectableFlags.DontClosePopups` | `ImGuiSelectableFlags.NoAutoClosePopups` |
| `ImGuiTreeNodeFlags.NavLeftJumpsBackHere` | `ImGuiTreeNodeFlags.NavLeftJumpsToParent` |
| `ImGuiTabBarFlags.FittingPolicyResizeDown` | `ImGuiTabBarFlags.FittingPolicyShrink` |
| `ImGuiMod.Shortcut` | `ImGuiMod.Ctrl` |
| `ImGuiStyle.TabMinWidthForCloseButton` | `ImGuiStyle.TabCloseButtonMinWidthUnselected` |
| `io.FontGlobalScale` | `ImGui.GetStyle().FontScaleMain` |
| `ImGui.SetWindowFontScale(s)` | `ImGui.PushFont(nil, size)` |

`ImGuiMod.None` was previously `-1` by mistake. It is now `0`.

---

## New features

### Fonts at any size

Fonts can now be drawn at any size without loading them again, and characters are loaded the first time they're shown. You no longer need a separate font for each size. Fonts you load in `onLoadingFonts` without glyph ranges can show any character the font file contains. EOP's built-in font is still limited to Latin and Cyrillic.

```lua
ImGui.PushFont(myFont, 24)   -- myFont at 24px
ImGui.Text("Big title")
ImGui.PopFont()

ImGui.PushFont(nil, 12)      -- current font, 12px
ImGui.Text("Small print")
ImGui.PopFont()

ImGui.PushFont(nil)          -- default font, as before

ImGui.GetStyle().FontScaleMain = 1.25  -- scale all ImGui text
```

### Keyboard shortcuts

Combine a key with modifiers using `+`.

By default a shortcut only fires while one of your ImGui windows is focused. When the player has clicked on the campaign map or battlefield, no ImGui window is focused, so use `ImGuiInputFlags.RouteGlobal` for hotkeys that should always work.

```lua
-- Only fires when your window (or its children) is focused
if ImGui.Shortcut(ImGuiMod.Ctrl + ImGuiKey.S) then
    saveSettings()
end

-- Fires from anywhere
if ImGui.Shortcut(ImGuiKey.F5, ImGuiInputFlags.RouteGlobal) then
    refresh()
end

-- Pressing Ctrl+Enter clicks the button; hovering shows the shortcut
ImGui.SetNextItemShortcut(ImGuiMod.Ctrl + ImGuiKey.Enter, ImGuiInputFlags.Tooltip)
if ImGui.Button("Send") then send() end
```

### Hyperlinks

```lua
if ImGui.TextLink("Show details") then showDetails = true end
ImGui.TextLinkOpenURL("Mod forum", "https://www.twcenter.net/")
```

### Text input

```lua
-- Wrap long lines in a multi-line box
text = ImGui.InputTextMultiline("##notes", text, 4096, 400, 200, ImGuiInputTextFlags.WordWrap)

-- Keep the end of a long path visible
path = ImGui.InputText("Path", path, 260, ImGuiInputTextFlags.ElideLeft)
```

### Tree lines

Draw lines connecting tree nodes, for family trees or other hierarchies:

```lua
if ImGui.TreeNodeEx("Dynasty", ImGuiTreeNodeFlags.DrawLinesToNodes) then
    -- child nodes...
    ImGui.TreePop()
end

-- Or turn it on for every tree:
ImGui.GetStyle().TreeLinesFlags = ImGuiTreeNodeFlags.DrawLinesFull
```

### Drawing flags

The new `ImDrawFlags` enum replaces magic numbers for closed paths and rounded corners:

```lua
DrawList:PathStroke(color, ImDrawFlags.Closed, 2.0)
DrawList:AddRect(x1, y1, x2, y2, color, 6.0, ImDrawFlags.RoundCornersTop, 1.0)
```

### Drawing shapes that aren't convex

```lua
DrawList:PathLineTo(x1, y1)
DrawList:PathLineTo(x2, y2)
DrawList:PathLineTo(x3, y3)
DrawList:PathLineTo(x4, y4)
DrawList:PathFillConcave(color)
```

### Smaller additions

| Function / field | Purpose |
|---|---|
| `ImGui.PushStyleVarX(var, x)` / `ImGui.PushStyleVarY(var, y)` | Change one axis of a two-value style setting |
| `ImGui.PushItemFlag(flag, on)` / `ImGui.PopItemFlag()` | Set an `ImGuiItemFlags` value for the following items, e.g. `ImGuiItemFlags.LiveEditOnInputScalar, false` to only apply typed values on Enter |
| `ImGui.IsMouseReleasedWithDelay(button, delay)` | Tell a single click apart from a double-click |
| `ImGui.TableGetHoveredColumn()` | Which table column the mouse is over |
| `ImGui.IsKeyChordPressed(chord)` | Check a key combo without focus routing |
| `DrawList:PathStroke(color, flags, thickness)` | Stroke a path with a thickness |
| `ImGuiSliderFlags.WrapAround` | Drag past the max to wrap round to the min |
| `ImGuiColorEditFlags.AlphaNoBg`, `AlphaPreviewHalf` | More control over how transparent colours are previewed |
| New `ImGuiCol` entries | `TextLink`, `TreeLines`, `InputTextCursor`, `CheckboxSelectedBg`, `TabSelectedOverline`, `DragDropTargetBg`, `UnsavedMarker` |
| New `ImGuiStyle` fields | Tree lines, image rounding/border, tab widths, menu-item and selectable rounding, separator size, and more |

### Debug helpers for script authors

- **Error recovery:** if a script errors part-way through a window and never reaches `End()` or `PopStyleVar()`, ImGui now cleans up at the end of the frame instead of leaving the UI broken. This is automatic in release builds of EOP.
- **Duplicate ID warnings:** ImGui can highlight widgets that share an ID, which is a common cause of buttons reacting to the wrong click.

EOP turns the on-screen tooltips for both off so players don't see them. Turn them on while developing:

```lua
function onLoadingFonts()
    ImGui.GetIO().ConfigDebugHighlightIdConflicts = true
    ImGui.GetIO().ConfigErrorRecoveryEnableTooltip = true
end
```
