function make_player(map_ref)
    local position = {
        x = CurrentStage.player_start.x * 16,
        y = CurrentStage.player_start.y * 16
    }
    local last_check = { x = position.x, y = position.y }
    local velocity = { x = 0, y = 0 }
    local size = { w = 32, h = 32 }
    local looking_back = CurrentStage.looking_back
    local state = kPlayerStates.idle
    local points, total_points = 0, 0
    local win = 0
    local dead = false
    local reset_frames = 0
    local should_reset = false
    local on_ground = false
    local win_frame = 0
    local tileset = nil

    local function action_button_is(kind)
        if kind == kState.press then
            return ui.btn(BTN_Z, pad_1) ~= false
        elseif kind == kState.pressed then
            return ui.btnp(BTN_Z, pad_1) ~= false
        else
            return false
        end
    end

    local function check_v_colisions()
        if map_ref.colides(position.x + size.w / 2 - 4, position.y + size.h, kColisionType.bottom)
            or map_ref.colides(position.x + size.w / 2 + 4, position.y + size.h, kColisionType.bottom) then
            if velocity.y > 0 then
                position.y = (((position.y + size.h) // 16) * 16) - size.h - 0.001
                velocity.y = 0
                on_ground = true
            end
        elseif map_ref.colides(position.x + size.w / 2 - 4, position.y + 12, kColisionType.top)
            or map_ref.colides(position.x + size.w / 2 + 4, position.y + 12, kColisionType.top) then
            if velocity.y < 0 then
                position.y = (((position.y + size.h) // 16) * 16) - (size.h - 4)
                velocity.y = 0
            end
        end
    end

    local function check_h_colisions()
        -- two sensors, in front of me, to the right
        if map_ref.colides(position.x + size.w - 6, position.y + size.h - 18, kColisionType.right)
            or map_ref.colides(position.x + size.w - 6, position.y + size.h - 1, kColisionType.right) then
            if velocity.x > 0 then
                position.x = position.x // 1
                velocity.x = 0
                state = kPlayerStates.idle
            end
        elseif map_ref.colides(position.x + 6, position.y + size.h - 18, kColisionType.left)
            or map_ref.colides(position.x + 6, position.y + size.h - 1, kColisionType.left) then
            if velocity.x < 0 then
                position.x = position.x // 1
                velocity.x = 0
                state = kPlayerStates.idle
            end
        end
    end

    local function apply_gravity()
        position.y = dead and position.y + velocity.y * 0.6 or position.y + velocity.y
        velocity.y = velocity.y > kMaxGravity and kMaxGravity or velocity.y + kGravity
    end

    local function mark_dead()
        velocity.y = -(kJumpForce * 0.8)
        dead = true
    end

    local function spring_jump()
        velocity.y = -(kJumpForce * 1.5)
        on_ground = false
    end

    local function enemy_taken_jump()
        velocity.y = -(kJumpForce * 0.7)
        on_ground = false
    end

    local function update(frame)

        velocity.x = 0

        if dead then
            state = kPlayerStates.dead
        else
            if ui.btn(DOWN, pad_1) then
                velocity.x = 0
                state = kPlayerStates.crouch
            elseif on_ground and action_button_is(kState.press) then
                velocity.y = -kJumpForce
                on_ground = false
            elseif ui.btn(LEFT, pad_1) then
                velocity.x = -kPlayerSpeed
                looking_back = true
                state = kPlayerStates.run
            elseif ui.btn(RIGHT, pad_1) then
                velocity.x = kPlayerSpeed
                looking_back = false
                state = kPlayerStates.run
            else
                state = kPlayerStates.idle
            end

            if velocity.y > 0.6 then
                state = kPlayerStates.fall
            elseif velocity.y < -0.6 then
                state = kPlayerStates.jump
            end
        end

        if not dead then check_h_colisions() end
        apply_gravity()
        if not dead then
            position.x = position.x + velocity.x
            check_v_colisions()
        end
    end

    local function box()
        local cfactor = (state == kPlayerStates.crouch) and 14 or 6
        return {
            x = math.floor(position.x + 0.5) + 8,
            y = math.floor(position.y + 0.5) + cfactor,
            width = 32 - 16,
            height = 32 - cfactor
        }
    end

    local function draw(frame, camera, player)
        if should_reset then
            position.x = last_check.x
            position.y = last_check.y
            dead = false
            should_reset = false
            camera.set_target(player)
            state = kPlayerStates.idle
            reset_frames = 42
        end

        local camx, camy = camera.getxy()
        local x, y = math.floor(position.x + 0.5), math.floor(position.y + 0.5)
        local fx, fy = x - camx, math.min(270, y - camy)

        ui.tile(tileset, 0 + (looking_back and 1024 or 0), fx, fy)
        if fy == 270 then should_reset = true end

        -- juicer
        if total_points <= 0 then return end

        local top = 16 + 40 - (40 * points / total_points) // 1
        ui.rect(3, 15, 11, 59, kColors.black)
        ui.rect(2, 14, 10, 58, kColors.yellow)
        ui.rectfill(3, 15, 9, 57, kColors.purple_dark)

        if points > 0 and points ~= total_points then
            ui.circfill(6 + math.floor(math.cos(frame / 12) + 0.5), top, 2, kColors.purple_light)
            ui.circfill(6 + math.floor(math.sin(frame / 12) + 0.5), top, 2, kColors.red_light)
        end

        ui.rectfill(4, top // 1, 8, 16 + 40, kColors.red_light)
    end

    local function overlay_before(frame, camera)

    end

    local function overlay_frame(frame, camera)
        local delta = nil
        local camx, camy = camera.getxy()
        local y = math.floor(position.y + 0.5)
        local x = math.floor(position.x + 0.5) + 8
        local fx, fy = x - camx, math.min(270, y - camy)

        if points == total_points then
            if win_frame == 0 then win_frame = frame end

            win = math.max(20, math.min(480, win + (480 - win) / 40))
            ui.fillp(
                170,
                85,
                170,
                85,
                170,
                85,
                170,
                85
            )

            ui.circfill(fx + 10, fy + 10, math.min(80, win // 3), kColors.purple_dark)
            ui.fillp()
            ui.circfill(fx + 10, fy + 10, math.min(60, win // 6), kColors.purple_dark)

            ui.tile(SpriteSheets['player.win.1'], frame % 60 < 30 and 1024 or 0, fx - 8, fy)

            local ff = math.floor(math.max(1, math.min(10, (win - 120) / 8)))
            ui.spr(SpriteSheets['cwin.' .. ff], math.max(0, fx - 38), math.max(-2, fy - 42))



        else
            if reset_frames > 0 then
                delta = math.min(reset_frames / 2, 8) // 1
                reset_frames = reset_frames - 1
            elseif fy > 270 - 32 then
                delta = (8 + math.min(8, 8 * ((fy - 240) / 30))) // 2
            elseif frame < 32 then
                delta = math.min((32 - frame) / 2, 8) // 1
            end

            if delta ~= nil then
                local dminus = 8 - delta
                local dplus  = 8 + delta
                for x = 0, 480, 16 do
                    for y = 0, 270, 16 do
                        ui.rectfill(
                            x + dminus, y + dminus,
                            x + dplus, y + dplus,
                            kColors.purple_dark)
                    end
                end
            end
        end
    end

    return {
        box = box,
        get_points = function() return points end,
        get_total_points = function() return total_points end,
        should_reset = function()
            return (win > 200 and action_button_is(kState.press))
        end,
        account_point = function() total_points = total_points + 1 end,
        mark_point = function(point_x, point_y)
            points = points + 1
            last_check.x = point_x
            last_check.y = point_y
            if points == total_points then
                MapStages.mark_current_as_done()
            end
        end,
        is_dead = function() return dead end,
        spring_jump = spring_jump,
        enemy_taken_jump = enemy_taken_jump,
        mark_dead = mark_dead,
        overlay = function()
            return {
                before_frame = function(frame, camera, player, map)
                    overlay_before(frame, camera)
                end,
                on_frame = function(frame, camera, player, map)
                    overlay_frame(frame, camera)
                end
            }
        end,
        before_frame = function(frame, camera, player, map)
            if win == 0 and reset_frames < 16 then update(frame) end
            local char_frame = 1 + ((frame // state.cadency) % state.frames)
            tileset = SpriteSheets['player.' .. state.asset .. '.' .. char_frame]
        end,
        on_frame = function(frame, camera, player, map)
            draw(frame, camera, player)
        end
    }
end
