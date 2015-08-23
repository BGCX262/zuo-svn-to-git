--[[
  Example addon using LibSpellOverlay-1.0 to create overlays for Hunter and Mage, helping
  out in specific situations by showing visual feedback or information to certain events!
]]

-- Do not load if LibStub is missing
if not LibStub then return end

-- Get the library
local LibSpellOverlay = LibStub:GetLibrary("LibSpellOverlay-1.0", 1)

-- Do not load if the library couldn't be found
if not LibSpellOverlay then return end

----------------------------------------------------------------------
-- HUNTER ------------------------------------------------------------
----------------------------------------------------------------------

if select(2, UnitClass("player")) == "HUNTER" then
  
  local addon = CreateFrame("Frame")
  
  -- Frenzy
  local spellID = 19615 -- "Frenzy Effect" is a unique spell, not affected by the talent rank!
  local spellName = GetSpellInfo(spellID)
  local stackColors = {
    {255, 255, 255}, -- 1 stack (white)
    {255, 255, 128}, -- 2 stacks (light-yellow)
    {255, 255, 0}, -- 3 stacks (yellow)
    {200, 255, 0}, -- 4 stacks (lime)
    {64, 255, 64}, -- 5 stacks (green)
  }
  addon:RegisterEvent("UNIT_AURA")
  addon:HookScript("OnEvent", function(addon, event, unit)
    if event ~= "UNIT_AURA" then return end
    if unit ~= "pet" then return end
    
    local _, _, _, stacks, _, duration = UnitBuff(unit, spellName)
    if (duration or 0) > 0 and stacks < 5 then -- hide at 5 stacks (there is a built in overlay for this!)
      local r, g, b = unpack( stacks and stackColors[stacks] or select(2, next(stackColors)) )
      LibSpellOverlay:Show(spellID, duration, "LEFT + RIGHT (FLIPPED)", 1, "Textures\\SpellActivationOverlays\\GenericArc_02", r, g, b)
    else
      LibSpellOverlay:Hide(spellID)
    end
  end)
  
  -- Mend Pet (in combat healing assistance)
  local spellID2 = 136 -- Mend Pet (what else could this be?)
  local spellName2 = GetSpellInfo(spellID2)
  local MendPetUpdate = function(addon, elapsed)
    if not addon.combat then return addon:SetScript("OnUpdate", nil) end
    addon.elapsed = (addon.elapsed or 0) + elapsed
    if addon.elapsed > .25 then
      if UnitExists("pet") and not UnitIsDead("pet") and not UnitBuff("pet", spellName2) and UnitHealth("pet")/UnitHealthMax("pet") < .75 then
        LibSpellOverlay:Show(spellID2, 86400, "CENTER", .25)
      else
        LibSpellOverlay:Hide(spellID2)
      end
      addon.elapsed = 0
    end
  end
  addon:RegisterEvent("PLAYER_REGEN_ENABLED")
  addon:RegisterEvent("PLAYER_REGEN_DISABLED")
  addon:HookScript("OnEvent", function(addon, event, unit)
    if event == "PLAYER_REGEN_ENABLED" or event == "PLAYER_REGEN_DISABLED" then
      addon.combat = event == "PLAYER_REGEN_DISABLED" and 1 or nil -- ref. comment at line 64
      addon:SetScript("OnUpdate", addon.combat and MendPetUpdate or nil)
      if not addon.combat then LibSpellOverlay:Hide(spellID2) end
    end
  end)
  
  -- Trigger the event upon addon loaded (player login) to show the auras if any are present -think of this as a hack!
  addon:GetScript("OnEvent")(addon, "UNIT_AURA", "player")
  addon:GetScript("OnEvent")(addon, "UNIT_AURA", "pet")
  
end

----------------------------------------------------------------------
-- MAGE --------------------------------------------------------------
----------------------------------------------------------------------

if select(2, UnitClass("player")) == "MAGE" then
  
  local addon = CreateFrame("Frame")
  
  -- Arcane Blast
  local spellID = 36032
  local spellName = GetSpellInfo(spellID)
  local stackColors = {
    {255, 255, 255}, -- 1 stack (white)
    {255, 128, 128}, -- 2 stacks (light orange)
    {255, 64, 64}, -- 3 stacks (orange)
    {255, 0, 0}, -- 4 stacks (red)
  }
  addon:RegisterEvent("UNIT_AURA")
  addon:HookScript("OnEvent", function(addon, event, unit)
    if event ~= "UNIT_AURA" then return end
    if unit ~= "player" then return end
    
    local _, _, _, stacks, _, duration = UnitDebuff(unit, spellName)
    if (duration or 0) > 0 then
      local r, g, b = unpack( stacks and stackColors[stacks] or select(2, next(stackColors)) )
      LibSpellOverlay:Show(spellID, duration, "BOTTOM (FLIPPED)", 1.2, "Textures\\SpellActivationOverlays\\GenericTop_01", r, g, b)
    else
      LibSpellOverlay:Hide(spellID)
    end
  end)
  
  -- Arcane Potency (show only during combat)
  local spellID2 = 31572 -- rank 1/2: 31571/31572
  local spellName2 = GetSpellInfo(spellID2)
  local stackColors2 = {
    {255, 255, 255}, -- 1 stack (white)
    {0, 255, 100}, -- 2 stacks (lime)
  }
  addon:RegisterEvent("PLAYER_REGEN_ENABLED")
  addon:RegisterEvent("PLAYER_REGEN_DISABLED")
  addon:HookScript("OnEvent", function(addon, event, unit)
    if event == "UNIT_AURA" then
      if unit ~= "player" then return end
      
      local _, _, _, stacks = UnitBuff(unit, spellName2)
      if addon.combat and (stacks or 0) > 0 then
        local r, g, b = unpack( stacks and stackColors2[stacks] or select(2, next(stackColors2)) )
        LibSpellOverlay:Show(spellID2, 86400, "TOP", 1.2, "Textures\\SpellActivationOverlays\\GenericTop_01", r, g, b)
      else
        LibSpellOverlay:Hide(spellID2)
      end
    elseif event == "PLAYER_REGEN_ENABLED" or event == "PLAYER_REGEN_DISABLED" then
      addon.combat = event == "PLAYER_REGEN_DISABLED" and 1 or nil -- InCombatLockdown() takes longer time to false after combat ends, hence I track it using this method
      addon:GetScript("OnEvent")(addon, "UNIT_AURA", "player")
    end
  end)
  
  -- Focus Magic
  local spellID3 = 54648 -- Focus Magic (buff when proc)
  local spellName3 = GetSpellInfo(spellID3)
  addon:HookScript("OnEvent", function(addon, event, unit)
    if event ~= "UNIT_AURA" then return end
    if unit ~= "player" then return end
    
    local _, _, _, _, _, duration = UnitBuff(unit, spellName3)
    if (duration or 0) > 0 then
      LibSpellOverlay:Show(spellID3, duration, "LEFT + RIGHT (FLIPPED)", .3, "Textures\\SpellActivationOverlays\\Slice_and_Dice", 255, 255, 255)
    else
      LibSpellOverlay:Hide(spellID3)
    end
  end)
  
  -- Trigger the event upon addon loaded (player login) to show the auras if any are present -think of this as a hack!
  addon:GetScript("OnEvent")(addon, "UNIT_AURA", "player")
  
end
