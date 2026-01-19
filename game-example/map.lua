function make_map()
    local data = require(CurrentStage.map_name)

    local function get_map_size()
        local size = { height = #data * 16, width = 0 }

        for _, vy in pairs(data) do
            for x, _ in pairs(vy) do
                if x * 16 > size.width then size.width = x * 16 end
            end
        end

        return size
    end

    local function draw(frame, camera)
        local camx, camy = camera.getxy()

        local ystart = 1 + camy // 16
        local yend = ystart + 17
        local xstart = 1 + camx // 16
        local xend = xstart + 30

        local tileset = SpriteSheets['tilemap.sunny.' .. 1 + ((frame // 8) % 4)]

        for y = ystart, yend do
            local line = data[y]
            if line ~= nil then
                for x = xstart, xend do
                    local vx = line[x]
                    if vx ~= nil then
                        local tile = vx[kMapID.tile]
                        if tile then 
                            ui.tile(tileset, tile - 1, 16 * (x - 1) - camx, 16 * (y - 1) - camy)
                        end 
                    end
                end
            end
        end
    end

    return {
        size = get_map_size(),
        before_frame = function(frame, camera, player, map)
            
        end,
        on_frame = function(frame, camera, player, map)
            draw(frame, camera)
        end,
        colides = function(x, y, type)
            local row = data[1 + y // 16]
            if row == nil then return false end
            local tile = row[1 + x // 16]
            if tile == nil then return false end
            local type_id = tile[kMapID.colision]
            if type_id == nil then return false end
            local types = kColisionTile[type_id]

            for _, atype in pairs(types) do
                if atype == type then return true end
            end
            return false
        end,
        get_pois = function()
            local pois = {}
            for y, line in pairs(data) do
                for x, tile in pairs(line) do
                    local poi = tile[kMapID.poi]
                    if poi ~= 0 and poi ~= nil then
                        table.insert(pois, { x = x, y = y, poi = poi })
                        -- print("poi at x:" .. x .. " y:" .. y .. " value:" .. poi)
                    end
                end
            end
            return pois
        end
    }
end
