function draw_box(box, camera)
    local camx, camy = camera.getxy()
    ui.draw_rect(box.x - camx, box.y - camy, box.x - camx + box.width, box.y - camy + box.height, false, 4)
end

function check_collision(a, b)
    return a.x < b.x + b.width and
        a.x + a.width > b.x and
        a.y < b.y + b.height and
        a.y + a.height > b.y
end

local function make_spike(data)
    local function box()
        return {
            x = (data.x - 1) * 16,
            y = (data.y - 1) * 16 + 6,
            width = 16,
            height = 10
        }
    end

    return {
        box = box,
        update_relative_position = function() return true end,
        before_frame = function(frame, player, map, camera)
            if player and check_collision(box(), player.box()) then
                camera.set_target { box = function() return box() end }
                player.mark_dead()
            end
        end,
        on_frame = function(frame, player, map, camera) end,
    }
end

local function make_spring(data)
    local spring = data
    local rx, ry
    local jump_frames = 0
    local tileset = SpriteSheets["poi.spring.1"]

    local function box()
        return {
            x = (spring.x - 1) * 16,
            y = (spring.y - 1) * 16,
            width = 16,
            height = 16
        }
    end

    local function update_relative_position(camx, camy)
        rx, ry = ((spring.x - 1) * 16) - camx, ((spring.y - 1) * 16) - camy
        return (rx < 480 and rx > -16) and (ry < 270 and ry > -16)
    end

    return {
        update_relative_position = update_relative_position,
        before_frame = function(frame, player, map, camera)
            jump_frames = math.max(jump_frames - 1, 0)

            if player and check_collision(box(), player.box()) then
                player.spring_jump()
                jump_frames = 12
            end
        end,
        on_frame = function(frame, player, map, camera)
            ui.tile(tileset, jump_frames == 0 and 0 or 1, rx, ry)
        end,
    }
end

