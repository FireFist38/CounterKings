# Editor Setup Steps: Equipment Upgrade Tab

To finalize the implementation in the Unreal Editor, please follow these steps for the `EquipWidget` and data configuration.

## 1. EquipWidget Blueprint Setup

Open your `WBP_EquipWidget` (or equivalent) and ensure the following widgets are present and named exactly as specified in the C++ header (`EquipWidget.h`).

### Tab Navigation
- **Btn_InventoryTab** (Button): Button to switch to the inventory view.
- **Btn_UpgradeTab** (Button): Button to switch to the upgrade view.
- **EquipTabSwitcher** (WidgetSwitcher): Should contain at least two children:
    - Index 0: Your existing Inventory/Equipment layout.
    - Index 1: The new **UpgradePanel**.

### Upgrade Panel (Index 1 of Switcher)
Inside the **UpgradePanel** (which can be a Canvas, Vertical Box, etc.), add these widgets:
- **Upgrade_SourceSlot** (UInventorySlotWidget): A slot widget to display the item selected for upgrade.
- **Upgrade_BeforeStats** (TextBlock): Displays current item stats.
- **Upgrade_AfterStats** (TextBlock): Displays preview of upgraded stats.
- **Upgrade_CostText** (TextBlock): Displays the gold cost.
- **Btn_ConfirmUpgrade** (Button): The button to execute the upgrade.

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
