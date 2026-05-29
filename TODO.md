# TODO - Equipment Upgrade Tab (Rarity Upgrade)

## Plan steps
1. UI: Implement “Upgrade” tab/panel switching + bind Upgrade button/panel widgets in `EquipWidget.h/.cpp`.
2. UI: Implement Upgrade slot eligibility rules (only non-perk/non-ability + eligible slot sources) and selection refresh for Before/After.
3. UI: Add/implement explicit “Confirm upgrade” button logic (disabled when invalid selection / Legendary / insufficient gold).
4. Preview: Reuse combat-number logic from `UShopContextMenuWidget::SetupContextMenu()` (or factor shared helpers) to compute Before/After AR/negation for previews.
5. Data-driven cost: Add a rarity upgrade cost data table and code to load/compute cost server+client.
6. Server: Add server RPC to `UInventoryComponent` to validate ownership, compute cost, deduct gold, and update item rarity.
7. Replication: Ensure rarity replication updates UI previews correctly after confirmation.
8. Tests: Compile + run functional UI test (singleplayer), then PIE 2-client network validation.

## Progress
- [x] Backend primitives scaffolded in:
  - [x] `InventoryUpgradeComponent.h/.cpp` (target rarity resolution, cost lookup, preview scaffolding)
- [x] UI wiring: Upgrade tab/panel + widgets + selection state
- [x] UI eligibility + preview computation (event-driven)
- [x] Server RPC to perform upgrade + deduct gold + update item rarity
- [x] Replication refresh verification
- [ ] Compile + PIE tests