local function make_beetle(beetle)
    local tx, ty = (beetle.x - 1) * 16, (beetle.y - 1) * 16
    local rx, ry
    local bx = 0
    local acell = 0.4
    local dead = 0
    local tileset = nil

    local smoke = (function()
        local out = {}
        for i = 1, 30, 1 do
            table.insert(out, { x = 0, y = 0, life = 0 })
        end
        return out
    end)()

    local draw_smoke = function(camx, camy, frame)
        for _, v in ipairs(smoke) do
            if v.life > 0 then
                ui.circfill((v.x - camx) // 1, (v.y - camy) // 1,
                    math.floor(v.life + 0.5), 23)
                v.life = v.life - 0.04
            end
        end
    end

    local make_smoke = function(frame)
        if frame % 10 ~= 0 then return end

        for _, v in ipairs(smoke) do
            if v.life <= 0 then
                v.life = 6
                v.x = tx + bx + (acell > 0 and 8 or 20)
                v.y = ty + 16 + math.random(6)
                return
            end
        end
    end

    local box = function()
        return {
            x = tx + bx // 1 + 6,
            y = ty + 14,
            width = 32 - 12,
            height = 32 - 18
        }
    end

    local function update_relative_position(camx, camy, map)
        if acell > 0 then
            local ix = (tx + bx // 1 + 32)
            local iy = (ty + 8)
            if map.colides(ix, iy, kColisionType.right) then acell = acell * -1 end
        else
            local ix = (tx + bx // 1)
            local iy = (ty + 8)
            if map.colides(ix, iy, kColisionType.left) then acell = acell * -1 end
        end

        rx, ry = tx - camx + bx // 1, ty - camy
        return (rx < 480 and rx > -32) and (ry < 270 and ry > -32)
    end

    return {
        update_relative_position = update_relative_position,
        faraway = function() dead = 0 end,
        before_frame = function(frame, player, map, camera)
            if player and dead == 0 then
                bx = bx + acell
                local pbox, tbox = player.box(), box()
                if check_collision(pbox, tbox) then
                    if pbox.y + pbox.height < tbox.y + tbox.height then
                        player.enemy_taken_jump()
                        dead = 1
                    else
                        player.mark_dead()
                        camera.set_target { box = function() return box() end }
                    end
                end

                make_smoke(frame)
            end

            if dead == 0 then
                tileset = SpriteSheets['poi.bettle.' .. (1 + ((frame // 3) % 6))]
            else
                dead = math.min(5, dead + 0.2)
                if dead < 5 then
                    tileset = SpriteSheets['poi.explode.' .. math.floor(dead // 1) % 5]
                end
            end
        end,
        on_frame = function(frame, player, map, camera)
            local camx, camy = camera.getxy()
            draw_smoke(camx, camy, frame)

            if tileset and dead < 5 then
                local flipped = (acell > 0) and true or false
                ui.spr(tileset, rx, ry, flipped)
            end
        end,
    }
end

local function make_bird(bird)
    print("Making bird at ", bird.x, bird.y)

    local tx, ty = (bird.x - 1) * 16, (bird.y - 3) * 16
    local rx, ry
    local by, oby = 0, 0
    local dead = 0
    local tileset = nil
    local box = function()
        return {
            x = tx + 6,
            y = ty + by // 1 + 14,
            width = 32 - 12,
            height = 32 - 18
        }
    end

    local function update_relative_position(camx, camy)
        rx, ry = tx - camx, ty - camy + by // 1
        return (rx < 480 and rx > -32) and (ry < 270 and ry > -32)
    end

    return {
        update_relative_position = update_relative_position,
        faraway = function() end,
        before_frame = function(frame, player, map, camera)
            if player and dead == 0 then
                by = (math.cos(ty + frame / 40) ^ 4) * 96
                local pbox, tbox = player.box(), box()
                if check_collision(pbox, tbox) then
                    if pbox.y + pbox.height < tbox.y + 8 then
                        player.enemy_taken_jump()
                        dead = 1
                    else
                        player.mark_dead()
                        camera.set_target { box = function() return box() end }
                    end
                end
            end

            if dead == 0 then
                if by < oby then
                    tileset = SpriteSheets['poi.bird.' .. (1 + ((frame // 3) % 4))]
                else
                    tileset = SpriteSheets['poi.bird.' .. (1 + ((frame // 9) % 2))]
                end
                oby = by
            else
                dead = math.min(5, dead + 0.2)
                if dead < 5 then
                    tileset = SpriteSheets['poi.explode.' .. math.floor(dead // 1) % 5]
                end
            end
        end,
        on_frame = function(frame, player, map, camera)
            if dead == 0 and player then
                local flipped = (tx < player.box().x) and true or false
                ui.spr(tileset, rx, ry, flipped)
            elseif dead < 5 then
                ui.spr(tileset, rx, ry)
            end
        end
    }
end

local function make_decal(decal, name, width_tiles, height_tiles)
    local rx, ry
    local sprite_name = SpriteSheets['poi.' .. name .. '.1']

    local function update_relative_position(camx, camy)
        rx, ry = ((decal.x - 1) * 16) - camx, ((decal.y - 8) * 16) - camy
        return (rx < 480 and rx > 32 * width_tiles * -1) and (ry < 270 and ry > 32 * height_tiles * -1)
    end

    return {
        update_relative_position = update_relative_position,
        before_frame = function(frame, player, map, camera)

        end,
        on_frame = function(frame, player, map, camera)
            ui.spr(sprite_name, rx, ry)
        end,
    }
end

local function make_cherry(cherry)
    local taken = 0
    local rx, ry
    local tileset = nil

    local box = function()
        return {
            x = (cherry.x - 1) * 16,
            y = (cherry.y - 1) * 16 + 2,
            width = 16,
            height = 16
        }
    end

    local function update_relative_position(camx, camy)
        rx, ry = (cherry.x * 16 - 22) - camx, (cherry.y * 16 - 22) - camy
        return (rx < 480 and rx > -32) and (ry < 270 and ry > -32) and taken < 7
    end

    local function check_pick(player)
        if player and check_collision(box(), player.box()) then
            player.mark_point(box().x, box().y)
            taken = 1
        end
    end

    return {
        update_relative_position = update_relative_position,
        before_frame = function(frame, player, map, camera)
            if taken == 0 then
                check_pick(player)
                tileset = SpriteSheets['poi.cherry.' .. 1 + (frame // 4 + cherry.x) % 7]
            else
                tileset = SpriteSheets['poi.cherry.' .. math.floor(8 + (taken // 1) % 7)]
                taken = math.min(taken + 0.2, 7)
            end
        end,
        on_frame = function(frame, player, map, camera)
            ui.spr(tileset, rx, ry, 0)
        end,
    }
end

local function make_poi_by_type(data, camera, player, map)
    if data.poi == kPoiType.spike then return make_spike(data) end
    if data.poi == kPoiType.spring then return make_spring(data) end
    if data.poi == kPoiType.beetle then return make_beetle(data) end
    if data.poi == kPoiType.palm then return make_decal(data, 'palm', 2, 4) end
    if data.poi == kPoiType.pine then return make_decal(data, 'pine', 2, 4) end
    if data.poi == kPoiType.house then return make_decal(data, 'house', 3, 3) end
    if data.poi == kPoiType.bird then return make_bird(data) end


    if data.poi == kPoiType.cherry then
        player.account_point()
        return make_cherry(data)
    end
end

function make_pois(camera, player, map)
    local all_pois = {}
    for _, poi in ipairs(map.get_pois()) do
        local created = make_poi_by_type(poi, camera, player, map)
        if created then table.insert(all_pois, created) end
    end

    return {
        before_frame = function(frame, camera, player, map)
            local player = player.is_dead() == false and player or nil
            local camx, camy = camera.getxy()

            for _, poi in ipairs(all_pois) do
                poi.will_draw = poi.update_relative_position(camx, camy, map)
                if poi.will_draw then
                    poi.before_frame(frame, player, map, camera)
                elseif poi.faraway then
                    poi.faraway()
                end
            end
        end,
        on_frame = function(frame, camera, player, map)
            local player = player.is_dead() == false and player or nil

            for _, poi in ipairs(all_pois) do
                if poi.will_draw then
                    poi.on_frame(frame, player, map, camera)
                end
            end

            --timer
            local timer = string.format("%02d:%02d", (frame // 60) // 60, (frame // 60) % 60)
            ui.print(timer, 1, 1, kColors.black)
            ui.print(timer, 0, 0, kColors.yellow)
        end
    }
end
