-- globals
PMapx, PMapy = 30 * 16, 35 * 16
MapStages = {
    current = nil,
    stages = {},
    stage_at = function(x, y)
        for _, stage in ipairs(MapStages.stages) do
            if stage.x == x and stage.y == y then return stage end
        end
    end,
    mark_current_as_done = function()
        if MapStages.current == nil then return end
        MapStages.current.done = true
        MapStages.stage_at(MapStages.current.x, MapStages.current.y).done = true
    end
}

function make_overworld()
    local data = require("world")

    local kPlayerDir = {
        up = 1,
        down = 2,
        left = 3,
        right = 4,
        none = 5
    }

    local kMapStage = {
        locked = 16,
        unlocked = 32,
        done = 24
    }

    local kMapRoute = {
        up_down = 13,
        left_right = 21,
        up_left = 23,
        down_left = 15,
        down_right = 14,
        up_right = 22
    }

    local move_to_next = 0
    local camx, camy = 480 / 2, 270 / 2
    local pdir, psteps = kPlayerDir.none, 0
    local prx, pry

    local deadZone = {
        x = (480 - 48) / 2,
        y = (270 - 16) / 2,
        width = 48,
        height = 16
    }

    size = {
        width = 60 * 16,
        height = 60 * 16
    }

    local function tile_at(x, y)
        local line = data[y]
        if line ~= nil then
            local cell = line[x]
            if cell ~= nil then
                return cell[kMapID.overlay]
            end
        end
    end

    local function is_stage_at(x, y)
        local tile = tile_at(x, y)
        tile = tile and tile % 112 or 0
        return tile == kMapStage.locked or tile == kMapStage.unlocked or tile == kMapStage.done
    end

    local function can_walk_at(x, y, extra)
        local tile = tile_at(x, y)

        if tile == nil then return false end
        tile = tile % 112

        if extra and extra.shallow then
            for _, v in pairs(kMapRoute) do
                if tile == v + 44 then return true end
            end
        else
            for _, v in pairs(kMapRoute) do
                if tile == v then return true end
            end
        end

        return false
    end

    local function update_player()
        local ptx, pty = 1 + PMapx // 16, 1 + PMapy // 16

        for _, v in ipairs(kStages) do
            if v.map_location.x == ptx and v.map_location.y == pty then
                CurrentStage = v
            end
        end

        if pdir == kPlayerDir.none then
            if ui.btn(RIGHT, pad_1) and can_walk_at(ptx + 1, pty) then
                psteps, pdir = 16, kPlayerDir.right
            elseif ui.btn(LEFT, pad_1) and can_walk_at(ptx - 1, pty) then
                psteps, pdir = 16, kPlayerDir.left
            elseif ui.btn(DOWN, pad_1) and can_walk_at(ptx, pty + 1) then
                psteps, pdir = 16, kPlayerDir.down
            elseif ui.btn(UP, pad_1) and can_walk_at(ptx, pty - 1) then
                psteps, pdir = 16, kPlayerDir.up
            end
        end

        if psteps == 0 then
            local current = tile_at(ptx, pty) % 112
            if current == kMapRoute.up_down or current == kMapRoute.left_right then
                psteps = 16 -- no direction change needed
            elseif current == kMapRoute.up_left then
                psteps, pdir = 16, pdir == kPlayerDir.down and kPlayerDir.left or kPlayerDir.up
            elseif current == kMapRoute.down_left then
                psteps, pdir = 16, pdir == kPlayerDir.up and kPlayerDir.left or kPlayerDir.down
            elseif current == kMapRoute.up_right then
                psteps, pdir = 16, pdir == kPlayerDir.down and kPlayerDir.right or kPlayerDir.up
            elseif current == kMapRoute.down_right then
                psteps, pdir = 16, pdir == kPlayerDir.up and kPlayerDir.right or kPlayerDir.down
            else
                pdir = kPlayerDir.none
                return
            end
        end

        if pdir == kPlayerDir.right then PMapx = PMapx + 2 end
        if pdir == kPlayerDir.left then PMapx = PMapx - 2 end
        if pdir == kPlayerDir.up then PMapy = PMapy - 2 end
        if pdir == kPlayerDir.down then PMapy = PMapy + 2 end

        psteps = psteps - 2
    end

    local function stage_is_done(x, y)
        local stage = MapStages.stage_at(x, y)
        return stage and stage.done or false
    end

    local function update_tile_at(x, y, visited)
        local root = visited == nil and true or false
        local visited = visited and visited or {}

        for _, v in ipairs(visited) do
            if v.x == x and v.y == y then return end
        end

        table.insert(visited, { x = x, y = y })
        local can_walk = can_walk_at(x, y, { shallow = true })

        if can_walk then
            data[y][x][kMapID.overlay] = data[y][x][kMapID.overlay] - 44
        elseif is_stage_at(x, y) then
            local is_done = stage_is_done(x, y)
            data[y][x][kMapID.overlay] = is_done and kMapStage.done or kMapStage.unlocked
            if is_done == false then return end
        end

        if root or can_walk then
            update_tile_at(x, y - 1, visited)
            update_tile_at(x, y + 1, visited)
            update_tile_at(x + 1, y, visited)
            update_tile_at(x - 1, y, visited)
        end
    end

    local function update_map()
        local ptx, pty = 1 + PMapx // 16, 1 + PMapy // 16
        update_tile_at(ptx, pty)
    end

    local function update_camera(current_frame)
        local playerCenterX = PMapx + 16 / 2;
        local playerCenterY = PMapy + 16 / 2;

        if playerCenterX < camx + deadZone.x then
            camx = lerp(camx, playerCenterX - deadZone.x, 1)
        elseif playerCenterX > camx + deadZone.x + deadZone.width then
            camx = lerp(camx, playerCenterX - deadZone.x - deadZone.width, 1)
        end

        if playerCenterY < camy + deadZone.y then
            camy = lerp(camy, playerCenterY - deadZone.y, 1)
        elseif playerCenterY > camy + deadZone.y + deadZone.height then
            camy = lerp(camy, playerCenterY - deadZone.y - deadZone.height, 1)
        end

        camx = math.max(0, math.min(camx, size.width - 480)) // 1;
        camy = math.max(0, math.min(camy, size.height - 270)) // 1;
        prx = PMapx - camx
        pry = PMapy - camy
    end

    local function will_fade(current_frame)
        if (current_frame < 48) or (move_to_next and move_to_next > 0) then
            return true
        else
            return false
        end
    end

    local function draw_fade(current_frame)
        if current_frame < 48 then
            DrawTrisFade(current_frame, 1 - current_frame / 48, 4)
        else
            if move_to_next > 0 then move_to_next = move_to_next + 1 end
            DrawTrisFade(current_frame, move_to_next / 48, 4)
        end
    end

    local function draw_player(current_frame, tiles)
        if pdir == kPlayerDir.none then
            ui.tile(tiles, 24, prx, pry - 7)
        else
            local extra, masking

            if pdir == kPlayerDir.down then
                extra, masking = 0, 0
            elseif pdir == kPlayerDir.up then
                extra, masking = 8, 0
            elseif pdir == kPlayerDir.right then
                extra, masking = 16, 0
            elseif pdir == kPlayerDir.left then
                extra, masking = 16, 1024
            end

            local char = extra + ({ 32, 33, 32, 34 })[1 + (current_frame // 6) % 4]
            ui.tile(tiles, char + masking, prx, pry - 6)
        end
    end

    local function draw_layer(frame, layer_type, tiles)
        local ystart = 1 + camy // 16
        local yend = ystart + 17
        local xstart = 1 + camx // 16
        local xend = xstart + 30

        for y = ystart, yend do
            local line = data[y]
            if line ~= nil then
                for x = xstart, xend do
                    local vx = line[x]
                    if vx ~= nil then
                        local tile = vx[layer_type]
                        if tile ~= nil and tile ~= 0 and type(tile) == "number" then
                            ui.tile(tiles, tile - 1, 16 * (x - 1) - camx, 16 * (y - 1) - camy)
                        end
                    end
                end
            end
        end
    end

    update_map()

    return {
        name = function() return "overworld" end,
        update = function() end,
        is_finished = function()
            return move_to_next == 42
        end,
        draw = function(current_frame)
            for i = 1, #Palette do
                local color = Palette[i]
                ui.palset(i - 1, color)
            end

            ui.cls(kColors.purple_dark)
            ui.camera(0,0)
            ui.clip(0, 0, 480, 270)

            pad_1, pad_2 = 0, 0
            update_camera(current_frame)
            update_player()

            if ui.btnp(BTN_Z, pad_1) and not will_fade(current_frame) then
                local ptx, pty = 1 + PMapx // 16, 1 + PMapy // 16
                print("Entering stage at ", ptx, pty)
                if is_stage_at(ptx, pty) then
                    MapStages.current = MapStages.current and MapStages.current or {}
                    MapStages.current.x = ptx
                    MapStages.current.y = pty
                    MapStages.current.done = stage_is_done(ptx, pty)
                    table.insert(MapStages.stages,
                        { x = MapStages.current.x, y = MapStages.current.y, done = MapStages.current.done })
                    move_to_next = 1
                end
            end

            local ff = ping_pong(current_frame // 8, 5)
            local world_tiles = SpriteSheets['tilemap.world.1']
            local props_tiles = SpriteSheets['tilemap.wprops.' .. ff]

            draw_layer(current_frame, kMapID.tile, world_tiles)
            draw_layer(current_frame, kMapID.overlay, props_tiles)

            draw_player(current_frame, props_tiles)
            draw_fade(current_frame)
        end
    }
end
