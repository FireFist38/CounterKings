# Editor Setup Steps: Equipment Upgrade Tab

To finalize the implementation in the Unreal Editor, please follow these steps for the **`WBP_Equipment`** widget.

## 1. UI Hierarchy & Binding Names

The C++ code in `UEquipWidget` uses `meta = (BindWidgetOptional)`, meaning you can name your widgets exactly as follows to "hook" them into the logic.

### Recommended Hierarchy for `WBP_Equipment`:

- **[Root]** (Canvas Panel)
    - **Btn_InventoryTab** (Button) - *Binding Name: `Btn_InventoryTab`*
    - **Btn_UpgradeTab** (Button) - *Binding Name: `Btn_UpgradeTab`*
    - **EquipTabSwitcher** (WidgetSwitcher) - *Binding Name: `EquipTabSwitcher`*
        - **[Index 0] InventoryLayout** (Canvas/VerticalBox)
            - *Keep your existing slots here (Inv_0, ArmorSlot, etc.)*
        - **[Index 1] UpgradePanel** (Canvas/VerticalBox) - *Binding Name: `UpgradePanel`*
            - **Upgrade_SourceSlot** (WBP_InventorySlot) - *Binding Name: `Upgrade_SourceSlot`*
            - **Upgrade_BeforeStats** (TextBlock) - *Binding Name: `Upgrade_BeforeStats`*
            - **Upgrade_AfterStats** (TextBlock) - *Binding Name: `Upgrade_AfterStats`*
            - **Upgrade_CostText** (TextBlock) - *Binding Name: `Upgrade_CostText`*
            - **Btn_ConfirmUpgrade** (Button) - *Binding Name: `Btn_ConfirmUpgrade`*

### Does the Upgrade Panel need to be its own WBP?
**No.** You can build the layout directly inside the `WidgetSwitcher` at Index 1. However, if you prefer to keep it clean, you can make a separate `WBP_UpgradePanel`, but the `WBP_Equipment` must still have the **Binding Names** assigned to the instances of those widgets.

**Pro-tip:** In the Designer tab, select each widget and check the **"Is Variable"** checkbox. Then, ensure the **Variable Name** matches the bold names above exactly.

## 2. Binding Names Cheat Sheet

| Widget Type | Variable Name (Binding) | Purpose |
| :--- | :--- | :--- |
| **WidgetSwitcher** | `EquipTabSwitcher` | Switches between Inventory and Upgrade views. |
| **Button** | `Btn_InventoryTab` | Button to show the standard inventory. |
| **Button** | `Btn_UpgradeTab` | Button to show the upgrade interface. |
| **Widget/Canvas** | `UpgradePanel` | The container for the upgrade UI elements. |
| **WBP_InventorySlot** | `Upgrade_SourceSlot` | Displays the item currently selected for upgrade. |
| **TextBlock** | `Upgrade_BeforeStats` | Shows the current stats (e.g., "Damage: 10"). |
| **TextBlock** | `Upgrade_AfterStats` | Shows the preview stats (e.g., "Damage: 15"). |
| **TextBlock** | `Upgrade_CostText` | Shows the gold cost (e.g., "Cost: 250 Gold"). |
| **Button** | `Btn_ConfirmUpgrade` | The button that actually triggers the upgrade. |

## 2. InventoryUpgradeComponent Configuration

The `UInventoryUpgradeComponent` needs to be attached to your `PlayerCharacter` (or the BP class that owns the inventory).

1.  Open your **PlayerCharacter BP**.
2.  Add the **InventoryUpgradeComponent**.
3.  In the component details, assign the **UpgradeCostTable**.

## 3. Data Table Setup

Create a new Data Table using the `FItemUpgradeCostRow` struct (defined in `ItemUpgradeCost.h`).

### Example Rows:
| Row Name | CurrentRarity | NextRarity | GoldCost |
| :--- | :--- | :--- | :--- |
| CommonToUncommon | Common | Uncommon | 100 |
| UncommonToRare | Uncommon | Rare | 250 |
| RareToLegendary | Rare | Legendary | 500 |

## 4. Slot Selection Logic

The selection logic is now automatic. When you are in the **Upgrade Tab**, clicking any slot in your inventory or equipment will:
1.  Broadcast the selection.
2.  `EquipWidget` will catch it and update the `Upgrade_SourceSlot`.
3.  Stats and cost will be calculated and displayed.

## 5. GASP Integration (Optional)

If you need the upgrade visuals to reflect in animations immediately, ensure that the `Update_PropertiesFromCharacter` function in your Animation Blueprint is reading the updated `Rarity` property from the item actors.

---
**Note:** If any widget is missing, the C++ code uses `BindWidgetOptional` where appropriate, but the functionality will be limited until the widgets are added and named correctly.
