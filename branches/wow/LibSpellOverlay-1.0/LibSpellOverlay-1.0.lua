--[[
  Name: LibSpellOverlay-1.0
  Author: Vladinator
  Documentation: http://www.wowinterface.com/downloads/info19691-LibSpellOverlay-1.0.html
  SVN: http://www.wowinterface.com/downloads/info19691-LibSpellOverlay-1.0.html
  Description: Helps manage custom spell activation overlay textures
  Dependencies: LibStub
  
  Short API summary:
  
  :Show(spellID, duration[, position][, scale][, texture][, r, g, b])
    spellID = number, preferably a real spell ID
    duration = between 1 and max int value
    position = ["CENTER", "TOP", "BOTTOM", "LEFT", "RIGHT", "TOPLEFT", "TOPRIGHT", "BOTTOMLEFT", "BOTTOMRIGHT", "RIGHT (FLIPPED)", "BOTTOM (FLIPPED)", "LEFT + RIGHT (FLIPPED)", "TOP + BOTTOM (FLIPPED)"]
    scale = number, .5 is half size, 2 is double size, e.g.
    texture = string, path to a blp/tga file to use as a texture
    r, g and b = color the texture, set to 255 to make it appear normal
  
  :Hide(spellID)
    spellID = number, hides the overlay by this spell ID
  
  :FreePositions()
    returns a table with positions that are available
]]

if not LibStub then return end

local MAJOR_VERSION = "LibSpellOverlay-1.0"
local MINOR_VERSION = 10000 + tonumber(("$Revision: 2$"):match("%d+"))

local lib = LibStub:NewLibrary(MAJOR_VERSION, MINOR_VERSION)
if not lib or lib.frame then return end

------------------------------------------------------------------------
-- Frame and databanks

lib.frame = CreateFrame("Frame")

local frame = SpellActivationOverlayFrame
local frame_show = "SPELL_ACTIVATION_OVERLAY_SHOW"
local frame_hide = "SPELL_ACTIVATION_OVERLAY_HIDE"

local inuse, queue, size = {}, {}, 0
local positions = {
  "CENTER",
  "TOP",
  "BOTTOM",
  "LEFT",
  "RIGHT",
  "TOPLEFT",
  "TOPRIGHT",
  "BOTTOMLEFT",
  "BOTTOMRIGHT",
}

------------------------------------------------------------------------
-- Background worker managing the custom frames

lib.frame:SetScript("OnUpdate", function(self, elapsed)
  if size == 0 then return end
  
  self.elapsed = (self.elapsed or 0) + elapsed
  if self.instant or self.elapsed > .1 then
    for spellID, duration in pairs(queue) do
      if duration <= 0 then
        inuse[spellID] = nil
        queue[spellID] = nil
        size = size - 1
        SpellActivationOverlay_OnEvent(frame, frame_hide, spellID)
      else
        queue[spellID] = queue[spellID] - self.elapsed
      end
    end
    
    if self.instant then
      self.instant = nil
    else
      self.elapsed = 0
    end
  end
end)

------------------------------------------------------------------------
-- Hook Blizzard functions to help track overlay positioning

hooksecurefunc("SpellActivationOverlay_ShowOverlay", function(self, spellID, texturePath, position, scale, r, g, b, vFlip, hFlip)
  if spellID and inuse[spellID] then
    for _, temp in pairs(inuse[spellID]) do
      if position == temp then
        return
      end
    end
    
    table.insert(inuse[spellID], position)
  end
end)

------------------------------------------------------------------------
-- lib:FreePositions()

function lib:FreePositions()
  local free, ret, pos = {}, {}
  
  for _, pos in pairs(positions) do
    free[pos] = 1
  end
  
  for _, spells in pairs(frame.overlaysInUse) do
    for _, temp in pairs(spells) do
      free[temp.position] = nil
    end
  end
  
  for i = 1, #positions do
    pos = positions[i]
    if free[pos] then
      table.insert(ret, pos)
    end
  end
  
  return ret
end

------------------------------------------------------------------------
-- lib:Show(spellID, duration[, position][, scale][, texture][, r, g, b])

function lib:Show(spellID, duration, position, scale, texture, r, g, b)
  if type(spellID) ~= "number" or type(duration) ~= "number" then
    return
  end
  
  if queue[spellID] then
    queue[spellID] = duration
    
    if duration == 0 then
      self.frame.instant = 1
      self.frame:GetScript("OnUpdate")(self.frame, 0)
      
    elseif scale or texture or (r and g and b) then
      for _, temp in pairs(inuse[spellID]) do
        local overlay = SpellActivationOverlay_GetOverlay(frame, spellID, temp)
        -- NYI: scaling is a tad harder to recalculate once overlay is created
        if texture then
          overlay.texture:SetTexture(texture)
        end
        if r and g and b then
          overlay.texture:SetVertexColor(r/255, g/255, b/255)
        end
      end
      
    end
    
  elseif duration == 0 then
    
  else
    position = position or unpack(self:FreePositions()) or "CENTER"
    scale = scale or 1
    texture = texture or select(3, GetSpellInfo(spellID)) or ""
    r, g, b = r or 255, g or 255, b or 255
    inuse[spellID] = {}
    queue[spellID] = duration
    size = size + 1
    SpellActivationOverlay_OnEvent(frame, frame_show, spellID, texture, position, scale, r, g, b)
    
  end
end

------------------------------------------------------------------------
-- lib:Hide(spellID)

function lib:Hide(spellID)
  return self:Show(spellID, 0)
end
